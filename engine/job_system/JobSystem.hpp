#pragma once

#include "ThreadPool.hpp"

#include <future>

namespace MFA
{
    class JobSystem
    {
    public:

        static std::shared_ptr<JobSystem> Instantiate();

        static void Destroy();

        [[nodiscard]] static bool HasInstance();

        explicit JobSystem();

        ~JobSystem();

        // I think callback system might be a better choice sometimes
        static std::future<void> AssignTask(std::function<void()> task)
        {
            auto instance = _instance.lock();
            if (instance == nullptr)
            {
                MFA_LOG_WARN("Failed to execute task");
                return {};
            }

            struct Params
            {
                std::promise<void> promise{};
            };
            auto params = std::make_shared<Params>();

            instance->threadPool.AssignTask(
            [task, params]()
            {
                task();
                params->promise.set_value();
            });
            return params->promise.get_future();
        }

        template <typename T>
        static std::future<T> AssignTask(std::function<T()> task)
        {
            auto instance = _instance.lock();
            if (instance == nullptr)
            {
                MFA_LOG_WARN("Failed to execute task");
                return {};
            }

            std::promise<T> promise{};
            struct Params
            {
                std::promise<T> promise{};
            };
            auto params = std::make_shared<Params>();

            instance->threadPool.AssignTask([task, params]() { params->promise.set_value(task()); });
            return params->promise.get_future();
        }

        [[nodiscard]]
        auto NumberOfAvailableThreads() const
        {
            return threadPool.NumberOfAvailableThreads();
        }

        [[nodiscard]]
        auto IsMainThread() const
        {
            return threadPool.IsMainThread();
        }

        // TODO: Add run on main thread!

    private:

		inline static std::weak_ptr<JobSystem> _instance {};

        static constexpr int MaxThreadCount = 4;
        static constexpr int MinThreadCount = 1;
        ThreadPool threadPool{std::max(std::min(MaxThreadCount, (int)(std::thread::hardware_concurrency() * 0.5f)), MinThreadCount)};

    };
} // namespace MFA


namespace MFA
{
    using JS = JobSystem;
}
