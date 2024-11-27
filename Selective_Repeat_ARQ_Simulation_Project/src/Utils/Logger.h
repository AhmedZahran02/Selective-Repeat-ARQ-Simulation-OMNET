#pragma once

#ifndef UTILS_LOGGER_H_
#define UTILS_LOGGER_H_

#include <fstream>
#include <iostream>
#include <string>
#include <mutex>
#include <memory>

class Logger
{
public:
    static Logger &getInstance(const std::string &filePath = "../output/output.txt")
    {
        static Logger instance(filePath);
        return instance;
    }

    std::fstream &GetFileStream()
    {
        std::lock_guard<std::mutex> lock(mtx); // Ensure thread safety if used in a multithreaded environment

        if (!fileStream)
        {
            fileStream.reset(new std::fstream(filePath, std::ios::out | std::ios::trunc)); // Open in "write" mode, which clears the file
            if (fileStream->is_open())
            {
                EV << "File cleared and opened for writing: " << filePath << std::endl;
            }
            else
            {
                EV << "Error opening file " << filePath << std::endl;
            }
        }
        else
        {
            fileStream.reset(new std::fstream(filePath, std::ios::out | std::ios::app));
            if (!fileStream->is_open())
            {
                EV << "Error opening file " << filePath << std::endl;
            }
        }
        return *fileStream;
    }

private:
    std::string filePath;
    std::unique_ptr<std::fstream> fileStream;
    std::mutex mtx;

    Logger(const std::string &path) : filePath(path), fileStream(nullptr) {}
    ~Logger()
    {
        if (fileStream && fileStream->is_open())
            fileStream->close();
    }

    Logger(const Logger &) = delete;
    Logger &operator=(const Logger &) = delete;
};

#endif
