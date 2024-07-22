// Fill out your copyright notice in the Description page of Project Settings.

#include "FRectViewExtension.h"
#include "RectangleShader.h"
#include "RenderGraphUtils.h"
#include "SceneRendering.h"
#include "PostProcess/PostProcessing.h"
#include "PostProcess/PostProcessMaterialInputs.h"

DECLARE_GPU_DRAWCALL_STAT(SuperRectanglePass);

FRectViewExtension::FRectViewExtension(const FAutoRegister& AutoRegister) : FSceneViewExtensionBase(AutoRegister){}

void FRectViewExtension::PrePostProcessPass_RenderThread(FRDGBuilder& GraphBuilder, const FSceneView& View,
                                                         const FPostProcessingInputs& Inputs)
{
	checkSlow(View.bIsViewInfo); // can't do dynamic_cast because FViewInfo doesn't have any virtual functions.
	
	const FIntRect Viewport = static_cast<const FViewInfo&>(View).ViewRect;
	FScreenPassTexture SceneColor((*Inputs.SceneTextures)->SceneColorTexture, Viewport);
	
	RDG_EVENT_SCOPE(GraphBuilder, "__PrePostProcessPass__SuperRectangleBlue");
	const FGlobalShaderMap* ViewShaderMap = static_cast<const FViewInfo&>(View).ShaderMap;
	
	auto TranslucencyAfterDofTexture = Inputs.TranslucencyViewResourcesMap.Get(ETranslucencyPass::TPT_TranslucencyAfterDOF);
	const FRDGTextureMSAA ParticleTextureMASS = TranslucencyAfterDofTexture.ColorTexture;
	
	FLinearColor MyColor = FLinearColor(1.0f, 0.0f, 1.0f);
	RenderRectangle(GraphBuilder, ViewShaderMap, Inputs, Viewport, SceneColor, MyColor, ParticleTextureMASS);
}



// RenderRectangle PARAMETER setup
void FRectViewExtension::RenderRectangle(
	FRDGBuilder& GraphBuilder,
	const FGlobalShaderMap* ViewShaderMap,
	const FPostProcessingInputs& Inputs,
	const FIntRect& ViewInfo,
	const FScreenPassTexture& SceneColor,
	const FLinearColor MyColor,
	const FRDGTextureMSAA ParticleTexture
	)
{
	// Viewport parameters
	const FScreenPassTextureViewport SceneColorTextureViewport(SceneColor);
	const FScreenPassTextureViewportParameters SceneTextureViewportParams = GetTextureViewportParameters(SceneColorTextureViewport);

	// thissa
	FRDGTextureRef OriginalParticleTexture = Inputs.TranslucencyViewResourcesMap.Get(ETranslucencyPass::TPT_TranslucencyAfterDOF).ColorTexture.Target;
	FRDGTextureDesc VitalParticleTextureDesc = OriginalParticleTexture->Desc;
	VitalParticleTextureDesc.Reset();
	VitalParticleTextureDesc.Flags |= TexCreate_UAV;
	//VitalParticleTextureDesc.Flags &= ~(TexCreate_RenderTargetable | TexCreate_FastVRAM);
	
	FRDGTextureRef VitalParticleTexture = GraphBuilder.CreateTexture(VitalParticleTextureDesc, TEXT("Particle Output Texture"));
	
	FRectShaderPSParams* PSParams = GraphBuilder.AllocParameters<FRectShaderPSParams>();
	// Render target binding slots
	FRenderTargetBindingSlots RenderTargets;
	RenderTargets[0] = FRenderTargetBinding(SceneColor.Texture, ERenderTargetLoadAction::ENoAction);
	RenderTargets[1] = FRenderTargetBinding(VitalParticleTexture, ERenderTargetLoadAction::ENoAction);

	PSParams->RenderTargets = RenderTargets;
	PSParams->Color = MyColor;
	PSParams->ParticleTexture = ParticleTexture.Target;
	PSParams->InputSampler = TStaticSamplerState<SF_Point, AM_Clamp, AM_Clamp, AM_Clamp>::GetRHI();
	PSParams->ViewParams = SceneTextureViewportParams;
	
	TShaderMapRef<FRectShaderPS> PixelShader(ViewShaderMap);

	// It is work!
	// AddCopyTexturePass(GraphBuilder, ParticleTexture.Target, SceneColor.Texture);

	AddFullscreenPass<FRectShaderPS>(
		GraphBuilder,
		ViewShaderMap,
		RDG_EVENT_NAME("Trancelucence Pass"),
		PixelShader,
		PSParams,
		ViewInfo);
	
	// input original particles, output SceneColor - OK
	// input my particles, output SceneColor - OK
	// input my particles, output original particles ?????
	AddCopyTexturePass(GraphBuilder, VitalParticleTexture,
		Inputs.TranslucencyViewResourcesMap.Get(ETranslucencyPass::TPT_TranslucencyAfterDOF).ColorTexture.Target);
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

	/*RDG_GPU_STAT_SCOPE(GraphBuilder, SuperRectanglePass);
	RDG_EVENT_SCOPE(GraphBuilder, "__AfterTonemap__SuperRectangle");

	const FIntRect ViewInfo = static_cast<const FViewInfo&>(View).ViewRect;
	const FGlobalShaderMap* ViewShaderMap = static_cast<const FViewInfo&>(View).ShaderMap;
	FLinearColor MyColor = FLinearColor(1.0f, 0.0f, 0.0f);*/

	// RenderRectangle(GraphBuilder, ViewShaderMap, ViewInfo, SceneColor, MyColor);

	return SceneColor;
}

