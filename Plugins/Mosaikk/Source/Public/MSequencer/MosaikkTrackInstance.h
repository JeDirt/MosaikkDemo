// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "EntitySystem/TrackInstance/MovieSceneTrackInstance.h"

#include "MosaikkTrackInstance.generated.h"

class UUserWidget;

UCLASS()
class MOSAIKK_API UMosaikkTrackInstance : public UMovieSceneTrackInstance
{
	GENERATED_BODY()

private:
	/** Called when this animator is first created to perform any initialization */
	virtual void OnInitialize() override;

	/** Called when the sequence is updated to apply animation */
	virtual void OnAnimate() override;

	/** Called when this animator is about to have its inputs updated */
	virtual void OnBeginUpdateInputs() override;

	/** Called when this animator has had an input added */
	virtual void OnInputAdded(const FMovieSceneTrackInstanceInput& InInput) override;

	/** Called when this animator has had an input removed */
	virtual void OnInputRemoved(const FMovieSceneTrackInstanceInput& InInput) override;

	/** Called after this animator has finished updating its inputs */
	virtual void OnEndUpdateInputs() override;

	/** Called when this animator is being destroyed in order that it can perform any final fixup */
	virtual void OnDestroyed() override;

protected:
	void ShowWidget(UUserWidget* Widget);
	void HideAllWidgets();

private:
	/**
	 * Widget that TrackInstance receives from associated Section.
	 * Section stores this Widget and TrackInstance manipulates widget's visibility based on section evaluation.
	 */
	TWeakObjectPtr<UUserWidget> CachedWidget;
};
