// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "MoviePipelineRenderPass.h"

#include "MosaikkMoviePipelineSetting.generated.h"

class FWidgetRenderer;
class SVirtualWindow;
class UTextureRenderTarget2D;

UCLASS()
class MOSAIKK_API UMosaikkMoviePipelineSetting : public UMoviePipelineRenderPass
{
	GENERATED_BODY()

protected:
	// UMoviePipelineRenderPass Interface
	virtual void SetupImpl(const MoviePipeline::FMoviePipelineRenderPassInitSettings& InPassInitSettings) override;
	virtual void TeardownImpl() override;
	virtual void GatherOutputPassesImpl(TArray<FMoviePipelinePassIdentifier>& ExpectedRenderPasses) override;
	virtual void RenderSample_GameThreadImpl(const FMoviePipelineRenderPassMetrics& InSampleState) override;
	// ~UMoviePipelineRenderPass Interface

public:
	virtual bool IsValidOnShots() const override;
	virtual bool IsValidOnPrimary() const override;

protected:
	/** If true, the Widgets image will be composited into the Final Image pass. Doesn't apply to multi-layer EXR files. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mosaikk Settings")
	bool bCompositeOntoFinalImage = true;

protected:
	TSharedPtr<FWidgetRenderer> WidgetRenderer;
	TSharedPtr<SVirtualWindow> VirtualWindow;

	FIntPoint OutputResolution;

	UPROPERTY(Transient)
	TObjectPtr<UTextureRenderTarget2D> RenderTarget;
};
