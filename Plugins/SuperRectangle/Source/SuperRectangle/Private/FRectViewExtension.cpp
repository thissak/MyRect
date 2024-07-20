// Fill out your copyright notice in the Description page of Project Settings.

#include "FRectViewExtension.h"
#include "RectangleShader.h"
#include "RenderGraphUtils.h"
#include "SceneRendering.h"
#include "PostProcess/PostProcessing.h"
#include "PostProcess/PostProcessMaterialInputs.h"

DECLARE_GPU_DRAWCALL_STAT(SuperRectanglePass);

FRectViewExtension::FRectViewExtension(const FAutoRegister& AutoRegister) : FSceneViewExtensionBase(AutoRegister){}

void FRectViewExtension::PostRenderBasePassDeferred_RenderThread(
	FRDGBuilder& GraphBuilder,
	FSceneView& View,
	const FRenderTargetBindingSlots& RenderTargets,
	TRDGUniformBufferRef<FSceneTextureUniformParameters> SceneTextures)
{
	/*FLinearColor MyColor = FLinearColor(0.0f, 1.0f, 0.0f); // green
	const FIntRect Viewport = static_cast<const FViewInfo&>(View).ViewRect;
	const FGlobalShaderMap* ViewShaderMap = static_cast<const FViewInfo&>(View).ShaderMap;

	// SceneTextures에서 SceneColor 텍스처를 가져옴
	auto SceneTextureParams = SceneTextures->GetContents();
	FRDGTextureRef SceneColorTexture = SceneTextureParams->SceneDepthTexture;
	
	// FScreenPassTexture로 변환
	FScreenPassTexture SceneColor;
	SceneColor.Texture = SceneColorTexture;
	SceneColor.ViewRect = Viewport;

	RDG_EVENT_SCOPE(GraphBuilder, "__PostRenderBasePassDeferred_RenderThread__");


	RenderRectangle(GraphBuilder, ViewShaderMap, Viewport, SceneColor, MyColor);*/
}

void FRectViewExtension::PrePostProcessPass_RenderThread(FRDGBuilder& GraphBuilder, const FSceneView& View,
                                                         const FPostProcessingInputs& Inputs)
{
	checkSlow(View.bIsViewInfo); // can't do dynamic_cast because FViewInfo doesn't have any virtual functions.
	
	const FIntRect Viewport = static_cast<const FViewInfo&>(View).ViewRect;
	FScreenPassTexture SceneColor((*Inputs.SceneTextures)->SceneColorTexture, Viewport);
	
	RDG_EVENT_SCOPE(GraphBuilder, "__PrePostProcessPass__SuperRectangleBlue");
	const FGlobalShaderMap* ViewShaderMap = static_cast<const FViewInfo&>(View).ShaderMap;
	
	FLinearColor MyColor = FLinearColor(0.0f, 0.0f, 1.0f);
	RenderRectangle(GraphBuilder, ViewShaderMap, Viewport, SceneColor, MyColor);
}

void FRectViewExtension::SubscribeToPostProcessingPass(EPostProcessingPass PassId,
	FAfterPassCallbackDelegateArray& InOutPassCallbacks, bool bIsPassEnabled)
{
	/*if (PassId == EPostProcessingPass::MotionBlur)
	{
		InOutPassCallbacks.Add(FAfterPassCallbackDelegate::CreateRaw(this,
			&FRectViewExtension::SuperRectanglePassAfterMotionBlur_RenderThread));
	}*/
	/*if (PassId == EPostProcessingPass::Tonemap)
	{
		InOutPassCallbacks.Add(FAfterPassCallbackDelegate::CreateRaw(this,
			&FRectViewExtension::SuperRectanglePassAfterTonemap_RenderThread));
	}*/
}


FScreenPassTexture FRectViewExtension::SuperRectanglePassAfterTonemap_RenderThread(
	FRDGBuilder& GraphBuilder,
	const FSceneView& View,
	const FPostProcessMaterialInputs& Inputs)
{
	const FScreenPassTexture SceneColor = Inputs.GetInput(EPostProcessMaterialInput::SeparateTranslucency);

	RDG_GPU_STAT_SCOPE(GraphBuilder, SuperRectanglePass);
	RDG_EVENT_SCOPE(GraphBuilder, "__AfterTonemap__SuperRectangle");

	const FIntRect ViewInfo = static_cast<const FViewInfo&>(View).ViewRect;
	const FGlobalShaderMap* ViewShaderMap = static_cast<const FViewInfo&>(View).ShaderMap;
	FLinearColor MyColor = FLinearColor(1.0f, 0.0f, 0.0f);

	RenderRectangle(GraphBuilder, ViewShaderMap, ViewInfo, SceneColor, MyColor);

	return SceneColor;
}

FScreenPassTexture FRectViewExtension::SuperRectanglePassAfterMotionBlur_RenderThread(
	FRDGBuilder& GraphBuilder,
	const FSceneView& View,
	const FPostProcessMaterialInputs& Inputs)
{
	const FScreenPassTexture SceneColor = Inputs.GetInput(EPostProcessMaterialInput::SeparateTranslucency);

	RDG_GPU_STAT_SCOPE(GraphBuilder, SuperRectanglePass);
	RDG_EVENT_SCOPE(GraphBuilder, "__AfterMotionBlur__SuperRectangle");

	const FIntRect ViewInfo = static_cast<const FViewInfo&>(View).ViewRect;
	const FGlobalShaderMap* ViewShaderMap = static_cast<const FViewInfo&>(View).ShaderMap;
	FLinearColor MyColor = FLinearColor(1.0f, 0.0f, 0.0f);

	RenderRectangle(GraphBuilder, ViewShaderMap, ViewInfo, SceneColor, MyColor);

	return SceneColor;

}

FScreenPassTexture FRectViewExtension::ReturnUntouchedSceneColorForPostProcessing(
	const FPostProcessMaterialInputs& InOutInputs)
{
	if (InOutInputs.OverrideOutput.IsValid())
	{
		return InOutInputs.OverrideOutput;
	}
	else
	{
		/** We don't want to modify scene texture in any way. We just want it to be passed back onto the next stage. */
		FScreenPassTexture SceneTexture = const_cast<FScreenPassTexture&>(InOutInputs.Textures[(uint32)EPostProcessMaterialInput::SceneColor]);
		return SceneTexture;
	}
}

void FRectViewExtension::RenderRectangle(
	FRDGBuilder& GraphBuilder,
	const FGlobalShaderMap* ViewShaderMap,
	const FIntRect& ViewInfo,
	const FScreenPassTexture& SceneColor,
	const FLinearColor MyColor
	)
{
	FRectShaderPSParams* PSParams = GraphBuilder.AllocParameters<FRectShaderPSParams>();
	PSParams->RenderTargets[0] = FRenderTargetBinding(SceneColor.Texture, ERenderTargetLoadAction::ENoAction);
	PSParams->Color = MyColor;

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

void FRectViewExtension::DrawFullscreenRectangle(FRHICommandList& RHICmdList, uint32 InstanceCount)
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





































