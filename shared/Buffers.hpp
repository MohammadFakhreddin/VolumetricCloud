#pragma once

#include <glm/glm.hpp>

namespace Buffers
{
    struct Camera
    {
        glm::mat4 viewProjection{};
        glm::vec3 position;
        float placeholder;
    };

    // Directional light for now
    struct DirectionalLight
    {
        glm::vec3 direction{};
        float ambientStrength{};
        glm::vec3 color{};
        float placeholder0{};
    };
}