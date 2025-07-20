#pragma once

#include "BedrockAssert.hpp"
#include "ScopeLock.hpp"

#include <queue>

namespace MFA {

template <typename T>
class ThreadSafeQueue {
public:

    bool TryToPush(T newData) {
        bool expectedValue = false;
        bool const desiredValue = true;
        if (mLock.compare_exchange_strong(expectedValue, desiredValue) == false) {
            return false;
        }

        MFA_ASSERT(mLock == true);
        mData.push(newData);
        mLock = false;
        return true;
    }

    void Push(T newData)
    {
        while (TryToPush(newData) == false);
    }

    // Returns front item
    bool TryToPop(T & outData, bool & isEmpty) {
        bool expectedValue = false;
        bool const desiredValue = true;

        isEmpty = true;

        if (mLock.compare_exchange_strong(expectedValue, desiredValue) == false) {
            return false;
        }

        MFA_ASSERT(mLock == true);
        bool success = true;
        if (mData.empty() == false) 
        {
            outData = mData.front();
            isEmpty = false;
            mData.pop();
        }

        mLock = false;
        return success;
    }

    void Pop(T & outData, bool& isEmpty)
    {
        while (TryToPop(outData, isEmpty) == false);
    }

    [[nodiscard]]
    T Pop()
    {
        MFA_ASSERT(IsEmpty() == false);

        bool isEmpty;
        T outData;
        while (TryToPop(outData, isEmpty) == false);

        return outData;
    }

    void PopAll()
    {
        bool expectedValue = false;
        bool const desiredValue = true;

        while(true)
        {
            if (mLock.compare_exchange_strong(expectedValue, desiredValue) == false) {
                continue;
            }
            break;
        }

        while (mData.empty() == false)
        {
            mData.pop();
        }
        mLock = false;
    }

    [[nodiscard]]
    bool IsEmpty() {
        MFA_SCOPE_LOCK(mLock)
        return mData.empty();
    }

    [[nodiscard]]
    size_t ItemCount()
    {
        MFA_SCOPE_LOCK(mLock)
        return mData.size();
    }

private:
    std::atomic<bool> mLock = false;
    std::queue<T> mData {};
};

}
