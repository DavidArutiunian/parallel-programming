#pragma once

#include <windows.h>

#include "LogFileWriter.h"
#include "LinkedList.h"

class LogBuffer
{
public:
    LogBuffer(const LogBuffer&) = delete;

    LogBuffer& operator=(const LogBuffer&) = delete;

    LogBuffer(LogBuffer&&) = delete;

    LogBuffer& operator=(LogBuffer&&) = delete;

    LogBuffer(LogFileWriter* file_writer);

    ~LogBuffer();

    void Append(const char* message);

    std::size_t Size() const;

private:
    constexpr static size_t MAX_SIZE = 256;

    std::size_t size = 0;

    LogFileWriter* file_writer;

    CRITICAL_SECTION* critical_section;
    LinkedList list;

    HANDLE thread_handle;
    HANDLE write_event;

    void Clear();

    void StartThread();

    void NotifyThread();

    const char* PopMessage();

    static DWORD WINAPI LogSizeMonitoringThread(CONST LPVOID lp_param);
};
