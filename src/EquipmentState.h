#pragma once

#include "Command.h"
#include "StateId.h"

#include <memory>
#include <string>

class Equipment;

// Base class for the State design pattern. Each concrete equipment state
// (Idle, Setup, Processing, Alarm, Complete) is one small subclass of this.
// Equipment holds "whichever state object is current" and forwards every
// incoming command to it via handleCommand(). A state does NOT switch
// Equipment to a new state itself - it just returns the state it wants to
// switch to (or nullptr, meaning "no transition, this command doesn't
// apply here"), and Equipment performs the actual switch after the call
// returns. See Equipment::handleCommand() for why it's done that way.
class EquipmentState {
public:
    virtual ~EquipmentState() = default;

    virtual StateId id() const = 0;
    virtual std::string name() const = 0;

    // Returns the next state to transition to, or nullptr to mean "stay
    // in the current state".
    virtual std::unique_ptr<EquipmentState> handleCommand(Equipment& equipment, CommandType command) = 0;

protected:
    // Shared by every state's "I don't handle this command" branch, so we
    // get one consistent log line instead of five slightly different
    // ones. Always returns nullptr, so a state can just
    // `return logIgnored(command);`.
    std::unique_ptr<EquipmentState> logIgnored(CommandType command) const;
};
