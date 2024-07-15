cmake_minimum_required(VERSION 3.0)

project(shellnova C ASM_NASM)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")

option(ENABLE_LOG "Enable logging")

configure_file("src/build.h.in" "build.h")

if(FORMAT_SHELLCODE)

## Set our custom linker script
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wl,-T${CMAKE_CURRENT_SOURCE_DIR}/src/linker.ld")

## Help debugging linker script sections
# set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wl,--print-map")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O0")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -nostdlib")

endif()


list(APPEND SRC_FILES
    src/main.c

    src/basic_z.h
    src/basic_z.c

    src/lib_d.h
    src/lib_d.c

    src/libc_d.h
    src/libc_d.c

    src/linker.h

    src/log.h

    src/print.h

    src/start.asm
    src/start.c
    src/start.h

    src/syscall_z.asm
    src/syscall_z.c
    src/syscall_z.h
)

if(ENABLE_LOG)
  list(APPEND SRC_FILES src/log.c)
endif()


add_executable(shellnova ${SRC_FILES})

target_include_directories(shellnova PUBLIC "${CMAKE_CURRENT_BINARY_DIR}")

if(FORMAT_SHELLCODE)
  add_custom_target(
    shc ALL
    "${CMAKE_CURRENT_SOURCE_DIR}/scripts/extract-shellcode.py"
    "shellnova" "shc.bin"
  )
  add_dependencies(shc shellnova)
endif()
