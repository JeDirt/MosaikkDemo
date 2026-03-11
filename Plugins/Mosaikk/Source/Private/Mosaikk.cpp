// Copyright Epic Games, Inc. All Rights Reserved.

#include "Mosaikk.h"

#include "Editor/LevelEditor/Private/SLevelEditor.h"
#include "ISequencerModule.h"
#include "LevelEditor.h"
#include "LevelSequence.h"
#include "Widgets/Layout/SConstraintCanvas.h"

#include "MCore/MosaikkCommands.h"
#include "MCore/MosaikkTrackEditor.h"
#include "MCore/MosaikkUtils.h"
#include "MCore/MovieSceneMosaikkEntitySystem.h"
#include "MRendering/MosaikkHostCanvasManager.h"
#include "Styling/SlateIconFinder.h"

#define LOCTEXT_NAMESPACE "FMosaikkModule"

DEFINE_LOG_CATEGORY_STATIC(LogMosaikk, Log, All);

static const FName MosaikkHUBTabName("Mosaikk HUB");

// TODO: first idea was to use separate custom viewport to project widgets,
// but the more I was implementing current logic, the more sceptical I was becoming about this approach.
// Seems not too convenient to use separate window just to preview how animated widgets gonna look like in movie.
// 
// Ideally everything should be spawned in editor viewport, so Artist can see result here and now.
// But in current implementation widgets that are pushed to the viewport seem not to correspond in scale properly.
// 
// Still in progress...
class SMosaikkStandaloneLevelViewport final : public SLevelViewport
{
public:
	virtual bool IsVisible() const override
	{
		// SLevelViewport expects a parent layout/tab setup that we don't have in this custom tab.
		return SEditorViewport::IsVisible();
	}
};

void FMosaikkModule::StartupModule()
{
	FMosaikkStyle::Initialize();
	FMosaikkStyle::ReloadTextures();

	FMosaikkCommands::Register();

	PluginCommands = MakeShareable(new FUICommandList);

	PluginCommands->MapAction(
		FMosaikkCommands::Get().OpenMosaikkHUBWindow,
		FExecuteAction::CreateRaw(this, &FMosaikkModule::OnMosaikkHUBButtonClicked),
		FCanExecuteAction());

	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(MosaikkHUBTabName, FOnSpawnTab::CreateRaw(this, &FMosaikkModule::OnSpawnMosaikkHUBTab))
		.SetDisplayName(LOCTEXT("FMosaikkHUBTabTitle", "Mosaikk HUB"))
		.SetMenuType(ETabSpawnerMenuType::Hidden);

	ISequencerModule& SequencerModule = FModuleManager::LoadModuleChecked<ISequencerModule>("Sequencer");

	FOnSequencerCreated::FDelegate OnSequencerCreated;
	OnSequencerCreated.BindRaw(this, &FMosaikkModule::OnSequencerCreated);
	SequencerCreatedHandle = SequencerModule.RegisterOnSequencerCreated(OnSequencerCreated);

	OnTabForegroundedDelegateHandle = FGlobalTabmanager::Get()->OnTabForegrounded_Subscribe(
		FOnActiveTabChanged::FDelegate::CreateRaw(
			this, &FMosaikkModule::OnTabForegrounded)
	);

	TrackEditorRegisteredHandle = SequencerModule.RegisterTrackEditor(FOnCreateTrackEditor::CreateStatic(&FMosaikkTrackEditor::CreateTrackEditor));

	/**
	 * Create and add extender for Sequencer.
	 * Thus we tell that Sequencer is gonna be modified - our custom MosaikkHUB button will be placed after NavigationTool section.
	 */
	SequencerToolbarExtender = MakeShared<FExtender>();
	SequencerToolbarExtender->AddToolBarExtension(
		TEXT("NavigationTool"),
		EExtensionHook::After,
		PluginCommands,
		FToolBarExtensionDelegate::CreateRaw(this, &FMosaikkModule::AddMosaikkHUBButton)
	);

	SequencerModule.GetToolBarExtensibilityManager()->AddExtender(SequencerToolbarExtender);
}

