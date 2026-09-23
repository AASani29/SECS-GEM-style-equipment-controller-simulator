#pragma once

#include <string>

// Every kind of thing that can happen to the state machine, whether it
// came from outside (a host system, the GUI) or from inside our own
// worker thread simulating time passing.
enum class CommandType {
    StartJob,           // external: Idle -> Setup
    SetupFinished,       // internal: Setup -> Processing (worker's simulated delay elapsed)
    ProcessingFinished,  // internal: Processing -> Complete (worker's simulated delay elapsed)
    Reset,               // external: Complete/Alarm -> Idle
    TriggerAlarm,        // external: any state -> Alarm (simulated fault)
    Shutdown             // internal: tells the worker thread to stop its loop
};

std::string toString(CommandType type);

// Just the type for now. In Stage 2, once commands can arrive over the
// network as SECS-II messages, we'll likely add fields here (e.g. a
// payload string) - kept minimal today on purpose.
struct Command {
    CommandType type;
};
