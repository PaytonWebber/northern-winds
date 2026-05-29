cmake_minimum_required(VERSION 3.14)
set(CMAKE_C_STANDARD 11)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Copy compile_commands.json to project root for clangd
add_custom_target(copy-compile-commands ALL
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_BINARY_DIR}/compile_commands.json
        ${CMAKE_SOURCE_DIR}/compile_commands.json
)

set(ENVSDK $ENV{PLAYDATE_SDK_PATH})

if (NOT ${ENVSDK} STREQUAL "")
    file(TO_CMAKE_PATH ${ENVSDK} SDK)
else()
    execute_process(
        COMMAND bash -c "egrep '^\\s*SDKRoot' $HOME/.Playdate/config"
        COMMAND head -n 1
        COMMAND cut -c9-
        OUTPUT_VARIABLE SDK
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
endif()

if (NOT EXISTS ${SDK})
    message(FATAL_ERROR "SDK Path not found; set ENV value PLAYDATE_SDK_PATH")
    return()
endif()

set(CMAKE_CONFIGURATION_TYPES "Debug;Release")
set(CMAKE_XCODE_GENERATE_SCHEME TRUE)

set(PLAYDATE_GAME_NAME northern-winds)
set(PLAYDATE_GAME_DEVICE northern-winds_DEVICE)

project(${PLAYDATE_GAME_NAME} C ASM)

file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS src/*.c)

if (TOOLCHAIN STREQUAL "armgcc")
    add_executable(${PLAYDATE_GAME_DEVICE} ${SRC_FILES})
    set(GAME_TARGET ${PLAYDATE_GAME_DEVICE})
else()
    add_library(${PLAYDATE_GAME_NAME} SHARED ${SRC_FILES})
    set(GAME_TARGET ${PLAYDATE_GAME_NAME})
endif()

target_compile_options(${GAME_TARGET} PRIVATE
    -Wall
    -Wextra
    -Wmissing-field-initializers
    -Werror=implicit-function-declaration
    -Wstrict-prototypes
    -Wmissing-prototypes
    -Wdouble-promotion
    -Wformat=2
    -Wshadow
    -Wconversion
)

target_include_directories(${GAME_TARGET} SYSTEM PRIVATE ${SDK}/C_API)

include(${SDK}/C_API/buildsupport/playdate_game.cmake)
