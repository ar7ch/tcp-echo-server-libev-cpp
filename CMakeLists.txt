cmake_minimum_required(VERSION 3.21)
project(tcp-echo-server VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)  # enable GNU extensions

# Set the output directory for binaries
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# generate compile_commands.json
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# warn if ${CMAKE_INCLUDE_PATH} is not set
# if(NOT DEFINED CMAKE_INCLUDE_PATH)
# 	message(WARNING "CMAKE_INCLUDE_PATH is not set")
# endif()

# include directories
# include_directories(${CMAKE_INCLUDE_PATH})

# Source files
set(SOURCES
	src/main.cpp
)

include(FetchContent)
FetchContent_Declare(
						spdlog
						GIT_REPOSITORY https://github.com/gabime/spdlog.git
						GIT_TAG v1.15.2
)
# download and import spdlog
FetchContent_MakeAvailable(spdlog)
# add executable
add_executable(${PROJECT_NAME} ${SOURCES})

target_include_directories(${PROJECT_NAME} SYSTEM PRIVATE ${CMAKE_SOURCE_DIR}/libs/libev)
target_link_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/libs/libev/.libs)
target_link_libraries(${PROJECT_NAME} PUBLIC spdlog::spdlog ev)
