// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "MovieSceneSection.h"

#include "EntitySystem/IMovieSceneEntityProvider.h"

#include "MosaikkSection.generated.h"

class USequenceInfoWidget;

UCLASS()
class MOSAIKK_API UMosaikkSection : public UMovieSceneSection,  public IMovieSceneEntityProvider
{
	GENERATED_BODY()

public:
	UMosaikkSection(const FObjectInitializer& ObjInit);

private:
	virtual void ImportEntityImpl(
		UMovieSceneEntitySystemLinker* EntityLinker,
		const FEntityImportParams& Params,
		FImportedEntity* OutImportedEntity
	) override;

public:
	/** This will be serialized, so when Editor or Sequencer are reopened we have a chance to instantiate widgets. */
	UPROPERTY()
	TSubclassOf<UUserWidget> AssociatedWidgetClass;
};