void FMosaikkModule::ShutdownModule()
{
	FMosaikkStyle::Shutdown();

	FMosaikkCommands::Unregister();

	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(MosaikkHUBTabName);
	FGlobalTabmanager::Get()->OnTabForegrounded_Unsubscribe(OnTabForegroundedDelegateHandle);

	/**
	 * This function may be called during shutdown to clean up your module.
	 * For modules that support dynamic reloading, we call this function before unloading the module.
	 */
	if (FModuleManager::Get().IsModuleLoaded("Sequencer"))
	{
		ISequencerModule& SequencerModule = FModuleManager::GetModuleChecked<ISequencerModule>("Sequencer");
		SequencerModule.UnregisterOnSequencerCreated(SequencerCreatedHandle);
		SequencerModule.UnRegisterTrackEditor(TrackEditorRegisteredHandle);

		UE_LOGFMT(LogMosaikk, Display, "{0}: Cleanup SequencerCreated callbacks.", __FUNCTION__);
	}
}

FMosaikkModule& FMosaikkModule::Get()
{
	return FModuleManager::LoadModuleChecked<FMosaikkModule>("Mosaikk");
}

TSharedPtr<SLevelViewport> FMosaikkModule::GetLevelEditorViewport()
{
	if (!FModuleManager::Get().IsModuleLoaded("LevelEditor"))
	{
		return nullptr;
	}

	const FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");
	const TSharedPtr<ILevelEditor> LevelEditor = LevelEditorModule.GetFirstLevelEditor();
	if (!LevelEditor.IsValid())
	{
		return nullptr;
	}

	const TSharedPtr<SLevelEditor> SlateLevelEditor = StaticCastSharedPtr<SLevelEditor>(LevelEditor);
	if (!SlateLevelEditor.IsValid())
	{
		return nullptr;
	}

	return SlateLevelEditor->GetActiveViewportInterface();
}

void FMosaikkModule::OnSequencerCreated(TSharedRef<ISequencer> CreatedSequencer)
{
	UMovieSceneSequence* Sequence = CreatedSequencer->GetRootMovieSceneSequence();
	const ULevelSequence* LevelSequence = Cast<ULevelSequence>(Sequence);
	if (!IsValid(LevelSequence))
	{
		// Not a level sequence (likely UMG, Niagara, Control Rig, etc.).
		return;
	}

	const TSharedPtr<SLevelViewport> LvlViewport = GetLevelEditorViewport();
	if (!LvlViewport.IsValid())
	{
		UE_LOGFMT(LogMosaikk, Error, "{0}: Failed to receive LevelViewport, can't push HostCanvas!", __FUNCTION__);
		return;
	}

	// When Sequencer is created - push HostCanvas to the viewport.
	if (const TSharedPtr<SConstraintCanvas> ActiveSinkHostCanvas = UMosaikkHostCanvasManager::Get().GetHostCanvas())
	{
		LvlViewport->AddOverlayWidget(ActiveSinkHostCanvas->AsShared());
		UE_LOGFMT(LogMosaikk, Display, "{0}: HostCanvas is created and pushed to LevelViewport.", __FUNCTION__);
	}

	if (!CreatedSequencer->OnCloseEvent().IsBoundToObject(this))
	{
		SequencerClosedHandle = CreatedSequencer->OnCloseEvent().AddRaw(this, &FMosaikkModule::OnSequencerClosed);
	}

	CachedSequencer = CreatedSequencer.ToWeakPtr();
}

void FMosaikkModule::OnSequencerClosed(TSharedRef<ISequencer> ClosedSequencer)
{
	const TSharedPtr<SLevelViewport> CurrentLvlViewport = GetLevelEditorViewport();
	if (!CurrentLvlViewport.IsValid())
	{
		UE_LOGFMT(LogMosaikk, Error, "{0}: Failed to receive LevelViewport, can not remove HostCanvas from LevelViewport!", __FUNCTION__);
		return;
	}

	FGlobalTabmanager::Get()->OnTabForegrounded_Unsubscribe(OnTabForegroundedDelegateHandle);
	ClosedSequencer->OnCloseEvent().Remove(SequencerClosedHandle);

	// When sequencer is closed - remove HostCanvas from Viewport, this canvas is used only in Sequencer context.
	if (const TSharedPtr<SConstraintCanvas> HostCanvasPtr = UMosaikkHostCanvasManager::Get().GetHostCanvas())
	{
		CurrentLvlViewport->RemoveOverlayWidget(HostCanvasPtr->AsShared());
		UE_LOGFMT(LogMosaikk, Error, "{0}: HostCanvas successfully removed from LevelViewport!", __FUNCTION__);
	}

	CachedSequencer.Reset();
}

