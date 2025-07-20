#include "SpritePipelineCommon.hlsl"

struct Input
{
    float3 position : POSITION0;
    float2 uv : TEXCOORD0;
};

struct Output
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
};

[[vk::push_constant]]
cbuffer {
    PushConsts pushConsts;
};

Output main(Input input)
{
    Output output;

    float4 projectedPosition = pushConsts.viewProjection * pushConsts.model * float4(input.position, 1.0);

    output.position = projectedPosition;
    output.uv = input.uv;

    return output;
}