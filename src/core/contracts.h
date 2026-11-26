#pragma once

#include "core/types.h"
#include "network/frame.h"

#include <QByteArrayView>
#include <QObject>
#include <QVector>
#include <optional>

namespace cloudya {

// Empty code means success. All failures have a stable code and a diagnostic message.
struct OperationResult {
    QString code;
    QString message;
    bool ok() const { return code.isEmpty(); }
};

template <typename T> struct ValueResult {
    std::optional<T> value;
    OperationResult result;
    bool ok() const { return result.ok() && value.has_value(); }
};

struct IndexSnapshot {
    QUuid folderId;
    QUuid snapshotId;
};

struct IndexPage {
    QVector<FileVersion> entries;
    quint64 nextOffset = 0;
    bool atEnd = false;
};

// Synchronous operations. The caller runs scans, hashing and disk I/O off the UI thread.
// Snapshots remain immutable until releaseSnapshot, including across page reads.
class IndexStore {
public:
    virtual ~IndexStore() = default;
    virtual OperationResult recoverPending() = 0;
    virtual OperationResult scan(const QUuid &folderId) = 0;
    virtual ValueResult<IndexSnapshot> createSnapshot(const QUuid &folderId) = 0;
    virtual ValueResult<IndexPage> readPage(const IndexSnapshot &snapshot,
                                            quint64 offset, quint32 maxEntries) = 0;
    virtual void releaseSnapshot(const IndexSnapshot &snapshot) = 0;
    virtual ValueResult<FileVersion> findVersion(const QUuid &versionId) = 0;
    virtual ValueResult<QByteArray> readChunk(const QUuid &versionId,
                                              quint64 offset, quint32 maxBytes) = 0;
    virtual OperationResult beginReceive(const QUuid &transferId,
                                          const FileVersion &version) = 0;
    virtual OperationResult appendReceive(const QUuid &transferId, quint64 offset,
                                           QByteArrayView bytes) = 0;
    virtual OperationResult commitReceive(const QUuid &transferId) = 0;
    virtual void abortReceive(const QUuid &transferId) = 0;
};

// Calls enqueue work and return without blocking the UI thread. A peer ID is emitted
// only after mutual TLS and the pinned certificate check succeed.
class PeerTransport : public QObject {
    Q_OBJECT
public:
    using QObject::QObject;
    virtual OperationResult listen(quint16 port) = 0;
    virtual OperationResult connectTo(const PeerEndpoint &endpoint) = 0;
    virtual OperationResult send(const QUuid &peerId, const network::Frame &frame) = 0;
    virtual void disconnectFrom(const QUuid &peerId) = 0;

signals:
    void peerAuthenticated(const QUuid &peerId);
    void frameReceived(const QUuid &peerId, const network::Frame &frame);
    void peerDisconnected(const QUuid &peerId, const QString &reason);
};

// Owns the exchange state machine. Signals report outcomes to the existing window.
class SyncController : public QObject {
    Q_OBJECT
public:
    using QObject::QObject;
    virtual OperationResult invite(const QUuid &folderId, const QUuid &peerId) = 0;
    virtual OperationResult start(const QUuid &folderId, const QUuid &peerId) = 0;
    virtual OperationResult resolveConflict(const QUuid &folderId,
                                             const QString &relativePath,
                                             const QUuid &chosenVersionId) = 0;

signals:
    void statusChanged(const SyncStatus &status);
};

} // namespace cloudya
