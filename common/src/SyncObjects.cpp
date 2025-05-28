// SyncObjects.h and SyncObjects.cpp: обёртка над Windows-мьютексом для синхронизации

#include "SyncObjects.h"
#include <stdexcept>

// Конструктор: создаёт или открывает именованный мьютекс в системе
SyncObjects::SyncObjects(const std::wstring& mutexName) {
    // Создаём мьютекс. nullptr — без атрибутов безопасности, FALSE — сразу не захватывать
    // mutexName.c_str() — уникальное имя мьютекса для межпроцессной синхронизации
    hMutex = CreateMutexW(nullptr, FALSE, mutexName.c_str());
    if (!hMutex)
        // Если не удалось получить дескриптор, бросаем исключение
        throw std::runtime_error("Failed to create mutex.");
}

// Деструктор: закрывает дескриптор мьютекса
SyncObjects::~SyncObjects() {
    // Освобождаем системный ресурс мьютекса
    CloseHandle(hMutex);
}

// Захват мьютекса: вход в критическую секцию
void SyncObjects::lock() {
    // Ожидаем бесконечно (INFINITE), пока мьютекс не станет доступен
    WaitForSingleObject(hMutex, INFINITE);
}

// Освобождение мьютекса: выход из критической секции
void SyncObjects::unlock() {
    ReleaseMutex(hMutex);
}
