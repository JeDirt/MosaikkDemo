// Copyright Epic Games, Inc. All Rights Reserved.

#include "Mosaikk.h"

#include "Editor/LevelEditor/Private/SLevelEditor.h"
#include "ISequencerModule.h"
#include "LevelEditor.h"
#include "LevelSequence.h"
#include "MovieSceneCaptureDialogModule.h"
#include "MovieSceneSequence.h"
#include "Widgets/Layout/SConstraintCanvas.h"

#include "MActor/MosaikkProxyActor.h"
#include "MCore/MosaikkCommands.h"
#include "MCore/MosaikkStyle.h"
#include "MCore/MosaikkTrackEditor.h"
#include "MCore/MosaikkUtils.h"
#include "MCore/MovieSceneMosaikkEntitySystem.h"

#define LOCTEXT_NAMESPACE "FMosaikkModule"

DEFINE_LOG_CATEGORY_STATIC(LogMosaikk, Log, All);

static const FName MosaikkHUBTabName("Mosaikk HUB");

void FMosaikkModule::StartupModule()
{
	FMosaikkStyle::Initialize();
	FMosaikkStyle::ReloadTextures();

	FMosaikkCommands::Register();

	PluginCommands = MakeShareable(new FUICommandList);

	PluginCommands->MapAction(
		FMosaikkCommands::Get().OpenMosaikkHUBWindow,
		FExecuteAction::CreateRaw(this, &FMosaikkModule::OnMosaikkButtonClicked_RESERVED),
		FCanExecuteAction());

	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(MosaikkHUBTabName, FOnSpawnTab::CreateRaw(this, &FMosaikkModule::OnSpawnPluginTab))
		.SetDisplayName(LOCTEXT("FMosaikkHUBTabTitle", "Mosaikk HUB"))
		.SetMenuType(ETabSpawnerMenuType::Hidden);

	// Hook into Sequencer creation.
	ISequencerModule& SequencerModule = FModuleManager::LoadModuleChecked<ISequencerModule>("Sequencer");

	FOnSequencerCreated::FDelegate OnSequencerCreated;
	OnSequencerCreated.BindRaw(this, &FMosaikkModule::OnSequencerCreated);
	SequencerCreatedHandle = SequencerModule.RegisterOnSequencerCreated(OnSequencerCreated);

	/**
	 * Create and add extender for Sequencer.
	 * Thus we tell that Sequencer is gonna be modified - our custom Mosaikk button will be placed after CurveEditor button.
	 */
	SequencerToolbarExtender = MakeShared<FExtender>();
	SequencerToolbarExtender->AddToolBarExtension(
		NAME_None,
		EExtensionHook::After,
		PluginCommands,
		FToolBarExtensionDelegate::CreateRaw(this, &FMosaikkModule::AddMosaikkButton_RESERVED)
	);

	OnTabForegroundedDelegateHandle = FGlobalTabmanager::Get()->OnTabForegrounded_Subscribe(
		FOnActiveTabChanged::FDelegate::CreateRaw(
			this, &FMosaikkModule::OnTabForegrounded)
	);

	SequencerModule.GetToolBarExtensibilityManager()->AddExtender(SequencerToolbarExtender);

	TrackEditorRegisteredHandle = SequencerModule.RegisterTrackEditor(FOnCreateTrackEditor::CreateStatic(&FMosaikkTrackEditor::CreateTrackEditor));
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

TSharedPtr<SConstraintCanvas> FMosaikkModule::GetHostCanvas() const
{
	return HostCanvas;
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
		// Not a level sequence (likely UMG, Niagara, Control Rig, etc.), so prevent binding to delegates and creating Canvas.
		return;
	}

	const TSharedPtr<SLevelViewport> LvlViewport = GetLevelEditorViewport();
	if (!LvlViewport.IsValid())
	{
		UE_LOGFMT(LogMosaikk, Error, "{0}: Failed to receive LevelViewport, can't push HostCanvas!", __FUNCTION__);
		return;
	}

	// When Sequencer is created - push our custom canvas to the viewport.
	if (!HostCanvas.IsValid())
	{
		HostCanvas = SNew(SConstraintCanvas);
		LvlViewport->AddOverlayWidget(HostCanvas->AsShared());
		UE_LOGFMT(LogMosaikk, Display, "{0}: HostCanvas is created and pushed to LevelViewport.", __FUNCTION__);
	}

	if (!CreatedSequencer->OnCloseEvent().IsBoundToObject(this))
	{
		SequencerClosedHandle = CreatedSequencer->OnCloseEvent().AddRaw(this, &FMosaikkModule::OnSequencerClosed);
	}

	if (!FEditorDelegates::PostPIEStarted.IsBoundToObject(this))
	{
		PostPIEStartedHandle = FEditorDelegates::PostPIEStarted.AddRaw(this, &FMosaikkModule::OnPostPIEStarted);
	}

	CachedSequencer = CreatedSequencer.ToWeakPtr();
}

