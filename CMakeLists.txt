cmake_minimum_required(VERSION 3.10)
project(lab2)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(inc/tmplInc inc/utilInc src/tmpl src/util)

add_executable(lab2 main.cpp)

target_compile_options(lab2 PRIVATE
    $<$<CONFIG:Debug>:
        -g
        -O0
        -Wall -Wextra -Werror
        # -fsanitize=address,undefined
        # -fno-omit-frame-pointer
    >
    $<$<CONFIG:Release>:
        -O3
        -DNDEBUG
        -Wall -Wextra -Werror
    >
)

# target_link_options(lab2 PRIVATE
#     $<$<CONFIG:Debug>:
#         -fsanitize=address,undefined
#     >
# )