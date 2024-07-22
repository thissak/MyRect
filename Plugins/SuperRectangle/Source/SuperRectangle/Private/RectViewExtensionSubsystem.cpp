// Fill out your copyright notice in the Description page of Project Settings.


#include "RectViewExtensionSubsystem.h"

void URectViewExtensionSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	UE_LOG(LogTemp, Warning, TEXT("FRectViewExtension Initialize"))
	this->RectViewExtension = FSceneViewExtensions::NewExtension<FRectViewExtension>();
}

void URectViewExtensionSubsystem::SetParticleAlpha(float NewAlpha)
{
	if (RectViewExtension.IsValid())
	{
		RectViewExtension->SetParticleAlpha(NewAlpha);
	}
}


