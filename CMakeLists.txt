cmake_minimum_required(VERSION 4.0)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_library(trytracelib STATIC src/trychainlib.c)

target_include_directories(trytracelib PUBLIC ${PROJECT_SOURCE_DIR}/include)

add_executable(testing_exec libtests/main.c)

target_link_libraries(testing_exec PRIVATE trytracelib)