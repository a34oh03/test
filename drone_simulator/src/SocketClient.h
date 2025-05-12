#pragma once
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "drone.pb.h"

class SocketClient {
public:
    SocketClient(const std::string& host, int port) : host(host), port(port), client_fd(-1) {}
    ~SocketClient();
    
    void connect();
    bool sendCommand(const DroneCommand& command);
    bool receiveResponse(DroneResponse& response);
    bool sendCommandAndGetResponse(const DroneCommand& command, DroneResponse& response);
    void disconnect();
    
private:
    std::string host;
    int port;
    int client_fd;
};