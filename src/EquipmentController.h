#pragma once

#include "Command.h"
#include "CommandQueue.h"
#include "Equipment.h"
#include "StateId.h"

#include <chrono>
#include <functional>
#include <string>
#include <thread>

// Called on the worker thread right after the equipment changes state.
using StateChangedCallback = std::function<void(StateId oldState, StateId newState)>;

// Called on the worker thread when a command arrived but the current
// state had no transition for it (so nothing changed).
using CommandIgnoredCallback = std::function<void(CommandType command, StateId currentState)>;

// Owns the Equipment state machine, its command queue, and the single
// worker thread that pulls commands off that queue and drives Equipment.
// This is the class everything else in the program talks to - the
// console demo, the network layer, and the GUI. Nobody outside this
// class touches Equipment or CommandQueue directly.
//
// This class knows nothing about Qt. The Qt side plugs in through the
// two callbacks above (see EquipmentAdapter).
class EquipmentController {
public:
    EquipmentController();
    ~EquipmentController();

    // Owns a thread and non-copyable state - don't allow copies.
    EquipmentController(const EquipmentController&) = delete;
    EquipmentController& operator=(const EquipmentController&) = delete;

    // Set these BEFORE calling start(). They are plain members with no
    // locking, which is safe only because the worker thread hasn't
    // started yet. They will be invoked on the worker thread.
    void setStateChangedCallback(StateChangedCallback callback);
    void setCommandIgnoredCallback(CommandIgnoredCallback callback);

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

    // Waits for the next command, but only until stepDeadline. If the
    // deadline passes with nothing in the queue, the simulated SETUP or
    // PROCESSING step has finished on its own, so we hand back the
    // matching "...Finished" command.
    Command waitForCommandOrStepTimeout(std::chrono::steady_clock::time_point stepDeadline);

    static constexpr int kSetupDelayMs = 1500;
    static constexpr int kProcessingDelayMs = 2500;

    Equipment equipment_;
    CommandQueue queue_;
    std::thread workerThread_;

    StateChangedCallback stateChangedCallback_;
    CommandIgnoredCallback commandIgnoredCallback_;
};
