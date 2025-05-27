#include "SharedMemory.h"
#include <stdexcept>

SharedMemory::SharedMemory(const std::wstring& name, size_t size)
    : size(size) {
    hMapFile = CreateFileMappingW(
        INVALID_HANDLE_VALUE, nullptr,
        PAGE_READWRITE, 0,
        static_cast<DWORD>(size),
        name.c_str()
    );
    if (!hMapFile) {
        DWORD err = GetLastError();
        throw std::runtime_error("CreateFileMapping failed, err=" + std::to_string(err));
    }

    lpBaseAddress = MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, size);
    if (!lpBaseAddress)
        throw std::runtime_error("Failed to map view of file.");

    VirtualLock(lpBaseAddress, size);
}

SharedMemory::~SharedMemory() {
    VirtualUnlock(lpBaseAddress, size);
    UnmapViewOfFile(lpBaseAddress);
    CloseHandle(hMapFile);
}

void* SharedMemory::getData() {
    return lpBaseAddress;
}

size_t SharedMemory::getSize() const {
    return size;
}
