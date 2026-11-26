#include "network/frame.h"
#include "core/types.h"
#include "core/contracts.h"

#include <QTest>
#include <QtEndian>
#include <type_traits>

using namespace cloudya::network;

class FrameTests : public QObject {
    Q_OBJECT
private slots:
    void splitAndConcatenatedFrames();
    void boundariesAndInvalidInput();
};

void FrameTests::splitAndConcatenatedFrames() {
    const QByteArray first = *encodeFrame({MessageType::Hello, "{\"protocol_version\":1}"});
    const QByteArray second = *encodeFrame({MessageType::SyncStart, "{}"});
    FrameDecoder decoder;
    auto part = decoder.feed(QByteArrayView(first.constData(), 3));
    QVERIFY(part.ok());
    QVERIFY(part.frames.isEmpty());
    const QByteArray rest = first.mid(3) + second;
    part = decoder.feed(QByteArrayView(rest));
    QVERIFY(part.ok());
    QCOMPARE(part.frames.size(), 2);
    QCOMPARE(part.frames[0].type, MessageType::Hello);
    QCOMPARE(part.frames[1].type, MessageType::SyncStart);
    QCOMPARE(decoder.bufferedBytes(), 0);
}

void FrameTests::boundariesAndInvalidInput() {
    static_assert(std::is_abstract_v<cloudya::IndexStore>);
    static_assert(std::is_abstract_v<cloudya::PeerTransport>);
    static_assert(std::is_abstract_v<cloudya::SyncController>);
    QVERIFY(QMetaType::fromType<cloudya::SyncStatus>().isValid());
    QVERIFY(QMetaType::fromType<cloudya::network::Frame>().isValid());
    cloudya::FileVersion version;
    QCOMPARE(version.availability, cloudya::ContentAvailability::Missing);
    const QByteArray payload(kMaxFrameBody - 2, 'x');
    const auto largest = encodeFrame({MessageType::FileChunk, payload});
    QVERIFY(largest.has_value());
    QCOMPARE(largest->size(), qsizetype(kMaxFrameBody + 4));
    QVERIFY(!encodeFrame({MessageType::FileChunk, payload + 'x'}).has_value());

    FrameDecoder decoder;
    QByteArray oversized(4, '\0');
    qToBigEndian<quint32>(kMaxFrameBody + 1, reinterpret_cast<uchar *>(oversized.data()));
    auto result = decoder.feed(QByteArrayView(oversized));
    QVERIFY(!result.ok());
    decoder.reset();

    QByteArray unknown = *encodeFrame({MessageType::Hello, {}});
    qToBigEndian<quint16>(99, reinterpret_cast<uchar *>(unknown.data() + 4));
    result = decoder.feed(QByteArrayView(unknown));
    QVERIFY(!result.ok());

    decoder.reset();
    const QByteArray valid = *encodeFrame({MessageType::Hello, {}});
    result = decoder.feed(QByteArrayView(valid + unknown));
    QVERIFY(!result.ok());
    QVERIFY(result.frames.isEmpty());
}

QTEST_GUILESS_MAIN(FrameTests)
#include "frame_tests.moc"
