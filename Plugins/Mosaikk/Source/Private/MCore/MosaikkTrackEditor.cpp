#include "MCore/MosaikkTrackEditor.h"

#include "Blueprint/UserWidget.h"
#include "ContentBrowserModule.h"
#include "IContentBrowserSingleton.h"
#include "ISequencerSection.h"
#include "MovieSceneSequenceEditor.h"
#include "SequencerSectionPainter.h"
#include "SequencerSettings.h"
#include "Styling/SlateIconFinder.h"
#include "WidgetBlueprint.h"

#include "MCore/MosaikkTypes.h"
#include "MSequencer/MosaikkSection.h"
#include "MSequencer/MosaikkTrack.h"

#define LOCTEXT_NAMESPACE "FMosaikkTrackEditor"

TSharedRef<SWidget> CreateWidgetAssetPicker(FOnAssetSelected OnAssetSelected, FOnAssetEnterPressed OnAssetEnterPressed, TWeakPtr<ISequencer> InSequencer)
{
	const TSharedPtr<ISequencer> Sequencer = InSequencer.Pin();
	UMovieSceneSequence* Sequence = Sequencer.IsValid() ? Sequencer->GetFocusedMovieSceneSequence() : nullptr;

	FAssetPickerConfig AssetPickerConfig;
	{
		AssetPickerConfig.OnAssetSelected = OnAssetSelected;
		AssetPickerConfig.OnAssetEnterPressed = OnAssetEnterPressed;
		AssetPickerConfig.bAllowNullSelection = false;
		AssetPickerConfig.bAddFilterUI = true;
		AssetPickerConfig.InitialAssetViewType = EAssetViewType::List;
		AssetPickerConfig.Filter.bRecursiveClasses = true;
		AssetPickerConfig.Filter.ClassPaths.Add(FTopLevelAssetPath(TEXT("/Script/UMGEditor.WidgetBlueprint")));
		AssetPickerConfig.SaveSettingsName = TEXT("SequencerAssetPicker");
		AssetPickerConfig.AdditionalReferencingAssets.Add(FAssetData(Sequence));
	}

	const FContentBrowserModule& ContentBrowserModule = FModuleManager::Get().LoadModuleChecked<FContentBrowserModule>(TEXT("ContentBrowser"));

	const float WidthOverride = Sequencer.IsValid() ? Sequencer->GetSequencerSettings()->GetAssetBrowserWidth() : 500.0f;
	const float HeightOverride = Sequencer.IsValid() ? Sequencer->GetSequencerSettings()->GetAssetBrowserHeight() : 400.0f;

	return SNew(SBox)
		.WidthOverride(WidthOverride)
		.HeightOverride(HeightOverride)
		[
			ContentBrowserModule.Get().CreateAssetPicker(AssetPickerConfig)
		];
}

UMovieSceneSection* FMosaikkSection::GetSectionObject()
{
	return &Section;
}

int32 FMosaikkSection::OnPaintSection(FSequencerSectionPainter& Painter) const
{
	return Painter.PaintSectionBackground();
}

TSharedRef<SWidget> FMosaikkSection::GenerateSectionWidget()
{
	return ISequencerSection::GenerateSectionWidget();
}

FMosaikkTrackEditor::FMosaikkTrackEditor(TSharedRef<ISequencer> InSequencer) : FMovieSceneTrackEditor(InSequencer)
{
	
}

TSharedRef<ISequencerTrackEditor> FMosaikkTrackEditor::CreateTrackEditor(TSharedRef<ISequencer> InSequencer)
{
	return MakeShared<FMosaikkTrackEditor>(InSequencer);
}

void FMosaikkTrackEditor::BuildAddTrackMenu(FMenuBuilder& MenuBuilder)
{
	UMovieSceneSequence* RootMovieSceneSequence = GetSequencer()->GetRootMovieSceneSequence();
	FMovieSceneSequenceEditor* SequenceEditor = FMovieSceneSequenceEditor::Find(RootMovieSceneSequence);
	if (SequenceEditor == nullptr)
	{
		return;
	}

	/**
	 * This part basically adds our custom track to the list of all available Sequncer tracks.
	 * FMosaikkTrackEditor::AddSequenceAnnotationTrackSubMenu creates submenu with AssetPicker where user can select which widget to use when creating track.
	 */
	MenuBuilder.AddSubMenu(
		LOCTEXT("AddMosaikkTrack", "Mosaikk Track"),
		LOCTEXT("AddMosaikkTrackTooltip", "Create Mosaikk Track with your UMG widget."),
		FNewMenuDelegate::CreateSP(this, &FMosaikkTrackEditor::AddMosaikkTrackSubMenuRoot, TArray<FGuid>()),
		false,
		FSlateIconFinder::FindIconForClass(UUserWidget::StaticClass())
	);
}

