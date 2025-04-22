cmake_minimum_required(VERSION 3.21)
project(tcp-echo-server VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)  # enable GNU extensions

# Set the output directory for binaries
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# generate compile_commands.json
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Source files
set(SOURCES
	${CMAKE_SOURCE_DIR}/src/main.cpp
	${CMAKE_SOURCE_DIR}/src/util.cpp
)
include_directories(${CMAKE_SOURCE_DIR}/src)

include(FetchContent)
# spdlog
FetchContent_Declare(
						spdlog
						GIT_REPOSITORY https://github.com/gabime/spdlog.git
						GIT_TAG v1.15.2
)
FetchContent_MakeAvailable(spdlog)

# cli11
FetchContent_Declare(
						CLI11
						GIT_REPOSITORY https://github.com/CLIUtils/CLI11.git
						GIT_TAG v2.5.0
)
FetchContent_MakeAvailable(CLI11)

# add executable
add_executable(${PROJECT_NAME} ${SOURCES})
target_link_libraries(${PROJECT_NAME} PUBLIC  CLI11::CLI11 spdlog::spdlog ev)
