#include "DebugShape3D.hlsli"

struct TransformationMatrix
{
    float4x4 WVP;
    float4x4 World;
};

ConstantBuffer<TransformationMatrix> gTransformationMatrix : register(b0);

struct VertexShaderInput
{
    float4 position : POSITION0;
    float4 color : COLOR0;
};

VertexShaderOutput main(VertexShaderInput input)
{
     // world座標
    float4 worldPos = mul(input.position, gTransformationMatrix.World);
    
    VertexShaderOutput output;
    output.position = mul(input.position, gTransformationMatrix.WVP);
    output.color = input.color;
    output.worldPosition = worldPos.xyz;
    return output;
}