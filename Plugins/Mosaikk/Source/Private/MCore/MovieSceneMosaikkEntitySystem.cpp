// Fill out your copyright notice in the Description page of Project Settings.

#include "MCore/MovieSceneMosaikkEntitySystem.h"

#include "EntitySystem/BuiltInComponentTypes.h"
#include "Evaluation/PreAnimatedState/MovieScenePreAnimatedObjectStorage.h"
#include "Evaluation/PreAnimatedState/MovieScenePreAnimatedStorageID.inl"

#include "MCore/MosaikkMovieSceneComponentTypes.h"
#include "MSequencer/MosaikkSection.h"
#include "Mosaikk.h"

struct FCachedPreAnimatedMosaikk
{
	FCachedPreAnimatedMosaikk(UMosaikkSection* InSection = nullptr) 
	: CachedSection(InSection) { }

	TWeakObjectPtr<UMosaikkSection> CachedSection;
};

template<typename BaseTraits>
struct FPreAnimatedWidgetStateTraits : BaseTraits
{
	using KeyType = FObjectKey;
	using StorageType = FCachedPreAnimatedMosaikk;

	FCachedPreAnimatedMosaikk CachePreAnimatedValue(FObjectKey InKey)
	{
		/** TODO: 
		 * Seems like a bad design to store a Section as cached PreAnimated value.
		 * For draft - ok, just wanted to make sure it works at least, but needs to be reimplemented for sure.
		 * 
		 * UMosaikkSection* is cached -> 
		 * UMosaikkSection* ends(entity ends evaluation) -> 
		 * Cached UMosaikkSection* removed from UMovieSceneMosaikkEntitySystem::SectionToMosaikkComponentEvalDataMap -> 
		 * Widget associated with this UMosaikkSection* removed from the screen
		 */
		UMosaikkSection* ReceivedSection = Cast<UMosaikkSection>(InKey.ResolveObjectPtr());
		if (!IsValid(ReceivedSection))
		{
			return {};
		}

		return { ReceivedSection };
	}

	void RestorePreAnimatedValue(FObjectKey InKey, FCachedPreAnimatedMosaikk PreAnimatedMosaikk, const UE::MovieScene::FRestoreStateParams& Params)
	{
		const FObjectKey SearchKey = FObjectKey(PreAnimatedMosaikk.CachedSection.Get());
		const auto Result = UMovieSceneMosaikkEntitySystem::SectionToMosaikkComponentEvalDataMap.Find(SearchKey);
		if (Result == nullptr)
		{
			return;
		}

		UMovieSceneMosaikkEntitySystem::RemoveWidgetFromSlot(Result->Widget.Get());
		UMovieSceneMosaikkEntitySystem::SectionToMosaikkComponentEvalDataMap.Remove(SearchKey);
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

	if (!MosaikkEntitySystem->GetMosaikkComponentEvalData(FObjectKey(MosaikkSection)))
	{
		UUserWidget* NewWidget = CreateWidget<UUserWidget>(
			GEditor->GetEditorWorldContext().World(), 
			MosaikkSection->AssociatedWidgetClass
		);
		MosaikkEntitySystem->AddNewWidget(FObjectKey(MosaikkSection), NewWidget);

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

TMap<FObjectKey, FMosaikkComponentEvaluationData> UMovieSceneMosaikkEntitySystem::SectionToMosaikkComponentEvalDataMap;

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
	HideAllWidgets();
	SectionToMosaikkComponentEvalDataMap.Reset();
}

void UMovieSceneMosaikkEntitySystem::OnSchedulePersistentTasks(UE::MovieScene::IEntitySystemScheduler* TaskScheduler)
{
	using namespace UE::MovieScene;

	const FBuiltInComponentTypes* BuiltInComponents = FBuiltInComponentTypes::Get();
	const FMosaikkMovieSceneTracksComponentTypes& MosaikkComponents = FMosaikkMovieSceneTracksComponentTypes::Get();

	FTaskID EvaluateMosaikkTask = FEntityTaskBuilder()
	.ReadEntityIDs()
	.Read(BuiltInComponents->RootInstanceHandle)
	.Read(MosaikkComponents.Mosaikk)
	.SetDesiredThread(Linker->EntityManager.GetGatherThread())
	.Schedule_PerAllocation<FEvaluateMosaikk>(&Linker->EntityManager, TaskScheduler, this);
}

FMosaikkComponentEvaluationData* UMovieSceneMosaikkEntitySystem::GetMosaikkComponentEvalData(const FObjectKey& InKey)
{
	return SectionToMosaikkComponentEvalDataMap.Find(InKey);
}

void UMovieSceneMosaikkEntitySystem::AddNewWidget(const FObjectKey& InKey, UUserWidget* InWidget)
{
	FMosaikkComponentEvaluationData Data;
	Data.Widget = InWidget;
	SectionToMosaikkComponentEvalDataMap.Add(InKey, Data);

	ShowWidget(InWidget);
}

void UMovieSceneMosaikkEntitySystem::ShowWidget(UUserWidget* Widget)
{
	const TSharedPtr<SOverlay> MosaikkViewportOverlay = FMosaikkModule::Get().GetMosaikkViewportOverlay();
	if (!IsValid(Widget) || !MosaikkViewportOverlay.IsValid())
	{
		return;
	}

	const TSharedPtr<SWidget> SlateWidget = Widget->TakeWidget();
	MosaikkViewportOverlay->AddSlot()[SlateWidget.ToSharedRef()];
}

void UMovieSceneMosaikkEntitySystem::RemoveWidgetFromSlot(UUserWidget* Widget)
{
	const TSharedPtr<SOverlay> MosaikkViewportOverlay = FMosaikkModule::Get().GetMosaikkViewportOverlay();
	if (!IsValid(Widget) || !MosaikkViewportOverlay.IsValid())
	{
		return;
	}

	MosaikkViewportOverlay->RemoveSlot(Widget->TakeWidget());
}

void UMovieSceneMosaikkEntitySystem::HideAllWidgets()
{
	const TSharedPtr<SOverlay> MosaikkViewportOverlay = FMosaikkModule::Get().GetMosaikkViewportOverlay();
	if (!MosaikkViewportOverlay.IsValid())
	{
		return;
	}

	MosaikkViewportOverlay->ClearChildren();
}
