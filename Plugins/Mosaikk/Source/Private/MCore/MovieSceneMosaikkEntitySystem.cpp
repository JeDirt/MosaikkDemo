// Fill out your copyright notice in the Description page of Project Settings.

#include "MCore/MovieSceneMosaikkEntitySystem.h"

#include "Engine/World.h"
#include "EntitySystem/BuiltInComponentTypes.h"
#include "Evaluation/PreAnimatedState/MovieScenePreAnimatedObjectStorage.h"
#include "Evaluation/PreAnimatedState/MovieScenePreAnimatedStorageID.inl"

#include "MCore/MosaikkMovieSceneComponentTypes.h"
#include "MRendering/MosaikkHostCanvasManager.h"
#include "MSequencer/MosaikkSection.h"

struct FCachedPreAnimatedMosaikk
{
	FCachedPreAnimatedMosaikk(UMosaikkSection* InSection = nullptr) 
	: SectionPtr(InSection) { }

	TWeakObjectPtr<UMosaikkSection> SectionPtr;
};

template<typename BaseTraits>
struct FPreAnimatedWidgetStateTraits : BaseTraits
{
	using KeyType = FObjectKey;
	using StorageType = FCachedPreAnimatedMosaikk;

	FCachedPreAnimatedMosaikk CachePreAnimatedValue(FObjectKey InKey)
	{
		return { Cast<UMosaikkSection>(InKey.ResolveObjectPtr()) };
	}

	void RestorePreAnimatedValue(FObjectKey InKey, FCachedPreAnimatedMosaikk PreAnimatedMosaikk, const UE::MovieScene::FRestoreStateParams& Params)
	{
		const auto LinkedSystem = Cast<UMovieSceneMosaikkEntitySystem>(Params.Linker->FindSystem(UMovieSceneMosaikkEntitySystem::StaticClass()));
		if (!IsValid(LinkedSystem))
		{
			return;
		}

		// Weak section can be stale during teardown; fall back to the incoming key if needed.
		const UObject* SectionObj = PreAnimatedMosaikk.SectionPtr.Get();
		const FObjectKey SectionKey = SectionObj ? FObjectKey(SectionObj) : InKey;
		const auto CompEvalData = LinkedSystem->GetMosaikkComponentEvalData(SectionKey);
		if (CompEvalData == nullptr)
		{
			return;
		}

		// Widget is already invalid, just remove it from map here and now.
		UUserWidget* WidgetToRemove = CompEvalData->WidgetPtr.Get();
		if (!IsValid(WidgetToRemove))
		{
			LinkedSystem->RemoveSectionWidgetEntry(SectionKey);
			return;
		}

		LinkedSystem->RemoveSectionWidget(SectionKey);
	}
};

using FPreAnimatedBoundObjectWidgetStateTraits = FPreAnimatedWidgetStateTraits<UE::MovieScene::FBoundObjectPreAnimatedStateTraits>;

struct FPreAnimatedWidgetStorage : UE::MovieScene::TPreAnimatedStateStorage_ObjectTraits<FPreAnimatedBoundObjectWidgetStateTraits>
{
	static UE::MovieScene::TAutoRegisterPreAnimatedStorageID<FPreAnimatedWidgetStorage> StorageID;
};

UE::MovieScene::TAutoRegisterPreAnimatedStorageID<FPreAnimatedWidgetStorage> FPreAnimatedWidgetStorage::StorageID;

/** Begin ~FEvaluateMosaikk implementation */
void FEvaluateMosaikk::ForEachAllocation(
	const FEntityAllocation* Allocation,
	UE::MovieScene::TRead<FMovieSceneEntityID> EntityIDs,
	UE::MovieScene::TRead<FRootInstanceHandle> RootInstanceHandles,
	UE::MovieScene::TRead<FMovieSceneMosaikkComponentData> MosaikkComponentDatas
) const
{
	using FBuiltInComponentTypes = UE::MovieScene::FBuiltInComponentTypes;

	const FBuiltInComponentTypes* BuiltInComponents = FBuiltInComponentTypes::Get();
	const bool bWantsRestoreState = Allocation->HasComponent(BuiltInComponents->Tags.RestoreState);

	const int32 Num = Allocation->Num();
	for (int32 Index = 0; Index < Num; ++Index)
	{
		const FMovieSceneEntityID& EntityID = EntityIDs[Index];
		const FRootInstanceHandle& RootInstanceHandle = RootInstanceHandles[Index];
		const FMovieSceneMosaikkComponentData& MosaikkData = MosaikkComponentDatas[Index];

		Evaluate(EntityID, MosaikkData, RootInstanceHandle, bWantsRestoreState);
	}
}

