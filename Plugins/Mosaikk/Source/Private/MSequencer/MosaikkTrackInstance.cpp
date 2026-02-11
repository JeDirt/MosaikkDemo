// Fill out your copyright notice in the Description page of Project Settings.

#include "MSequencer/MosaikkTrackInstance.h"

#include "Blueprint/UserWidget.h"

#include "Mosaikk.h"
#include "MSequencer/MosaikkSection.h"

void UMosaikkTrackInstance::OnInitialize()
{
	
}

void UMosaikkTrackInstance::OnAnimate()
{
	
}

void UMosaikkTrackInstance::OnBeginUpdateInputs()
{
	
}

void UMosaikkTrackInstance::OnInputAdded(const FMovieSceneTrackInstanceInput& InInput)
{
	UMosaikkSection* Section = Cast<UMosaikkSection>(InInput.Section);
	if (!IsValid(Section) || CachedWidget.IsValid())
	{
		return;
	}

	CachedWidget = Section->AssociatedWidgetInstance;
	if (!CachedWidget.IsValid())
	{
		return;
	}
	
	ShowWidget(CachedWidget.Get());
}

void UMosaikkTrackInstance::OnInputRemoved(const FMovieSceneTrackInstanceInput& InInput)
{
	
}

void UMosaikkTrackInstance::OnEndUpdateInputs()
{
	
}

void UMosaikkTrackInstance::OnDestroyed()
{
	HideAllWidgets();
}

void UMosaikkTrackInstance::ShowWidget(UUserWidget* Widget)
{
	const TSharedPtr<SOverlay> MosaikkViewportOverlay = FMosaikkModule::Get().GetMosaikkViewportOverlay();
	if (!IsValid(Widget) || !MosaikkViewportOverlay.IsValid())
	{
		return;
	}

	if (MosaikkViewportOverlay->GetChildren()->NumSlot() != 0)
	{
		return;
	}

	const TSharedPtr<SWidget> SlateWidget = Widget->TakeWidget();
	MosaikkViewportOverlay->AddSlot()[SlateWidget.ToSharedRef()];
}

void UMosaikkTrackInstance::HideAllWidgets()
{
	const TSharedPtr<SOverlay> MosaikkViewportOverlay = FMosaikkModule::Get().GetMosaikkViewportOverlay();
	if (!MosaikkViewportOverlay.IsValid())
	{
		return;
	}

	MosaikkViewportOverlay->ClearChildren();
}
