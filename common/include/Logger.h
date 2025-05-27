#pragma once
#include <string>
#include <fstream>

class Logger {
public:
    Logger(const std::string& filename);
    ~Logger();

    void log(const std::string& message);

private:
    std::ofstream out;
};
