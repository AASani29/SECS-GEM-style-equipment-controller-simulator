#include "HostClient.h"

#include <QHostAddress>

HostClient::HostClient(quint16 port, QObject* parent)
    : QObject(parent), port_(port), socket_(new QTcpSocket(this)) {
    connect(socket_, &QTcpSocket::connected, this, &HostClient::onSocketConnected);
    connect(socket_, &QTcpSocket::disconnected, this, &HostClient::onSocketDisconnected);
    connect(socket_, &QTcpSocket::readyRead, this, &HostClient::onReadyRead);
    connect(socket_, &QAbstractSocket::errorOccurred, this, &HostClient::onSocketError);
}

void HostClient::connectToEquipment() {
    if (socket_->state() != QAbstractSocket::UnconnectedState) {
        emit logMessage(QStringLiteral("Host is already connected (or connecting)"));
        return;
    }

    emit logMessage(QStringLiteral("Host connecting to 127.0.0.1:%1 ...").arg(port_));
    socket_->connectToHost(QHostAddress::LocalHost, port_);
}

void HostClient::onSocketConnected() {
    framer_.clear();
    emit logMessage(QStringLiteral("Host connected to equipment"));
    emit connectionChanged(true);
}

void HostClient::onSocketDisconnected() {
    emit logMessage(QStringLiteral("Host lost connection to equipment"));
    emit connectionChanged(false);
}

void HostClient::onSocketError(QAbstractSocket::SocketError /*socketError*/) {
    emit logMessage(QStringLiteral("Host socket error: %1").arg(socket_->errorString()));
}

void HostClient::sendAreYouThere() {
    sendMessage(makeAreYouThere(nextTransactionId_++));
}

void HostClient::sendRemoteCommand(const QString& remoteCommand) {
    sendMessage(makeRemoteCommand(nextTransactionId_++, remoteCommand));
}

void HostClient::sendMessage(const SecsMessage& message) {
    if (socket_->state() != QAbstractSocket::ConnectedState) {
        emit logMessage(QStringLiteral("Host is not connected; message not sent"));
        return;
    }
    socket_->write(SecsFramer::encode(message));
    emit logMessage(QStringLiteral("Host TX  %1").arg(describe(message)));
}

void HostClient::onReadyRead() {
    framer_.appendData(socket_->readAll());

    SecsMessage message;
    while (true) {
        const FrameResult result = framer_.nextMessage(message);
        if (result == FrameResult::NeedMoreData) {
            break;
        }
        if (result == FrameResult::ProtocolError) {
            emit logMessage(QStringLiteral("Host received a malformed frame; closing connection"));
            framer_.clear();
            socket_->disconnectFromHost();
            return;
        }
        handleMessage(message);
    }
}

void HostClient::handleMessage(const SecsMessage& message) {
    emit logMessage(QStringLiteral("Host RX  %1").arg(describe(message)));

    if (isMessage(message, 6, 11)) {
        // S6F11 event report from the equipment: acknowledge it with S6F12.
        sendMessage(makeEventReportAck(message.transactionId));
    }
    // S1F2 and S2F42 are replies to our own requests; logging them above
    // is all the host needs to do with them.
}
