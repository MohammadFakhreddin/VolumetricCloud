#include "ScopeLock.hpp"

#include "BedrockAssert.hpp"

#include <thread>

namespace MFA
{

    //==================================================================================================================

    bool TryLock(std::atomic<bool> &lock)
    {
        bool expectedValue = false;
        bool const desiredValue = true;
        if (lock.compare_exchange_strong(expectedValue, desiredValue) == true)
        {
            return true;
        }
        return false;
    }

    //==================================================================================================================

    void Lock(std::atomic<bool> &lock)
    {
        while (true)
        {
            bool expectedValue = false;
            bool const desiredValue = true;
            if (lock.compare_exchange_strong(expectedValue, desiredValue) == true)
            {
                break;
            }
            std::this_thread::yield();
        }
        MFA_ASSERT(lock == true);
    }

    //==================================================================================================================

    void Unlock(std::atomic<bool> &lock)
    {
        MFA_ASSERT(lock == true);
        lock = false;
    }

    //==================================================================================================================

    ScopeLock::ScopeLock(std::atomic<bool> & lock)
        : mLock(lock)
    {
        while (true)
        {
            bool expectedValue = false;
            bool const desiredValue = true;
            if (mLock.compare_exchange_strong(expectedValue, desiredValue) == true)
            {
                break;
            }
            std::this_thread::yield();
        }
        MFA_ASSERT(mLock == true);
    }

    ScopeLock::~ScopeLock()
    {
        MFA_ASSERT(mLock == true);
        mLock = false;
    }

    //==================================================================================================================

}
