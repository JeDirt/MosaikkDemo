// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "MosaikkHostCanvasManager.generated.h"

class SConstraintCanvas;

UCLASS()
class UMosaikkHostCanvasManager : public UEngineSubsystem
{
	GENERATED_BODY()

public:
	// Begin USubsystem interface
	void Initialize(FSubsystemCollectionBase& Collection) override;
	void Deinitialize() override;
	// ~End USubsystem interface

public:
	/** Convenience method to retrieve the MosaikkWidgetSinkManager Editor subsystem. */
	static UMosaikkHostCanvasManager& Get();

	/** Returns managed Host SConstraintCanvas. */
	TSharedPtr<SConstraintCanvas> GetHostCanvas();

	/**
	 * Pushes passed widget to the Host SConstraintCanvas.
	 * 
	 * @param Widget				Widget to push
	 */
	void PushWidgetToHostCanvas(UUserWidget* Widget) const;

	/**
	 * Removes passed widget from Host SConstraintCanvas.
	 * 
	 * @param Widget				Widget to remove 
	 */
	void RemoveWidgetFromHostCanvas(UUserWidget* Widget) const;

	/**
	 * Removes all slots from **HostCanvas**.
	 */
	void ClearHostCanvas();

private:
	/**
	 * Canvas that is used for displaying UMG widgets in Sequencer and captured movie.
	 * Added to Editor Viewport when Sequencer is opened.
	 * Removed from Editor Viewport when Sequencer is closed.
	 */
	TSharedPtr<SConstraintCanvas> HostCanvas;
};


