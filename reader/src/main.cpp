#include <windows.h>
#include <mmsystem.h>    // для timeGetTime
#include <iostream>
#include <string>
#include <random>

#include "SharedMemory.h"
#include "SyncObjects.h"
#include "Logger.h"

#pragma comment(lib, "winmm.lib")  // чтобы линковать timeGetTime

int main(int argc, char* argv[]) {
    // Параметры
    const std::wstring shmName    = L"Lab4_SharedMem";
    const std::wstring mxReadersName = L"Lab4_ReadersMutex";
    const std::wstring mxWriteName   = L"Lab4_WriteMutex";
    if (argc < 2) {
        std::cerr << "Usage: reader.exe <id>\n";
        return 1;
    }
    const std::string idStr   = argv[1];
    const std::string logName = "reader_" + idStr + ".log";

    // Размер страницы
    SYSTEM_INFO si;
    GetSystemInfo(&si);
    size_t pageSize = si.dwPageSize;

    size_t pageCount   = 12;
    size_t controlSize = sizeof(int);
    size_t shmSize     = controlSize + pageCount * pageSize;

    // Общая память
    SharedMemory shm(shmName, shmSize);
    char* base = static_cast<char*>(shm.getData());
    int* readersCount = reinterpret_cast<int*>(base);
    char* buffer      = base + controlSize;

    // Синхронизаторы
    SyncObjects mxReaders(mxReadersName);
    SyncObjects mxWrite(mxWriteName);

    // Логгер
    Logger logger(logName);

    // Таймеры timeGetTime
    timeBeginPeriod(1);

    // Генерация случайных задержек
    std::mt19937 gen((unsigned)timeGetTime());
    std::uniform_int_distribution<> dist(500, 1500);

    for (int iter = 0; iter < 10; ++iter) {
        logger.log("WAITING to read");
        mxReaders.lock();
        if ((*readersCount)++ == 0) {
            mxWrite.lock();
        }
        mxReaders.unlock();

        int page = gen() % pageCount;
        logger.log("START READING page " + std::to_string(page));
        Sleep(dist(gen));
        logger.log("END READING   page " + std::to_string(page));

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