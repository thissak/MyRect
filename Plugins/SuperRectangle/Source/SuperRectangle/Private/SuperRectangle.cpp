// Copyright Epic Games, Inc. All Rights Reserved.

#include "SuperRectangle.h"

#include "Interfaces/IPluginManager.h"

#define LOCTEXT_NAMESPACE "FSuperRectangleModule"

void FSuperRectangleModule::StartupModule()
{
	FString BaseDir = IPluginManager::Get().FindPlugin(TEXT("SuperRectangle"))->GetBaseDir();
	FString PluginShaderDir = FPaths::Combine(BaseDir + TEXT("/Shaders"));
	AddShaderSourceDirectoryMapping(TEXT("/SuperShader"), PluginShaderDir);
}

void FSuperRectangleModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FSuperRectangleModule, SuperRectangle)