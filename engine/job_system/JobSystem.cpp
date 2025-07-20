#include "JobSystem.hpp"

//----------------------------------------------------------------------------------------------------------------------

std::shared_ptr<MFA::JobSystem> MFA::JobSystem::Instantiate()
{
    std::shared_ptr<MFA::JobSystem> shared_ptr = _instance.lock();
    if (shared_ptr == nullptr)
    {
        shared_ptr = std::make_shared<MFA::JobSystem>();
        _instance = shared_ptr;
    }
    return shared_ptr;
}

//----------------------------------------------------------------------------------------------------------------------

void MFA::JobSystem::Destroy()
{
    _instance.reset();
}

//----------------------------------------------------------------------------------------------------------------------

bool MFA::JobSystem::HasInstance() { return !_instance.expired(); }

//----------------------------------------------------------------------------------------------------------------------

MFA::JobSystem::JobSystem() = default;
MFA::JobSystem::~JobSystem() = default;

//----------------------------------------------------------------------------------------------------------------------