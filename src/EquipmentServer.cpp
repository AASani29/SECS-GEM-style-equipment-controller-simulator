#include "EquipmentServer.h"

#include <QHostAddress>

EquipmentServer::EquipmentServer(quint16 port, QObject* parent)
    : QObject(parent), port_(port), tcpServer_(new QTcpServer(this)) {
    connect(tcpServer_, &QTcpServer::newConnection, this, &EquipmentServer::onNewConnection);
}

void EquipmentServer::startListening() {
    if (tcpServer_->listen(QHostAddress::LocalHost, port_)) {
        emit logMessage(QStringLiteral("Equipment listening on 127.0.0.1:%1").arg(port_));
    } else {
        emit logMessage(QStringLiteral("Equipment could not listen on port %1: %2")
                            .arg(port_)
                            .arg(tcpServer_->errorString()));
    }
}

void EquipmentServer::onNewConnection() {
    while (tcpServer_->hasPendingConnections()) {
        QTcpSocket* newSocket = tcpServer_->nextPendingConnection();

        // Keep it simple: one host at a time.
        if (clientSocket_ != nullptr) {
            emit logMessage(QStringLiteral("Equipment rejected an extra host connection"));
            newSocket->close();
            newSocket->deleteLater();
            continue;
        }

        clientSocket_ = newSocket;
        framer_.clear();
        connect(clientSocket_, &QTcpSocket::readyRead, this, &EquipmentServer::onReadyRead);
        connect(clientSocket_, &QTcpSocket::disconnected, this, &EquipmentServer::onClientDisconnected);
        emit logMessage(QStringLiteral("Equipment accepted a host connection"));
    }
}

void EquipmentServer::onClientDisconnected() {
    emit logMessage(QStringLiteral("Host disconnected from equipment"));
    if (clientSocket_ != nullptr) {
        clientSocket_->deleteLater();
        clientSocket_ = nullptr;
    }
}

void EquipmentServer::onReadyRead() {
    if (clientSocket_ == nullptr) {
        return;
    }

    // readyRead can deliver half a message or three messages at once, so
    // we feed the bytes to the framer and then pull out every complete
    // message it can find.
    framer_.appendData(clientSocket_->readAll());

    SecsMessage message;
    while (true) {
        const FrameResult result = framer_.nextMessage(message);
        if (result == FrameResult::NeedMoreData) {
            break;
        }
        if (result == FrameResult::ProtocolError) {
            emit logMessage(QStringLiteral("Equipment received a malformed frame; closing connection"));
            framer_.clear();
            clientSocket_->disconnectFromHost();
            return;
        }
        handleMessage(message);
    }
}

void EquipmentServer::handleMessage(const SecsMessage& message) {
    emit logMessage(QStringLiteral("Equipment RX  %1").arg(describe(message)));

    if (isMessage(message, 1, 1)) {
        // S1F1 "Are You There" -> answer with S1F2 "On Line Data".
        sendMessage(makeOnLineData(message.transactionId, QStringLiteral("SIM-EQP"), QStringLiteral("1.0")));
    } else if (isMessage(message, 2, 41)) {
        handleRemoteCommand(message);
    } else if (isMessage(message, 6, 12)) {
        // The host acknowledged one of our S6F11 event reports. Nothing to do.
    } else {
        emit logMessage(QStringLiteral("Equipment does not support this message; ignored"));
    }
}

void EquipmentServer::handleRemoteCommand(const SecsMessage& message) {
    const QString remoteCommand = payloadField(message, QStringLiteral("RCMD"));

    if (remoteCommand == QStringLiteral("START")) {
        emit commandRequested(CommandType::StartJob);
        sendMessage(makeRemoteCommandAck(message.transactionId, kHcackWillPerformLater));
    } else if (remoteCommand == QStringLiteral("RESET")) {
        emit commandRequested(CommandType::Reset);
        sendMessage(makeRemoteCommandAck(message.transactionId, kHcackWillPerformLater));
    } else {
        sendMessage(makeRemoteCommandAck(message.transactionId, kHcackCommandDoesNotExist));
    }
}

void EquipmentServer::sendEventReport(StateId oldState, StateId newState) {
    if (clientSocket_ == nullptr) {
        emit logMessage(QStringLiteral("Equipment has no host connected; S6F11 not sent"));
        return;
    }

    sendMessage(makeEventReport(nextTransactionId_++,
                                QString::fromStdString(toString(oldState)),
                                QString::fromStdString(toString(newState))));
}

void EquipmentServer::sendMessage(const SecsMessage& message) {
    if (clientSocket_ == nullptr) {
        return;
    }
    clientSocket_->write(SecsFramer::encode(message));
    emit logMessage(QStringLiteral("Equipment TX  %1").arg(describe(message)));
}
