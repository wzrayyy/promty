cmake_minimum_required(VERSION 3.25)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_BUILD_TYPE Release)
project(promt-puppy CXX)

file(GLOB sources src/*.cpp)
add_executable(${PROJECT_NAME}
    app/main.cpp
    ${sources}
)

set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++")
target_link_libraries(${PROJECT_NAME} ole32 ws2_32 oleaut32 uuid)
target_link_libraries(${PROJECT_NAME} winpthread -static)

target_include_directories(${PROJECT_NAME} PUBLIC include)
