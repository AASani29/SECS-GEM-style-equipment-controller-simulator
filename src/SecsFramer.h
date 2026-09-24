#pragma once

#include "SecsMessage.h"

#include <QByteArray>

// TCP is a stream of bytes, not a stream of messages: one send() on the
// other side can arrive as several readyRead() chunks, and several sends
// can arrive glued together in one chunk. The framer's job is to cut that
// byte stream back into whole messages.
//
// Wire format (all multi-byte numbers are big-endian):
//
//   bytes 0-3    length: number of bytes that follow this field
//                (6 + payload size)
//   byte  4      stream
//   byte  5      function
//   bytes 6-9    transaction id
//   bytes 10...  payload
//
// Real SECS-II over HSMS uses a 10-byte header (session id, W-bit, PType,
// SType, system bytes) and typed data items instead of text. This is a
// deliberately simpler layout with the same idea: length-prefixed frames.

enum class FrameResult {
    NeedMoreData,  // not enough bytes yet for a whole message
    MessageReady,  // one complete message was decoded
    ProtocolError  // the length field is impossible; the stream is garbage
};

class SecsFramer {
public:
    // Turns a message into bytes ready to write to a socket.
    static QByteArray encode(const SecsMessage& message);

    // Call with whatever bytes just arrived from the socket.
    void appendData(const QByteArray& data);

    // Try to pull one complete message out of the buffered bytes. Call it
    // in a loop until it stops returning MessageReady.
    FrameResult nextMessage(SecsMessage& message);

    // Throw away any buffered bytes (used when a connection is reset).
    void clear();

private:
    static constexpr int kLengthFieldSize = 4;
    static constexpr int kHeaderSizeAfterLength = 6; // stream + function + transaction id
    static constexpr quint32 kMaxMessageLength = 1024 * 1024; // sanity limit

    QByteArray buffer_;
};
