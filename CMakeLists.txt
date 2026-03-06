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

file(GLOB_RECURSE SOURCES "src/*.cpp" "src/*.h")

add_executable(MyGB ${SOURCES})

# --- SDL2 ---
find_package(SDL2 REQUIRED)

# 2. Incluye los directorios de las cabeceras (.h)
target_include_directories(MyGB PRIVATE ${SDL2_INCLUDE_DIRS})

# 3. Linkea los binarios precompilados (.so / .a) a tu ejecutable
target_link_libraries(MyGB PRIVATE ${SDL2_LIBRARIES})