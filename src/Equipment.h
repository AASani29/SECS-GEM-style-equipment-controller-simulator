#pragma once

#include "EquipmentState.h"

#include <memory>
#include <mutex>
#include <string>

// The state machine itself. Equipment doesn't know *how* commands arrive
// (a queue, the network, a GUI button) - it only knows how to react to
// one once it gets here, by asking the current EquipmentState what should
// happen next and applying that transition. All the transition rules
// live in the EquipmentState subclasses (see EquipmentStates.h).
class Equipment {
public:
    Equipment();

    // Forwards the command to whichever state we're currently in, and
    // switches state if that state says to.
    void handleCommand(CommandType command);

    // Safe to call from any thread.
    StateId currentStateId() const;
    std::string currentStateName() const;

private:
    std::unique_ptr<EquipmentState> currentState_;

    // Right now, only our one worker thread ever calls handleCommand(),
    // so this mutex isn't strictly load-bearing yet. It's here from day
    // one because Stage 2 adds a network thread that will need to read
    // the current state safely, and it's easier to design that boundary
    // correctly up front than to retrofit locking later.
    mutable std::mutex stateMutex_;
};
