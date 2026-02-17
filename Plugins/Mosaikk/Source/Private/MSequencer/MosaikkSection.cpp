// Fill out your copyright notice in the Description page of Project Settings.

#include "MSequencer/MosaikkSection.h"

#include "EntitySystem/BuiltInComponentTypes.h"

#include "MCore/MosaikkMovieSceneComponentTypes.h"
#include "Tracks/MovieScenePropertyTrack.h"

UMosaikkSection::UMosaikkSection(const FObjectInitializer& ObjInit) : Super(ObjInit)
{
	// This section must be public as the object animator system needs to reference it and it lives in a different package.
	// Without this flag, object reinstancing will clear out the pointer to the section with FArchiveReplaceOrClearExternalReferences
	SetFlags(RF_Public);

	const int32 LinkerCustomVersion = GetLinkerCustomVersion(FSequencerObjectVersion::GUID);
	const EMovieSceneCompletionMode CompletionMode = 
		LinkerCustomVersion < FSequencerObjectVersion::WhenFinishedDefaultsToProjectDefault 
		? EMovieSceneCompletionMode::RestoreState : EMovieSceneCompletionMode::ProjectDefault;
	EvalOptions.EnableAndSetCompletionMode(CompletionMode);
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

	OutImportedEntity->AddBuilder(
		FEntityBuilder()
		.AddTag(BuiltInComponents->Tags.Root)
		.Add(MosaikkComponents.Mosaikk, FMovieSceneMosaikkComponentData { this })
	);
}

bool UMosaikkSection::PopulateEvaluationFieldImpl(
	const TRange<FFrameNumber>& EffectiveRange,
	const FMovieSceneEvaluationFieldEntityMetaData& InMetaData, 
	FMovieSceneEntityComponentFieldBuilder* OutFieldBuilder
)
{
	return false;
}
