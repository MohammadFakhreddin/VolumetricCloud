#pragma once

#include "ThreadSafeQueue.hpp"

#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <vector>

namespace MFA
{

    class ThreadPool
    {
    public:

        using Task = std::function<void()>;
        
        explicit ThreadPool(int threadCount);
        // We can have a threadPool with custom number of threads

        ~ThreadPool();

        ThreadPool(ThreadPool const &) noexcept = delete;
        ThreadPool(ThreadPool &&) noexcept = delete;
        ThreadPool & operator = (ThreadPool const &) noexcept = delete;
        ThreadPool & operator = (ThreadPool &&) noexcept = delete;

        [[nodiscard]]
        bool IsMainThread() const;

        void AssignTask(Task const & task);

        void AssignTask(int threadIdx, Task const & task) const;

        void CancelTasks() const;

        void Terminate();

        [[nodiscard]]
        int NumberOfAvailableThreads() const;
        
        class ThreadObject
        {
        public:

            explicit ThreadObject(int threadNumber, ThreadPool & parent);

            ~ThreadObject() = default;

            ThreadObject(ThreadObject const &) noexcept = delete;
            ThreadObject(ThreadObject &&) noexcept = delete;
            ThreadObject & operator = (ThreadObject const &) noexcept = delete;
            ThreadObject & operator = (ThreadObject &&) noexcept = delete;

            void Join() const;

            [[nodiscard]]
            bool IsFree();

            void Notify();

            [[nodiscard]]
            int GetThreadNumber() const;

            bool AwakeCondition(int idx);

            void AssignTask(Task const & task);

            void CancelTasks();

        private:

            void mainLoop();
            
            ThreadPool & mParent;

            int mThreadNumber;

            std::condition_variable mCondition;

            std::unique_ptr<std::thread> mThread;

            std::atomic<bool> mIsBusy = false;

            ThreadSafeQueue<Task> mTasks{};

        };

        bool AllThreadsAreIdle() const;

        std::vector<std::string> Exceptions();

    private:
        
        std::vector<std::unique_ptr<ThreadObject>> mThreadObjects;

        bool mIsAlive = true;

        int mNumberOfThreads = 0;

        ThreadSafeQueue<std::string> mExceptions{};

        int mNextTaskIdx {};

        std::thread::id mMainThreadId{};

    };

}
