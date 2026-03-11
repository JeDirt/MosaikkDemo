// Fill out your copyright notice in the Description page of Project Settings.

#include "MRendering/MosaikkHostCanvasManager.h"

#include "Blueprint/UserWidget.h"
#include "Styling/CoreStyle.h"
#include "Widgets/Layout/SConstraintCanvas.h"

void UMosaikkHostCanvasManager::Initialize(FSubsystemCollectionBase& Collection)
{
	UEngineSubsystem::Initialize(Collection);

	HostCanvas = SNew(SConstraintCanvas);
	HostCanvas->AddSlot()
		.Anchors(FAnchors(0.0f, 0.0f, 1.0f, 1.0f))
		.Offset(FMargin(0.0f))
		.Alignment(FVector2D(0.0f, 0.0f));
}

void UMosaikkHostCanvasManager::Deinitialize()
{
	HostCanvas.Reset();
	UEngineSubsystem::Deinitialize();
}

UMosaikkHostCanvasManager& UMosaikkHostCanvasManager::Get()
{
	return *GEngine->GetEngineSubsystem<UMosaikkHostCanvasManager>();
}

TSharedPtr<SConstraintCanvas> UMosaikkHostCanvasManager::GetHostCanvas()
{
	return HostCanvas;
}

void UMosaikkHostCanvasManager::PushWidgetToHostCanvas(UUserWidget* Widget) const
{
	if (!IsValid(Widget))
	{
		return;
	}

	const TSharedRef<SWidget> SlateWidget = Widget->TakeWidget();
	HostCanvas->AddSlot()
		.Anchors(FAnchors(0.0f, 0.0f, 1.0f, 1.0f))
		.Offset(FMargin(0.0f))
		.Alignment(FVector2D(0.0f, 0.0f))
	[
		SlateWidget
	];
}

void UMosaikkHostCanvasManager::RemoveWidgetFromHostCanvas(UUserWidget* Widget) const
{
	if (!IsValid(Widget))
	{
		return;
	}

	HostCanvas->RemoveSlot(Widget->TakeWidget());
}

void UMosaikkHostCanvasManager::ClearHostCanvas()
{
	HostCanvas->ClearChildren();
}

