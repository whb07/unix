cmake_minimum_required(VERSION 3.20)
project(unix C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_COMPILER  "clang-12")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Wundef -Wpointer-arith -Wconversion -Werror=implicit-function-declaration -Werror=missing-prototypes")

add_library(lib
        src/malloc.c
#        src/alloc.c
        src/bio.c
        src/bio.h src/malloc.h src/slp.h src/prf.h src/alloc.h src/alloc.c
        src/subr.h src/iget.h src/pipe.h src/clock.h src/clock.c
        src/asmcalls.h src/sig.h src/fio.c src/fio.h src/nami.h src/iget.c src/maths.h src/rdwri.h src/nami.c)

# Define headers for this library. PUBLIC headers are used for
# compiling the library, and will be added to consumers' build
# paths.
target_include_directories(lib PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
        PRIVATE src)


add_executable(unix main.c)
