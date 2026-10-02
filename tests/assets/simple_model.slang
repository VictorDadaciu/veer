struct VSInput
{
    float3 Pos;
    float3 Normal;
    float2 UV;
};

Sampler2D textures[];

struct CameraData
{
    float4x4 projection;
    float4x4 view;
};

struct ModelData
{
    float4x4 transform;
    uint texIndex;
};

struct VSOutput
{
    float4 Pos : SV_POSITION;
    float3 Normal;
    float2 UV;
    float3 LightVec;
    float3 ViewVec;
    uint TexIndex;
};

[shader("vertex")]
VSOutput main(VSInput input, uniform CameraData *cameraData, uniform ModelData *modelData)
{
    VSOutput output;
    output.Normal = mul(((float3x3)mul(cameraData->view, modelData.transform)), input.Normal);
    output.UV = input.UV;
    float4 fragPos = mul(mul(cameraData->view, modelData.transform), float4(input.Pos, 1.0));
    output.Pos = mul(cameraData->projection, fragPos);
    // Calculate view vectors required for lighting
    float4 lightPos = mul(cameraData->view, float4(10.0, 10.0, 10.0, 1.0));
    output.LightVec = (lightPos - fragPos).xyz;
    output.ViewVec = -fragPos.xyz;
    return output;
}

[shader("fragment")]
float4 main(VSOutput input)
{
    // Phong lighting
    float3 N = normalize(input.Normal);
    float3 L = normalize(input.LightVec);
    float3 V = normalize(input.ViewVec);
    float3 H = normalize(L + V);
    float diffuse = saturate(dot(N, L));
    float specular = float(diffuse > 0.0) * pow(max(dot(N, H), 0.0), 16.0);
    // Sample from texture
    float3 color = textures[NonUniformResourceIndex(input.TexIndex)].Sample(input.UV).rgb;
    return float4(diffuse * color + specular + 0.05, 1.0);
}
