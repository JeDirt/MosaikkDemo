// Fill out your copyright notice in the Description page of Project Settings.

#include "MRendering/MosaikkMoviePipelineSetting.h"

#include "Engine/TextureRenderTarget2D.h"
#include "MoviePipelineCameraSetting.h"
#include "MoviePipelineOutputBuilder.h"
#include "MoviePipelineQueue.h"
#include "Slate/WidgetRenderer.h"
#include "Widgets/Layout/SConstraintCanvas.h"

#include "MRendering/MosaikkHostCanvasManager.h"

void UMosaikkMoviePipelineSetting::SetupImpl(const MoviePipeline::FMoviePipelineRenderPassInitSettings& InPassInitSettings)
{
	const float CameraOverscan = GetPipeline()->GetCachedCameraOverscan(INDEX_NONE);
	const FIntRect CropRect = UMoviePipelineBlueprintLibrary::GetOverscanCropRectangle(GetPipeline()->GetPipelinePrimaryConfig(), GetPipeline()->GetActiveShotList()[GetPipeline()->GetCurrentShotIndex()], CameraOverscan);

	// Composited elements should be sized to the original frustum size, as the final image is either cropped to that size, or the composite will be offset
	// to match the original frustum
	OutputResolution = CropRect.Size();
	const int32 MaxResolution = GetMax2DTextureDimension();
	if (OutputResolution.X > MaxResolution || OutputResolution.Y > MaxResolution)
	{
		GetPipeline()->Shutdown(true);
		return;
	}

	VirtualWindow = SNew(SVirtualWindow).Size(FVector2D(OutputResolution.X, OutputResolution.Y));
	if (FSlateApplication::IsInitialized())
	{
		FSlateApplication::Get().RegisterVirtualWindow(VirtualWindow.ToSharedRef());
	}

	RenderTarget = NewObject<UTextureRenderTarget2D>();
	RenderTarget->ClearColor = FLinearColor::Transparent;

	constexpr bool bInForceLinearGamma = false;
	RenderTarget->InitCustomFormat(OutputResolution.X, OutputResolution.Y, EPixelFormat::PF_B8G8R8A8, bInForceLinearGamma);

	constexpr bool bApplyGammaCorrection = false;
	WidgetRenderer = MakeShared<FWidgetRenderer>(bApplyGammaCorrection);
}

void UMosaikkMoviePipelineSetting::TeardownImpl()
{
	FlushRenderingCommands();

	if (FSlateApplication::IsInitialized() && VirtualWindow.IsValid())
	{
		FSlateApplication::Get().UnregisterVirtualWindow(VirtualWindow.ToSharedRef());
	}

	VirtualWindow = nullptr;

	WidgetRenderer = nullptr;
	RenderTarget = nullptr;
}

void UMosaikkMoviePipelineSetting::GatherOutputPassesImpl(TArray<FMoviePipelinePassIdentifier>& ExpectedRenderPasses)
{
	if (WidgetRenderer == nullptr)
	{
		return;
	}

	const UMoviePipelineExecutorShot* CurrentShot = GetPipeline()->GetActiveShotList()[GetPipeline()->GetCurrentShotIndex()];
	const UMoviePipelineCameraSetting* CameraSettings = GetPipeline()->FindOrAddSettingForShot<UMoviePipelineCameraSetting>(CurrentShot);
	const int32 NumCameras = CameraSettings->bRenderAllCameras ? CurrentShot->SidecarCameras.Num() : 1;
	for (int32 CameraIndex = 0; CameraIndex < NumCameras; CameraIndex++)
	{
		FMoviePipelinePassIdentifier PassIdentifierForCurrentCamera;
		PassIdentifierForCurrentCamera.Name = TEXT("MosaikkPass");

		// If we're not rendering all cameras, we need to pass -1 so we pick up the real camera name.
		const int32 LocalCameraIndex = CameraSettings->bRenderAllCameras ? CameraIndex : -1;
		PassIdentifierForCurrentCamera.CameraName = CurrentShot->GetCameraName(LocalCameraIndex);

		ExpectedRenderPasses.Add(PassIdentifierForCurrentCamera);
	}
}

