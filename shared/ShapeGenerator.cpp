#include "ShapeGenerator.hpp"

#include <ext/scalar_constants.hpp>

namespace ShapeGenerator
{
    //======================================================================================================================

    Mesh Cylinder(float const radius, float const height, int const segments)
    {
        Vertices vertices{};
        Indices indices{};
        Normals normals{};

        float halfHeight = height / 2.0f;
        float angleStep = 2.0f * glm::pi<float>() / (float)segments;

        // Generate vertices and normals
        for (int i = 0; i < segments; ++i) {
            float angle = i * angleStep;
            float x = radius * cos(angle);
            float z = radius * sin(angle);

            // Bottom vertex
            vertices.emplace_back(x, -halfHeight, z);
            normals.emplace_back(x, 0.0f, z);

            // Top vertex
            vertices.emplace_back(x, halfHeight, z);
            normals.emplace_back(x, 0.0f, z);
        }

        // Add top and bottom center vertices
        vertices.emplace_back(0.0f, -halfHeight, 0.0f); // Bottom center
        normals.emplace_back(0.0f, -1.0f, 0.0f);

        vertices.emplace_back(0.0f, halfHeight, 0.0f); // Top center
        normals.emplace_back(0.0f, 1.0f, 0.0f);

        int bottomCenterIndex = segments * 2;
        int topCenterIndex = segments * 2 + 1;

        // Generate indices for the side faces
        for (int i = 0; i < segments; ++i) {
            int next = (i + 1) % segments;

            // Side face
            indices.emplace_back(i * 2);
            indices.emplace_back(next * 2);
            indices.emplace_back(i * 2 + 1);

            indices.emplace_back(i * 2 + 1);
            indices.emplace_back(next * 2);
            indices.emplace_back(next * 2 + 1);
        }

        // Generate indices for the bottom face
        for (int i = 0; i < segments; ++i) {
            int next = (i + 1) % segments;

            indices.emplace_back(bottomCenterIndex);
            indices.emplace_back(next * 2);
            indices.emplace_back(i * 2);
        }

        // Generate indices for the top face
        for (int i = 0; i < segments; ++i) {
            int next = (i + 1) % segments;

            indices.emplace_back(topCenterIndex);
            indices.emplace_back(i * 2 + 1);
            indices.emplace_back(next * 2 + 1);
        }

        return std::make_tuple(vertices, indices, normals);
    }

    //======================================================================================================================

    Mesh Sphere(float radius, int const slices, int const stacks)
    {
        Vertices vertices{};
        Indices indices{};
        Normals normals{};

        // Generate vertices and normals
        for (int stack = 0; stack <= stacks; ++stack)
        {
            float phi = glm::pi<float>() * (float)stack / (float)stacks; // From 0 to π
            float y = cos(phi) * radius;
            float ringRadius = sin(phi) * radius;

            for (int slice = 0; slice <= slices; ++slice)
            {
                float theta = 2.0f * glm::pi<float>() * (float)slice / (float)slices; // From 0 to 2π
                float x = ringRadius * cos(theta);
                float z = ringRadius * sin(theta);

                glm::vec3 vertex(x, y, z);
                glm::vec3 normal = glm::normalize(vertex);

                vertices.emplace_back(vertex);
                normals.emplace_back(normal);
            }
        }

        // Generate indices
        for (int stack = 0; stack < stacks; ++stack)
        {
            for (int slice = 0; slice < slices; ++slice)
            {
                int current = stack * (slices + 1) + slice;
                int next = current + slices + 1;

                // First triangle
                indices.emplace_back(current);
                indices.emplace_back(next);
                indices.emplace_back(current + 1);

                // Second triangle
                indices.emplace_back(current + 1);
                indices.emplace_back(next);
                indices.emplace_back(next + 1);
            }
        }

        return std::make_tuple(vertices, indices, normals);
    }

    //======================================================================================================================

    Mesh Quad()
    {
        Vertices vertices{};
        Indices indices{};
        Normals normals{};

        vertices.emplace_back(-1.0f, -1.0f, 0.0f);
        vertices.emplace_back(+1.0f, -1.0f, 0.0f);
        vertices.emplace_back(-1.0f, +1.0f, 0.0f);
        vertices.emplace_back(+1.0f, +1.0f, 0.0f);

        indices.emplace_back(0);
        indices.emplace_back(1);
        indices.emplace_back(2);
        indices.emplace_back(2);
        indices.emplace_back(1);
        indices.emplace_back(3);

        normals.emplace_back(0.0f, 0.0f, -1.0f);
        normals.emplace_back(0.0f, 0.0f, -1.0f);
        normals.emplace_back(0.0f, 0.0f, -1.0f);
        normals.emplace_back(0.0f, 0.0f, -1.0f);

        return std::make_tuple(vertices, indices, normals);
    }

    //======================================================================================================================
}