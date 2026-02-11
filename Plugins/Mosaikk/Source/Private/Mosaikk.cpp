// Copyright Epic Games, Inc. All Rights Reserved.

#include "Mosaikk.h"

#include "Editor/LevelEditor/Private/SLevelEditor.h"
#include "ISequencerModule.h"
#include "LevelEditor.h"
#include "LevelSequence.h"
#include "MovieSceneCaptureDialogModule.h"

#include "MActor/MosaikkProxyActor.h"
#include "MCore/MosaikkCommands.h"
#include "MCore/MosaikkStyle.h"
#include "MCore/MosaikkTrackEditor.h"

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

	SequencerModule.GetToolBarExtensibilityManager()->AddExtender(SequencerToolbarExtender);

	TrackEditorRegisteredHandle = SequencerModule.RegisterTrackEditor(FOnCreateTrackEditor::CreateStatic(&FMosaikkTrackEditor::CreateTrackEditor));
}

void FMosaikkModule::ShutdownModule()
{
	FMosaikkStyle::Shutdown();

	FMosaikkCommands::Unregister();

	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(MosaikkHUBTabName);
	
	/**
	 * This function may be called during shutdown to clean up your module.
	 * For modules that support dynamic reloading, we call this function before unloading the module.
	 */
	if (FModuleManager::Get().IsModuleLoaded("Sequencer"))
	{
		ISequencerModule& SequencerModule = FModuleManager::GetModuleChecked<ISequencerModule>("Sequencer");
		SequencerModule.UnregisterOnSequencerCreated(SequencerCreatedHandle);
		SequencerModule.UnRegisterTrackEditor(TrackEditorRegisteredHandle);

		UE_LOG(LogMosaikk, Display, TEXT("FMosaikkModule::ShutdownModule: Cleanup SequencerCreated callbacks."));
	}
}

FMosaikkModule& FMosaikkModule::Get()
{
	return FModuleManager::LoadModuleChecked<FMosaikkModule>("Mosaikk");
}

TSharedPtr<SOverlay> FMosaikkModule::GetMosaikkViewportOverlay() const
{
	return MosaikkViewportOverlay;
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
		// Not a level sequence (likely UMG, Niagara, Control Rig, etc.), so prevent binding to delegates and creating Overlay.
		return;
	}

	const TSharedPtr<SLevelViewport> LvlViewport = GetLevelEditorViewport();
	if (!LvlViewport.IsValid())
	{
		UE_LOG(LogMosaikk, Error, TEXT("FMosaikkModule::OnSequencerCreated: Failed to receive LevelViewport, can't push MosaikkViewportOverlay!"));
		return;
	}

	// When Sequencer is created - push our custom overlay to the viewport.
	if (!MosaikkViewportOverlay.IsValid())
	{
		MosaikkViewportOverlay = SNew(SOverlay);
		LvlViewport->AddOverlayWidget(MosaikkViewportOverlay->AsShared());
		UE_LOG(LogMosaikk, Display, TEXT("FMosaikkModule::OnSequencerCreated: MosaikkViewportOverlay is created and pushed to LevelViewport"));
	}

	if (!CreatedSequencer->OnCloseEvent().IsBoundToObject(this))
	{
		SequencerClosedHandle = CreatedSequencer->OnCloseEvent().AddRaw(this, &FMosaikkModule::OnSequencerClosed);
	}

	if (!FEditorDelegates::PostPIEStarted.IsBoundToObject(this))
	{
		PostPIEStartedHandle = FEditorDelegates::PostPIEStarted.AddRaw(this, &FMosaikkModule::OnPostPIEStarted);
	}
}

void FMosaikkModule::OnSequencerClosed(TSharedRef<ISequencer> ClosedSequencer)
{
	const TSharedPtr<SLevelViewport> CurrentLvlViewport = GetLevelEditorViewport();
	if (!CurrentLvlViewport.IsValid())
	{
		UE_LOG(LogMosaikk, Error, TEXT("FMosaikkModule::OnSequencerClosed: Failed to receive LevelViewport, MosaikkViewportOverlay might not be removed from LevelViewport!"));
		return;
	}

	ClosedSequencer->OnCloseEvent().Remove(SequencerClosedHandle);
	FEditorDelegates::PostPIEStarted.Remove(PostPIEStartedHandle);

	// When sequencer is closed - remove our custom overlay from viewport, this overlay is used only in Sequencer context.
	if (MosaikkViewportOverlay.IsValid())
	{
		CurrentLvlViewport->RemoveOverlayWidget(MosaikkViewportOverlay->AsShared());
		MosaikkViewportOverlay.Reset();
		UE_LOG(LogMosaikk, Display, TEXT("FMosaikkModule::OnSequencerClosed: MosaikkViewportOverlay successfully removed from LevelViewport!"));
	}
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
	FText WidgetText = FText::Format(
		LOCTEXT("WindowWidgetText", "Add code to {0} in {1} to override this window's contents"),
		FText::FromString(TEXT("FTestTabModule::OnSpawnPluginTab")),
		FText::FromString(TEXT("TestTab.cpp"))
		);

	return SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			// Put your tab content here!
			SNew(SBox)
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(WidgetText)
			]
		];
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FMosaikkModule, Mosaikk)