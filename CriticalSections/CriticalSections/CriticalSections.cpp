#include <windows.h>
#include <string>
#include <iostream>

constexpr int THREADS_COUNT = 2;
constexpr int CORES_COUNT = 2;
constexpr int CYCLES_COUNT = 100'000;

CRITICAL_SECTION CriticalSection;

DWORD WINAPI ThreadProc(CONST LPVOID lp_param)
{
    EnterCriticalSection(&CriticalSection);

    int* working_variable = static_cast<int*>(lp_param);

    for (int i = 0; i < CYCLES_COUNT; i++)
    {
        int j = *working_variable;
        *working_variable = j + 1;
    }

    std::cout << *working_variable << std::endl;

    LeaveCriticalSection(&CriticalSection);

    ExitThread(0);
}


int main(int argc, char* argv[])
{
    int working_variable = 0;

    if (!InitializeCriticalSectionAndSpinCount(&CriticalSection, 0x00000400))
    {
        std::cerr << "Cannot initialize critical section" << std::endl;
        return EXIT_FAILURE;
    }

    HANDLE* handles = new HANDLE[THREADS_COUNT];
    for (int i = 0; i < THREADS_COUNT; i++)
    {
        handles[i] = CreateThread(nullptr, 0, &ThreadProc, &working_variable, CREATE_SUSPENDED, nullptr);
        if (!SetThreadAffinityMask(handles[i], (1 << CORES_COUNT) - 1))
        {
            std::cerr << "Error occured when setting thread affinity" << std::endl;
            return EXIT_FAILURE;
        }
    }

    for (int i = 0; i < THREADS_COUNT; i++)
    {
        ResumeThread(handles[i]);
    }

    WaitForMultipleObjects(THREADS_COUNT, handles, true, INFINITE);

    std::cout << "Result: " << working_variable << std::endl;

    delete[] handles;

    DeleteCriticalSection(&CriticalSection);

    return EXIT_SUCCESS;
}
