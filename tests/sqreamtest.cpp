#define private public
#include <coro/audio/AlsaSink.h>
#undef private
#include <coro/audio/AppSource.h>
#include <coro/audio/SbcDecoder.h>

#include <QAudioOutput>
#include <QCoreApplication>
#include <QNetworkDatagram>
#include <QUdpSocket>

using namespace coro::audio;

int main2(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    AudioBuffer buffer(3+5+1152);
    AlsaSink sink;

    QUdpSocket socket;
    socket.bind(QHostAddress::AnyIPv4, 4010, QUdpSocket::ShareAddress);
    socket.joinMulticastGroup(QHostAddress("239.255.77.77"));
    QObject::connect(&socket, &QUdpSocket::readyRead, [&]() {
        while (socket.hasPendingDatagrams()) {
            auto data = buffer.acquire(3+5+1152);
            assert(socket.readDatagram(data+3, 5+1152) == 5+1152);
            buffer.commit(3+5+1152);

            sink.process({ AudioCodec::RawInt16, SampleRate::Rate44100, Channels::Stereo }, buffer);
        }
    });

    return a.exec();
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    QAudioFormat format;
    format.setSampleRate(44100);
    format.setChannelCount(2);
    format.setSampleSize(16);
    format.setCodec("audio/pcm");
    format.setByteOrder(QAudioFormat::LittleEndian);
    format.setSampleType(QAudioFormat::SignedInt);

    QAudioOutput audioOut(format);
    auto io = audioOut.start();

    QUdpSocket socket;
    socket.bind(QHostAddress::AnyIPv4, 4010, QUdpSocket::ShareAddress);
    socket.joinMulticastGroup(QHostAddress("239.255.77.77"));
    QObject::connect(&socket, &QUdpSocket::readyRead, [&]() {
        while (socket.hasPendingDatagrams()) {
            auto packet = socket.receiveDatagram().data();
            io->write(packet.data()+5, packet.size()-5);
        }
    });

    return a.exec();
}

