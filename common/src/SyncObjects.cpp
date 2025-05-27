#include "SyncObjects.h"
#include <stdexcept>

SyncObjects::SyncObjects(const std::wstring& mutexName) {
    hMutex = CreateMutexW(nullptr, FALSE, mutexName.c_str());
    if (!hMutex)
        throw std::runtime_error("Failed to create mutex.");
}

SyncObjects::~SyncObjects() {
    CloseHandle(hMutex);
}

void SyncObjects::lock() {
    WaitForSingleObject(hMutex, INFINITE);
}

void SyncObjects::unlock() {
    ReleaseMutex(hMutex);
}
