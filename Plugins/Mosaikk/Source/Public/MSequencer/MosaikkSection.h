// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "MovieSceneSection.h"

#include "EntitySystem/IMovieSceneEntityProvider.h"

#include "MosaikkSection.generated.h"

UCLASS()
class MOSAIKK_API UMosaikkSection : public UMovieSceneSection,  public IMovieSceneEntityProvider
{
	GENERATED_BODY()

public:
	UMosaikkSection(const FObjectInitializer& ObjInit);

private:
	//~ IMovieSceneEntityProvider interface
	virtual void ImportEntityImpl(
		UMovieSceneEntitySystemLinker* EntityLinker,
		const FEntityImportParams& Params,
		FImportedEntity* OutImportedEntity
	) override;

	virtual bool PopulateEvaluationFieldImpl(
		const TRange<FFrameNumber>& EffectiveRange,
		const FMovieSceneEvaluationFieldEntityMetaData& InMetaData,
		FMovieSceneEntityComponentFieldBuilder* OutFieldBuilder
	) override;
	// ~End IMovieSceneEntityProvider

public:
	/** This will be serialized, so when Editor or Sequencer are reopened we have a chance to instantiate widgets. */
	UPROPERTY()
	TSubclassOf<UUserWidget> AssociatedWidgetClass;
};
