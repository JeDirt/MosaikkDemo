// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Blueprint/UserWidget.h"

#include "MosaikkHostWidget.generated.h"

class UCanvasPanel;

UCLASS()
class MOSAIKK_API UMosaikkHostWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeOnInitialized() override;
	virtual void NativeDestruct() override;

protected:
	UPROPERTY()
	TObjectPtr<UCanvasPanel> RootCanvas;
};
