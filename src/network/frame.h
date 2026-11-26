#pragma once

#include <QByteArray>
#include <QByteArrayView>
#include <QString>
#include <QVector>
#include <optional>

namespace cloudya::network {

constexpr quint32 kMaxFrameBody = 131072; // Type + payload; excludes 4-byte length.
constexpr qsizetype kMaxReadBuffer = 2 * 1024 * 1024;

enum class MessageType : quint16 {
    Hello = 1, HelloAck, FolderInvite, FolderAccept, FolderReject,
    IndexPage, IndexEnd, FileRequest, FileBegin, FileChunk, FileEnd,
    FileStored, SyncComplete, Error, SyncStart
};

bool isKnownMessageType(quint16 value);

struct Frame {
    MessageType type;
    QByteArray payload;
};

// Returns nullopt if type is unknown or the payload exceeds the 128 KiB frame limit.
std::optional<QByteArray> encodeFrame(const Frame &frame);

struct DecodeResult {
    QVector<Frame> frames;
    QString error;
    bool ok() const { return error.isEmpty(); }
};

class FrameDecoder {
public:
    DecodeResult feed(QByteArrayView bytes);
    void reset();
    qsizetype bufferedBytes() const { return buffer_.size(); }

private:
    QByteArray buffer_;
    bool failed_ = false;
};

} // namespace cloudya::network
