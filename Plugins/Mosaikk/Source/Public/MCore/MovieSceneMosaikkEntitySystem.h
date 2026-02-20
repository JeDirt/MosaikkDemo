// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "EntitySystem/MovieSceneEntitySystem.h"

#include "Blueprint/UserWidget.h"

#include "MovieSceneMosaikkEntitySystem.generated.h"

struct FMosaikkComponentEvaluationData
{
	/** The Widget that was created to be shown in a section. */
	TWeakObjectPtr<UUserWidget> Widget;
};

struct FEvaluateMosaikk
{
	using FEntityAllocation   = UE::MovieScene::FEntityAllocation;
	using FMovieSceneEntityID = UE::MovieScene::FMovieSceneEntityID;
	using FRootInstanceHandle = UE::MovieScene::FRootInstanceHandle;

public:
	FEvaluateMosaikk(class UMovieSceneMosaikkEntitySystem* InMosaikkEntitySystem) 
		: MosaikkEntitySystem(InMosaikkEntitySystem) { }

	void ForEachAllocation(
		const FEntityAllocation* Allocation,
		UE::MovieScene::TRead<FMovieSceneEntityID> EntityIDs,
		UE::MovieScene::TRead<FRootInstanceHandle> RootInstanceHandles,
		UE::MovieScene::TRead<struct FMovieSceneMosaikkComponentData> MosaikkComponentDatas
	) const;

private:
	void Evaluate(
		const FMovieSceneEntityID& EntityID,
		const FMovieSceneMosaikkComponentData& InMosaikkData,
		const FRootInstanceHandle& RootInstanceHandle,
		bool bWantsRestoreState
	) const;

public:
	TWeakObjectPtr<UMovieSceneMosaikkEntitySystem> MosaikkEntitySystem;
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
	// ~End UMovieSceneEntitySystem interface
	
	FMosaikkComponentEvaluationData* GetMosaikkComponentEvalData(const FObjectKey& InKey);
	void AddNewWidget(const FObjectKey& InKey, UUserWidget* InWidget);

public:
	/** Pre-animated state */
	TSharedPtr<struct FPreAnimatedWidgetStorage> PreAnimatedStorage;

	static TMap<FObjectKey, FMosaikkComponentEvaluationData> SectionToMosaikkComponentEvalDataMap;
};
