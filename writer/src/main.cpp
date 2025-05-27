#include <windows.h>
#include <mmsystem.h>
#include <iostream>
#include <string>
#include <random>

#include "SharedMemory.h"
#include "SyncObjects.h"
#include "Logger.h"

#pragma comment(lib, "winmm.lib")

int main(int argc, char* argv[]) {
    const std::wstring shmName  = L"Lab4_SharedMem";
    const std::wstring mxWriteName = L"Lab4_WriteMutex";
    // Проверяем, что передан идентификатор процесса
    if (argc < 2) {
        std::cerr << "Usage: writer.exe <id>\n";
        return 1;
    }
    const std::string idStr    = argv[1];
    const std::string  logName = "writer_" + idStr + ".log";

    SYSTEM_INFO si;
    GetSystemInfo(&si);
    size_t pageSize = si.dwPageSize;

    size_t pageCount   = 12;
    size_t controlSize = sizeof(int);
    size_t shmSize     = controlSize + pageCount * pageSize;

    SharedMemory shm(shmName, shmSize);
    char* base   = static_cast<char*>(shm.getData());
    char* buffer = base + controlSize;

    SyncObjects mxWrite(mxWriteName);
    Logger logger(logName);

    timeBeginPeriod(1);
    std::mt19937 gen((unsigned)timeGetTime());
    std::uniform_int_distribution<> dist(500, 1500);

    for (int iter = 0; iter < 10; ++iter) {
        logger.log("WAITING to write");
        mxWrite.lock();

        int page = gen() % pageCount;
        logger.log("START WRITING page " + std::to_string(page));
        Sleep(dist(gen));
        logger.log("END WRITING   page " + std::to_string(page));

        mxWrite.unlock();
        logger.log("RELEASED writing");
    }

    timeEndPeriod(1);
    return 0;
}