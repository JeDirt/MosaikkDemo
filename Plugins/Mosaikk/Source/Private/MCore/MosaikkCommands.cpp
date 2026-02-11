// Copyright Epic Games, Inc. All Rights Reserved.

#include "MCore/MosaikkCommands.h"

#define LOCTEXT_NAMESPACE "FMosaikkModule"

void FMosaikkCommands::RegisterCommands()
{
	UI_COMMAND(OpenMosaikkHUBWindow, "Mosaikk HUB", "Bring up Mosaikk HUB window", EUserInterfaceActionType::Button, FInputChord());
}

#undef LOCTEXT_NAMESPACE
