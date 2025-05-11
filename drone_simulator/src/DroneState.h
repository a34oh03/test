#pragma once
#include "drone.pb.h"
#include <memory>
#include <functional>

class DroneState {
public:
    using Observer = std::function<void(const DroneResponse&)>;
    
    void attach(Observer observer);
    void detach(Observer observer);
    bool executeCommand(const DroneCommand& command);
    
private:
    enum class State {
        GROUND,
        FLYING,
        LANDING
    };
    
    State current_state = State::GROUND;
    
    bool handleTakeoff();
    bool handleMove(const MoveParameters& params);
    bool handleRotate(const RotateParameters& params);
    bool handleLand();
    bool handleStatus();
    void notifyObservers(const DroneResponse& response);
    
    std::vector<Observer> observers;
};
