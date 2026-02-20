// Fill out your copyright notice in the Description page of Project Settings.

#include "MWidget/MosaikkHostWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/CanvasPanel.h"

#include "Mosaikk.h"

void UMosaikkHostWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	RootCanvas = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("RootCanvas"));
	if (!IsValid(RootCanvas))
	{
		return;
	}

	if (!IsValid(WidgetTree->RootWidget))
	{
		WidgetTree->RootWidget = RootCanvas;
	}

	const TSharedRef<SConstraintCanvas> SlateRootCanvas = StaticCastSharedRef<SConstraintCanvas>(RootCanvas->TakeWidget());
	if (!SlateRootCanvas.ToWeakPtr().IsValid())
	{
		return;
	}

	const TSharedPtr<SConstraintCanvas> HostCanvas = FMosaikkModule::Get().GetHostCanvas();
	SlateRootCanvas->AddSlot()
		.Anchors(FAnchors(0.0f, 0.0f, 1.0f, 1.0f))
		.Offset(FMargin(0.0f))
		.Alignment(FVector2D(0.0f, 0.0f))
	[
		HostCanvas.ToSharedRef()
	];
}

void UMosaikkHostWidget::NativeDestruct()
{
	Super::NativeDestruct();

	if (!IsValid(RootCanvas))
	{
		return;
	}

	WidgetTree->RemoveWidget(RootCanvas);
	WidgetTree->RootWidget = nullptr;
}

