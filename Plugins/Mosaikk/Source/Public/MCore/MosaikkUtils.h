// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

class FMosaikkWidgetUtils
{
public:
	/**
	 * Pushes passed widget to the **FMosaikkModule::HostCanvas** as new slot.
	 * 
	 * @param Widget				Widget to push
	 */
	static void PushWidgetToHostCanvas(UUserWidget* Widget);

	/**
	 * Removes passed widget from **FMosaikkModule::HostCanvas** as slot.
	 * 
	 * @param Widget				Widget to remove 
	 */
	static void RemoveWidgetFromHostCanvas(UUserWidget* Widget);

	/**
	 * Removes all slots from **FMosaikkModule::HostCanvas**.
	 */
	static void ClearHostCanvas();
};

class FMosaikkSequencerUtils
{
public:
	/**
	 * Returns true if this Sequencer's **RootMovieSceneSequence's** has at least one MosaikkTrack.
	 */
	static bool HasMosaikkTracks(const TSharedPtr<class ISequencer>& Sequencer);
};
