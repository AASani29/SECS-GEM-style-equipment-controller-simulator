#pragma once

#include "MetaTypes.h"

#include <QMainWindow>
#include <QString>

class QComboBox;
class QLabel;
class QPlainTextEdit;
class QPushButton;

// The dashboard: current equipment state, a scrolling message log, and
// the buttons that drive the simulation.
//
// THREADING RULE: this window lives on the GUI thread, and QWidgets may
// only be touched from that thread. So:
//   - The public slots below are the ONLY way other threads change what
//     is on screen. They are connected with queued connections, which
//     means Qt runs them on the GUI thread no matter which thread emitted
//     the signal.
//   - The signals below are how the window asks other threads to do
//     something. The window never calls into the network or equipment
//     objects directly.
class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);

public slots:
    void appendLog(const QString& text);
    void onEquipmentStateChanged(StateId oldState, StateId newState);
    void onConnectionChanged(bool connected);

signals:
    void connectRequested();
    void areYouThereRequested();
    void remoteCommandRequested(const QString& remoteCommand);
    void alarmRequested();

private:
    void setStateLabel(StateId state);

    QLabel* stateLabel_;
    QLabel* connectionLabel_;
    QPlainTextEdit* logView_;
    QPushButton* connectButton_;
    QPushButton* areYouThereButton_;
    QComboBox* commandComboBox_;
    QPushButton* sendCommandButton_;
    QPushButton* alarmButton_;
};
