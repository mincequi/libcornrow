#include <coro/audio/AppSource.h>
#include <coro/audio/SbcDecoder.h>

#include <QAudioOutput>
#include <QCoreApplication>
#include <QNetworkDatagram>
#include <QUdpSocket>

using namespace coro::audio;

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

