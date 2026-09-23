#include "EquipmentController.h"

#include <chrono>

EquipmentController::EquipmentController() = default;

EquipmentController::~EquipmentController() {
    stop();
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

void EquipmentController::workerLoop() {
    while (true) {
        Command command = queue_.waitAndPop();
        if (command.type == CommandType::Shutdown) {
            break;
        }

        equipment_.handleCommand(command.type);

        // SETUP and PROCESSING represent real work happening on the
        // tool, so we simulate that by sleeping here on the worker
        // thread, then feeding the "finished" event back into our own
        // queue - the same pattern a real equipment driver uses when a
        // hardware interrupt or callback tells it a step has completed.
        if (equipment_.currentStateId() == StateId::Setup) {
            std::this_thread::sleep_for(std::chrono::milliseconds(kSetupDelayMs));
            queue_.push(Command{CommandType::SetupFinished});
        } else if (equipment_.currentStateId() == StateId::Processing) {
            std::this_thread::sleep_for(std::chrono::milliseconds(kProcessingDelayMs));
            queue_.push(Command{CommandType::ProcessingFinished});
        }
    }
}
