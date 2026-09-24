#pragma once

#include "SecsFramer.h"
#include "SecsMessage.h"

#include <QAbstractSocket>
#include <QObject>
#include <QString>
#include <QTcpSocket>

// The "Host" side of the SECS link: connects to the equipment, sends
// requests, and logs the replies and event reports that come back.
//
// THREADING: like EquipmentServer, this object and its QTcpSocket live on
// the network thread and are only ever touched from there. The GUI asks
// it to do things by emitting signals that are connected to the public
// slots below.
class HostClient : public QObject {
    Q_OBJECT

public:
    explicit HostClient(quint16 port, QObject* parent = nullptr);

public slots:
    void connectToEquipment();
    void sendAreYouThere();                              // S1F1
    void sendRemoteCommand(const QString& remoteCommand); // S2F41

signals:
    void logMessage(const QString& text);
    void connectionChanged(bool connected);

private slots:
    void onSocketConnected();
    void onSocketDisconnected();
    void onSocketError(QAbstractSocket::SocketError socketError);
    void onReadyRead();

private:
    void handleMessage(const SecsMessage& message);
    void sendMessage(const SecsMessage& message);

    quint16 port_;
    QTcpSocket* socket_; // child of this, so it moves threads with us
    SecsFramer framer_;
    quint32 nextTransactionId_ = 1;
};
