// Fill out your copyright notice in the Description page of Project Settings.

#include "MCore/MovieSceneMosaikkEntitySystem.h"

#include "MCore/MosaikkMovieSceneComponentTypes.h"

UMovieSceneMosaikkEntitySystem::UMovieSceneMosaikkEntitySystem(const FObjectInitializer& ObjInit) : UMovieSceneEntitySystem(ObjInit)
{
	RelevantComponent = FMosaikkMovieSceneTracksComponentTypes::Get().Mosaikk;
}

void UMovieSceneMosaikkEntitySystem::OnLink()
{
	UE_LOGFMT(LogTemp, Warning, "{0} has been called!", __FUNCTION__);
}

void UMovieSceneMosaikkEntitySystem::OnUnlink()
{
	UE_LOGFMT(LogTemp, Warning, "{0} has been called!", __FUNCTION__);
}

void UMovieSceneMosaikkEntitySystem::OnSchedulePersistentTasks(UE::MovieScene::IEntitySystemScheduler* TaskScheduler)
{
	UE_LOGFMT(LogTemp, Warning, "{0} has been called!", __FUNCTION__);
}

void UMovieSceneMosaikkEntitySystem::OnRun(FSystemTaskPrerequisites& InPrerequisites, FSystemSubsequentTasks& Subsequents)
{
	UE_LOGFMT(LogTemp, Warning, "{0} has been called!", __FUNCTION__);
}
