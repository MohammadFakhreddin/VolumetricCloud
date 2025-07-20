#pragma once

#include "BedrockCommon.hpp"

#include <atomic>

namespace MFA {

    bool TryLock(std::atomic<bool>& lock);

    void Lock(std::atomic<bool>& lock);

    void Unlock(std::atomic<bool>& lock);

    class ScopeLock
    {
    public:
        explicit ScopeLock(std::atomic<bool> & lock);
        ~ScopeLock();

        ScopeLock(ScopeLock const &) noexcept = delete;
        ScopeLock(ScopeLock &&) noexcept = delete;
        ScopeLock & operator = (ScopeLock const &) noexcept = delete;
        ScopeLock & operator = (ScopeLock &&) noexcept = delete;

    private:
        std::atomic<bool> & mLock;
    };

}

#define MFA_SCOPE_LOCK(lock)        MFA::ScopeLock MFA_UNIQUE_NAME(__scopeLock) {lock};
