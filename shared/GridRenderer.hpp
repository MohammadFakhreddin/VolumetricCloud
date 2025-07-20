#pragma once
#include "BufferTracker.hpp"
#include "GridPipeline.hpp"

class GridRenderer
{
public:

    using Pipeline = GridPipeline;
    using Position = Pipeline::Position;
    using Color = Pipeline::Color;
    using Thickness = Pipeline::Thickness;

    explicit GridRenderer(std::shared_ptr<Pipeline> pipeline);

    void Draw(
        MFA::RT::CommandRecordState & recordState,
        Pipeline::PushConstants const & pushConstants
    ) const;

private:

    void AllocateBuffers();

    std::shared_ptr<Pipeline> _pipeline;
    std::shared_ptr<MFA::RT::BufferAndMemory> _vertexBuffer;
    std::shared_ptr<MFA::RT::BufferAndMemory> _indexBuffer;
    int _indexCount;

};
