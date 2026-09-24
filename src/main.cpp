#include "EquipmentAdapter.h"
#include "EquipmentServer.h"
#include "HostClient.h"
#include "MainWindow.h"
#include "MetaTypes.h"

#include <QApplication>
#include <QThread>

// Any free local port works; the equipment listens here and the host
// connects to it.
static const quint16 kSecsPort = 15000;

// Threads in this program:
//   1. GUI thread        - main(): QApplication and MainWindow.
//   2. Network thread    - networkThread below: EquipmentServer, HostClient
//                          and their sockets. Nothing else touches them.
//   3. Equipment worker  - the std::thread inside EquipmentController,
//                          owned by EquipmentAdapter.
//
// This file's only job is to create the objects and connect them. Every
// cross-thread connection is explicitly Qt::QueuedConnection so it is
// obvious in the code where a thread boundary is crossed.
int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    registerMetaTypes();

    MainWindow window;

    // Created here on the GUI thread, then moved to the network thread.
    // (No parent: an object with a parent can't be moved on its own.)
    EquipmentAdapter* adapter = new EquipmentAdapter;
    EquipmentServer* server = new EquipmentServer(kSecsPort);
    HostClient* host = new HostClient(kSecsPort);

    QThread networkThread;
    adapter->moveToThread(&networkThread);
    server->moveToThread(&networkThread);
    host->moveToThread(&networkThread);

    // --- GUI thread -> network thread: "please do this" requests ---
    QObject::connect(&window, &MainWindow::connectRequested,
                     host, &HostClient::connectToEquipment, Qt::QueuedConnection);
    QObject::connect(&window, &MainWindow::areYouThereRequested,
                     host, &HostClient::sendAreYouThere, Qt::QueuedConnection);
    QObject::connect(&window, &MainWindow::remoteCommandRequested,
                     host, &HostClient::sendRemoteCommand, Qt::QueuedConnection);
    QObject::connect(&window, &MainWindow::alarmRequested,
                     adapter, &EquipmentAdapter::triggerAlarm, Qt::QueuedConnection);

    // --- Network thread -> GUI thread: things to show on screen ---
    QObject::connect(host, &HostClient::logMessage,
                     &window, &MainWindow::appendLog, Qt::QueuedConnection);
    QObject::connect(host, &HostClient::connectionChanged,
                     &window, &MainWindow::onConnectionChanged, Qt::QueuedConnection);
    QObject::connect(server, &EquipmentServer::logMessage,
                     &window, &MainWindow::appendLog, Qt::QueuedConnection);

    // --- Equipment worker thread -> GUI thread ---
    // The adapter's signals are emitted by the worker thread itself.
    QObject::connect(adapter, &EquipmentAdapter::stateChanged,
                     &window, &MainWindow::onEquipmentStateChanged, Qt::QueuedConnection);
    QObject::connect(adapter, &EquipmentAdapter::logMessage,
                     &window, &MainWindow::appendLog, Qt::QueuedConnection);

    // --- Between the equipment side and the network side ---
    // Host sent a command over TCP -> onto the equipment's command queue.
    QObject::connect(server, &EquipmentServer::commandRequested,
                     adapter, &EquipmentAdapter::submitCommand, Qt::QueuedConnection);
    // Equipment changed state -> S6F11 event report out over TCP.
    QObject::connect(adapter, &EquipmentAdapter::stateChanged,
                     server, &EquipmentServer::sendEventReport, Qt::QueuedConnection);

    // --- Thread lifecycle ---
    // Start listening once the network thread's event loop is running.
    QObject::connect(&networkThread, &QThread::started, server, &EquipmentServer::startListening);

    // Delete the network-side objects on the network thread itself, as
    // its event loop shuts down. (QObjects should be destroyed on the
    // thread they live on.)
    QObject::connect(&networkThread, &QThread::finished, adapter, &QObject::deleteLater);
    QObject::connect(&networkThread, &QThread::finished, server, &QObject::deleteLater);
    QObject::connect(&networkThread, &QThread::finished, host, &QObject::deleteLater);

    networkThread.start();
    window.show();

    const int exitCode = app.exec();

    // The window is closed: stop the network thread's event loop and wait
    // for it to finish (which also deletes the objects above).
    networkThread.quit();
    networkThread.wait();
    return exitCode;
}
