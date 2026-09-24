#include "EquipmentController.h"

#include <optional>
#include <utility>

EquipmentController::EquipmentController() = default;

EquipmentController::~EquipmentController() {
    stop();
}

void EquipmentController::setStateChangedCallback(StateChangedCallback callback) {
    stateChangedCallback_ = std::move(callback);
}

void EquipmentController::setCommandIgnoredCallback(CommandIgnoredCallback callback) {
    commandIgnoredCallback_ = std::move(callback);
}

void EquipmentController::start() {
    workerThread_ = std::thread(&EquipmentController::workerLoop, this);
}

void EquipmentController::stop() {
    if (workerThread_.joinable()) {
        queue_.push(Command{CommandType::Shutdown});
        workerThread_.join();
    }
}

void EquipmentController::submitCommand(CommandType type) {
    queue_.push(Command{type});
}

StateId EquipmentController::currentStateId() const {
    return equipment_.currentStateId();
}

std::string EquipmentController::currentStateName() const {
    return equipment_.currentStateName();
}

Command EquipmentController::waitForCommandOrStepTimeout(std::chrono::steady_clock::time_point stepDeadline) {
    std::optional<Command> command = queue_.waitAndPopUntil(stepDeadline);
    if (command.has_value()) {
        return *command;
    }

    // Timed out: the simulated step ran to completion and nothing
    // interrupted it.
    if (equipment_.currentStateId() == StateId::Setup) {
        return Command{CommandType::SetupFinished};
    }
    return Command{CommandType::ProcessingFinished};
}

void EquipmentController::workerLoop() {
    // SETUP and PROCESSING represent real work happening on the tool.
    // We simulate that with a deadline: when we enter one of those
    // states, we note when the step should finish. While the step is
    // running we wait on the queue *until that deadline*, so an external
    // command (like an alarm) is handled immediately, and if nothing
    // arrives the step finishes on its own.
    bool stepInProgress = false;
    std::chrono::steady_clock::time_point stepDeadline;

    while (true) {
        Command command = stepInProgress ? waitForCommandOrStepTimeout(stepDeadline)
                                         : queue_.waitAndPop();
        if (command.type == CommandType::Shutdown) {
            break;
        }

        const StateId stateBefore = equipment_.currentStateId();
        equipment_.handleCommand(command.type);
        const StateId stateAfter = equipment_.currentStateId();

        if (stateAfter == stateBefore) {
            // Every real transition changes the state, so "no change"
            // means the state ignored the command. The step timer (if
            // any) keeps running with its original deadline.
            if (commandIgnoredCallback_) {
                commandIgnoredCallback_(command.type, stateAfter);
            }
            continue;
        }

        if (stateChangedCallback_) {
            stateChangedCallback_(stateBefore, stateAfter);
        }

        // Start (or cancel) the step timer for the state we just entered.
        if (stateAfter == StateId::Setup) {
            stepInProgress = true;
            stepDeadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(kSetupDelayMs);
        } else if (stateAfter == StateId::Processing) {
            stepInProgress = true;
            stepDeadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(kProcessingDelayMs);
        } else {
            stepInProgress = false;
        }
    }
}
