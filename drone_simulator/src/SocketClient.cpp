#include "SocketClient.h"
#include <cstring>
#include <stdexcept>
#include <unistd.h>
#include <arpa/inet.h>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <google/protobuf/io/zero_copy_stream.h>
#include <iostream>
#include <chrono>
#include <thread>
#include <errno.h>
#include <string.h>

SocketClient::~SocketClient() {
    if (client_fd >= 0) {
        close(client_fd);
    }
}

void SocketClient::connect() {
    std::cout << "Connecting to server at " << host << ":" << port << std::endl;
    
    // 연결 시도 횟수
    const int MAX_RETRIES = 5;
    int retry_count = 0;
    
    while (retry_count < MAX_RETRIES) {
        // 소켓 생성
        client_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (client_fd < 0) {
            throw std::runtime_error("Socket creation failed: " + std::string(strerror(errno)));
        }
        
        // 서버 주소 설정
        sockaddr_in server_addr;
        memset(&server_addr, 0, sizeof(server_addr));
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(port);
        
        if (inet_pton(AF_INET, host.c_str(), &server_addr.sin_addr) <= 0) {
            throw std::runtime_error("Invalid address: " + std::string(strerror(errno)));
        }
        
        // 서버에 연결
        if (::connect(client_fd, (sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
            std::cout << "Connection attempt " << (retry_count + 1) << " failed: " << strerror(errno) << std::endl;
            close(client_fd);
            std::this_thread::sleep_for(std::chrono::seconds(1));
            retry_count++;
            continue;
        }
        
        std::cout << "Connected to server" << std::endl;
        return;
    }
    
    throw std::runtime_error("Failed to connect after " + std::to_string(MAX_RETRIES) + " attempts");
}

bool SocketClient::sendCommand(const DroneCommand& command) {
    std::cout << "Sending command: " << command.command() << std::endl;
    
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
    
    // 명령어 직렬화
    std::string serialized;
    if (!command.SerializeToString(&serialized)) {
        std::cout << "Failed to serialize command" << std::endl;
        return false;
    }
    
    std::cout << "Serialized command size: " << serialized.size() << " bytes" << std::endl;
    std::cout << "Serialized command: " << serialized << std::endl;
    
    // 명령어 길이 전송
    uint32_t command_size = serialized.size();
    if (write(client_fd, &command_size, sizeof(command_size)) < 0) {
        std::cout << "Failed to send command size: " << strerror(errno) << std::endl;
        return false;
    }
    
    // 명령어 전송
    ssize_t bytes_sent = 0;
    while (bytes_sent < serialized.size()) {
        ssize_t result = write(client_fd, serialized.data() + bytes_sent, serialized.size() - bytes_sent);
        if (result < 0) {
            std::cout << "Failed to send command: " << strerror(errno) << std::endl;
            return false;
        }
        bytes_sent += result;
    }
    
    if (bytes_sent != serialized.size()) {
        std::cout << "Failed to send complete command" << std::endl;
        return false;
    }
    
    // 명령어 간에 1초 딜레이 추가
    std::this_thread::sleep_for(std::chrono::seconds(1));
    
    return true;
}

bool SocketClient::receiveResponse(DroneResponse& response) {
    // 응답 수신
    char buffer[1024];
    
    // 응답 길이 수신
    uint32_t response_size;
    if (read(client_fd, &response_size, sizeof(response_size)) < 0) {
        std::cout << "Failed to receive response size: " << strerror(errno) << "\n";
        return false;
    }
    
    // 응답 데이터 수신
    ssize_t bytes_read = 0;
    while (bytes_read < response_size) {
        ssize_t result = read(client_fd, buffer + bytes_read, response_size - bytes_read);
        if (result <= 0) {
            std::cout << "Failed to receive response: " << strerror(errno) << "\n";
            return false;
        }
        bytes_read += result;
    }
    
    // 읽은 바이트 수만큼만 사용
    buffer[bytes_read] = '\0';
    
    // 응답 파싱
    google::protobuf::io::ArrayInputStream input(buffer, bytes_read);
    if (!response.ParseFromZeroCopyStream(&input)) {
        std::cout << "Failed to parse response\n";
        return false;
    }
    
    std::cout << "Command response: " << response.message() << "\n";
    
    return response.success();
}

bool SocketClient::sendCommandAndGetResponse(const DroneCommand& command, DroneResponse& response) {
    if (!sendCommand(command)) {
        return false;
    }
    return receiveResponse(response);
}

void SocketClient::disconnect() {
    close(client_fd);
    std::cout << "Disconnected from server" << std::endl;
}
