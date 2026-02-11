// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "MovieSceneNameableTrack.h"

#include "MosaikkTrack.generated.h"

UCLASS()
class MOSAIKK_API UMosaikkTrack : public UMovieSceneNameableTrack
{
	GENERATED_BODY()

public:
	UMosaikkTrack();

	// Begin UMovieSceneTrack interface
	virtual bool IsEmpty() const override;
	virtual void AddSection(UMovieSceneSection& Section) override;
	virtual void RemoveSection( UMovieSceneSection& Section ) override;
	virtual void RemoveSectionAt(int32 SectionIndex) override;
	virtual bool SupportsType(TSubclassOf<UMovieSceneSection> SectionClass) const override;
	virtual UMovieSceneSection* CreateNewSection() override;
	virtual const TArray<UMovieSceneSection*>& GetAllSections() const override;
	virtual bool HasSection(const UMovieSceneSection& Section) const override;
	virtual bool SupportsMultipleRows() const override;
	virtual EMovieSceneTrackEasingSupportFlags SupportsEasing(FMovieSceneSupportsEasingParams& Params) const override;
	virtual void RemoveAllAnimationData() override;
	// ~End UMovieSceneTrack interface

#if WITH_EDITORONLY_DATA
	virtual FText GetDefaultDisplayName() const override;
	virtual void PostInitProperties() override;
#endif

protected:
	/** The sections owned by this track .*/
	UPROPERTY()
	TArray<TObjectPtr<UMovieSceneSection>> Sections;
};
