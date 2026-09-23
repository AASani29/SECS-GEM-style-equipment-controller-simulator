#pragma once

#include "EquipmentState.h"

// The five concrete states. Each one is small enough that we keep them
// together in one file/pair instead of five separate ones - the whole
// transition table is easier to scan this way. If any of these grow more
// complex later, split them out.

class IdleState : public EquipmentState {
public:
    StateId id() const override { return StateId::Idle; }
    std::string name() const override { return "IDLE"; }
    std::unique_ptr<EquipmentState> handleCommand(Equipment& equipment, CommandType command) override;
};

class SetupState : public EquipmentState {
public:
    StateId id() const override { return StateId::Setup; }
    std::string name() const override { return "SETUP"; }
    std::unique_ptr<EquipmentState> handleCommand(Equipment& equipment, CommandType command) override;
};

class ProcessingState : public EquipmentState {
public:
    StateId id() const override { return StateId::Processing; }
    std::string name() const override { return "PROCESSING"; }
    std::unique_ptr<EquipmentState> handleCommand(Equipment& equipment, CommandType command) override;
};

class AlarmState : public EquipmentState {
public:
    StateId id() const override { return StateId::Alarm; }
    std::string name() const override { return "ALARM"; }
    std::unique_ptr<EquipmentState> handleCommand(Equipment& equipment, CommandType command) override;
};

class CompleteState : public EquipmentState {
public:
    StateId id() const override { return StateId::Complete; }
    std::string name() const override { return "COMPLETE"; }
    std::unique_ptr<EquipmentState> handleCommand(Equipment& equipment, CommandType command) override;
};
