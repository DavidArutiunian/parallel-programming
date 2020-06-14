#pragma once

#include <cstdlib>
#include <string>
#include <fstream>

class LogFileWriter
{
    std::ofstream output;
public:
    LogFileWriter() = default;

    LogFileWriter(std::string path);

    virtual ~LogFileWriter() = default;

    virtual void Write(char** buffer, std::size_t size);
};
