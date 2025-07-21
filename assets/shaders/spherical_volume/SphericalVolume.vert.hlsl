#include "SphereicalVolume.common.hlsl"

struct Input
{
    [[vk::location(0)]] float3 position : POSITION0;
    [[vk::location(1)]] float2 uv : TEXCOORD0;
    [[vk::location(2)]] float3 color : COLOR;
    [[vk::location(3)]] float radius
};

struct Output
{
    float4 position : SV_POSITION;
    [[vk::location(0)]] float2 uv : TEXCOORD0;
    [[vk::location(1)]] float3 color : COLOR;
    [[vk::location(3)]] float radius;
};

struct PushConsts
{
    float time;
};
[[vk::push_constant]]
cbuffer {
    PushConsts pushConsts;
};

Output main(Input input)
{
    Output output;
    output.position = pushConsts.model * float4(input.position.xyz, 1.0);
    output.uv = input.uv;
    output.color = input.color;
    output.radius = input.radius;
    return output;
}