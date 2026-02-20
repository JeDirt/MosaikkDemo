// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"

class AMosaikkProxyActor;
class ISequencer;
class SConstraintCanvas;
class SLevelViewport;

class FMosaikkModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	static FMosaikkModule& Get();

public:
	TSharedPtr<SConstraintCanvas> GetHostCanvas() const;
	TSharedPtr<SLevelViewport> GetLevelEditorViewport();

protected:
	void OnSequencerCreated(TSharedRef<ISequencer> CreatedSequencer);
	void OnSequencerClosed(TSharedRef<ISequencer> ClosedSequencer);

	void OnPostPIEStarted(bool bSimulating);

	void AddMosaikkButton_RESERVED(FToolBarBuilder& ToolbarBuilder);
	void OnMosaikkButtonClicked_RESERVED();

	TSharedRef<SDockTab> OnSpawnPluginTab(const FSpawnTabArgs& SpawnTabArgs);
	void OnTabForegrounded(TSharedPtr<SDockTab> PreviouslyActive, TSharedPtr<SDockTab> NewlyActivated);

private:
	/**
	 * Canvas that is used for displaying UMG widget in sequencer.
	 * Added to Editor Viewport when Sequencer is opened.
	 * Removed from Editor Viewport when Sequencer is closed.
	 *
	 * Using this canvas we can show widgets in Editor Viewport any time we want, not only PIE.
	 */
	TSharedPtr<SConstraintCanvas> HostCanvas;

	TSharedPtr<FExtender> SequencerToolbarExtender;

	FDelegateHandle TrackEditorRegisteredHandle;
	FDelegateHandle SequencerCreatedHandle;
	FDelegateHandle SequencerClosedHandle;
	FDelegateHandle PostPIEStartedHandle;
	FDelegateHandle OnTabForegroundedDelegateHandle;

	TSharedPtr<FUICommandList> PluginCommands;

	TWeakPtr<ISequencer> CachedSequencer;
};
