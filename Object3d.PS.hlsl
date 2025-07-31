#include "Object3d.hlsli"

struct Material
{
    float4 color;
    int enableLightig;
    float4x4 uvTransform;
    int lightingMode;
};

struct DirectionalLight
{
    float4 color;
    float3 direction;
    float intensity;
};

ConstantBuffer<Material> gMaterial : register(b0);
Texture2D<float4> gTexture : register(t0);
SamplerState gSampler : register(s0);
ConstantBuffer<DirectionalLight> gDiretionalLight : register(b1);

struct PixelShaderOutput
{
    float4 color : SV_TARGET0;
};

PixelShaderOutput main(VertexShaderOutput input)
{
    float4 transformedUV = mul(float4(input.texcoord, 0.0f, 1.0f), gMaterial.uvTransform);
    float4 textureColor = gTexture.Sample(gSampler, transformedUV.xy);
    PixelShaderOutput output;
    
    float NdotL = dot(normalize(input.normal), -gDiretionalLight.direction);
    
    if (gMaterial.enableLightig != 0)
    {
        float ligtingFactor = 1.0f;
        
        if (gMaterial.lightingMode == 1)
        {
            // Lambert
            ligtingFactor = max(NdotL, 0.0f);
        }
        else if (gMaterial.lightingMode == 2)
        {
            // Half-Lambert
            ligtingFactor = pow(NdotL * 0.5f + 0.5f, 2.0f);
        }
        
        output.color = gMaterial.color * textureColor * gDiretionalLight.color * ligtingFactor * gDiretionalLight.intensity;
    }
    else
    {
        output.color = gMaterial.color * textureColor;
    }
    return output;
}