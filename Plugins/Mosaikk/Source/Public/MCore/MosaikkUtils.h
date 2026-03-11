// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

class FMosaikkSequencerUtils
{
public:
	/**
	 * Returns true if this Sequencer's **RootMovieSceneSequence's** has at least one MosaikkTrack.
	 */
	static bool HasMosaikkTracks(const TSharedPtr<class ISequencer>& Sequencer);
};
