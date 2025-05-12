#include "SocketServer.h"
#include <cstring>
#include <stdexcept>
#include <thread>
#include <unistd.h>
#include <arpa/inet.h>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <google/protobuf/io/zero_copy_stream.h>
#include <iostream>
#include <chrono>

void SocketServer::start() {
    std::cout << "Starting server on port " << port << std::endl;
    
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == 0) {
        throw std::runtime_error("Socket creation failed");
    }
    
    // 포트 재사용 옵션 설정
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        throw std::runtime_error("Setsockopt failed");
    }
    
    sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);
    
    if (bind(server_fd, (sockaddr*)&address, sizeof(address)) < 0) {
        throw std::runtime_error("Bind failed");
    }
    
    if (listen(server_fd, 3) < 0) {
        throw std::runtime_error("Listen failed");
    }
    
    std::cout << "Server listening on port " << port << std::endl;
    std::cout << "Waiting for client connections..." << std::endl;
    
    while (true) {
        sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        int client_fd = accept(server_fd, (sockaddr*)&client_addr, &client_len);
        if (client_fd < 0) {
            std::cout << "Accept failed" << std::endl;
            continue;
        }
        
        std::cout << "New client connected" << std::endl;
        
        // 클라이언트를 위한 새로운 스레드 생성
        std::thread client_thread(&SocketServer::handleClient, this, client_fd);
        client_thread.detach();
    }
}

void SocketServer::handleClient(int client_fd) {
    std::cout << "Handling client connection..." << std::endl;
    
    while (true) {
        // 명령어 길이 수신
        uint32_t command_size;
        ssize_t result = read(client_fd, &command_size, sizeof(command_size));
        if (result < 0) {
            std::cout << "Failed to receive command size: " << strerror(errno) << std::endl;
            break;
        } else if (result == 0) {
            std::cout << "Client disconnected" << std::endl;
            break;
        }
        
        // 명령어 수신
        char buffer[1024];
        ssize_t bytes_read = 0;
        while (bytes_read < command_size) {
            result = read(client_fd, buffer + bytes_read, command_size - bytes_read);
            if (result < 0) {
                std::cout << "Failed to receive command: " << strerror(errno) << std::endl;
                break;
            } else if (result == 0) {
                std::cout << "Client disconnected during command reception" << std::endl;
                break;
            }
            bytes_read += result;
        }
        
        if (bytes_read != command_size) {
            std::cout << "Failed to receive complete command" << std::endl;
            break;
        }
        
        // protobuf 메시지 파싱
        DroneCommand command;
        google::protobuf::io::ArrayInputStream input(buffer, bytes_read);
        if (!command.ParseFromZeroCopyStream(&input)) {
            std::cout << "Failed to parse command" << std::endl;
            break;
        }
        
        std::cout << "Received command: " << command.command() << std::endl;
        
        // 명령어 파라미터 출력
        if (command.command() == CommandType::MOVE) {
            const auto& params = command.move();
            std::cout << "  Move parameters: x=" << params.x()
                      << ", y=" << params.y()
                      << ", z=" << params.z()
                      << ", speed=" << params.speed() << std::endl;
        } else if (command.command() == CommandType::ROTATE) {
            const auto& params = command.rotate();
            std::cout << "  Rotate parameters: yaw=" << params.yaw()
                      << ", pitch=" << params.pitch()
                      << ", roll=" << params.roll()
                      << ", speed=" << params.speed() << std::endl;
        }
        
        // 명령어 처리
        DroneResponse response;
        bool success = droneState.executeCommand(command);
        
        if (success) {
            response.set_success(true);
            response.set_message("Command executed successfully");
            std::cout << "Command executed successfully" << std::endl;
        } else {
            response.set_success(false);
            response.set_message("Command execution failed");
            std::cout << "Command execution failed" << std::endl;
        }
        
        // 응답 직렬화
        std::string serialized;
        if (!response.SerializeToString(&serialized)) {
            std::cout << "Failed to serialize response" << std::endl;
            break;
        }
        
        // 응답 길이 전송
        uint32_t response_size = serialized.size();
        if (write(client_fd, &response_size, sizeof(response_size)) < 0) {
            std::cout << "Failed to send response size: " << strerror(errno) << std::endl;
            break;
        }
        
        // 응답 전송
        ssize_t bytes_sent = 0;
        while (bytes_sent < serialized.size()) {
            ssize_t result = write(client_fd, serialized.data() + bytes_sent, serialized.size() - bytes_sent);
            if (result < 0) {
                std::cout << "Failed to send response: " << strerror(errno) << std::endl;
                break;
            }
            bytes_sent += result;
        }
        
        if (bytes_sent != serialized.size()) {
            std::cout << "Failed to send complete response" << std::endl;
            break;
        }
    }
    
    close(client_fd);
    std::cout << "Client connection closed" << std::endl;
}
