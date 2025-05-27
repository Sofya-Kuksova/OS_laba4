#pragma once
#include <windows.h>
#include <string>

class SyncObjects {
public:
    SyncObjects(const std::wstring& mutexName);
    ~SyncObjects();

    void lock();
    void unlock();

private:
    HANDLE hMutex;
};
