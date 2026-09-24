#pragma once

#include "MetaTypes.h"
#include "SecsFramer.h"
#include "SecsMessage.h"

#include <QObject>
#include <QString>
#include <QTcpServer>
#include <QTcpSocket>

// The "Equipment" side of the SECS link: listens for a host connection,
// answers its messages, and reports state changes to it.
//
// THREADING: this object (and the QTcpServer / QTcpSocket it owns) lives
// on the network thread. Only code running on that thread ever touches
// the sockets. Everyone else talks to this class through signals and
// slots, never by calling its methods directly.
class EquipmentServer : public QObject {
    Q_OBJECT

public:
    explicit EquipmentServer(quint16 port, QObject* parent = nullptr);

public slots:
    void startListening();

    // Connected to EquipmentAdapter::stateChanged. Turns a state change
    // into an S6F11 event report for the host.
    void sendEventReport(StateId oldState, StateId newState);

signals:
    void logMessage(const QString& text);

    // A valid remote command arrived from the host. Connected to
    // EquipmentAdapter::submitCommand.
    void commandRequested(CommandType command);

private slots:
    void onNewConnection();
    void onReadyRead();
    void onClientDisconnected();

private:
    void handleMessage(const SecsMessage& message);
    void handleRemoteCommand(const SecsMessage& message);
    void sendMessage(const SecsMessage& message);

    quint16 port_;
    QTcpServer* tcpServer_;               // child of this, so it moves threads with us
    QTcpSocket* clientSocket_ = nullptr;  // the one connected host, if any
    SecsFramer framer_;
    quint32 nextTransactionId_ = 1;
};
