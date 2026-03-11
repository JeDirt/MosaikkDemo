// Fill out your copyright notice in the Description page of Project Settings.

#include "MCore/MosaikkUtils.h"

#include "Algo/AnyOf.h"
#include "ISequencer.h"
#include "MovieScene.h"

#include "MSequencer/MosaikkTrack.h"

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
