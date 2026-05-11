cmake_minimum_required(VERSION 3.14)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
project(endless_afternoon)

if(CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_CONFIGURATION_TYPES
        Debug
        Release
        RelWithDebInfo
        MinSizeRel
        Arcademia
        Showcase
        CACHE STRING "" FORCE
    )
endif()


# raylib
find_package(raylib QUIET)
if (NOT raylib_FOUND)
    include(FetchContent)
    FetchContent_Declare(
        raylib
        GIT_REPOSITORY https://github.com/raysan5/raylib.git
        GIT_TAG 6.0
        GIT_SHALLOW 1
    )
    FetchContent_MakeAvailable(raylib)
endif()

# raylib-cpp
find_package(raylib_cpp QUIET)
if (NOT raylib_cpp_FOUND)

    include(FetchContent)
    FetchContent_Declare(
        raylib_cpp
        GIT_REPOSITORY https://github.com/RobLoach/raylib-cpp.git
        GIT_TAG v6.0.0
    )

    FetchContent_MakeAvailable(raylib_cpp)
endif()

# Automatically grab all source/header files
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS src/*.cpp)
file(GLOB_RECURSE HEADERS CONFIGURE_DEPENDS include/*.h)

add_executable(${PROJECT_NAME}
    ${SOURCES}
    ${HEADERS}
)

target_include_directories(${PROJECT_NAME} PUBLIC include)

target_link_libraries(${PROJECT_NAME}
    PUBLIC
    raylib
    raylib_cpp
)

set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_STANDARD 20
)

target_compile_definitions(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:Arcademia>:ARCADEMIA>
)


target_compile_definitions(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:Showcase>:SHOWCASE>
)

set(CMAKE_CXX_FLAGS_ARCADEMIA "${CMAKE_CXX_FLAGS_RELEASE}")
set(CMAKE_CXX_FLAGS_SHOWCASE "${CMAKE_CXX_FLAGS_RELEASE}")

# Copy Resources

file(GLOB_RECURSE RESOURCE_FILES "${CMAKE_SOURCE_DIR}/resources/*")

add_custom_command(TARGET endless_afternoon POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/resources
        $<TARGET_FILE_DIR:endless_afternoon>/resources
)

# ---------------------------------------------------------------------------
# clangd support
# ---------------------------------------------------------------------------
set(CLANGD_FLAGS
    "-I${CMAKE_SOURCE_DIR}/include"
    "-I${CMAKE_BINARY_DIR}/_deps/raylib-src/src"
    "-I${CMAKE_BINARY_DIR}/_deps/raylib_cpp-src/include"
    "-std=c++20"
)
list(JOIN CLANGD_FLAGS "\n" CLANGD_FLAGS_STR)
file(GENERATE
    OUTPUT "${CMAKE_SOURCE_DIR}/compile_flags.txt"
    CONTENT "${CLANGD_FLAGS_STR}\n"
)