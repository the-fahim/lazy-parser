
# CMakeLists.txt — LazyParser
cmake_minimum_required(VERSION 3.20)
project(LazyParser VERSION 1.0.0 LANGUAGES CXX)

# ── C++23 ───────────────────────────────────────────────────────────────────
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ── Build type ───────────────────────────────────────────────────────────────
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type" FORCE)
endif()

# ── Boost ────────────────────────────────────────────────────────────────────
find_package(Boost REQUIRED COMPONENTS
    filesystem
    system
    program_options
    regex
    container
    algorithm
)

# ── Options ──────────────────────────────────────────────────────────────────
option(BUILD_TESTS    "Build tests"              ON)
option(BUILD_EXAMPLES "Build examples"           ON)
option(ENABLE_COVERAGE "Enable coverage reporting" OFF)

# ── Output directories ───────────────────────────────────────────────────────
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# ── Include dirs ─────────────────────────────────────────────────────────────
include_directories(${Boost_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# ── Library ──────────────────────────────────────────────────────────────────
add_library(lazyparser_lib STATIC
    src/parser.cpp
    src/grammar.cpp
    src/symbol.cpp
    src/state.cpp
    src/action.cpp
)

target_compile_definitions(lazyparser_lib PRIVATE LAZYPARSER_BUILDING)
target_include_directories(lazyparser_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${Boost_INCLUDE_DIRS}
)
target_link_libraries(lazyparser_lib PUBLIC ${Boost_LIBRARIES})

# ── Main executable ───────────────────────────────────────────────────────────
add_executable(lazyparser src/main.cpp)
target_link_libraries(lazyparser PRIVATE lazyparser_lib)

# ── Generator tool ────────────────────────────────────────────────────────────
add_executable(lazyparser_generator tools/lazyparser_generator.cpp)
target_link_libraries(lazyparser_generator PRIVATE lazyparser_lib)

# ── Tests ─────────────────────────────────────────────────────────────────────
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# ── Examples ──────────────────────────────────────────────────────────────────
if(BUILD_EXAMPLES)
    # Examples contain Python scripts only — no C++ targets needed here
    # add_subdirectory(examples)
endif()

# ── Install ───────────────────────────────────────────────────────────────────
install(TARGETS lazyparser lazyparser_lib lazyparser_generator
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

install(DIRECTORY include/lazyparser
    DESTINATION include
)

# ── Coverage ──────────────────────────────────────────────────────────────────
if(ENABLE_COVERAGE)
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR
       CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        target_compile_options(lazyparser_lib PRIVATE --coverage)
        target_link_options(lazyparser_lib    PRIVATE --coverage)
    endif()
endif()