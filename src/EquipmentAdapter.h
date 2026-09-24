#pragma once

#include "EquipmentController.h"
#include "MetaTypes.h"

#include <QObject>
#include <QString>

// Qt-friendly face of EquipmentController.
//
// EquipmentController runs on a plain std::thread and knows nothing about
// Qt. Everything else in the app (network layer, GUI) talks in signals and
// slots. This class is the bridge between the two worlds:
//
//   Qt world -> worker:  slots (submitCommand, triggerAlarm) push onto the
//                        controller's mutex-protected command queue.
//   worker -> Qt world:  the controller's callbacks run on the WORKER
//                        thread and simply emit signals. With queued
//                        connections, each receiver's slot then runs on
//                        the receiver's own thread.
class EquipmentAdapter : public QObject {
    Q_OBJECT

public:
    explicit EquipmentAdapter(QObject* parent = nullptr);
    ~EquipmentAdapter() override;

public slots:
    void submitCommand(CommandType command);
    void triggerAlarm();

signals:
    // Emitted from the equipment worker thread.
    void stateChanged(StateId oldState, StateId newState);
    void logMessage(const QString& text);

private:
    EquipmentController controller_;
};
