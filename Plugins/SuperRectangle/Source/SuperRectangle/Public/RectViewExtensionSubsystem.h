// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/EngineSubsystem.h"
#include "FRectViewExtension.h"
#include "RectViewExtensionSubsystem.generated.h"

/**
 * 
 */
UCLASS()
class SUPERRECTANGLE_API URectViewExtensionSubsystem : public UEngineSubsystem
{
	GENERATED_BODY()

public:
	void virtual Initialize(FSubsystemCollectionBase& Collection) override;

protected:
	TSharedPtr<FRectViewExtension, ESPMode::ThreadSafe> RectViewExtension;
	
};
