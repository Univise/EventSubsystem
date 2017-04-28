/* Copyright (2017) Dominik Peacock. All Rights Reserved. */

#pragma once 

#include "ModuleManager.h"

class FEventSubsystemModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};