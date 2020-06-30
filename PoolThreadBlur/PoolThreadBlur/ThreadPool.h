#pragma once

#include <iostream>
#include <vector>
#include <queue>
#include <memory>
#include <functional>
#include <windows.h>
#include <stdexcept>
#include <cstddef>

class ThreadPool
{
public:
    ThreadPool(std::size_t);

    void Enqueue(LPTHREAD_START_ROUTINE runnable, CONST LPVOID params);

    ~ThreadPool() noexcept;

private:
    std::vector<HANDLE> workers;

    std::queue<HANDLE> tasks;

    bool stop;

    static DWORD WINAPI WorkerThread(CONST LPVOID lp_param);
};

inline ThreadPool::ThreadPool(std::size_t threads) : stop(false) {
    for (std::size_t i = 0; i < threads; i++)
    {
        HANDLE worker = CreateThread(nullptr, 0, &WorkerThread, this, CREATE_SUSPENDED, nullptr);
        workers.push_back(worker);
    }
}

inline ThreadPool::~ThreadPool() noexcept
{
    stop = true;

    for (auto&& worker : workers) {
        ResumeThread(worker);
    }

    WaitForMultipleObjects(workers.size(), &workers.front(), true, INFINITE);
}

inline DWORD WINAPI ThreadPool::WorkerThread(CONST LPVOID lp_param)
{
    ThreadPool* context = static_cast<ThreadPool*>(lp_param);

    for (;;)
    {
        HANDLE task;

        if (context->stop && context->tasks.empty())
        {
            ExitThread(EXIT_SUCCESS);
        }

        task = context->tasks.front();
        context->tasks.pop();

        ResumeThread(task);
        WaitForSingleObject(task, INFINITE);
    }
}

inline void ThreadPool::Enqueue(LPTHREAD_START_ROUTINE runnable, CONST LPVOID params)
{
    HANDLE task = CreateThread(nullptr, 0, runnable, params, CREATE_SUSPENDED, nullptr);
    tasks.push(task);
}
