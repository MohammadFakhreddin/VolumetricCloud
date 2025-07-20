#include "SpritePipelineCommon.hlsl"

struct Input
{
    float2 uv : TEXCOORD0;
    float4 color: COLOR0;
};

struct Output
{
    float4 color : SV_Target0;
};

Texture2D imageTexture : register(t0, space0);
SamplerState imageSampler : register(s0, space0);

[[vk::push_constant]]
cbuffer {
    PushConsts pushConsts;
};

float4 main(Input input) : SV_TARGET
{
    float4 color = imageTexture.Sample(imageSampler, input.uv) * pushConsts.color;
    return color;
}