bool FMosaikkTrackEditor::SupportsType(TSubclassOf<UMovieSceneTrack> Type) const
{
	return Type == UMosaikkTrack::StaticClass();
}

TSharedRef<ISequencerSection> FMosaikkTrackEditor::MakeSectionInterface(UMovieSceneSection& SectionObject, UMovieSceneTrack& Track, FGuid ObjectBinding)
{
	UMosaikkSection* MosaikkSection = Cast<UMosaikkSection>(&SectionObject);
	check( SupportsType( SectionObject.GetOuter()->GetClass() ) && MosaikkSection != nullptr );

	return MakeShareable( new FMosaikkSection( *MosaikkSection ) );
}

void FMosaikkTrackEditor::AddMosaikkTrackSubMenuRoot(FMenuBuilder& MenuBuilder, TArray<FGuid>)
{
	MenuBuilder.AddWidget(
		CreateWidgetAssetPicker(
			FOnAssetSelected::CreateSP(this, &FMosaikkTrackEditor::AddTrackToSequence),
			FOnAssetEnterPressed::CreateSP(this, &FMosaikkTrackEditor::AddTrackToSequenceEnterPressed), GetSequencer()),
		FText::GetEmpty(),
	true
	);
}

void FMosaikkTrackEditor::AddTrackToSequence(const FAssetData& InAssetData)
{
	FSlateApplication::Get().DismissAllMenus();
	
	UWidgetBlueprint* SelectedWidget = Cast<UWidgetBlueprint>(InAssetData.GetAsset());
	UMovieScene* MovieScene = GetFocusedMovieScene();
	if (!SelectedWidget || !MovieScene)
	{
		return;
	}
	
	if (MovieScene->IsReadOnly())
	{
		return;
	}

	const TSubclassOf<UUserWidget> WidgetClass = Cast<UClass>(SelectedWidget->GeneratedClass);
	if (!WidgetClass || !WidgetClass->IsChildOf(UUserWidget::StaticClass()))
	{
		return;
	}

	const FScopedTransaction Transaction(LOCTEXT("AddMosaikkTrackDescription", "Add Mosaikk track."));
	MovieScene->Modify();

	UMosaikkTrack* Track = CastChecked<UMosaikkTrack>(MovieScene->AddTrack<UMosaikkTrack>());
	UMosaikkSection* Section = Cast<UMosaikkSection>(Track->CreateNewSection());
	check(Section);

	Section->AssociatedWidgetClass = WidgetClass;

	// Setup section range.
	const UMosaikkSettings* MosaikkSettingsCDO = GetDefault<UMosaikkSettings>();
	const FFrameTime StartFrameTime = ConvertFrameTime(MosaikkSettingsCDO->DefaultSectionRange.X, MovieScene->GetDisplayRate(), MovieScene->GetTickResolution());
	const FFrameTime EndFrameTime = ConvertFrameTime(MosaikkSettingsCDO->DefaultSectionRange.Y, MovieScene->GetDisplayRate(), MovieScene->GetTickResolution());
	Section->SetRange(TRange<FFrameNumber>(
		StartFrameTime.RoundToFrame(),
		EndFrameTime.RoundToFrame()
	));

	Track->Modify();
	Track->AddSection(*Section);
	Track->SetDisplayName(FText::FromString("Mosaikk Track"));

	if (GetSequencer().IsValid())
	{
		GetSequencer()->OnAddTrack(Track, FGuid());
	}
}

void FMosaikkTrackEditor::AddTrackToSequenceEnterPressed(const TArray<FAssetData>& InAssetData)
{
	if (InAssetData.Num() > 0)
	{
		AddTrackToSequence(InAssetData[0].GetAsset());
	}
}

#undef LOCTEXT_NAMESPACE
