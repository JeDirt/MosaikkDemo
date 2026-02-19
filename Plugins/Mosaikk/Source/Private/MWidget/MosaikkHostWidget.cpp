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

	const TSharedPtr<SConstraintCanvas> MosaikkViewportCanvas = FMosaikkModule::Get().GetMosaikkViewportCanvas();
	SlateRootCanvas->AddSlot()[MosaikkViewportCanvas.ToSharedRef()];
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