void FMosaikkModule::OnTabForegrounded(TSharedPtr<SDockTab> PreviouslyActive, TSharedPtr<SDockTab> NewlyActivated)
{
	if (!PreviouslyActive.IsValid() || !NewlyActivated.IsValid())
	{
		return;
	}

	const TSharedPtr<ISequencer> Sequencer = CachedSequencer.Pin();
	if (!FMosaikkSequencerUtils::HasMosaikkTracks(Sequencer))
	{
		UE_LOGFMT(LogMosaikk, Display, "{0}: skip call, no Mosaikk tracks found.", __FUNCTION__);
		return;
	}

	const FTabId NewlyActivatedTabId = NewlyActivated->GetLayoutIdentifier();
	if (NewlyActivatedTabId.TabType == LevelEditorTabIds::Sequencer)
	{
		UE_LOGFMT(LogMosaikk, Display, "{0}: HostCanvas is cleared.", __FUNCTION__);

		UMosaikkHostCanvasManager::Get().ClearHostCanvas();
		return;
	}

	const FTabId PreviouslyActiveTabId = PreviouslyActive->GetLayoutIdentifier();
	if (PreviouslyActiveTabId == LevelEditorTabIds::Sequencer)
	{
		if (Sequencer.IsValid())
		{
			UE_LOGFMT(LogMosaikk, Display, "{0}: Sequencer reopened, call ForceEvaluate.", __FUNCTION__);

			// Re-evaluate all entities/sections at current time.
			Sequencer->ForceEvaluate();
		}
	}
}

void FMosaikkModule::AddMosaikkHUBButton(FToolBarBuilder& ToolbarBuilder)
{
	ToolbarBuilder.AddSeparator();
	ToolbarBuilder.BeginSection("MosaikkHUB");

	ToolbarBuilder.AddToolBarButton(
		FMosaikkCommands::Get().OpenMosaikkHUBWindow,
		NAME_None,
		FText::GetEmpty(),
		LOCTEXT("OpenMosaikkHUBTooltip", "Open the Mosaikk HUB."),
		FSlateIcon(FSlateIconFinder::FindIconForClass(UUserWidget::StaticClass())) // TODO: replace with my own Icon.
	);

	ToolbarBuilder.EndSection();
}

void FMosaikkModule::OnMosaikkHUBButtonClicked()
{
	FGlobalTabmanager::Get()->TryInvokeTab(MosaikkHUBTabName);
}

TSharedRef<SDockTab> FMosaikkModule::OnSpawnMosaikkHUBTab(const FSpawnTabArgs& SpawnTabArgs)
{
	const FText WidgetText = FText::Format(
		LOCTEXT("WindowWidgetText", "Add code to {0} in {1} to override this window's contents"),
		FText::FromString(TEXT("FTestTabModule::OnSpawnPluginTab")),
		FText::FromString(TEXT("TestTab.cpp"))
	);
	
	FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");
	TSharedPtr<ILevelEditor> LevelEditor = LevelEditorModule.GetFirstLevelEditor();
	
	FAssetEditorViewportConstructionArgs VPArgs;
	VPArgs.ViewportType = LVT_Perspective;
	VPArgs.bRealtime = true;
	VPArgs.ConfigKey = FName(TEXT("Mosaikk.CameraCutsPreview"));

	MosaikkViewport = SNew(SMosaikkStandaloneLevelViewport, VPArgs).ParentLevelEditor(LevelEditor);
	MosaikkViewport->SetAllowsCinematicControl(false);
	MosaikkViewport->GetLevelViewportClient().SetRealtime(true);
	return SNew(SDockTab)
		[
			MosaikkViewport.ToSharedRef()
		];
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FMosaikkModule, Mosaikk)
