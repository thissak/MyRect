// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SceneViewExtension.h"

class SUPERRECTANGLE_API FRectViewExtension : public FSceneViewExtensionBase
{
public:
	FRectViewExtension(const FAutoRegister& AutoRegister);

	virtual void SetupViewFamily(FSceneViewFamily& InViewFamily) override {};
	virtual void SetupView(FSceneViewFamily& InViewFamily, FSceneView& InView) override {};
	virtual void BeginRenderViewFamily(FSceneViewFamily& InViewFamily) override {};
	virtual void PreRenderViewFamily_RenderThread(FRDGBuilder& GraphBuilder, FSceneViewFamily& InViewFamily) override {};
	virtual void PreRenderView_RenderThread(FRDGBuilder& GraphBuilder, FSceneView& InView) override {};
	virtual void PostRenderBasePassDeferred_RenderThread(FRDGBuilder& GraphBuilder, FSceneView& InView,
		const FRenderTargetBindingSlots& RenderTargets, TRDGUniformBufferRef<FSceneTextureUniformParameters> SceneTextures) override {};
	virtual void PrePostProcessPass_RenderThread(FRDGBuilder& GraphBuilder, const FSceneView& View, const FPostProcessingInputs& Inputs) override {};

	virtual void SubscribeToPostProcessingPass(EPostProcessingPass Pass, FAfterPassCallbackDelegateArray& InOutPassCallbacks, bool bIsPassEnabled) override;

};
