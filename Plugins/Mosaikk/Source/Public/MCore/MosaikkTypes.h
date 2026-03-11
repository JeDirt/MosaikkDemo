#pragma once

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
};

