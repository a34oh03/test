#include "DroneState.h"
#include <iostream>
#include <thread>

void DroneState::attach(Observer observer) {
    observers.push_back(observer);
}

void DroneState::detach(Observer observer) {
    auto it = std::find_if(observers.begin(), observers.end(), [&](const Observer& obs) {
        return &obs == &observer;
    });
    if (it != observers.end()) {
        observers.erase(it);
    }
}

bool DroneState::executeCommand(const DroneCommand& command) {
    switch (command.command()) {
        case CommandType::TAKEOFF:
            return handleTakeoff();
        case CommandType::MOVE:
            return handleMove(command.move());
        case CommandType::ROTATE:
            return handleRotate(command.rotate());
        case CommandType::LAND:
            return handleLand();
        case CommandType::STATUS:
            return handleStatus();
        default:
            return false;
    }
}

bool DroneState::handleStatus() {
    DroneResponse response;
    response.set_success(true);
    switch (current_state) {
        case State::GROUND:
            response.set_message("Current state: GROUND");
            break;
        case State::FLYING:
            response.set_message("Current state: FLYING");
            break;
        case State::LANDING:
            response.set_message("Current state: LANDING");
            break;
    }
    notifyObservers(response);
    return true;
}

bool DroneState::handleTakeoff() {
    if (current_state != State::GROUND) {
        std::cout << "Failed to takeoff: Current state is " << static_cast<int>(current_state) << " (not on ground)\n";
        DroneResponse response;
        response.set_success(false);
        response.set_message("Cannot takeoff: not on ground");
        notifyObservers(response);
        return false;
    }
    
    std::cout << "Executing takeoff command...\n";
    current_state = State::FLYING;
    DroneResponse response;
    response.set_success(true);
    response.set_message("Takeoff successful");
    notifyObservers(response);
    return true;
}

bool DroneState::handleMove(const MoveParameters& params) {
    if (current_state != State::FLYING) {
        std::cout << "Failed to move: Current state is " << static_cast<int>(current_state) << " (not flying)\n";
        DroneResponse response;
        response.set_success(false);
        response.set_message("Cannot move: not flying");
        notifyObservers(response);
        return false;
    }
    
    std::cout << "Executing move command: x=" << params.x()
              << ", y=" << params.y()
              << ", z=" << params.z()
              << ", speed=" << params.speed() << "\n";
    DroneResponse response;
    response.set_success(true);
    response.set_message("Move successful");
    notifyObservers(response);
    return true;
}

bool DroneState::handleRotate(const RotateParameters& params) {
    if (current_state != State::FLYING) {
        std::cout << "Failed to rotate: Current state is " << static_cast<int>(current_state) << " (not flying)\n";
        DroneResponse response;
        response.set_success(false);
        response.set_message("Cannot rotate: not flying");
        notifyObservers(response);
        return false;
    }
    
    std::cout << "Executing rotate command: yaw=" << params.yaw()
              << ", pitch=" << params.pitch()
              << ", roll=" << params.roll()
              << ", speed=" << params.speed() << "\n";
    DroneResponse response;
    response.set_success(true);
    response.set_message("Rotate successful");
    notifyObservers(response);
    return true;
}

bool DroneState::handleLand() {
    if (current_state != State::FLYING) {
        std::cout << "Failed to land: Current state is " << static_cast<int>(current_state) << " (not flying)\n";
        DroneResponse response;
        response.set_success(false);
        response.set_message("Cannot land: not flying");
        notifyObservers(response);
        return false;
    }
    
    std::cout << "Executing land command...\n";
    current_state = State::LANDING;
    
    // 착륙 완료 후 지상 상태로 전환
    std::this_thread::sleep_for(std::chrono::seconds(1));
    current_state = State::GROUND;
    
    DroneResponse response;
    response.set_success(true);
    response.set_message("Land successful");
    notifyObservers(response);
    return true;
}

void DroneState::notifyObservers(const DroneResponse& response) {
    for (auto& observer : observers) {
        observer(response);
    }
}
