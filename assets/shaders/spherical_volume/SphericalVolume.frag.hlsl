#include "SphereicalVolume.common.hlsl"

struct Input
{
    [[vk::location(0)]] float2 uv : TEXCOORD0;
    [[vk::location(1)]] float3 color : COLOR;
    [[vk::location(3)]] float radius;
};

struct Output
{
    float4 color : SV_Target0;
};

struct PushConsts
{
    float time;
};
[[vk::push_constant]]
cbuffer {
    PushConsts pushConsts;
};

float4 main(Input input) : SV_TARGET
{
    float strength = textureFont.Sample(samplerFont, input.uv).r;
    if (strength < 0.5)
    {
        discard;
    }
    return float4(input.color * strength, 1.0);
}