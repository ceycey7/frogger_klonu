cmake_minimum_required(VERSION 3.10)
project(frogger_klonu C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

find_package(PkgConfig REQUIRED)
pkg_check_modules(ALLEGRO REQUIRED
    allegro-5
    allegro_primitives-5
    allegro_font-5
)

add_executable(frogger_klonu main.c)

target_include_directories(frogger_klonu PRIVATE ${ALLEGRO_INCLUDE_DIRS})
target_link_libraries(frogger_klonu PRIVATE ${ALLEGRO_LIBRARIES})
target_compile_options(frogger_klonu PRIVATE ${ALLEGRO_CFLAGS_OTHER})