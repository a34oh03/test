#include <iostream>
#include <sstream>
#include "SocketServer.h"
#include "SocketClient.h"
#include "CommandFactory.h"

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " [server|client]" << std::endl;
        return 1;
    }
    
    if (std::string(argv[1]) == "server") {
        SocketServer server(8888);
        server.start();
        
        // 서버가 계속 실행되도록
        while (true) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    } else if (std::string(argv[1]) == "client") {
        SocketClient client("127.0.0.1", 8888);
        client.connect();
        
        std::cout << "Available commands:\n";
        std::cout << "  takeoff - Takeoff command\n";
        std::cout << "  move x y z speed - Move command (e.g., move 1 2 3 1)\n";
        std::cout << "  rotate yaw pitch roll speed - Rotate command (e.g., rotate 45 0 0 1)\n";
        std::cout << "  land - Land command\n";
        std::cout << "  quit - Exit program\n";
        
        std::string command;
        while (true) {
            std::cout << "\nEnter command: ";
            std::getline(std::cin, command);
            
            if (command == "quit") {
                // 현재 상태 확인
                DroneCommand statusCommand;
                statusCommand.set_command(CommandType::STATUS);
                
                DroneResponse response;
                if (client.sendCommandAndGetResponse(statusCommand, response)) {
                    std::cout << "Current state: " << response.message() << "\n";
                    
                    // 상태에 따라 처리
                    if (response.message() == "Current state: FLYING") {
                        DroneCommand landCommand;
                        landCommand.set_command(CommandType::LAND);
                        if (client.sendCommandAndGetResponse(landCommand, response)) {
                            std::cout << "Automatic landing initiated\n";
                        }
                    }
                }
                break;
            }
            
            std::istringstream iss(command);
            std::string cmd;
            iss >> cmd;
            
            if (cmd == "takeoff") {
                auto takeoffCommand = DroneCommandFactory::createTakeoffCommand();
                if (client.sendCommand(*takeoffCommand)) {
                    std::cout << "Takeoff command sent successfully\n";
                }
            } else if (cmd == "move") {
                float x, y, z, speed;
                if (iss >> x >> y >> z >> speed) {
                    auto moveCommand = DroneCommandFactory::createMoveCommand(x, y, z, speed);
                    if (client.sendCommand(*moveCommand)) {
                        std::cout << "Move command sent successfully\n";
                    }
                } else {
                    std::cout << "Invalid move command format. Usage: move x y z speed\n";
                }
            } else if (cmd == "rotate") {
                float yaw, pitch, roll, speed;
                if (iss >> yaw >> pitch >> roll >> speed) {
                    auto rotateCommand = DroneCommandFactory::createRotateCommand(yaw, pitch, roll, speed);
                    if (client.sendCommand(*rotateCommand)) {
                        std::cout << "Rotate command sent successfully\n";
                    }
                } else {
                    std::cout << "Invalid rotate command format. Usage: rotate yaw pitch roll speed\n";
                }
            } else if (cmd == "land") {
                auto landCommand = DroneCommandFactory::createLandCommand();
                if (client.sendCommand(*landCommand)) {
                    std::cout << "Land command sent successfully\n";
                }
            } else {
                std::cout << "Unknown command. Available commands: takeoff, move, rotate, land, quit\n";
            }
        }
        
        client.disconnect();
    } else {
        std::cerr << "Invalid argument. Use 'server' or 'client'" << std::endl;
        return 1;
    }
    
    return 0;
}
