#include "BufferTracker.hpp"

#include "BedrockAssert.hpp"
#include "LogicalDevice.hpp"

namespace MFA
{

    //-----------------------------------------------------------------------------------------------

    HostVisibleBufferTracker::HostVisibleBufferTracker(
        std::shared_ptr<RT::BufferGroup> bufferGroup, 
        Alias const & data
    ) : HostVisibleBufferTracker(std::move(bufferGroup))
    {
        for (auto const& buffer : mBufferGroup->buffers)
        {
            RB::UpdateHostVisibleBuffer(
                LogicalDevice::GetVkDevice(),
                *buffer,
                Alias(data.Ptr(), data.Len())
            );
        }
    }
    
    //-----------------------------------------------------------------------------------------------
    
    HostVisibleBufferTracker::HostVisibleBufferTracker(std::shared_ptr<RT::BufferGroup> bufferGroup)
        : mBufferGroup(std::move(bufferGroup))
    {
        mData = Memory::AllocSize(mBufferGroup->bufferSize);
        mDirtyCounter = 0;
    }

    //-----------------------------------------------------------------------------------------------
    
    void HostVisibleBufferTracker::Update(RT::CommandRecordState const & recordState)
    {
        if (mDirtyCounter > 0)
        {
            RB::UpdateHostVisibleBuffer(
                LogicalDevice::GetVkDevice(),
                *mBufferGroup->buffers[recordState.frameIndex % mBufferGroup->buffers.size()],
                Alias(mData->Ptr(), mData->Len())
            );
            --mDirtyCounter;
        }
    }

    //-----------------------------------------------------------------------------------------------

    void HostVisibleBufferTracker::SetData(Alias const & data)
    {
        mDirtyCounter = (int)mBufferGroup->buffers.size();
        MFA_ASSERT(data.Len() <= mData->Len());
        std::memcpy(mData->Ptr(), data.Ptr(), data.Len());
    }

    //-----------------------------------------------------------------------------------------------

    uint8_t * HostVisibleBufferTracker::Data()
    {
        mDirtyCounter = (int)mBufferGroup->buffers.size();
        return mData->Ptr();
    }

    //-----------------------------------------------------------------------------------------------

    LocalBufferTracker::LocalBufferTracker(
        std::shared_ptr<RT::BufferGroup> localBuffer,
        std::shared_ptr<RT::BufferGroup> hostVisibleBuffer,
        Alias const & data
    )
        : mLocalBuffer(std::move(localBuffer))
        , mHostVisibleBuffer(std::move(hostVisibleBuffer))
    {
        mData = Memory::AllocSize(mLocalBuffer->bufferSize);
        SetData(data);
    }

    //-----------------------------------------------------------------------------------------------

    LocalBufferTracker::LocalBufferTracker(
        std::shared_ptr<RT::BufferGroup> localBuffer,
        std::shared_ptr<RT::BufferGroup> hostVisibleBuffer
    )
        : mLocalBuffer(std::move(localBuffer))
        , mHostVisibleBuffer(std::move(hostVisibleBuffer))
    {
        mData = Memory::AllocSize(mLocalBuffer->bufferSize);
    }

    //-----------------------------------------------------------------------------------------------

    void LocalBufferTracker::Update(RT::CommandRecordState const & recordState)
    {
        if (mDirtyCounter > 0)
        {
            RB::UpdateHostVisibleBuffer(
                LogicalDevice::GetVkDevice(),
                *mHostVisibleBuffer->buffers[recordState.frameIndex % mHostVisibleBuffer->buffers.size()],
                Alias(mData->Ptr(), mData->Len())
            );
            RB::UpdateLocalBuffer(
                recordState.commandBuffer,
                *mLocalBuffer->buffers[recordState.frameIndex % mLocalBuffer->buffers.size()],
                *mHostVisibleBuffer->buffers[recordState.frameIndex % mHostVisibleBuffer->buffers.size()]
            );
            --mDirtyCounter;
        }
    }

    //-----------------------------------------------------------------------------------------------

    void LocalBufferTracker::SetData(Alias const & data)
    {
        mDirtyCounter = (int)mLocalBuffer->buffers.size();
        MFA_ASSERT(data.Len() <= mData->Len());
        std::memcpy(mData->Ptr(), data.Ptr(), data.Len());
    }

    //-----------------------------------------------------------------------------------------------

    uint8_t * LocalBufferTracker::Data()
    {
        // Returns the data and resets the counter.
        mDirtyCounter = (int)LogicalDevice::GetMaxFramePerFlight();
        return mData->Ptr();
    }

    //-----------------------------------------------------------------------------------------------

    RT::BufferGroup const & LocalBufferTracker::HostVisibleBuffer() const
    {
        return *mHostVisibleBuffer;
    }

    //-----------------------------------------------------------------------------------------------

    RT::BufferGroup const & LocalBufferTracker::LocalBuffer() const
    {
        return *mLocalBuffer;
    }
    
    //-----------------------------------------------------------------------------------------------
    
}
