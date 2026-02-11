// Fill out your copyright notice in the Description page of Project Settings.

#include "MWidget/MosaikkHostWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Overlay.h"

#include "Mosaikk.h"

void UMosaikkHostWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	RootOverlay = WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass(), TEXT("RootOverlay"));
	if (!IsValid(RootOverlay))
	{
		return;
	}

	if (!IsValid(WidgetTree->RootWidget))
	{
		WidgetTree->RootWidget = RootOverlay;
	}

	const TSharedRef<SOverlay> SlateRootOverlay = StaticCastSharedRef<SOverlay>(RootOverlay->TakeWidget());
	if (!SlateRootOverlay.ToWeakPtr().IsValid())
	{
		return;
	}

	const TSharedPtr<SOverlay> MosaikkViewportOverlay = FMosaikkModule::Get().GetMosaikkViewportOverlay();
	SlateRootOverlay->AddSlot()[MosaikkViewportOverlay.ToSharedRef()];
}

void UMosaikkHostWidget::NativeDestruct()
{
	Super::NativeDestruct();

	if (!IsValid(RootOverlay))
	{
		return;
	}

	WidgetTree->RemoveWidget(RootOverlay);
	WidgetTree->RootWidget = nullptr;
}

