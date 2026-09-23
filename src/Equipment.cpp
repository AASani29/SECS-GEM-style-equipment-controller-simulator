#include "Equipment.h"

#include "EquipmentStates.h"

#include <iostream>

Equipment::Equipment() : currentState_(std::make_unique<IdleState>()) {}

void Equipment::handleCommand(CommandType command) {
    std::lock_guard<std::mutex> lock(stateMutex_);

    std::unique_ptr<EquipmentState> nextState = currentState_->handleCommand(*this, command);
    if (nextState != nullptr) {
        std::cout << "[Equipment] " << currentState_->name() << " -> " << nextState->name() << "\n";
        // The old state object is destroyed right here, by this
        // assignment - only after currentState_->handleCommand() above
        // has already returned. It's never destroyed while one of its
        // own member functions is still running on the call stack.
        currentState_ = std::move(nextState);
    }
}

StateId Equipment::currentStateId() const {
    std::lock_guard<std::mutex> lock(stateMutex_);
    return currentState_->id();
}

std::string Equipment::currentStateName() const {
    std::lock_guard<std::mutex> lock(stateMutex_);
    return currentState_->name();
}
