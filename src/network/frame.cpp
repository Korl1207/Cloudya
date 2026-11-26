#include "network/frame.h"

#include <QtEndian>

namespace cloudya::network {

namespace {
constexpr qsizetype kMaxFramesPerFeed = 4096;
}

bool isKnownMessageType(quint16 value) {
    return value >= static_cast<quint16>(MessageType::Hello)
        && value <= static_cast<quint16>(MessageType::SyncStart);
}

std::optional<QByteArray> encodeFrame(const Frame &frame) {
    const auto type = static_cast<quint16>(frame.type);
    if (!isKnownMessageType(type) || frame.payload.size() > kMaxFrameBody - 2)
        return std::nullopt;
    QByteArray bytes(6, '\0');
    qToBigEndian<quint32>(static_cast<quint32>(frame.payload.size() + 2),
                          reinterpret_cast<uchar *>(bytes.data()));
    qToBigEndian<quint16>(type, reinterpret_cast<uchar *>(bytes.data() + 4));
    bytes.append(frame.payload);
    return bytes;
}

DecodeResult FrameDecoder::feed(QByteArrayView bytes) {
    DecodeResult result;
    if (failed_) {
        result.error = QStringLiteral("Decoder is in failed state");
        return result;
    }
    if (bytes.size() > kMaxReadBuffer - buffer_.size()) {
        failed_ = true;
        buffer_.clear();
        result.error = QStringLiteral("Read buffer limit exceeded");
        return result;
    }
    buffer_.append(bytes.data(), bytes.size());
    while (buffer_.size() >= 4) {
        if (result.frames.size() >= kMaxFramesPerFeed) {
            result.error = QStringLiteral("Too many frames in one read");
            break;
        }
        const quint32 length = qFromBigEndian<quint32>(
            reinterpret_cast<const uchar *>(buffer_.constData()));
        if (length < 2 || length > kMaxFrameBody) {
            result.error = QStringLiteral("Invalid frame length");
            break;
        }
        const qsizetype frameSize = static_cast<qsizetype>(length) + 4;
        if (buffer_.size() < frameSize)
            break;
        const quint16 rawType = qFromBigEndian<quint16>(
            reinterpret_cast<const uchar *>(buffer_.constData() + 4));
        if (!isKnownMessageType(rawType)) {
            result.error = QStringLiteral("Unknown message type");
            break;
        }
        result.frames.append({static_cast<MessageType>(rawType),
                              buffer_.mid(6, length - 2)});
        buffer_.remove(0, frameSize);
    }
    if (!result.ok()) {
        result.frames.clear();
        failed_ = true;
        buffer_.clear();
    }
    return result;
}

void FrameDecoder::reset() {
    buffer_.clear();
    failed_ = false;
}

} // namespace cloudya::network
