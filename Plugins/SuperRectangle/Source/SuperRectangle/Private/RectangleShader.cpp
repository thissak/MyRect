#include "RectangleShader.h"

IMPLEMENT_SHADER_TYPE(, FRectShaderVS, TEXT("/MySuperShaders/RectShader.usf"), TEXT("RectVSMain"), SF_Vertex);
IMPLEMENT_SHADER_TYPE(, FRectShaderPS, TEXT("/MySuperShaders/RectShader.usf"), TEXT("RectPSMain"), SF_Pixel);

TGlobalResource<FRectShaderVertexBuffer> GRectShaderVertexBuffer;