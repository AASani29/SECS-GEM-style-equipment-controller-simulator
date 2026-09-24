#include "SecsFramer.h"

namespace {

void appendUInt32BigEndian(QByteArray& bytes, quint32 value) {
    bytes.append(static_cast<char>((value >> 24) & 0xFF));
    bytes.append(static_cast<char>((value >> 16) & 0xFF));
    bytes.append(static_cast<char>((value >> 8) & 0xFF));
    bytes.append(static_cast<char>(value & 0xFF));
}

quint32 readUInt32BigEndian(const unsigned char* bytes) {
    return (static_cast<quint32>(bytes[0]) << 24) |
           (static_cast<quint32>(bytes[1]) << 16) |
           (static_cast<quint32>(bytes[2]) << 8) |
           static_cast<quint32>(bytes[3]);
}

} // namespace

QByteArray SecsFramer::encode(const SecsMessage& message) {
    const quint32 length = static_cast<quint32>(kHeaderSizeAfterLength) +
                           static_cast<quint32>(message.payload.size());

    QByteArray bytes;
    appendUInt32BigEndian(bytes, length);
    bytes.append(static_cast<char>(message.stream));
    bytes.append(static_cast<char>(message.function));
    appendUInt32BigEndian(bytes, message.transactionId);
    bytes.append(message.payload);
    return bytes;
}

void SecsFramer::appendData(const QByteArray& data) {
    buffer_.append(data);
}

void SecsFramer::clear() {
    buffer_.clear();
}

FrameResult SecsFramer::nextMessage(SecsMessage& message) {
    if (buffer_.size() < kLengthFieldSize) {
        return FrameResult::NeedMoreData;
    }

    const unsigned char* bytes = reinterpret_cast<const unsigned char*>(buffer_.constData());
    const quint32 length = readUInt32BigEndian(bytes);

    // A length smaller than the header, or absurdly large, means we've
    // lost sync with the sender. Better to report it than to wait forever
    // for a gigabyte that will never come.
    if (length < static_cast<quint32>(kHeaderSizeAfterLength) || length > kMaxMessageLength) {
        return FrameResult::ProtocolError;
    }

    const int totalFrameSize = kLengthFieldSize + static_cast<int>(length);
    if (buffer_.size() < totalFrameSize) {
        return FrameResult::NeedMoreData;
    }

    message.stream = bytes[kLengthFieldSize];
    message.function = bytes[kLengthFieldSize + 1];
    message.transactionId = readUInt32BigEndian(bytes + kLengthFieldSize + 2);

    const int payloadStart = kLengthFieldSize + kHeaderSizeAfterLength;
    const int payloadSize = static_cast<int>(length) - kHeaderSizeAfterLength;
    message.payload = buffer_.mid(payloadStart, payloadSize);

    buffer_.remove(0, totalFrameSize);
    return FrameResult::MessageReady;
}
