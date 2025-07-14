# Set the minimum version of CMake required for your project
cmake_minimum_required(VERSION 3.10)

# Define the project name and version
project(CDGMagic VERSION 1.0 LANGUAGES CXX)

# Set version string
add_definitions(-DCDGMAGIC_APP_VERSION_STRING="CD+Graphics Magic EXPERIMENTAL ${PROJECT_VERSION}")
add_definitions(-DMAX_PATH=8192)
add_definitions(-D_CDGMAGIC_NOLANESHANDLEREDRAW_)
add_definitions(-D_CDGMAGIC_USEPCLINUXAUDIO_)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/Modules)

# Set the C++ standard to use
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

# Optional: Print a message indicating the selected compiler
message(STATUS "Using C++ compiler: ${CMAKE_CXX_COMPILER}")

# Set the location of the vcpkg toolchain file if VCPKG_ROOT is defined (environment variable for vcpkg)
if(DEFINED ENV{VCPKG_ROOT})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
    message(STATUS "Using vcpkg toolchain file at ${CMAKE_TOOLCHAIN_FILE}")
else()
    message(WARNING "VCPKG_ROOT is not set. Ensure dependencies are available.")
endif()

# Add the include directory where your source files are
include_directories(${PROJECT_SOURCE_DIR})

file(GLOB HEADERS Source/*.h)
file(GLOB SOURCES Source/*.cpp)

# Create the executable for your project
add_executable(${PROJECT_NAME} ${SOURCES})

# Find and link FLTK
find_package(FLTK REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE fltk)

# Find and link PortAudio
find_package(PortAudio REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE portaudio)

# Optionally, you can add platform-specific configurations
if(WIN32)
    # Windows-specific configurations (e.g., link against additional libraries)
    target_link_libraries(${PROJECT_NAME} PRIVATE ws2_32)
elseif(UNIX)
    # Unix-specific configurations (e.g., enable pthreads)
    find_package(Threads REQUIRED)
    target_link_libraries(${PROJECT_NAME} PRIVATE Threads::Threads)
    find_package(X11 REQUIRED)
    target_link_libraries(${PROJECT_NAME} PRIVATE X11)
endif()

# Print a summary message with build information
message(STATUS "Project ${PROJECT_NAME} configured with the following options:")
message(STATUS "  C++ Compiler: ${CMAKE_CXX_COMPILER}")
message(STATUS "  C++ Standard: ${CMAKE_CXX_STANDARD}")
