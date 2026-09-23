#include "EquipmentStates.h"

#include "Equipment.h"

#include <memory>

std::unique_ptr<EquipmentState> IdleState::handleCommand(Equipment& equipment, CommandType command) {
    switch (command) {
        case CommandType::StartJob:
            return std::make_unique<SetupState>();
        case CommandType::TriggerAlarm:
            return std::make_unique<AlarmState>();
        default:
            return logIgnored(command);
    }
}

std::unique_ptr<EquipmentState> SetupState::handleCommand(Equipment& equipment, CommandType command) {
    switch (command) {
        case CommandType::SetupFinished:
            return std::make_unique<ProcessingState>();
        case CommandType::TriggerAlarm:
            return std::make_unique<AlarmState>();
        default:
            return logIgnored(command);
    }
}

std::unique_ptr<EquipmentState> ProcessingState::handleCommand(Equipment& equipment, CommandType command) {
    switch (command) {
        case CommandType::ProcessingFinished:
            return std::make_unique<CompleteState>();
        case CommandType::TriggerAlarm:
            return std::make_unique<AlarmState>();
        default:
            return logIgnored(command);
    }
}

std::unique_ptr<EquipmentState> AlarmState::handleCommand(Equipment& equipment, CommandType command) {
    switch (command) {
        case CommandType::Reset:
            return std::make_unique<IdleState>();
        default:
            // While in ALARM, everything else is ignored on purpose - a
            // real tool won't run a process step until the fault is
            // cleared.
            return logIgnored(command);
    }
}

std::unique_ptr<EquipmentState> CompleteState::handleCommand(Equipment& equipment, CommandType command) {
    switch (command) {
        case CommandType::Reset:
            return std::make_unique<IdleState>();
        case CommandType::TriggerAlarm:
            return std::make_unique<AlarmState>();
        default:
            return logIgnored(command);
    }
}
