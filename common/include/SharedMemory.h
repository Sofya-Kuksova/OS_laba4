#pragma once
#include <windows.h>
#include <string>

class SharedMemory {
public:
    SharedMemory(const std::wstring& name, size_t size);
    ~SharedMemory();

    void* getData();
    size_t getSize() const;

private:
    HANDLE hMapFile;
    LPVOID lpBaseAddress;
    size_t size;
};
