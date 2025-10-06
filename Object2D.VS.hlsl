#include "Object2D.hlsli"

struct SpriteTransformation
{
    float4x4 WVP;
};

ConstantBuffer<SpriteTransformation> gSpriteTransformation : register(b0);

struct VertexShaderInput
{
    float4 position : POSITION0;
    float2 texcoord : TEXCOORD0;
};

VertexShaderOutput main(VertexShaderOutput input)
{
    VertexShaderOutput output;
    output.position = mul(input.position, gSpriteTransformation.WVP);
    output.texcoord = input.texcoord;
    
    return output;
}