void FEvaluateMosaikk::Evaluate(
	const FMovieSceneEntityID& EntityID,
	const FMovieSceneMosaikkComponentData& InMosaikkData, 
	const FRootInstanceHandle& RootInstanceHandle,
	bool bWantsRestoreState
) const
{
	UMosaikkSection* MosaikkSection = InMosaikkData.Section;
	if (!ensureMsgf(MosaikkSection, TEXT("No valid MosaikkSection found in MosaikkComponentData!")))
	{
		return;
	}

	// Reuse existing widget mapping for this section; if it's stale, clean it up.
	const FObjectKey SectionKey = FObjectKey(MosaikkSection);
	
	if (!MosaikkEntitySystem->GetMosaikkComponentEvalData(SectionKey))
	{
		// Prefer the section's world, but fall back to the editor world for MRQ contexts.
		UWorld* World = IsValid(MosaikkSection->GetWorld()) ? MosaikkSection->GetWorld()
			   : (GEditor ? GEditor->GetEditorWorldContext().World() : nullptr);

		if (!IsValid(World))
		{
			return;
		}

		/**
		 * TODO: often widget creation can cause decent memory pressure, 
		 * for example if user is gonna oftenly switch between sections.
		 * Bunch of widget copies are gonna hang in memory until next GC tick, ideally we should create single widget instance for 
		 * each associated section early(for example during startup) and reuse them.
		 */
		UUserWidget* NewWidget = CreateWidget<UUserWidget>(World, MosaikkSection->AssociatedWidgetClass);
		MosaikkEntitySystem->AddSectionWidget(SectionKey, NewWidget);

		MosaikkEntitySystem->PreAnimatedStorage->BeginTrackingEntity(
			EntityID, 
			bWantsRestoreState, 
			RootInstanceHandle, 
			MosaikkSection
		);

		using FCachePreAnimatedValueParams = UE::MovieScene::FCachePreAnimatedValueParams;
		MosaikkEntitySystem->PreAnimatedStorage->CachePreAnimatedValue(
			FCachePreAnimatedValueParams(), 
			MosaikkSection
		);
	}
}
/** End ~FEvaluateMosaikk implementation */

UMovieSceneMosaikkEntitySystem::UMovieSceneMosaikkEntitySystem(const FObjectInitializer& ObjInit) : UMovieSceneEntitySystem(ObjInit)
{
	RelevantComponent = FMosaikkMovieSceneTracksComponentTypes::Get().Mosaikk;
	Phase = UE::MovieScene::ESystemPhase::Scheduling;
}

void UMovieSceneMosaikkEntitySystem::OnLink()
{
	PreAnimatedStorage = Linker->PreAnimatedState.GetOrCreateStorage<FPreAnimatedWidgetStorage>();
}

void UMovieSceneMosaikkEntitySystem::OnUnlink()
{
	UMosaikkHostCanvasManager::Get().ClearHostCanvas();

	for (const auto& Entry : SectionEvalDataMap)
	{
		Entry.Value.WidgetPtr->MarkAsGarbage();
	}

	SectionEvalDataMap.Reset();
}

void UMovieSceneMosaikkEntitySystem::OnSchedulePersistentTasks(UE::MovieScene::IEntitySystemScheduler* TaskScheduler)
{
	using namespace UE::MovieScene;

	const FBuiltInComponentTypes* BuiltInComponents = FBuiltInComponentTypes::Get();
	const FMosaikkMovieSceneTracksComponentTypes& MosaikkComponents = FMosaikkMovieSceneTracksComponentTypes::Get();

	FEntityTaskBuilder()
	.ReadEntityIDs()
	.Read(BuiltInComponents->RootInstanceHandle)
	.Read(MosaikkComponents.Mosaikk)
	.SetDesiredThread(Linker->EntityManager.GetGatherThread())
	.Schedule_PerAllocation<FEvaluateMosaikk>(&Linker->EntityManager, TaskScheduler, this);
}

void UMovieSceneMosaikkEntitySystem::AddReferencedObjects(UObject* InThis, FReferenceCollector& Collector)
{
	Super::AddReferencedObjects(InThis, Collector);
#if WITH_EDITOR
	UMovieSceneMosaikkEntitySystem* This = CastChecked<UMovieSceneMosaikkEntitySystem>(InThis);
	for (auto& Entry : This->SectionEvalDataMap)
	{
		Collector.AddReferencedObject(Entry.Value.WidgetPtr);
	}
#endif
}

FMosaikkComponentEvaluationData* UMovieSceneMosaikkEntitySystem::GetMosaikkComponentEvalData(const FObjectKey& InKey)
{
	return SectionEvalDataMap.Find(InKey);
}

void UMovieSceneMosaikkEntitySystem::AddSectionWidget(const FObjectKey& InKey, UUserWidget* InWidget)
{
	SectionEvalDataMap.Add(InKey, FMosaikkComponentEvaluationData(InWidget));

	UMosaikkHostCanvasManager::Get().PushWidgetToHostCanvas(InWidget);
}

void UMovieSceneMosaikkEntitySystem::RemoveSectionWidget(const FObjectKey& InKey)
{
	auto CompEvalData = GetMosaikkComponentEvalData(InKey);
	if (CompEvalData == nullptr)
	{
		return;
	}

	UUserWidget* WidgetToRemove = CompEvalData->WidgetPtr.Get();
	UMosaikkHostCanvasManager::Get().RemoveWidgetFromHostCanvas(WidgetToRemove);
	RemoveSectionWidgetEntry(InKey);

	WidgetToRemove->MarkAsGarbage();
}

void UMovieSceneMosaikkEntitySystem::RemoveSectionWidgetEntry(const FObjectKey& InKey)
{
	SectionEvalDataMap.Remove(InKey);
}
