cmake_minimum_required(VERSION 3.10)
project(drone_simulator)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Protobuf
find_package(Protobuf REQUIRED)
protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${CMAKE_CURRENT_SOURCE_DIR}/proto/drone.proto)

# Set include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${PROTOBUF_INCLUDE_DIRS}
)

# Source files
set(SOURCES
    src/main.cpp
    src/SocketClient.cpp
    src/SocketServer.cpp
    src/DroneState.cpp
    ${PROTO_SRCS}
)

# Headers
set(HEADERS
    src/SocketClient.h
    src/SocketServer.h
    src/CommandFactory.h
    src/DroneState.h
    ${PROTO_HDRS}
)

# Add executable
add_executable(drone_simulator ${SOURCES} ${HEADERS})

# Link libraries
target_link_libraries(drone_simulator PRIVATE protobuf::libprotobuf)
