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
        std::cerr << "Usage: reader.exe <id>\n";
        return 1;
    }
    const std::string idStr   = argv[1];
    const std::string logName = "reader_" + idStr + ".log";

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

    SyncObjects mxReaders(mxReadersName);
    SyncObjects mxWrite(mxWriteName);
    Logger logger(logName);

    timeBeginPeriod(1);
    std::mt19937 gen((unsigned)timeGetTime());
    std::uniform_int_distribution<> dist(500, 1500);

    for (int iter = 0; iter < 10; ++iter) {
        logger.log("WAITING to read");
        
        mxReaders.lock();
        if ((*readersCount)++ == 0) {
            mxWrite.lock();
        }
        mxReaders.unlock();

        // Ищем ГРЯЗНУЮ страницу (состояние 1) для чтения
        int page = -1;
        int attempts = pageCount * 2;
        while (attempts-- > 0) {
            int candidate = gen() % pageCount;
            if (pageStates[candidate] == 1) {
                page = candidate;
                break;
            }
        }

        if (page != -1) {
            logger.log("START READING page " + std::to_string(page));
            Sleep(dist(gen));
            logger.log("END READING   page " + std::to_string(page));
            // После чтения помечаем страницу как ЧИСТУЮ (0)
            pageStates[page] = 0;
        } else {
            logger.log("SKIPPED READING - no dirty pages available");
        }

        mxReaders.lock();
        if (--(*readersCount) == 0) {
            mxWrite.unlock();
        }
        mxReaders.unlock();
        logger.log("RELEASED reading");
    }

    timeEndPeriod(1);
    return 0;
}
