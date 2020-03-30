#pragma once

class LogFileWriter
{
public:
    virtual ~LogFileWriter() = default;

    virtual void Write(char** buffer, std::size_t size) = 0;
};
