// Fill out your copyright notice in the Description page of Project Settings.

#include "FRectViewExtension.h"
#include "RectangleShader.h"
#include "RenderGraphUtils.h"
#include "SceneRendering.h"
#include "PostProcess/PostProcessMaterialInputs.h"

DECLARE_GPU_DRAWCALL_STAT(SuperRectanglePass);

FRectViewExtension::FRectViewExtension(const FAutoRegister& AutoRegister) : FSceneViewExtensionBase(AutoRegister){}

void FRectViewExtension::SubscribeToPostProcessingPass(EPostProcessingPass Pass,
	FAfterPassCallbackDelegateArray& InOutPassCallbacks, bool bIsPassEnabled)
{
	if (Pass == EPostProcessingPass::Tonemap)
	{
		InOutPassCallbacks.Add(FAfterPassCallbackDelegate::CreateRaw(this, &FRectViewExtension::SuperRectanglePass_RenderThread));
	}
}

FScreenPassTexture FRectViewExtension::SuperRectanglePass_RenderThread(
	FRDGBuilder& GraphBuilder,
	const FSceneView& View,
	const FPostProcessMaterialInputs& Inputs)
{
	const FScreenPassTexture SceneColor = Inputs.GetInput(EPostProcessMaterialInput::SceneColor);

	RDG_GPU_STAT_SCOPE(GraphBuilder, SuperRectanglePass);
	RDG_EVENT_SCOPE(GraphBuilder, "SuperRectanglePass");

	const FIntRect ViewInfo = static_cast<const FViewInfo&>(View).ViewRect;
	const FGlobalShaderMap* ViewShaderMap = static_cast<const FViewInfo&>(View).ShaderMap;

	RenderRectangle(GraphBuilder, ViewShaderMap, ViewInfo, SceneColor);

	return SceneColor;
}

void FRectViewExtension::RenderRectangle(
	FRDGBuilder& GraphBuilder,
	const FGlobalShaderMap* ViewShaderMap,
	const FIntRect& ViewInfo,
	const FScreenPassTexture& SceneColor)
{
	FRectShaderPSParams* PSParams = GraphBuilder.AllocParameters<FRectShaderPSParams>();
	PSParams->RenderTargets[0] = FRenderTargetBinding(SceneColor.Texture, ERenderTargetLoadAction::ENoAction);

	TShaderMapRef<FRectShaderPS> PixelShader(ViewShaderMap);

	AddFullscreenPass<FRectShaderPS>(
		GraphBuilder,
		ViewShaderMap,
		RDG_EVENT_NAME("SuperRectanglePass"),
		PixelShader,
		PSParams,
		ViewInfo);
}

template <typename TShaderClass>
void FRectViewExtension::AddFullscreenPass(
	FRDGBuilder& GraphBuilder,
	const FGlobalShaderMap* GlobalShaderMap,
	FRDGEventName&& PassName,
	const TShaderRef<TShaderClass>& PixelShader,
	typename TShaderClass::FParameters* Parameters,
	const FIntRect& Viewport,
	FRHIBlendState* BlendState,
	FRHIRasterizerState* RasterizerState,
	FRHIDepthStencilState* DepthStencilState,
	uint32 StencilRef)
{
	check(PixelShader.IsValid());
	ClearUnusedGraphResources(PixelShader, Parameters);

	GraphBuilder.AddPass(
		Forward<FRDGEventName>(PassName),
		Parameters,
		ERDGPassFlags::Raster,
		[Parameters, GlobalShaderMap, PixelShader, Viewport, BlendState,
			RasterizerState, DepthStencilState, StencilRef]
			(FRHICommandList& RHICmdList)
		{
			FRectViewExtension::DrawFullscreenPixelShader(RHICmdList, GlobalShaderMap, PixelShader,
				*Parameters, Viewport, BlendState, RasterizerState, DepthStencilState, StencilRef);
		});
}

template <typename TShaderClass>
void FRectViewExtension::DrawFullscreenPixelShader(
	FRHICommandList& RHICmdList,
	const FGlobalShaderMap* GlobalShaderMap,
	const TShaderRef<TShaderClass>& PixelShader,
	const typename TShaderClass::FParameters& Parameters,
	const FIntRect& Viewport, FRHIBlendState* BlendState,
	FRHIRasterizerState* RasterizerState,
	FRHIDepthStencilState* DepthStencilState,
	uint32 StencilRef)
{
	check(PixelShader.IsValid());
	RHICmdList.SetViewport((float)Viewport.Min.X, (float)Viewport.Min.Y, 0.0f,
		(float)Viewport.Max.X, (float)Viewport.Max.Y, 1.0f);

	// Begin Setup GPU Pipeline state object
	FGraphicsPipelineStateInitializer GraphicsPSOInit;
	TShaderMapRef<FRectShaderVS> VertexShader(GlobalShaderMap);

	RHICmdList.ApplyCachedRenderTargets(GraphicsPSOInit);
	GraphicsPSOInit.BlendState = TStaticBlendState<>::GetRHI();
	GraphicsPSOInit.RasterizerState = TStaticRasterizerState<>::GetRHI();
	GraphicsPSOInit.DepthStencilState = TStaticDepthStencilState<false, CF_Always>::GetRHI();

	GraphicsPSOInit.BoundShaderState.VertexDeclarationRHI = GRectShaderVertexDeclaration.VertexDeclarationRHI;
	GraphicsPSOInit.BoundShaderState.VertexShaderRHI = VertexShader.GetVertexShader();
	GraphicsPSOInit.BoundShaderState.PixelShaderRHI = PixelShader.GetPixelShader();
	GraphicsPSOInit.PrimitiveType = PT_TriangleList;

	GraphicsPSOInit.BlendState = BlendState ? BlendState : GraphicsPSOInit.BlendState;
	GraphicsPSOInit.RasterizerState = RasterizerState ? RasterizerState : GraphicsPSOInit.RasterizerState;
	GraphicsPSOInit.DepthStencilState = DepthStencilState ? DepthStencilState : GraphicsPSOInit.DepthStencilState;

	// End GPU pipeline state object setup
	SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit, StencilRef);
	SetShaderParameters(RHICmdList, PixelShader, PixelShader.GetPixelShader(),Parameters);
	DrawFullscreenRectangle(RHICmdList, 1);
}






