FScreenPassTexture FRectViewExtension::SuperRectanglePassAfterMotionBlur_RenderThread(
	FRDGBuilder& GraphBuilder,
	const FSceneView& View,
	const FPostProcessMaterialInputs& Inputs)
{
	const FScreenPassTexture SceneColor = Inputs.GetInput(EPostProcessMaterialInput::SeparateTranslucency);

	/*RDG_GPU_STAT_SCOPE(GraphBuilder, SuperRectanglePass);
	RDG_EVENT_SCOPE(GraphBuilder, "__AfterMotionBlur__SuperRectangle");

	const FIntRect ViewInfo = static_cast<const FViewInfo&>(View).ViewRect;
	const FGlobalShaderMap* ViewShaderMap = static_cast<const FViewInfo&>(View).ShaderMap;
	FLinearColor MyColor = FLinearColor(1.0f, 1.0f, 0.0f);*/

	// RenderRectangle(GraphBuilder, ViewShaderMap, ViewInfo, SceneColor, MyColor,);

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


FScreenPassTextureViewportParameters FRectViewExtension::GetTextureViewportParameters(
	const FScreenPassTextureViewport& InViewport)
{
	const FVector2f Extent(InViewport.Extent);
	const FVector2f ViewportMin(InViewport.Rect.Min.X, InViewport.Rect.Min.Y);
	const FVector2f ViewportMax(InViewport.Rect.Max.X, InViewport.Rect.Max.Y);
	const FVector2f ViewportSize = ViewportMax - ViewportMin;

	FScreenPassTextureViewportParameters Parameters;

	if (!InViewport.IsEmpty()) {
		Parameters.Extent = FVector2f(Extent);
		Parameters.ExtentInverse = FVector2f(1.0f / Extent.X, 1.0f / Extent.Y);

		Parameters.ScreenPosToViewportScale = FVector2f(0.5f, -0.5f) * ViewportSize;	
		Parameters.ScreenPosToViewportBias = (0.5f * ViewportSize) + ViewportMin;	

		Parameters.ViewportMin = InViewport.Rect.Min;
		Parameters.ViewportMax = InViewport.Rect.Max;

		Parameters.ViewportSize = ViewportSize;
		Parameters.ViewportSizeInverse = FVector2f(1.0f / Parameters.ViewportSize.X, 1.0f / Parameters.ViewportSize.Y);

		Parameters.UVViewportMin = ViewportMin * Parameters.ExtentInverse;
		Parameters.UVViewportMax = ViewportMax * Parameters.ExtentInverse;

		Parameters.UVViewportSize = Parameters.UVViewportMax - Parameters.UVViewportMin;
		Parameters.UVViewportSizeInverse = FVector2f(1.0f / Parameters.UVViewportSize.X, 1.0f / Parameters.UVViewportSize.Y);

		Parameters.UVViewportBilinearMin = Parameters.UVViewportMin + 0.5f * Parameters.ExtentInverse;
		Parameters.UVViewportBilinearMax = Parameters.UVViewportMax - 0.5f * Parameters.ExtentInverse;
	}

	return Parameters;
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



































