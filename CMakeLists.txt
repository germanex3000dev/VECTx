cmake_minimum_required(VERSION 3.16)

project(VECTx
    VERSION 0.1.0
    DESCRIPTION "A small dynamically typed scripting language"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

add_library(vectx_core
    src/Lexer.cpp
    src/Parser.cpp
    src/Interpreter.cpp
    src/Environment.cpp
    src/Value.cpp
)

target_include_directories(vectx_core PUBLIC src)

add_executable(vectx src/main.cpp)
target_link_libraries(vectx PRIVATE vectx_core)

if(MSVC)
    target_compile_options(vectx_core PRIVATE /W4)
    target_compile_options(vectx PRIVATE /W4)
else()
    target_compile_options(vectx_core PRIVATE -Wall -Wextra)
    target_compile_options(vectx PRIVATE -Wall -Wextra)
endif()

# Keep the binary inside the build tree. A stale `vectx` binary is checked in
# at the repository root, and an in-source build would overwrite it.
set_target_properties(vectx PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

enable_testing()

add_executable(vectx_tests tests/test_language.cpp)
target_link_libraries(vectx_tests PRIVATE vectx_core)

if(MSVC)
    target_compile_options(vectx_tests PRIVATE /W4)
else()
    target_compile_options(vectx_tests PRIVATE -Wall -Wextra)
endif()

add_test(NAME language COMMAND vectx_tests)
