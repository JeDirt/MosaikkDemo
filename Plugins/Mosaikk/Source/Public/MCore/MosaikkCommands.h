// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Framework/Commands/Commands.h"

#include "MosaikkStyle.h"

class FMosaikkCommands : public TCommands<FMosaikkCommands>
{
public:

	FMosaikkCommands()
		: TCommands<FMosaikkCommands>(TEXT("MosaikkTab"), NSLOCTEXT("Contexts", "MosaikkTab", "Mosaikk Command IDK"), NAME_None, FMosaikkStyle::GetStyleSetName())
	{
	}

	// TCommands<> interface
	virtual void RegisterCommands() override;

public:
	TSharedPtr<FUICommandInfo> OpenMosaikkHUBWindow;
};