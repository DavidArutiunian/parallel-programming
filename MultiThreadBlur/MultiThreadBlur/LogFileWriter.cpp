#include "LogFileWriter.h"

LogFileWriter::LogFileWriter(std::string path): output(std::ofstream(path))
{
}

void LogFileWriter::Write(char ** buffer, std::size_t size)
{
    for (std::size_t i = 0; i < size; i++)
    {
        output << buffer[i];
    }
}
