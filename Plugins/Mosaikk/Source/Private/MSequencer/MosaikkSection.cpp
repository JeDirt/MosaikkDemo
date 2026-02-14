// Fill out your copyright notice in the Description page of Project Settings.

#include "MSequencer/MosaikkSection.h"

#include "EntitySystem/BuiltInComponentTypes.h"

#include "MCore/MosaikkMovieSceneComponentTypes.h"

UMosaikkSection::UMosaikkSection(const FObjectInitializer& ObjInit) : Super(ObjInit)
{
	// This section must be public as the object animator system needs to reference it and it lives in a different package.
	// Without this flag, object reinstancing will clear out the pointer to the section with FArchiveReplaceOrClearExternalReferences
	SetFlags(RF_Public);
}

void UMosaikkSection::ImportEntityImpl(
	UMovieSceneEntitySystemLinker* EntityLinker,
	const FEntityImportParams& Params,
	FImportedEntity* OutImportedEntity
)
{
	using namespace UE::MovieScene;

	const FBuiltInComponentTypes* BuiltInComponents = FBuiltInComponentTypes::Get();
	const FMosaikkMovieSceneTracksComponentTypes& MosaikkComponents = FMosaikkMovieSceneTracksComponentTypes::Get();
	const FGuid ObjectBindingID = Params.GetObjectBindingID();

	OutImportedEntity->AddBuilder(
			FEntityBuilder()
			.AddConditional(BuiltInComponents->GenericObjectBinding, ObjectBindingID, ObjectBindingID.IsValid())
			.AddTagConditional(BuiltInComponents->Tags.Root, !ObjectBindingID.IsValid())
			.Add(MosaikkComponents.Mosaikk, FMovieSceneMosaikkComponentData { this })
	);
}
