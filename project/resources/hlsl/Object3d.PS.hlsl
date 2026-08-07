#include "Object3d.hlsli"

static const int kMaxPointLights = 512;
static const int kMaxSpotLights = 512;

struct Material
{
    float4 color;
    int enableLightig;
    float4x4 uvTransform;
    float shininess;
};

struct DirectionalLight
{
    float4 color;
    float3 direction;
    float intensity;
};

struct PointLight
{
    float4 color;
    float3 position;
    float intensity;
    float radius;
    float decay;
    float2 pad;
};

struct SpotLight
{
    float4 color;
    float3 position;
    float intensity;
    float3 direction;
    float distance;
    float decay;
    float cosAngle;
    float cosFalloffStart;
    float pad;
};

struct PointLightGroup
{
    PointLight gPointLights[kMaxPointLights];
    int gPointLightCount;
    float3 gPointLightPad;
};

struct SpotLightGroup
{
    SpotLight gSpotLights[kMaxSpotLights];
    int gSpotLightCount;
    float3 gSpotLightPad;
};

struct Camera
{
    float3 worldPosition;
};

ConstantBuffer<Material> gMaterial : register(b0);
Texture2D<float4> gTexture : register(t0);
SamplerState gSampler : register(s0);
ConstantBuffer<DirectionalLight> gDiretionalLight : register(b1);
ConstantBuffer<Camera> gCamera : register(b2);
ConstantBuffer<PointLightGroup> gPointLightGroup : register(b3);
ConstantBuffer<SpotLightGroup> gSpotLightGroup : register(b4);

struct PixelShaderOutput
{
    float4 color : SV_TARGET0;
};

float3 CalcDirectionalLight(VertexShaderOutput input, float specularPow, float4 textureColor)
{
    float NdotL = dot(normalize(input.normal), -gDiretionalLight.direction);
    float diffuseCos = saturate(NdotL);
    float3 diffuse = gMaterial.color.rgb * textureColor.rgb * gDiretionalLight.color.rgb * diffuseCos * gDiretionalLight.intensity;
    float3 specular = gDiretionalLight.color.rgb * gDiretionalLight.intensity * specularPow;
        
    return diffuse + specular;
}

float3 CalcPointLight(VertexShaderOutput input, float4 textureColor, PointLight light)
{
    float3 toEyePoint = normalize(gCamera.worldPosition - input.worldPosition);
    float3 lightDirPoint = normalize(light.position - input.worldPosition);
    float3 halfVectorPoint = normalize(lightDirPoint + toEyePoint);
    float NdotHPoint = dot(normalize(input.normal), halfVectorPoint);
    float specularPowPoint = pow(saturate(NdotHPoint), gMaterial.shininess);
    float3 pointLightDirection = normalize(input.worldPosition - light.position);
    
    float distance = length(light.position - input.worldPosition);
    float factor = pow(saturate(-distance / light.radius + 1.0f), light.decay);
    float NdotL = dot(normalize(input.normal), lightDirPoint);
    float diffuseCos = saturate(NdotL);
    float3 diffuse = gMaterial.color.rgb * textureColor.rgb * light.color.rgb * diffuseCos * light.intensity * factor;
    float3 specular = light.color.rgb * light.intensity * specularPowPoint * factor;
    
    return diffuse + specular;
}

float3 CalcSpotLight(VertexShaderOutput input, float4 textureColor, SpotLight light)
{
    float3 toEyeSpot = normalize(gCamera.worldPosition - input.worldPosition);
    float3 lightDirSpot = normalize(light.position - input.worldPosition);
    float3 halfVectorSpot = normalize(lightDirSpot + toEyeSpot);
    float NdotHSpot = dot(normalize(input.normal), halfVectorSpot);
    float specularPowSpot = pow(saturate(NdotHSpot), gMaterial.shininess);
    float3 spotLightDirectionOnSurface = normalize(input.worldPosition - light.position);
    
    float distance = length(light.position - input.worldPosition);
    float attenuationFactor = pow(saturate(-distance / light.distance + 1.0f), light.decay);
    float cosAngle = dot(normalize(-spotLightDirectionOnSurface), light.direction);
    float falloffFactor = saturate((cosAngle - light.cosAngle) / (light.cosFalloffStart - light.cosAngle));
    float NdotL = dot(normalize(input.normal), normalize(-spotLightDirectionOnSurface));
    float diffuseCos = saturate(NdotL);
    float3 diffuse = gMaterial.color.rgb * textureColor.rgb * light.color.rgb * diffuseCos * light.intensity * attenuationFactor * falloffFactor;
    float3 specular = light.color.rgb * light.intensity * specularPowSpot * attenuationFactor * falloffFactor;
    
    return diffuse + specular;
}

PixelShaderOutput main(VertexShaderOutput input)
{
    float4 transformedUV = mul(float4(input.texcoord, 0.0f, 1.0f), gMaterial.uvTransform);
    float4 textureColor = gTexture.Sample(gSampler, transformedUV.xy);

     // 視線ベクトル
    float3 toEyeDirection = normalize(gCamera.worldPosition - input.worldPosition);
    float3 halfVectorDirection = normalize(-gDiretionalLight.direction + toEyeDirection);
    float NdotHDir = dot(normalize(input.normal), halfVectorDirection);
    float specularPowDir = pow(saturate(NdotHDir), gMaterial.shininess);
    
    PixelShaderOutput output;
    if (gMaterial.enableLightig != 0)
    {
        // Directional
        float3 directionalColor = CalcDirectionalLight(input, specularPowDir, textureColor);

        float3 pointColor = 0.0f.xxx;
        for (int i = 0; i < gPointLightGroup.gPointLightCount; ++i)
        {
            pointColor += CalcPointLight(input, textureColor, gPointLightGroup.gPointLights[i]);
        }

        float3 spotColor = 0.0f.xxx;
        for (int k = 0; k < gSpotLightGroup.gSpotLightCount; ++k)
        {
            spotColor += CalcSpotLight(input, textureColor, gSpotLightGroup.gSpotLights[k]);
        }
        
        output.color.rgb = directionalColor + pointColor + spotColor;

        if (textureColor.a == 0.0f)
        {
            discard;
        }
        
        output.color.a = gMaterial.color.a * textureColor.a;

        if (output.color.a == 0.0f)
        {
            discard;
        }
    }
    else
    {
        output.color = gMaterial.color * textureColor;
        
        if (output.color.a == 0.0f)
        {
            discard;
        }
    }
    return output;
}