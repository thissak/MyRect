#pragma once
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
	virtual void InitRHI(FRHICommandListBase& RHICmdList) override
	{
		TResourceArray<FColorVertex, VERTEXBUFFER_ALIGNMENT> Vertices;
		Vertices.SetNumUninitialized(4);

		Vertices[0].Position = FVector2f(-1.0f, 1.0f);
		Vertices[1].Position = FVector2f(-1.0f, -1.0f);
		Vertices[2].Position = FVector2f(1.0f, -1.0f);
		Vertices[3].Position = FVector2f(1.0f, 1.0f);

		Vertices[0].Color = FVector4f(1.0f, 0.0f, 0.0f, 1.0f);
		Vertices[1].Color = FVector4f(0.0f, 1.0f, 0.0f, 1.0f);
		Vertices[2].Color = FVector4f(0.0f, 0.0f, 1.0f, 1.0f);
		Vertices[3].Color = FVector4f(1.0f, 0.0f, 0.0f, 1.0f);

		FRHIResourceCreateInfo CreateInfo(TEXT("FScreenRectangleVertexBuffer"), &Vertices);
		VertexBufferRHI = RHICmdList.CreateVertexBuffer(Vertices.GetResourceDataSize(), BUF_Static, CreateInfo);
	}
};

// index buffer
class FRectShaderIndexBuffer : public FIndexBuffer
{
public:
	virtual void InitRHI(FRHICommandListBase& RHICmdList) override
	{
		const uint16 Indices[] = { 0, 1, 2, 2 ,3, 0};

		TResourceArray<uint16, INDEXBUFFER_ALIGNMENT> IndexBuffer;
		uint32 NumIndices = UE_ARRAY_COUNT(Indices);
		IndexBuffer.AddUninitialized(NumIndices);
		FMemory::Memcpy(IndexBuffer.GetData(), Indices, NumIndices * sizeof(uint16));

		FRHIResourceCreateInfo CreateInfo(TEXT("FRectangleIndexBuffer"), &IndexBuffer);
		IndexBufferRHI = RHICmdList.CreateIndexBuffer(sizeof(uint16), IndexBuffer.GetResourceDataSize(), BUF_Static, CreateInfo);
	}
};

// VertexDeclaration
class FRectShaderVertexDelaration : public FRenderResource
{
public:
	FVertexDeclarationRHIRef VertexDeclarationRHI;

	virtual ~FRectShaderVertexDelaration() {}

	virtual void InitRHI(FRHICommandListBase& RHICmdList) override
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

extern SUPERRECTANGLE_API TGlobalResource<FRectShaderVertexBuffer> GRectShaderVSBuffer;
extern SUPERRECTANGLE_API TGlobalResource<FRectShaderIndexBuffer> GRectShaderIndexBuffer;
extern SUPERRECTANGLE_API TGlobalResource<FRectShaderVertexDelaration> GRectShaderVertexDelaration;











