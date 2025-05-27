cmake_minimum_required(VERSION 3.15)
project(lab4_ipc LANGUAGES CXX)

# 1) Сборка библиотеки common
add_library(common STATIC
    common/src/SharedMemory.cpp
    common/src/SyncObjects.cpp
    common/src/Logger.cpp
)
target_include_directories(common PUBLIC
    ${CMAKE_SOURCE_DIR}/common/include
)

# 2) Сборка reader.exe
add_executable(reader
    reader/src/main.cpp
)
target_include_directories(reader PUBLIC
    ${CMAKE_SOURCE_DIR}/common/include
    ${CMAKE_SOURCE_DIR}/reader/include
)
target_link_libraries(reader
    PRIVATE
        common
        winmm        # ← подключаем библиотеку для timeGetTime и др.
)

# 3) Сборка writer.exe
add_executable(writer
    writer/src/main.cpp
)
target_include_directories(writer PUBLIC
    ${CMAKE_SOURCE_DIR}/common/include
    ${CMAKE_SOURCE_DIR}/writer/include
)
target_link_libraries(writer
    PRIVATE
        common
        winmm        # ← подключаем библиотеку для timeGetTime и др.
)
