#pragma once

#include "MWidget/MosaikkHostWidget.h"

#include "MosaikkTypes.generated.h"

UCLASS(Config=EditorPerProjectUserSettings, DefaultConfig, meta = (DisplayName = "Mosaikk Settings"))
class UMosaikkSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	/**
	 * Default range of the section on created Mosaikk track.
	 * X - default first frame.
	 * Y - default last frame.
	 */
	UPROPERTY(Config, EditAnywhere, Category=Appearence)
	FIntPoint DefaultSectionRange = { 10, 30 };

	UPROPERTY(Config, EditAnywhere, Category=Appearence)
	FColor DefaultTrackTint = FColor(192,0,61,75);

	/**
	 * Host widget that is created via AMosaikkProxyActor during movie capture.
	 * This widget is a container for all widgets that are going to be rendered during movie capture.
	 *
	 * I leave it exposed as a setting for now, but you probably won't need to change it, except you want some unique behavior.
	 * Check **UMosaikkHostWidget** and **AMosaikkProxyActor** classes to see how it works.
	 */
	UPROPERTY(Config, EditAnywhere, Category=Setup)
	TSubclassOf<UMosaikkHostWidget> MosaikkHostWidgetClass = UMosaikkHostWidget::StaticClass();
};

