#ifndef SPRITE_PIPELINE_COMMON_H
#define SPRITE_PIPELINE_COMMON_H

struct PushConsts
{
    float4 color;
    float4x4 model;
    float4x4 viewProjection;
};

#endif