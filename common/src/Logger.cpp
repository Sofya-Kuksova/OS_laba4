#include "Logger.h"
#include <windows.h>
#include <sstream>

Logger::Logger(const std::string& filename) {
    out.open(filename, std::ios::app);
}

Logger::~Logger() {
    out.close();
}

void Logger::log(const std::string& message) {
    DWORD time = timeGetTime();
    std::ostringstream ss;
    ss << "[" << time << " ms] " << message << "\n";
    out << ss.str();
    out.flush();
}
