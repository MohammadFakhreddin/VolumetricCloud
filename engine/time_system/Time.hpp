#pragma once

#include <functional>
#include <memory>

#include "../job_system/ThreadSafeQueue.hpp"

namespace MFA
{

    class Time
    {
    public:

        using UpdateTask = std::function<bool()>;

        static std::unique_ptr<Time> Instantiate(int maxFramerate = 120,int minFramerate = 30);

        explicit Time(int maxFramerate, int minFramerate);

        ~Time();

        void Update();

        // Return false if no longer need to be repeated
        static void AddUpdateTask(UpdateTask task);

        static int DeltaTimeMs();

        static float DeltaTimeSec();

        static float NowSec();

        static bool HasInstance();

    private:

        static inline Time * Instance = nullptr;
        
        int _startTimeMs{};
        int _nowMs{};
        int _minDeltaTimeMs{};
        int _maxDeltaTimeMs{};
        
        int _deltaTimeMs {};
        float _deltaTimeSec {};
        float _timeSec {};

        ThreadSafeQueue<UpdateTask> _updateTasks;
        std::queue<UpdateTask> _pUpdateTasks;

    };

}
