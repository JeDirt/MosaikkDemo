// Fill out your copyright notice in the Description page of Project Settings.

#include "MActor/MosaikkProxyActor.h"

#include "Blueprint/UserWidget.h"

#include "MCore/MosaikkTypes.h"

AMosaikkProxyActor::AMosaikkProxyActor()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AMosaikkProxyActor::BeginPlay()
{
	Super::BeginPlay();

	if (IsValid(MosaikkHostWidget))
	{
		MosaikkHostWidget->RemoveFromParent();
		MosaikkHostWidget = nullptr;
	}

	const UMosaikkSettings* MosaikkSettingsCDO = GetDefault<UMosaikkSettings>();
	if (!MosaikkSettingsCDO->MosaikkHostWidgetClass)
	{
		return;
	}

	MosaikkHostWidget = CreateWidget<UMosaikkHostWidget>(GetWorld(), MosaikkSettingsCDO->MosaikkHostWidgetClass);
	if (IsValid(MosaikkHostWidget))
	{
		MosaikkHostWidget->AddToViewport();
	}
}

void AMosaikkProxyActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	if (IsValid(MosaikkHostWidget))
	{
		MosaikkHostWidget->RemoveFromParent();
		MosaikkHostWidget = nullptr;
	}
}

