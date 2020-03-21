#include <windows.h>
#include <string>
#include <fstream>
#include <vector>
#include <chrono>

constexpr int THREAD_COUNT = 2;
constexpr int CYCLES_COUNT = 100000;

struct ThreadParams
{
    size_t thread_no;
    std::chrono::steady_clock::time_point start_time;
    std::ofstream* file;
};

DWORD WINAPI ThreadProc(CONST LPVOID lp_param)
{
    const ThreadParams* params = static_cast<ThreadParams*>(lp_param);
    const std::chrono::steady_clock::time_point start_time = params->start_time;
    std::ofstream* file = params->file;
    const size_t thread_no = params->thread_no;

    for (int i = 0; i < CYCLES_COUNT; i++)
    {
        std::chrono::steady_clock::time_point now_time = std::chrono::steady_clock::now();
        const auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now_time - start_time).count();
        *file << thread_no << "|" << duration << std::endl;
    }

    ExitThread(0); // функция устанавливает код завершения потока в 0
}


int wmain()
{
    getchar();

    const std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();

    std::vector<std::ofstream*> files;
    files.push_back(new std::ofstream("t1.log"));
    files.push_back(new std::ofstream("t2.log"));

    std::vector<ThreadParams*> params;

    // создание двух потоков
    HANDLE* handles = new HANDLE[THREAD_COUNT];
    for (int i = 0; i < THREAD_COUNT; i++)
    {
        params.push_back(new ThreadParams{static_cast<size_t>(i), start, files.at(i)});
        handles[i] = CreateThread(NULL, 0, &ThreadProc, params.at(i), CREATE_SUSPENDED, NULL);
    }

    // запуск двух потоков
    for (int i = 0; i < THREAD_COUNT; i++)
    {
        ResumeThread(handles[i]);
    }

    // ожидание окончания работы двух потоков
    WaitForMultipleObjects(THREAD_COUNT, handles, true, INFINITE);

    for (int i = 0; i < THREAD_COUNT; i++)
    {
        auto file = files.at(i);
        file->close();
        delete file;

        const auto param = params.at(i);
        delete param;
    }
    return 0;
}
