struct VSIn
{
    float3 position : POSITION0;
};

struct VSOut
{
    float4 position : SV_POSITION;
    float2 gridPosition : POSITION0;
};

struct PushConsts
{
    float4x4 viewProjMat;
};
[[vk::push_constant]]
cbuffer {
    PushConsts pushConsts;
};

VSOut main(VSIn input) {
    VSOut output;

    float3 scaledInput = input.position * 100;
    output.position = mul(pushConsts.viewProjMat, float4(scaledInput, 1.0));
    output.gridPosition = scaledInput.xy;

    return output;
}