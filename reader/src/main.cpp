// reader.cpp: процесс-чтение (consumer)
#include <windows.h>
#include <mmsystem.h>    // для timeGetTime
#include <iostream>
#include <string>
#include <random>

#include "SharedMemory.h"    // общая память
#include "SyncObjects.h"     // мьютексы
#include "Logger.h"          // логирование

#pragma comment(lib, "winmm.lib")  // линковка библиотеки для timeGetTime

int main(int argc, char* argv[]) {
    // Имена объектов разделяемой памяти и мьютексов для читателей/писателя
    const std::wstring shmName        = L"Lab4_SharedMem";
    const std::wstring mxReadersName  = L"Lab4_ReadersMutex";
    const std::wstring mxWriteName    = L"Lab4_WriteMutex";

    // Проверка аргументов: ожидаем идентификатор процесса
    if (argc < 2) {
        std::cerr << "Usage: reader.exe <id>\n";
        return 1;
    }
    const std::string idStr   = argv[1];
    const std::string logName = "reader_" + idStr + ".log";

    // Размер страницы системы
    SYSTEM_INFO si;
    GetSystemInfo(&si);
    size_t pageSize = si.dwPageSize;

    // Параметры разделяемой памяти
    size_t pageCount   = 12;
    size_t controlSize = sizeof(int);                    // под счётчик читателей
    size_t shmSize     = controlSize + pageCount * pageSize;

    // Открываем разделяемую память
    SharedMemory shm(shmName, shmSize);
    char* base = static_cast<char*>(shm.getData());    // указатель на начало
    int* readersCount = reinterpret_cast<int*>(base);  // счётчик читателей хранится в первых 4 байтах
  
    // Создаём/открываем мьютексы
    SyncObjects mxReaders(mxReadersName);  // защищает счётчик читателей
    SyncObjects mxWrite(mxWriteName);      // синхронизирует с писателем

    // Логгер для записи событий
    Logger logger(logName);

    // Устанавливаем разрешение таймера в 1 мс
    timeBeginPeriod(1);

    // Генерация случайных задержек по времени
    std::mt19937 gen((unsigned)timeGetTime());
    std::uniform_int_distribution<> dist(500, 1500);

    // Основной цикл: 10 итераций чтения
    for (int iter = 0; iter < 10; ++iter) {
        logger.log("WAITING to read");
        // Вход в раздел чтения: увеличиваем счётчик
        mxReaders.lock();
        if ((*readersCount)++ == 0) {
            // если это первый читатель — блокируем писателя
            mxWrite.lock();
        }
        mxReaders.unlock();

        // Выполняем чтение
        int page = gen() % pageCount;
        logger.log("START READING page " + std::to_string(page));
        Sleep(dist(gen));                             // имитация операции чтения
        logger.log("END READING   page " + std::to_string(page));

        // Выход из раздела чтения: уменьшаем счётчик
        mxReaders.lock();
        if (--(*readersCount) == 0) {
            // если был последний читатель — разблокируем писателя
            mxWrite.unlock();
        }
        mxReaders.unlock();
        logger.log("RELEASED reading");
    }

    // Возврат периода таймера
    timeEndPeriod(1);
    return 0;
}
