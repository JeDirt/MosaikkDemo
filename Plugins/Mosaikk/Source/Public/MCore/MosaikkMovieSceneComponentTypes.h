#pragma once

#include "EntitySystem/MovieSceneEntitySystemLinker.h"

#include "EntitySystem/MovieSceneEntityFactoryTemplates.h"

#include "MosaikkMovieSceneComponentTypes.generated.h"

class UMosaikkSection;

USTRUCT()
struct FMovieSceneMosaikkComponentData
{
	GENERATED_BODY()

	UPROPERTY()
	TObjectPtr<UMosaikkSection> Section = nullptr;
};

/**
 * Mosaikk Components that work in tandem with UMovieSceneMosaikkSystem.
 * Components are linked to a UMovieSceneMosaikkSystem which manages entities and handles evaluation of widgets.
 */
class FMosaikkMovieSceneTracksComponentTypes
{
public:
	using FComponentRegistry = UE::MovieScene::FComponentRegistry;
	using EComponentTypeFlags = UE::MovieScene::EComponentTypeFlags;

	static FMosaikkMovieSceneTracksComponentTypes& Get() 
	{
		static FMosaikkMovieSceneTracksComponentTypes Instance;
		return Instance;
	}

private:
	FMosaikkMovieSceneTracksComponentTypes()
	{
		FComponentRegistry* ComponentRegistry = UMovieSceneEntitySystemLinker::GetComponents();
		ComponentRegistry->NewComponentType(&Mosaikk, TEXT("Mosaikk"));
	}

	FMosaikkMovieSceneTracksComponentTypes(const FMosaikkMovieSceneTracksComponentTypes&) = delete;
	FMosaikkMovieSceneTracksComponentTypes(FMosaikkMovieSceneTracksComponentTypes&&) = delete;
	FMosaikkMovieSceneTracksComponentTypes& operator=(FMosaikkMovieSceneTracksComponentTypes&&) = delete;
	FMosaikkMovieSceneTracksComponentTypes& operator=(const FMosaikkMovieSceneTracksComponentTypes&) = delete;

public:
	UE::MovieScene::TComponentTypeID<FMovieSceneMosaikkComponentData> Mosaikk;
};
