// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"

class AMosaikkProxyActor;
class ISequencer;
class SLevelViewport;

class FMosaikkModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	static FMosaikkModule& Get();

public:
	TSharedPtr<SOverlay> GetMosaikkViewportOverlay() const;
	TSharedPtr<SLevelViewport> GetLevelEditorViewport();

protected:
	void OnSequencerCreated(TSharedRef<ISequencer> CreatedSequencer);
	void OnSequencerClosed(TSharedRef<ISequencer> ClosedSequencer);

	void OnPostPIEStarted(bool bSimulating);

	void AddMosaikkButton_RESERVED(FToolBarBuilder& ToolbarBuilder);
	void OnMosaikkButtonClicked_RESERVED();

	TSharedRef<class SDockTab> OnSpawnPluginTab(const class FSpawnTabArgs& SpawnTabArgs);
	

private:
	/**
	 * Overlay that is used for displaying UMG widget in sequencer.
	 * Added to Editor Viewport when Sequencer is opened.
	 * Removed from Editor Viewport when Sequencer is closed.
	 *
	 * Using this overlay we can show widgets in Editor Viewport any time we want, not only PIE.
	 */
	TSharedPtr<SOverlay> MosaikkViewportOverlay;

	TSharedPtr<FExtender> SequencerToolbarExtender;

	FDelegateHandle TrackEditorRegisteredHandle;
	FDelegateHandle SequencerCreatedHandle;
	FDelegateHandle SequencerClosedHandle;
	FDelegateHandle PostPIEStartedHandle;

	TSharedPtr<class FUICommandList> PluginCommands;
};
