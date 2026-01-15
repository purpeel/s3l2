cmake_minimum_required(VERSION 3.10)
project(lab2)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(inc/tmplInc inc/utilInc src/tmpl src/util inc/Benchmark)

find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

add_executable(vfs-app vfs-app/main.cpp)
add_executable(test-lab2 test-main.cpp)

add_executable(unit-tests tests/unit_tests.cpp)
target_link_libraries(unit-tests ${GTEST_LIBRARIES} pthread)

add_executable(benchmarks inc/Benchmark/main.cpp)

set(COMMON_COMPILE_OPTIONS
    $<$<CONFIG:Debug>:
        -g -O0
        -Wall -Wextra -Werror
        -fsanitize=address,undefined
        -fno-omit-frame-pointer
    >
    $<$<CONFIG:Release>:
        -O3 -DNDEBUG
        -Wall -Wextra -Werror
    >
)

set(COMMON_LINK_OPTIONS
    $<$<CONFIG:Debug>:
        -fsanitize=address,undefined
    >
)

set(ALL_TARGETS vfs-app test-lab2 unit-tests benchmarks)

foreach(t ${ALL_TARGETS})
    target_compile_options(${t} PRIVATE ${COMMON_COMPILE_OPTIONS})
    target_link_options(${t} PRIVATE ${COMMON_LINK_OPTIONS})
endforeach()
