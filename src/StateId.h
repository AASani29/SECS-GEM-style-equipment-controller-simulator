#pragma once

#include <string>

// The five states in our simplified equipment lifecycle. Kept as a plain
// enum (rather than only having the EquipmentState subclasses) so other
// code - logging, the GUI label, a switch statement - can compare and
// display "what state are we in" without needing an EquipmentState object
// or any dynamic_cast trickery.
enum class StateId {
    Idle,
    Setup,
    Processing,
    Alarm,
    Complete
};

std::string toString(StateId id);
