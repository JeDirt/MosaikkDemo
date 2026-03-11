// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"

class ISequencer;
class SLevelViewport;

class FMosaikkModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	static FMosaikkModule& Get();

public:
	static TSharedPtr<SLevelViewport> GetLevelEditorViewport();

protected:
	void OnSequencerCreated(TSharedRef<ISequencer> CreatedSequencer);
	void OnSequencerClosed(TSharedRef<ISequencer> ClosedSequencer);

	void OnTabForegrounded(TSharedPtr<SDockTab> PreviouslyActive, TSharedPtr<SDockTab> NewlyActivated);

	void AddMosaikkHUBButton(FToolBarBuilder& ToolbarBuilder);
	void OnMosaikkHUBButtonClicked();

	TSharedRef<SDockTab> OnSpawnMosaikkHUBTab(const FSpawnTabArgs& SpawnTabArgs);

private:
	TSharedPtr<FExtender> SequencerToolbarExtender;

	FDelegateHandle TrackEditorRegisteredHandle;
	FDelegateHandle SequencerCreatedHandle;
	FDelegateHandle SequencerClosedHandle;
	FDelegateHandle OnTabForegroundedDelegateHandle;

	TSharedPtr<FUICommandList> PluginCommands;

	TWeakPtr<ISequencer> CachedSequencer;
	
	TSharedPtr<SLevelViewport> MosaikkViewport;
};
