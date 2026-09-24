#include "EquipmentAdapter.h"

EquipmentAdapter::EquipmentAdapter(QObject* parent) : QObject(parent) {
    // These lambdas run on the equipment WORKER thread, not on whatever
    // thread this adapter lives in. That's fine: emitting a signal is just
    // a function call, and for queued connections Qt copies the arguments
    // and posts an event to each receiver's thread instead of running the
    // receiver's code here.
    controller_.setStateChangedCallback([this](StateId oldState, StateId newState) {
        emit stateChanged(oldState, newState);
    });

    controller_.setCommandIgnoredCallback([this](CommandType command, StateId currentState) {
        emit logMessage(QStringLiteral("Equipment ignored %1 while in %2")
                            .arg(QString::fromStdString(toString(command)),
                                 QString::fromStdString(toString(currentState))));
    });

    controller_.start();
}

EquipmentAdapter::~EquipmentAdapter() {
    // Stop the worker explicitly, while this object is still fully alive:
    // the worker's callbacks use `this`, so it must be joined before any
    // part of the adapter starts to be destroyed.
    controller_.stop();
}

void EquipmentAdapter::submitCommand(CommandType command) {
    controller_.submitCommand(command);
}

void EquipmentAdapter::triggerAlarm() {
    controller_.submitCommand(CommandType::TriggerAlarm);
}
