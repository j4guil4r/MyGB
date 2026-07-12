cmake_minimum_required(VERSION 3.28)
project(MyGB VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Buscamos SDL2
find_package(SDL2 REQUIRED)

enable_testing()

set(CORE_SOURCES "src/cpu.cpp" "src/bus.cpp" "src/ppu.cpp" "src/cartridge.cpp")

# ---  EMULADOR (Tetris) ---
add_executable(MyGB src/main.cpp src/ui.cpp ${CORE_SOURCES})
target_include_directories(MyGB PRIVATE ${SDL2_INCLUDE_DIRS} "src")
target_link_libraries(MyGB PRIVATE ${SDL2_LIBRARIES})

# --- LOS TESTS (Blargg) ---
add_executable(MyGB_Tests tests/main_test.cpp ${CORE_SOURCES})
target_include_directories(MyGB_Tests PRIVATE "src")


add_test(NAME BlarggTests COMMAND MyGB_Tests)