// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonRenderResources.h"
#include "RectangleShader.h"
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

protected:
	template<typename TShaderClass>
	static void AddFullscreenPass(
		FRDGBuilder& GraphBuilder,
		const FGlobalShaderMap* GlobalShaderMap,
		FRDGEventName&& PassName,
		const TShaderRef<TShaderClass>& PixelShader,
		typename TShaderClass::FParameters* Parameters,
		const FIntRect& Viewport,
		FRHIBlendState* BlendState = nullptr,
		FRHIRasterizerState* RasterizerState = nullptr,
		FRHIDepthStencilState* DepthStencilState = nullptr,
		uint32 StencilRef = 0);

	template<typename TShaderClass>
	static void DrawFullscreenPixelShader(
		FRHICommandList& CmdList,
		const FGlobalShaderMap* GlobalShaderMap,
		const TShaderRef<TShaderClass>& PixelShader,
		const typename TShaderClass::FParameters& Parameters,
		const FIntRect& Viewport,
		FRHIBlendState* BlendState = nullptr,
		FRHIRasterizerState* RasterizerState = nullptr,
		FRHIDepthStencilState* DepthStencilState = nullptr,
		uint32 StencilRef = 0);

	static inline void DrawFullscreenRectangle(FRHICommandList& RHICmdList, uint32 InstanceCount)
	{
		RHICmdList.SetStreamSource(0, GRectShaderVertexBuffer.VertexBufferRHI, 0);
		RHICmdList.DrawIndexedPrimitive(
			GRectShaderIndexBuffer.IndexBufferRHI,
			0,
			0,
			4,
			0,
			2,
			InstanceCount);
	}
	
	FScreenPassTexture SuperRectanglePass_RenderThread(FRDGBuilder& GraphBuilder, const FSceneView& view,
		const FPostProcessMaterialInputs& Inputs);

public:
	static void RenderRectangle(
	FRDGBuilder& GraphBuilder,
	const FGlobalShaderMap* ViewShaderMap,
	const FIntRect& View,
	const FScreenPassTexture& SceneColor);

	
};

