// writer.cpp: процесс-запись (producer)
#include <windows.h>
#include <mmsystem.h>    // для функции timeGetTime
#include <iostream>
#include <string>
#include <random>

#include "SharedMemory.h"    // общая память
#include "SyncObjects.h"     // мьютексы
#include "Logger.h"          // логирование

#pragma comment(lib, "winmm.lib")  // линковка библиотеки для timeGetTime

int main(int argc, char* argv[]) {
    // Имя объекта разделяемой памяти и мьютекса для записи
    const std::wstring shmName      = L"Lab4_SharedMem";
    const std::wstring mxWriteName = L"Lab4_WriteMutex";

    // Проверка аргументов: ожидаем идентификатор процесса
    if (argc < 2) {
        std::cerr << "Usage: writer.exe <id>\n";
        return 1;
    }
    // Строка идентификатора для именования лог-файла
    const std::string idStr    = argv[1];
    const std::string logName  = "writer_" + idStr + ".log";

    // Получение размера системной страницы
    SYSTEM_INFO si;
    GetSystemInfo(&si);
    size_t pageSize = si.dwPageSize;  // обычно 4096 байт

    // Параметры буфера: число страниц и служебный размер (счётчик)
    size_t pageCount   = 12;
    size_t controlSize = sizeof(int);                    // резерв под счётчик читателей
    size_t shmSize     = controlSize + pageCount * pageSize;  // общий размер

    // Создаём или открываем разделяемую память
    SharedMemory shm(shmName, shmSize);

    // Мьютекс записи: защищает буфер от одновременной записи и чтения
    SyncObjects mxWrite(mxWriteName);
    // Логгер для записи событий в файл
    Logger logger(logName);

    // Устанавливаем период таймера времени в 1 мс для timeGetTime
    timeBeginPeriod(1);
    // Генератор случайных чисел: сидируемся по текущему времени
    std::mt19937 gen((unsigned)timeGetTime());
    // Равномерное распределение: задержки от 500 до 1500 мс
    std::uniform_int_distribution<> dist(500, 1500);

    // Основной цикл: имитируем 10 циклов записи
    for (int iter = 0; iter < 10; ++iter) {
        logger.log("WAITING to write");            // ожидаем разрешения на запись
        mxWrite.lock();                              // захватываем мьютекс записи

        int page = gen() % pageCount;                // случайная страница для записи
        logger.log("START WRITING page " + std::to_string(page));
        Sleep(dist(gen));                            // имитация операции записи
        logger.log("END WRITING   page " + std::to_string(page));

        mxWrite.unlock();                            // освобождаем мьютекс
        logger.log("RELEASED writing");
    }

    // Восстанавливаем период таймера по умолчанию
    timeEndPeriod(1);
    return 0;
}
