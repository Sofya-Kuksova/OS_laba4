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
    const std::wstring shmName        = L"Lab4_SharedMem";
    const std::wstring mxWriteName    = L"Lab4_WriteMutex";
    if (argc < 2) {
        std::cerr << "Usage: writer.exe <id>\n";
        return 1;
    }
    const std::string idStr   = argv[1];
    const std::string logName = "writer_" + idStr + ".log";

    SYSTEM_INFO si;
    GetSystemInfo(&si);
    size_t pageSize = si.dwPageSize;

    const size_t pageCount   = 12;
    const size_t controlSize = sizeof(int) + pageCount * sizeof(char);
    const size_t shmSize     = controlSize + pageCount * pageSize;

    SharedMemory shm(shmName, shmSize);
    char* base = static_cast<char*>(shm.getData());
    int* readersCount = reinterpret_cast<int*>(base);
    char* pageStates = base + sizeof(int);

    // Инициализация состояний при создании
    if (shm.createdNew()) {
        for (size_t i = 0; i < pageCount; ++i) pageStates[i] = 0;
    }

    SyncObjects mxWrite(mxWriteName);
    Logger logger(logName);

    timeBeginPeriod(1);
    std::mt19937 gen((unsigned)timeGetTime());
    std::uniform_int_distribution<> distTime(500, 1500);
    std::uniform_int_distribution<> distPage(0, pageCount - 1);

    for (int iter = 0; iter < 10; ++iter) {
        logger.log("WAITING to write");
        mxWrite.lock();

        // Одна итерация выбора страницы:
        int page = -1;
        // 1. Случайная попытка
        int candidate = distPage(gen);
        if (pageStates[candidate] == 0) {
            page = candidate;
        } else {
            // 2. Линейный обход всех страниц
            for (size_t i = 0; i < pageCount; ++i) {
                if (pageStates[i] == 0) {
                    page = static_cast<int>(i);
                    break;
                }
            }
        }

        if (page != -1) {
            logger.log("START WRITING page " + std::to_string(page));
            Sleep(distTime(gen));
            logger.log("END WRITING   page " + std::to_string(page));
            pageStates[page] = 1;  // грязная
        } else {
            logger.log("SKIPPED WRITING - no clean pages available");
        }

        mxWrite.unlock();
        logger.log("RELEASED writing");
    }

    timeEndPeriod(1);
    return 0;
}
