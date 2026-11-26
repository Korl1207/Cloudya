#pragma once

#include <QByteArray>
#include <QHash>
#include <QHostAddress>
#include <QSet>
#include <QString>
#include <QUuid>

namespace cloudya {

struct DeviceIdentity {
    QUuid deviceId;
    QString displayName;
    QByteArray certificateSha256; // Exactly 32 bytes, SHA-256 of certificate DER.
};

struct PeerEndpoint {
    QUuid deviceId;
    QHostAddress address;
    quint16 port = 0;
};

struct SharedFolder {
    QUuid folderId;
    QUuid ownerDeviceId;
    QString displayName;
};

struct LocalFolderBinding {
    QUuid folderId;
    QString absoluteRoot;
    QSet<QUuid> allowedPeerIds;
};

enum class ContentAvailability { Available, Missing, Pending };

struct FileVersion {
    QUuid versionId;
    QUuid folderId;
    QString relativePath; // Protocol separators are '/'; never an absolute path.
    quint64 size = 0;
    QByteArray sha256; // Exactly 32 bytes.
    QHash<QUuid, quint64> vector;
    ContentAvailability availability = ContentAvailability::Missing;
};

enum class SyncPhase { Idle, Connecting, Pairing, Syncing, Conflict, Error, Complete };

struct SyncStatus {
    SyncPhase phase = SyncPhase::Idle;
    QUuid folderId;
    QString relativePath;
    quint64 bytesDone = 0;
    quint64 bytesTotal = 0;
    QString errorCode;
    QString errorMessage;
};

} // namespace cloudya
