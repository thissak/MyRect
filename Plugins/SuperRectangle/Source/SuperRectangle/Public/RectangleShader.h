#pragma once
#include "ScreenPass.h"
#include "ShaderParameterStruct.h"

// vertex shader class
BEGIN_SHADER_PARAMETER_STRUCT(FRectShaderVSParams,)
END_SHADER_PARAMETER_STRUCT()
class FRectShaderVS : public FGlobalShader
{
public:
	DECLARE_GLOBAL_SHADER(FRectShaderVS)
	using FParameters = FRectShaderVSParams;
	SHADER_USE_PARAMETER_STRUCT(FRectShaderVS, FGlobalShader)

	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		return true;
	}
};


// pixel shader class
BEGIN_SHADER_PARAMETER_STRUCT(FRectShaderPSParams, )
	SHADER_PARAMETER(FLinearColor, Color)
	SHADER_PARAMETER_RDG_TEXTURE(Texture2D, ParticleTexture)
	SHADER_PARAMETER_STRUCT(FScreenPassTextureViewportParameters, ViewParams)
	SHADER_PARAMETER_SAMPLER(SamplerState, InputSampler)
	RENDER_TARGET_BINDING_SLOTS()
END_SHADER_PARAMETER_STRUCT()
class FRectShaderPS : public FGlobalShader
{
public:
	DECLARE_GLOBAL_SHADER(FRectShaderPS)
	using FParameters = FRectShaderPSParams;
	SHADER_USE_PARAMETER_STRUCT(FRectShaderPS, FGlobalShader)
};

struct FColorVertex
{
public:
	FVector2f Position;
	FVector4f Color;
};

// vertex buffer
class FRectShaderVertexBuffer : public FVertexBuffer
{
public:
	virtual void InitRHI(FRHICommandListBase& RHICmdList) 
	{
		TResourceArray<FColorVertex, VERTEXBUFFER_ALIGNMENT> Vertices;
		Vertices.SetNumUninitialized(4);

		Vertices[0].Position = FVector2f(-1.0f, -1.0f); // left bottom
		Vertices[1].Position = FVector2f(1.0f, -1.0f); //right bottom
		Vertices[2].Position = FVector2f(-1.0f, 1.0f); // left top
		Vertices[3].Position = FVector2f(1.0f, 1.0f); // right top

		Vertices[0].Color = FVector4f(1.0f, 0.0f, 0.0f, 1.0f);
		Vertices[1].Color = FVector4f(0.0f, 1.0f, 0.0f, 1.0f);
		Vertices[2].Color = FVector4f(0.0f, 0.0f, 1.0f, 1.0f); //blue2
		Vertices[3].Color = FVector4f(1.0f, 0.0f, 1.0f, 1.0f);

		FRHIResourceCreateInfo CreateInfo(TEXT("FScreenRectangleVertexBuffer"), &Vertices);
		VertexBufferRHI = RHICmdList.CreateVertexBuffer(Vertices.GetResourceDataSize(), BUF_Static, CreateInfo);
	}
};

// index buffer
class FRectShaderIndexBuffer : public FIndexBuffer
{
public:
	virtual void InitRHI(FRHICommandListBase& RHICmdList) 
	{
		const uint16 Indices[] = { 2, 0, 1, 2, 3, 1 };

		TResourceArray<uint16, INDEXBUFFER_ALIGNMENT> IndexBuffer;
		uint32 NumIndices = UE_ARRAY_COUNT(Indices);
		IndexBuffer.AddUninitialized(NumIndices);
		FMemory::Memcpy(IndexBuffer.GetData(), Indices, NumIndices * sizeof(uint16));

		FRHIResourceCreateInfo CreateInfo(TEXT("FRectangleIndexBuffer"), &IndexBuffer);
		IndexBufferRHI = RHICmdList.CreateIndexBuffer(sizeof(uint16), IndexBuffer.GetResourceDataSize(), BUF_Static, CreateInfo);
	}
};

// VertexDeclaration
class FRectShaderVertexDeclaration : public FRenderResource
{
public:
	FVertexDeclarationRHIRef VertexDeclarationRHI;

	virtual ~FRectShaderVertexDeclaration() override {}

	virtual void InitRHI(FRHICommandListBase& RHICmdList) 
	{
		FVertexDeclarationElementList Elements;
		uint16 Stride = sizeof(FColorVertex);
		Elements.Add(FVertexElement(0, STRUCT_OFFSET(FColorVertex, Position), VET_Float2, 0, Stride));
		Elements.Add(FVertexElement(0, STRUCT_OFFSET(FColorVertex, Color), VET_Float4, 1, Stride));
		VertexDeclarationRHI = PipelineStateCache::GetOrCreateVertexDeclaration(Elements);
	}

	virtual void ReleaseRHI() override
	{
		VertexDeclarationRHI.SafeRelease();
	}
};

extern SUPERRECTANGLE_API TGlobalResource<FRectShaderVertexBuffer> GRectShaderVertexBuffer;
extern SUPERRECTANGLE_API TGlobalResource<FRectShaderIndexBuffer> GRectShaderIndexBuffer;
extern SUPERRECTANGLE_API TGlobalResource<FRectShaderVertexDeclaration> GRectShaderVertexDeclaration;











