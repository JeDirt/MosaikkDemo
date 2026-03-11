// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "EntitySystem/MovieSceneEntitySystem.h"

#include "Blueprint/UserWidget.h"

#include "MovieSceneMosaikkEntitySystem.generated.h"

struct FMosaikkComponentEvaluationData
{
	FMosaikkComponentEvaluationData(UUserWidget* InWidget = nullptr) : WidgetPtr(InWidget) { }

	/** The Widget that was created to be shown in a section. */
	TObjectPtr<UUserWidget> WidgetPtr;
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

	static void AddReferencedObjects(UObject* InThis, FReferenceCollector& Collector);

	FMosaikkComponentEvaluationData* GetMosaikkComponentEvalData(const FObjectKey& InKey);
	void AddSectionWidget(const FObjectKey& InKey, UUserWidget* InWidget);

	/**
	 * Removes Widget associated with this Section(stored as FObjectKey) from **SectionEvalDataMap**,
	 * removes it from screen and marks it as garbage.
	 */
	void RemoveSectionWidget(const FObjectKey& InKey);

	/** Just removes entry from **SectionEvalDataMap** by passed Section(stored as FObjectKey). */
	void RemoveSectionWidgetEntry(const FObjectKey& InKey);

public:
	/** Pre-animated state */
	TSharedPtr<struct FPreAnimatedWidgetStorage> PreAnimatedStorage;

protected:
	TMap<FObjectKey, FMosaikkComponentEvaluationData> SectionEvalDataMap;
};
