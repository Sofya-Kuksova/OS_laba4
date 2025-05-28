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
    const std::wstring mxReadersName  = L"Lab4_ReadersMutex";
    const std::wstring mxWriteName    = L"Lab4_WriteMutex";
    if (argc < 2) {
        std::cerr << "Usage: writer.exe <id>\n";
        return 1;
    }
    const std::string idStr    = argv[1];
    const std::string logName  = "writer_" + idStr + ".log";

    SYSTEM_INFO si;
    GetSystemInfo(&si);
    size_t pageSize = si.dwPageSize;

    size_t pageCount   = 12;
    size_t controlSize = sizeof(int) + pageCount * sizeof(char);
    size_t shmSize     = controlSize + pageCount * pageSize;

    SharedMemory shm(shmName, shmSize);
    char* base = static_cast<char*>(shm.getData());
    int* readersCount = reinterpret_cast<int*>(base);
    char* pageStates = base + sizeof(int);
    char* buffer = base + controlSize;

    // Инициализация состояний страниц при первом создании памяти
    if (shm.createdNew()) {
        for (int i = 0; i < pageCount; ++i) {
            pageStates[i] = 0; // Изначально все страницы ЧИСТЫЕ
        }
    }

    SyncObjects mxWrite(mxWriteName);
    Logger logger(logName);

    timeBeginPeriod(1);
    std::mt19937 gen((unsigned)timeGetTime());
    std::uniform_int_distribution<> dist(500, 1500);

    for (int iter = 0; iter < 10; ++iter) {
        logger.log("WAITING to write");
        mxWrite.lock();

        // Ищем ЧИСТУЮ страницу (состояние 0) для записи
        int page = -1;
        int attempts = pageCount * 2;
        while (attempts-- > 0) {
            int candidate = gen() % pageCount;
            if (pageStates[candidate] == 0) {
                page = candidate;
                break;
            }
        }

        if (page != -1) {
            logger.log("START WRITING page " + std::to_string(page));
            Sleep(dist(gen));
            logger.log("END WRITING   page " + std::to_string(page));
            // После записи помечаем страницу как ГРЯЗНУЮ (1)
            pageStates[page] = 1;
        } else {
            logger.log("SKIPPED WRITING - no clean pages available");
        }

        mxWrite.unlock();
        logger.log("RELEASED writing");
    }

    timeEndPeriod(1);
    return 0;
}
