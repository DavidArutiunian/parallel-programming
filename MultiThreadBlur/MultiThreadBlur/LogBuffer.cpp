#include <stdexcept>
#include <cstddef>
#include <iostream>
#include <string>

#include "LogBuffer.h"
#include "catch.hpp"

LogBuffer::LogBuffer(LogFileWriter* file_writer)
    : file_writer(file_writer),
      critical_section(new CRITICAL_SECTION)
{
    if (!InitializeCriticalSectionAndSpinCount(critical_section, 0x00000400))
    {
        throw std::runtime_error("InitializeCriticalSectionAndSpinCount failed");
    }
    write_event = CreateEvent(nullptr, TRUE, FALSE, TEXT("WriteEvent"));
    if (write_event == nullptr)
    {
        throw std::runtime_error("CreateEvent failed");
    }
    StartThread();
}

LogBuffer::~LogBuffer()
{
    if (size != 0)
    {
        NotifyThread();
    }

    CloseHandle(thread_handle);
    CloseHandle(write_event);

    assert(critical_section != nullptr);
    DeleteCriticalSection(critical_section);

    Clear();
}

void LogBuffer::Append(const char* message)
{
    assert(critical_section != nullptr);

    EnterCriticalSection(critical_section);

    if (size >= MAX_SIZE)
    {
        NotifyThread();
    }
    else
    {
        size++;
    }

    list.Add(message);

    LeaveCriticalSection(critical_section);
}

std::size_t LogBuffer::Size() const
{
    return size;
}

void LogBuffer::Clear()
{
    list.Clear();
    size = 0;
}

void LogBuffer::StartThread()
{
    thread_handle = CreateThread(nullptr, 0, &ThreadFunc, static_cast<void*>(this), 0, nullptr);
}

void LogBuffer::NotifyThread()
{
    if (!SetEvent(write_event))
    {
        throw std::runtime_error("SetEvent failed");
    }
    DWORD wait_result = WaitForSingleObject(thread_handle, INFINITE);
    switch (wait_result)
    {
    case WAIT_OBJECT_0:
        ResetEvent(write_event);
        StartThread();
        break;
    default:
        throw std::runtime_error("WaitForSingleObject failed");
    }
}

const char* LogBuffer::PopMessage()
{
    const char* result = list.Pop();
    size--;
    return result;
}

DWORD WINAPI LogBuffer::ThreadFunc(CONST LPVOID lp_param)
{
    LogBuffer* context = static_cast<LogBuffer*>(lp_param);
    const DWORD wait_result = WaitForSingleObject(context->write_event, INFINITE);

    if (wait_result == WAIT_OBJECT_0)
    {
        if (context->Size() == 0)
        {
            ExitThread(EXIT_SUCCESS);
        }

        const auto size = context->Size();
        const auto memory_size = context->Size() * sizeof(char*);
        char** buffer = static_cast<char**>(malloc(memory_size));
        for (int i = context->Size() - 1; i >= 0; i--)
        {
            const char* message = context->PopMessage();
            const auto message_memory_size = strlen(message) * sizeof(char);
            buffer[i] = static_cast<char*>(malloc(message_memory_size));
            strcpy(buffer[i], message);
        }

        context->file_writer->Write(buffer, size);
    }

    ExitThread(EXIT_SUCCESS);
}

SCENARIO("LogBuffer")
{
    class TestLogFileWriter : public LogFileWriter
    {
    public:
        char** buffer = nullptr;

        virtual void Write(char** buffer, std::size_t size) override
        {
            const auto buffer_memory_size = size * sizeof(char*);
            this->buffer = static_cast<char**>(malloc(buffer_memory_size));
            for (std::size_t i = 0; i < size; i++)
            {
                const char* message = buffer[i];
                const auto message_memory_size = strlen(message) * sizeof(char);
                this->buffer[i] = static_cast<char*>(malloc(message_memory_size));
                strcpy(this->buffer[i], buffer[i]);
            }
        }
    };

    GIVEN("A test LogFileWriter implementation")
    {
        TestLogFileWriter file_writer;
        LogBuffer* log_buffer = new LogBuffer(&file_writer);

        WHEN("257 messages have been appended")
        {
            for (std::size_t i = 0; i <= 256; i++)
            {
                log_buffer->Append(std::to_string(i).c_str());
            }

            THEN("LogFileWriter contains array of numeric strings from 0 to 256")
            {
                for (std::size_t i = 0; i < 256; i++)
                {
                    char* message = file_writer.buffer[i];
                    REQUIRE(strcmp(message, std::to_string(i).c_str()) == 0);
                }
            }

            AND_WHEN("More 257 messages have been appended")
            {
                for (std::size_t i = 0; i <= 256; i++)
                {
                    log_buffer->Append(std::to_string(i).c_str());
                }

                THEN("LogFileWriter container another array of numeric strings from 0 to 256")
                {
                    for (std::size_t i = 0; i < 256; i++)
                    {
                        char* message = file_writer.buffer[i];
                        REQUIRE(strcmp(message, std::to_string(i).c_str()) == 0);
                    }
                }
            }

            AND_WHEN("Only 256 / 2 new messages have been appended")
            {
                for (std::size_t i = 0; i <= 256 / 2; i++)
                {
                    log_buffer->Append(std::to_string(i).c_str());
                }

                AND_WHEN("Destructor is called")
                {
                    delete log_buffer;

                    THEN("LogFileWriter contains rest of messages")
                    {
                        for (std::size_t i = 0; i < 256 / 2; i++)
                        {
                            char* message = file_writer.buffer[i];
                            assert(strcmp(message, std::to_string(i).c_str()) == 0);
                        }
                    }
                }
            }
        }
    }
}
