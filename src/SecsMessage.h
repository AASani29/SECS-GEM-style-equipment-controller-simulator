#pragma once

#include <QByteArray>
#include <QString>
#include <QtGlobal>

// One SECS-II-style message, simplified for learning (see the README for
// exactly what is and isn't modelled). "S2F41" means stream 2, function
// 41: stream groups related messages, function picks one within it.
//
// The transaction id lets a sender match a reply to its request: the
// reply carries the same id as the request it answers.
struct SecsMessage {
    quint8 stream = 0;
    quint8 function = 0;
    quint32 transactionId = 0;
    QByteArray payload; // plain ASCII text such as "RCMD=START"
};

// Result codes carried in the S2F42 reply payload (HCACK).
constexpr quint8 kHcackCommandDoesNotExist = 1;
constexpr quint8 kHcackWillPerformLater = 4; // accepted; completion is reported by an event

// True if the message is exactly S<stream>F<function>.
inline bool isMessage(const SecsMessage& message, quint8 stream, quint8 function) {
    return message.stream == stream && message.function == function;
}

// --- Builders: one per message type we support ---

SecsMessage makeAreYouThere(quint32 transactionId);                                    // S1F1
SecsMessage makeOnLineData(quint32 transactionId, const QString& modelName,
                           const QString& softwareRevision);                           // S1F2
SecsMessage makeRemoteCommand(quint32 transactionId, const QString& remoteCommand);    // S2F41
SecsMessage makeRemoteCommandAck(quint32 transactionId, quint8 hcack);                 // S2F42
SecsMessage makeEventReport(quint32 transactionId, const QString& oldStateName,
                            const QString& newStateName);                              // S6F11
SecsMessage makeEventReportAck(quint32 transactionId);                                 // S6F12

// Payloads are "KEY=value;KEY=value" text. Returns the value for `key`,
// or an empty string if the key isn't present.
QString payloadField(const SecsMessage& message, const QString& key);

// Human-readable one-liner for the log, e.g.
// "S2F41 Host Command Send (tid=3) [RCMD=START]".
QString describe(const SecsMessage& message);
