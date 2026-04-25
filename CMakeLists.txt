cmake_minimum_required(VERSION 4.0)

project(trychainlib C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_library(trychain STATIC src/libtrychain.c)

target_include_directories(trychain PUBLIC ${PROJECT_SOURCE_DIR}/include)

add_executable(testing_exec libtests/main.c)

target_link_libraries(testing_exec PRIVATE trychain)