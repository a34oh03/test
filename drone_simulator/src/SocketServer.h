#pragma once
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <thread>
#include "drone.pb.h"
#include "DroneState.h"

class SocketServer {
public:
    SocketServer(int port) : port(port), server_fd(-1), droneState() {}
    ~SocketServer() {
        if (server_fd >= 0) {
            close(server_fd);
        }
    }
    
    void start();
    void acceptConnections();
    void handleClient(int client_fd);
    
private:
    int server_fd;
    int port;
    DroneState droneState;
};
