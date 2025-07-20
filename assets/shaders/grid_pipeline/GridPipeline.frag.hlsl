struct PSIn
{
    float2 gridPosition : POSITION0;
};

struct PSOut
{
    float4 color : SV_Target0;
};

struct PushConsts
{
    float4x4 viewProjMat;
};
[[vk::push_constant]]
cbuffer {
    PushConsts pushConsts;
};

bool IsInsideGrid(float2 gridPos, float density, float thickness)
{
    float xMod = abs(fmod(gridPos.x, density));
    float yMod = abs(fmod(gridPos.y, density));
    if (
        (xMod < thickness || xMod > density - thickness) ||
        (yMod < thickness || yMod > density - thickness)
    )
    {
        return true;
    }
    return false;
}

PSOut main(PSIn input) {
    PSOut output;

    float densityModifier = 5.0f;
    float thicknessModifier = 10.0f;
    {// Primary line
        float density = 10.0 * densityModifier;
        float thickness = 0.05 * thicknessModifier;
        float4 color = float4(0.5, 0.0, 0.0, 0.9);
        if (IsInsideGrid(input.gridPosition, density, thickness))
        {
            output.color = color;
            return output;
        }
    }

    {// Secondary line
        float density = 2.5 * densityModifier;
        float thickness = 0.025 * thicknessModifier;
        float4 color = float4(0.0, 0.5, 0.5, 0.75);
        if (IsInsideGrid(input.gridPosition, density, thickness))
        {
            output.color = color;
            return output;
        }
    }

    {// Trinity line
        float density = 0.5 * densityModifier;
        float thickness = 0.025 * 0.75 * thicknessModifier;
        float4 color = float4(0.5, 0.5, 0.5, 0.5);
        if (IsInsideGrid(input.gridPosition, density, thickness))
        {
            output.color = color;
            return output;
        }
    }

    discard;
}