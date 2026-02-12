// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "EntitySystem/MovieSceneEntitySystem.h"

#include "MovieSceneMosaikkEntitySystem.generated.h"

UCLASS()
class MOSAIKK_API UMovieSceneMosaikkEntitySystem : public UMovieSceneEntitySystem
{
	GENERATED_BODY()

public:
	using FInstanceHandle = UE::MovieScene::FInstanceHandle;
	using FMovieSceneEntityID = UE::MovieScene::FMovieSceneEntityID;

	UMovieSceneMosaikkEntitySystem(const FObjectInitializer& ObjInit);

public:
	// Begin UMovieSceneEntitySystem interface
	virtual void OnLink() override;
	virtual void OnUnlink() override;
	virtual void OnSchedulePersistentTasks(UE::MovieScene::IEntitySystemScheduler* TaskScheduler) override;
	virtual void OnRun(FSystemTaskPrerequisites& InPrerequisites, FSystemSubsequentTasks& Subsequents) override;
	// ~End UMovieSceneEntitySystem interface
};