void UMosaikkMoviePipelineSetting::RenderSample_GameThreadImpl(const FMoviePipelineRenderPassMetrics& InSampleState)
{
	if (WidgetRenderer == nullptr || InSampleState.bDiscardResult)
	{
		return;
	}
	
	const bool bFirstTile = InSampleState.GetTileIndex() == 0;
	const bool bFirstSpatial = InSampleState.SpatialSampleIndex == (InSampleState.SpatialSampleCount - 1);
	const bool bFirstTemporal = InSampleState.TemporalSampleIndex == (InSampleState.TemporalSampleCount - 1);

	if (bFirstTile && bFirstSpatial && bFirstTemporal)
	{
		const UMoviePipelineExecutorShot* CurrentShot = GetPipeline()->GetActiveShotList()[GetPipeline()->GetCurrentShotIndex()];
		const UMoviePipelineCameraSetting* CameraSettings = GetPipeline()->FindOrAddSettingForShot<UMoviePipelineCameraSetting>(CurrentShot);
		const int32 NumCameras = CameraSettings->bRenderAllCameras ? CurrentShot->SidecarCameras.Num() : 1;

		const TSharedPtr<SConstraintCanvas> HostCanvasPtr = UMosaikkHostCanvasManager::Get().GetHostCanvas();
		if (!HostCanvasPtr.IsValid())
		{
			return;
		}

		for (int32 CameraIndex = 0; CameraIndex < NumCameras; CameraIndex++)
		{
			// If we're not rendering all cameras, we need to pass -1 so we pick up the real camera name.
			const int32 LocalCameraIndex = CameraSettings->bRenderAllCameras ? CameraIndex : -1;

			FMoviePipelinePassIdentifier PassIdentifierForCurrentCamera;
			PassIdentifierForCurrentCamera.Name = TEXT("MosaikkPass");
			PassIdentifierForCurrentCamera.CameraName = CurrentShot->GetCameraName(LocalCameraIndex);

			// Put the widget in our window
			VirtualWindow->SetContent(HostCanvasPtr.ToSharedRef());

			// Update the Widget with the latest frame information

			// Draw the widget to the render target. This leaves the texture in SRV state so no transition is needed.
			// Note: If this draw call is updated, be sure to also update the one below.
			WidgetRenderer->DrawWindow(RenderTarget, VirtualWindow->GetHittestGrid(), VirtualWindow.ToSharedRef(),
				1.f, OutputResolution, InSampleState.OutputState.TimeData.FrameDeltaTime);

			// If this is the first frame being rendered, then re-render. This will ensure that all layout computations are correct (for example, large
			// text blocks may not have their layouts computed by the time the first render above occurs, especially if the text is driven by the
			// UMG blueprint).
			if (InSampleState.OutputState.ShotOutputFrameNumber == 0)
			{
				WidgetRenderer->DrawWindow(RenderTarget, VirtualWindow->GetHittestGrid(), VirtualWindow.ToSharedRef(),
					1.f, OutputResolution, InSampleState.OutputState.TimeData.FrameDeltaTime);
			}

			FRenderTarget* BackbufferRenderTarget = RenderTarget->GameThread_GetRenderTargetResource();
			TSharedPtr<FMoviePipelineOutputMerger, ESPMode::ThreadSafe> OutputBuilder = GetPipeline()->OutputBuilder;

			ENQUEUE_RENDER_COMMAND(BurnInRenderTargetResolveCommand)(
				[InSampleState, PassIdentifierForCurrentCamera, bComposite = bCompositeOntoFinalImage, BackbufferRenderTarget, OutputBuilder](FRHICommandListImmediate& RHICmdList)
				{
					const FIntRect SourceRect = FIntRect(0, 0, BackbufferRenderTarget->GetSizeXY().X, BackbufferRenderTarget->GetSizeXY().Y);

					// Read the data back to the CPU
					TArray<FColor> RawPixels;
					RawPixels.SetNum(SourceRect.Width() * SourceRect.Height());

					FReadSurfaceDataFlags ReadDataFlags(ERangeCompressionMode::RCM_MinMax);
					ReadDataFlags.SetLinearToGamma(false);

					RHICmdList.ReadSurfaceData(BackbufferRenderTarget->GetRenderTargetTexture(), SourceRect, RawPixels, ReadDataFlags);

					TSharedRef<FImagePixelDataPayload, ESPMode::ThreadSafe> FrameData = MakeShared<FImagePixelDataPayload, ESPMode::ThreadSafe>();
					FrameData->PassIdentifier = PassIdentifierForCurrentCamera;
					FrameData->SampleState = InSampleState;
					FrameData->bRequireTransparentOutput = true;
					FrameData->SortingOrder = 4;
					FrameData->bCompositeToFinalImage = bComposite;

					TUniquePtr<FImagePixelData> PixelData = MakeUnique<TImagePixelData<FColor>>(SourceRect.Size(), TArray64<FColor>(MoveTemp(RawPixels)), FrameData);

					OutputBuilder->OnCompleteRenderPassDataAvailable_AnyThread(MoveTemp(PixelData));
				});
		}
	}
}

bool UMosaikkMoviePipelineSetting::IsValidOnShots() const
{
	return true;
}

bool UMosaikkMoviePipelineSetting::IsValidOnPrimary() const
{
	return true;
}
