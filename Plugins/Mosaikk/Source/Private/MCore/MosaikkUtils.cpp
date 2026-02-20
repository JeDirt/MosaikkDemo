// Fill out your copyright notice in the Description page of Project Settings.

#include "MCore/MosaikkUtils.h"

#include "Algo/AnyOf.h"
#include "Blueprint/UserWidget.h"
#include "ISequencer.h"
#include "MovieScene.h"
#include "Widgets/Layout/SConstraintCanvas.h"

#include "MSequencer/MosaikkTrack.h"
#include "Mosaikk.h"

void FMosaikkWidgetUtils::PushWidgetToHostCanvas(UUserWidget* Widget)
{
	const TSharedPtr<SConstraintCanvas> HostCanvas = FMosaikkModule::Get().GetHostCanvas();
	if (!IsValid(Widget) || !HostCanvas.IsValid())
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

void FMosaikkWidgetUtils::RemoveWidgetFromHostCanvas(UUserWidget* Widget)
{
	const TSharedPtr<SConstraintCanvas> HostCanvas = FMosaikkModule::Get().GetHostCanvas();
	if (!IsValid(Widget) || !HostCanvas.IsValid())
	{
		return;
	}

	HostCanvas->RemoveSlot(Widget->TakeWidget());
}

void FMosaikkWidgetUtils::ClearHostCanvas()
{
	const TSharedPtr<SConstraintCanvas> HostCanvas = FMosaikkModule::Get().GetHostCanvas();
	if (!HostCanvas.IsValid())
	{
		return;
	}

	HostCanvas->ClearChildren();
}

bool FMosaikkSequencerUtils::HasMosaikkTracks(const TSharedPtr<ISequencer>& Sequencer)
{
	if (!Sequencer.IsValid())
	{
		return false;
	}

	const UMovieSceneSequence* RootSequence = Sequencer->GetRootMovieSceneSequence();
	if (!IsValid(RootSequence))
	{
		return false;
	}

	const UMovieScene* MovieScene = RootSequence->GetMovieScene();
	if (!IsValid(MovieScene))
	{
		return false;
	}

	return Algo::AnyOf(MovieScene->GetTracks(), [](const UMovieSceneTrack* Track)
	{
		return IsValid(Track) && Track->IsA<UMosaikkTrack>();
	});
}
