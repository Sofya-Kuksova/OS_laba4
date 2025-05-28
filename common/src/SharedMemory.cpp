#include "SharedMemory.h"
#include <stdexcept>

SharedMemory::SharedMemory(const std::wstring& name, size_t size) 
    : m_handle(NULL), m_data(NULL), m_createdNew(false) {
    
    // Пытаемся открыть существующий объект
    m_handle = OpenFileMappingW(FILE_MAP_ALL_ACCESS, FALSE, name.c_str());
    
    if (m_handle == NULL) {
        // Создаем новый объект
        m_handle = CreateFileMappingW(
            INVALID_HANDLE_VALUE,
            NULL,
            PAGE_READWRITE,
            0,
            static_cast<DWORD>(size),
            name.c_str()
        );
        
        if (m_handle == NULL) {
            throw std::runtime_error("CreateFileMapping failed");
        }
        m_createdNew = true;  // Устанавливаем флаг создания
    }
    
    // Отображаем память в адресное пространство
    m_data = MapViewOfFile(m_handle, FILE_MAP_ALL_ACCESS, 0, 0, size);
    if (m_data == NULL) {
        CloseHandle(m_handle);
        throw std::runtime_error("MapViewOfFile failed");
    }
}

SharedMemory::~SharedMemory() {
    if (m_data) UnmapViewOfFile(m_data);
    if (m_handle) CloseHandle(m_handle);
}
