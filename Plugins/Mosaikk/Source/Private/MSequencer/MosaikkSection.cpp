// Fill out your copyright notice in the Description page of Project Settings.

#include "MSequencer/MosaikkSection.h"

#include "Blueprint/UserWidget.h"
#include "EntitySystem/BuiltInComponentTypes.h"

#include "MCore/MosaikkMovieSceneComponentTypes.h"

#include "MSequencer/MosaikkTrackInstance.h"

UMosaikkSection::UMosaikkSection(const FObjectInitializer& ObjInit) : Super(ObjInit)
{
	// This section must be public as the object animator system needs to reference it and it lives in a different package.
	// Without this flag, object reinstancing will clear out the pointer to the section with FArchiveReplaceOrClearExternalReferences
	SetFlags(RF_Public);
}

#if WITH_EDITOR
void UMosaikkSection::PostLoad()
{
	Super::PostLoad();

	/**
	 * When Editor is restarted we need to re-instance Widget that was added previously to the section.
	 * Since AssociatedWidgetClass is set up on section creation and serialized we can easily do it here.
	 */
	if (!IsValid(AssociatedWidgetInstance))
	{
		AssociatedWidgetInstance = CreateWidget<UUserWidget>(GEditor->GetEditorWorldContext().World(), AssociatedWidgetClass);
	}
}
#endif

void UMosaikkSection::ImportEntityImpl(
	UMovieSceneEntitySystemLinker* EntityLinker,
	const FEntityImportParams& Params,
	FImportedEntity* OutImportedEntity
)
{
	using namespace UE::MovieScene;

	const FBuiltInComponentTypes* BuiltInComponents = FBuiltInComponentTypes::Get();
	const FMosaikkMovieSceneTracksComponentTypes& MosaikkComponents = FMosaikkMovieSceneTracksComponentTypes::Get();
	FGuid ObjectBindingID = Params.GetObjectBindingID();

	OutImportedEntity->AddBuilder(
		FEntityBuilder()
		.Add(MosaikkComponents.Mosaikk, FMovieSceneMosaikkComponentData { this })
		.Add(BuiltInComponents->TrackInstance, FMovieSceneTrackInstanceComponent{ decltype(FMovieSceneTrackInstanceComponent::Owner)(this), UMosaikkTrackInstance::StaticClass() })
		.AddConditional(BuiltInComponents->GenericObjectBinding, ObjectBindingID, ObjectBindingID.IsValid())
		.AddTagConditional(BuiltInComponents->Tags.Root, !ObjectBindingID.IsValid())
	);
}
