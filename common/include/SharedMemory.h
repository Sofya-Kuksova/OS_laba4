#pragma once
#include <windows.h>
#include <string>

class SharedMemory {
public:
    SharedMemory(const std::wstring& name, size_t size);
    ~SharedMemory();
    
    void* getData() const { return m_data; }
    bool createdNew() const { return m_createdNew; }  // Добавлен новый метод

private:
    HANDLE m_handle;
    void* m_data;
    bool m_createdNew;  // Флаг создания нового объекта
};
