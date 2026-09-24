#include "SecsMessage.h"

#include <QStringList>

namespace {

SecsMessage makeMessage(quint8 stream, quint8 function, quint32 transactionId,
                        const QString& payloadText = QString()) {
    SecsMessage message;
    message.stream = stream;
    message.function = function;
    message.transactionId = transactionId;
    message.payload = payloadText.toUtf8();
    return message;
}

QString messageName(quint8 stream, quint8 function) {
    if (stream == 1 && function == 1) {
        return QStringLiteral("Are You There");
    }
    if (stream == 1 && function == 2) {
        return QStringLiteral("On Line Data");
    }
    if (stream == 2 && function == 41) {
        return QStringLiteral("Host Command Send");
    }
    if (stream == 2 && function == 42) {
        return QStringLiteral("Host Command Acknowledge");
    }
    if (stream == 6 && function == 11) {
        return QStringLiteral("Event Report Send");
    }
    if (stream == 6 && function == 12) {
        return QStringLiteral("Event Report Acknowledge");
    }
    return QStringLiteral("Unknown");
}

} // namespace

SecsMessage makeAreYouThere(quint32 transactionId) {
    return makeMessage(1, 1, transactionId);
}

SecsMessage makeOnLineData(quint32 transactionId, const QString& modelName,
                           const QString& softwareRevision) {
    const QString text = QStringLiteral("MDLN=%1;SOFTREV=%2").arg(modelName, softwareRevision);
    return makeMessage(1, 2, transactionId, text);
}

SecsMessage makeRemoteCommand(quint32 transactionId, const QString& remoteCommand) {
    return makeMessage(2, 41, transactionId, QStringLiteral("RCMD=%1").arg(remoteCommand));
}

SecsMessage makeRemoteCommandAck(quint32 transactionId, quint8 hcack) {
    return makeMessage(2, 42, transactionId, QStringLiteral("HCACK=%1").arg(static_cast<int>(hcack)));
}

SecsMessage makeEventReport(quint32 transactionId, const QString& oldStateName,
                            const QString& newStateName) {
    const QString text =
        QStringLiteral("CEID=STATE_CHANGE;FROM=%1;TO=%2").arg(oldStateName, newStateName);
    return makeMessage(6, 11, transactionId, text);
}

SecsMessage makeEventReportAck(quint32 transactionId) {
    return makeMessage(6, 12, transactionId, QStringLiteral("ACKC6=0"));
}

QString payloadField(const SecsMessage& message, const QString& key) {
    const QString text = QString::fromUtf8(message.payload);
    const QStringList pairs = text.split(QLatin1Char(';'), Qt::SkipEmptyParts);

    for (const QString& pair : pairs) {
        const int equalsIndex = pair.indexOf(QLatin1Char('='));
        if (equalsIndex < 0) {
            continue;
        }
        if (pair.left(equalsIndex) == key) {
            return pair.mid(equalsIndex + 1);
        }
    }
    return QString();
}

QString describe(const SecsMessage& message) {
    QString text = QStringLiteral("S%1F%2 %3 (tid=%4)")
                       .arg(static_cast<int>(message.stream))
                       .arg(static_cast<int>(message.function))
                       .arg(messageName(message.stream, message.function))
                       .arg(message.transactionId);

    if (!message.payload.isEmpty()) {
        text += QStringLiteral(" [%1]").arg(QString::fromUtf8(message.payload));
    }
    return text;
}
