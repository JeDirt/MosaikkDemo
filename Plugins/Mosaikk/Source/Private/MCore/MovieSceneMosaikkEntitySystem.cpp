// Fill out your copyright notice in the Description page of Project Settings.

#include "MCore/MovieSceneMosaikkEntitySystem.h"

#include "EntitySystem/BuiltInComponentTypes.h"

#include "MCore/MosaikkMovieSceneComponentTypes.h"
#include "MSequencer/MosaikkSection.h"
#include "Mosaikk.h"

struct FEvaluateMosaikk
{
	using FInstanceHandle		 = UE::MovieScene::FInstanceHandle;
	using FMovieSceneEntityID	 = UE::MovieScene::FMovieSceneEntityID;
	using FInstanceRegistry		 = UE::MovieScene::FInstanceRegistry;
	using FRootInstanceHandle	 = UE::MovieScene::FRootInstanceHandle;
	using FEntityAllocation		 = UE::MovieScene::FEntityAllocation;
	using FSequenceInstance		 = UE::MovieScene::FSequenceInstance;
	using FBuiltInComponentTypes = UE::MovieScene::FBuiltInComponentTypes;

public:
	FEvaluateMosaikk(UMovieSceneMosaikkEntitySystem* InMosaikkEntitySystem) : MosaikkEntitySystem(InMosaikkEntitySystem)
	{
		InstanceRegistry = MosaikkEntitySystem->GetLinker()->GetInstanceRegistry();
	}

	void ForEachAllocation(
		const FEntityAllocation* Allocation,
		UE::MovieScene::TRead<FMovieSceneEntityID> EntityIDs,
		UE::MovieScene::TRead<FRootInstanceHandle> RootInstanceHandles,
		UE::MovieScene::TRead<FInstanceHandle> InstanceHandles,
		UE::MovieScene::TRead<FMovieSceneMosaikkComponentData> MosaikkComponentDatas
	) const
	{
		const int32 Num = Allocation->Num();
		for (int32 Index = 0; Index < Num; ++Index)
		{
			const FMovieSceneEntityID& EntityID = EntityIDs[Index];
			const FRootInstanceHandle& RootInstanceHandle = RootInstanceHandles[Index];
			const FInstanceHandle& InstanceHandle = InstanceHandles[Index];
			const FMovieSceneMosaikkComponentData& MosaikkData = MosaikkComponentDatas[Index];

			const FSequenceInstance& Instance = InstanceRegistry->GetInstance(InstanceHandle);

			Evaluate(EntityID, MosaikkData, Instance, RootInstanceHandle);
		}
	}

private:
	void Evaluate(
		const FMovieSceneEntityID& EntityID,
		const FMovieSceneMosaikkComponentData& InMosaikkData,
		const FSequenceInstance& Instance,
		const FRootInstanceHandle& RootInstanceHandle
	) const
	{
		UMosaikkSection* MosaikkSection = InMosaikkData.Section;
		if (!ensureMsgf(MosaikkSection, TEXT("No valid MosaikkSection found in MosaikkComponentData!")))
		{
			return;
		}

		if (MosaikkEntitySystem->WidgetMap.Find(EntityID) == nullptr)
		{
			// TODO: THIS WORKS FOR THE CASE WHEN 2 SECTIONS ARE SEQUENTIALLY ALIGNED IN SEQUENCER
			// BUT IF 2 SECTIONS ARE ON THE SAME TIME AND ARE THE SAME LENGTH - ONLY 1 WIDGET IS SHOWN
			MosaikkEntitySystem->HideAllWidgets();
			UUserWidget* NewWidget = CreateWidget<UUserWidget>(
				GEditor->GetEditorWorldContext().World(), MosaikkSection->AssociatedWidgetClass);
			MosaikkEntitySystem->WidgetMap.Add(EntityID, NewWidget);
			MosaikkEntitySystem->ShowWidget(NewWidget);
		}
	}
	
public:
	UMovieSceneMosaikkEntitySystem* MosaikkEntitySystem;
	const FInstanceRegistry* InstanceRegistry;
};

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

	HideAllWidgets();
	WidgetMap.Reset();
}

void UMovieSceneMosaikkEntitySystem::OnSchedulePersistentTasks(UE::MovieScene::IEntitySystemScheduler* TaskScheduler)
{
	UE_LOGFMT(LogTemp, Warning, "{0} has been called!", __FUNCTION__);
}

void UMovieSceneMosaikkEntitySystem::OnRun(FSystemTaskPrerequisites& InPrerequisites, FSystemSubsequentTasks& Subsequents)
{
	UE_LOGFMT(LogTemp, Warning, "{0} has been called!", __FUNCTION__);

	using namespace UE::MovieScene;

	const FBuiltInComponentTypes* BuiltInComponents = FBuiltInComponentTypes::Get();
	const FMosaikkMovieSceneTracksComponentTypes& MosaikkComponents = FMosaikkMovieSceneTracksComponentTypes::Get();

	FEntityTaskBuilder()
	.ReadEntityIDs()
	.Read(BuiltInComponents->RootInstanceHandle)
	.Read(BuiltInComponents->InstanceHandle)
	.Read(MosaikkComponents.Mosaikk)
	.SetDesiredThread(Linker->EntityManager.GetGatherThread())
	.template Dispatch_PerAllocation<FEvaluateMosaikk>(&Linker->EntityManager, InPrerequisites, &Subsequents, this);
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

void UMovieSceneMosaikkEntitySystem::HideAllWidgets()
{
	const TSharedPtr<SOverlay> MosaikkViewportOverlay = FMosaikkModule::Get().GetMosaikkViewportOverlay();
	if (!MosaikkViewportOverlay.IsValid())
	{
		return;
	}

	MosaikkViewportOverlay->ClearChildren();
}
