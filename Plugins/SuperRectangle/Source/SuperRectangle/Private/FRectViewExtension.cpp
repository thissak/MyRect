// Fill out your copyright notice in the Description page of Project Settings.

#include "FRectViewExtension.h"

DECLARE_GPU_DRAWCALL_STAT(RectanglePass);

FRectViewExtension::FRectViewExtension(const FAutoRegister& AutoRegister) : FSceneViewExtensionBase(AutoRegister){}

void FRectViewExtension::SubscribeToPostProcessingPass(EPostProcessingPass Pass,
	FAfterPassCallbackDelegateArray& InOutPassCallbacks, bool bIsPassEnabled)
{
	if (Pass == EPostProcessingPass::Tonemap)
	{
		
	}
}
