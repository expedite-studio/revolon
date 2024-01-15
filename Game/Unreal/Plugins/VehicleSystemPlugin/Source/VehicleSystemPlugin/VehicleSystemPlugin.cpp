// Copyright 2019-2022 Seven47 Software. All Rights Reserved.
// Unauthorized copying of this file, via any medium is strictly prohibited

#include "VehicleSystemPlugin.h"

#define LOCTEXT_NAMESPACE "FVehicleSystemPluginModule"

void FVehicleSystemPluginModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	
}

void FVehicleSystemPluginModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
	
}

#undef LOCTEXT_NAMESPACE
	
//#if WITH_EDITOR
//IMPLEMENT_PRIMARY_GAME_MODULE(FDefaultGameModuleImpl, VehicleSystemPlugin, "VehicleSystemPlugin")
//#else
IMPLEMENT_MODULE(FVehicleSystemPluginModule, VehicleSystemPlugin)
//#endif