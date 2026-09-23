#pragma once

#include "Command.h"
#include "CommandQueue.h"
#include "Equipment.h"
#include "StateId.h"

#include <string>
#include <thread>

// Owns the Equipment state machine, its command queue, and the single
// worker thread that pulls commands off that queue and drives Equipment.
// This is the class everything else in the program talks to - the
// console demo today, and later the network layer and the GUI. Nobody
// outside this class touches Equipment or CommandQueue directly.
class EquipmentController {
public:
    EquipmentController();
    ~EquipmentController();

    // Owns a thread and non-copyable state - don't allow copies.
    EquipmentController(const EquipmentController&) = delete;
    EquipmentController& operator=(const EquipmentController&) = delete;

    void start(); // launches the worker thread
    void stop();  // asks the worker thread to shut down and joins it

    // Thread-safe: any thread can call this to queue up work for the
    // equipment.
    void submitCommand(CommandType type);

    // Thread-safe reads of the current state.
    StateId currentStateId() const;
    std::string currentStateName() const;

private:
    void workerLoop(); // runs entirely on workerThread_

    static constexpr int kSetupDelayMs = 1500;
    static constexpr int kProcessingDelayMs = 2500;

    Equipment equipment_;
    CommandQueue queue_;
    std::thread workerThread_;
};
