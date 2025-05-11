#pragma once
#include "drone.pb.h"
#include <memory>
#include <functional>

template<typename T>
class CommandFactory {
public:
    using CommandPtr = std::unique_ptr<T>;
    
    template<typename... Args>
    static CommandPtr create(Args&&... args) {
        return std::make_unique<T>(std::forward<Args>(args)...);
    }
};

class DroneCommandFactory {
public:
    static std::unique_ptr<DroneCommand> createTakeoffCommand() {
        auto command = std::make_unique<DroneCommand>();
        command->set_command(CommandType::TAKEOFF);
        return command;
    }
    
    static std::unique_ptr<DroneCommand> createMoveCommand(float x, float y, float z, float speed) {
        auto command = std::make_unique<DroneCommand>();
        command->set_command(CommandType::MOVE);
        
        auto moveParams = command->mutable_move();
        moveParams->set_x(x);
        moveParams->set_y(y);
        moveParams->set_z(z);
        moveParams->set_speed(speed);
        
        return command;
    }
    
    static std::unique_ptr<DroneCommand> createRotateCommand(float yaw, float pitch, float roll, float speed) {
        auto command = std::make_unique<DroneCommand>();
        command->set_command(CommandType::ROTATE);
        
        auto rotateParams = command->mutable_rotate();
        rotateParams->set_yaw(yaw);
        rotateParams->set_pitch(pitch);
        rotateParams->set_roll(roll);
        rotateParams->set_speed(speed);
        
        return command;
    }
    
    static std::unique_ptr<DroneCommand> createLandCommand() {
        auto command = std::make_unique<DroneCommand>();
        command->set_command(CommandType::LAND);
        return command;
    }
};
