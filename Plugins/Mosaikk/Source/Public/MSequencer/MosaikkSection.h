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

#if WITH_EDITOR
	virtual void PostLoad() override;
#endif

private:
	virtual void ImportEntityImpl(
		UMovieSceneEntitySystemLinker* EntityLinker,
		const FEntityImportParams& Params,
		FImportedEntity* OutImportedEntity
	) override;

public:
	// This will be serialized, so Editor or Sequencer reopened we have a chance to instantiate widgets.
	UPROPERTY()
	TSubclassOf<UUserWidget> AssociatedWidgetClass;

	// This will not be serialized, this is valid only while Sequencer is opened.
	UPROPERTY(Transient)
	UUserWidget* AssociatedWidgetInstance = nullptr;
};
