// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "EntitySystem/MovieSceneEntitySystem.h"

#include "Blueprint/UserWidget.h"

#include "MovieSceneMosaikkEntitySystem.generated.h"

struct FMosaikkComponentEvaluationData
{
	/** The Widget that was created to be shown in a section. */
	TWeakObjectPtr<UUserWidget> Widget;

	/** TODO: now does nothing to widget's opacity, ideally opacity can be controlled in Sequencer. */
	double Opacity = 1.0f;
};

UCLASS()
class MOSAIKK_API UMovieSceneMosaikkEntitySystem : public UMovieSceneEntitySystem
{
	GENERATED_BODY()

public:
	UMovieSceneMosaikkEntitySystem(const FObjectInitializer& ObjInit);

public:
	// Begin UMovieSceneEntitySystem interface
	virtual void OnLink() override;
	virtual void OnUnlink() override;
	virtual void OnSchedulePersistentTasks(UE::MovieScene::IEntitySystemScheduler* TaskScheduler) override;
	virtual void OnRun(FSystemTaskPrerequisites& InPrerequisites, FSystemSubsequentTasks& Subsequents) override;
	// ~End UMovieSceneEntitySystem interface

public:
	/** 
	 * Map of all created Widgets. 
	 * 
	 * Section: FInstanceHandle, FObjectKey
	 * to
	 * FMosaikkComponentEvaluationData
	 */
	using FInstanceHandle = UE::MovieScene::FInstanceHandle;
	using FInstanceObjectKey = TTuple<FInstanceHandle, FObjectKey>;
	using FWidgetBySection = TMap<FInstanceObjectKey, FMosaikkComponentEvaluationData>;
	FWidgetBySection SectionToWidgetMap;
	
	// TODO: CAN BE GC'ed potentially
	TMap<UE::MovieScene::FMovieSceneEntityID, UUserWidget*> WidgetMap;

	void ShowWidget(UUserWidget* Widget);
	void HideAllWidgets();
	
	
};
