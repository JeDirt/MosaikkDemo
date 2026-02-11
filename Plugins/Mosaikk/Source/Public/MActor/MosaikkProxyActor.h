// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"

#include "MosaikkProxyActor.generated.h"

class UMosaikkHostWidget;

UCLASS()
class MOSAIKK_API AMosaikkProxyActor : public AActor
{
	GENERATED_BODY()

public:
	AMosaikkProxyActor();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	UPROPERTY()
	TObjectPtr<UMosaikkHostWidget> MosaikkHostWidget = nullptr;
};