void FMosaikkModule::OnSequencerClosed(TSharedRef<ISequencer> ClosedSequencer)
{
	const TSharedPtr<SLevelViewport> CurrentLvlViewport = GetLevelEditorViewport();
	if (!CurrentLvlViewport.IsValid())
	{
		UE_LOGFMT(LogMosaikk, Error, "{0}: Failed to receive LevelViewport, HostCanvas might not be removed from LevelViewport!", __FUNCTION__);
		return;
	}

	FGlobalTabmanager::Get()->OnTabForegrounded_Unsubscribe(OnTabForegroundedDelegateHandle);

	ClosedSequencer->OnCloseEvent().Remove(SequencerClosedHandle);
	FEditorDelegates::PostPIEStarted.Remove(PostPIEStartedHandle);

	// When sequencer is closed - remove our custom canvas from viewport, this canvas is used only in Sequencer context.
	if (HostCanvas.IsValid())
	{
		CurrentLvlViewport->RemoveOverlayWidget(HostCanvas->AsShared());
		HostCanvas.Reset();
		UE_LOGFMT(LogMosaikk, Error, "{0}: HostCanvas successfully removed from LevelViewport!", __FUNCTION__);
	}

	CachedSequencer.Reset();
}

void FMosaikkModule::OnPostPIEStarted(bool bSimulating)
{
	const TSharedPtr<FMovieSceneCaptureBase> CurrentCapture = IMovieSceneCaptureDialogModule::Get().GetCurrentCapture();
	if (!CurrentCapture.IsValid())
	{
		// Don't proceed with spawning MosaikkProxyActor if PIE Viewport that started is not started in context of capturing movie.
		return;
	}

	UWorld* CapturedWorld = CurrentCapture->GetWorld();
	if (!IsValid(CapturedWorld))
	{
		return;
	}

	// Spawn proxy actor that will add actual widget to the screen during capture.
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	SpawnParams.ObjectFlags |= RF_Transient;
	SpawnParams.bAllowDuringConstructionScript = true;

	CapturedWorld->SpawnActor<AMosaikkProxyActor>(SpawnParams);
}

void FMosaikkModule::AddMosaikkButton_RESERVED(FToolBarBuilder& ToolbarBuilder)
{
	ToolbarBuilder.AddSeparator();
	ToolbarBuilder.BeginSection("MosaikkHUB");
	
		ToolbarBuilder.AddToolBarButton(
			FMosaikkCommands::Get().OpenMosaikkHUBWindow,
			NAME_None,
			FText::FromString("Mosaikk HUB"),
			FText::FromString("Open the Mosaikk HUB window."),
			FSlateIcon(FAppStyle::GetAppStyleSetName(), "Icons.Camera")
		);
	
	ToolbarBuilder.EndSection();
}

void FMosaikkModule::OnMosaikkButtonClicked_RESERVED()
{
	FGlobalTabmanager::Get()->TryInvokeTab(MosaikkHUBTabName);
}

TSharedRef<SDockTab> FMosaikkModule::OnSpawnPluginTab(const class FSpawnTabArgs& SpawnTabArgs)
{
	const FText WidgetText = FText::Format(
		LOCTEXT("WindowWidgetText", "Add code to {0} in {1} to override this window's contents"),
		FText::FromString(TEXT("FTestTabModule::OnSpawnPluginTab")),
		FText::FromString(TEXT("TestTab.cpp"))
	);

	return SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			SNew(SBox)
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(WidgetText)
			]
		];
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
		FMosaikkWidgetUtils::ClearHostCanvas();

		// TODO: get rid of this global map at ALL!
		UMovieSceneMosaikkEntitySystem::SectionToMosaikkComponentEvalDataMap.Reset();
		UE_LOGFMT(LogMosaikk, Display, "{0}: HostCanvas is cleared.", __FUNCTION__);

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

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FMosaikkModule, Mosaikk)