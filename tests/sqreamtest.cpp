#define private public
#include <coro/audio/AlsaSink.h>
#undef private
#include <coro/audio/AppSource.h>
#include <coro/audio/SbcDecoder.h>

#include <QAudioOutput>
#include <QCoreApplication>
#include <QNetworkDatagram>
#include <QTimer>
#include <QUdpSocket>

using namespace coro;
using namespace coro::audio;


#include <coro/audio/ScreamSource.h>

#include <thread>
#include <asio/io_service.hpp>
#include <asio/placeholders.hpp>
#include <asio/steady_timer.hpp>
#include <asio/strand.hpp>
#include <asio/ip/address.hpp>
#include <asio/ip/multicast.hpp>
#include <asio/ip/udp.hpp>
#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include <boost/system/error_code.hpp>
#include <loguru/loguru.hpp>

using namespace asio;
namespace ph = asio::placeholders;

class UdpWorker2
{
public:
    UdpWorker2(AlsaSink& sink) :
        m_alsaSink(sink),
        m_timeout(m_ioService, asio::chrono::seconds(5)),
        m_socket(m_ioService),
        m_localEndpoint(ip::address::from_string("0.0.0.0"), 4010),
        m_buffer(3+5+(1152))
    {
        m_socket.open(m_localEndpoint.protocol());
        m_socket.set_option(ip::udp::socket::reuse_address(true));
        m_socket.bind(m_localEndpoint);
        m_socket.set_option(asio::ip::multicast::join_group(ip::address::from_string("239.255.77.77")));
        doReceive();
        startTimer();
    }

    ~UdpWorker2()
    {
    }

    void startTimer()
    {
        m_timeout.expires_at(m_timeout.expiry() + chrono::seconds(5));
        m_timeout.async_wait(std::bind(&UdpWorker2::onTimeout, this));
    }

    void onTimeout()
    {
        startTimer();

        m_isStarted = !m_isStarted;

        if (m_isStarted) {
            doReceive();
        }

        // 5 time outs without buffer reception, stop.
        if (m_bufferCount < -4) {
            LOG_F(INFO, "Time out. No buffers received for a while.");
            m_bufferCount = 0;
            return;
        }

        // No buffers received since last timeout.
        if (m_bufferCount <= 0) {
            --m_bufferCount;
        } else {
            m_bufferCount = 0;
        }
    }

    void doReceive()
    {
        m_buffer.acquire(3+5+1152);
        m_buffer.commit(3+5+1152);
        m_socket.async_receive_from(
                    asio::buffer(m_buffer.data()+3, 5+1152),
                    m_remoteEndpoint,
                    boost::bind(&UdpWorker2::onReceive, this, ph::bytes_transferred));
    }

    void onReceive(std::size_t bytesTransferred)
    {
        if (!m_isStarted) {
            m_buffer.clear();
            return;
        }

        if (bytesTransferred != (5+1152)) {
            LOG_F(ERROR, "invalid bytes count: %d", bytesTransferred);
            m_buffer.clear();
            goto end;
        }

        ++m_bufferCount;
        m_buffer.trimFront(8);
        m_alsaSink.process( { audio::AudioCodec::RawInt16,
                               SampleRate::Rate44100,
                               audio::Channels::Stereo },
                             m_buffer);
end:
        doReceive();
    }

    AlsaSink&           m_alsaSink;
    io_service          m_ioService;
    steady_timer        m_timeout;
    ip::udp::socket     m_socket;
    ip::udp::endpoint   m_localEndpoint;
    ip::udp::endpoint   m_remoteEndpoint;
    int                 m_bufferCount = 0;
    AudioBuffer         m_buffer;
    bool                m_isStarted = false;
};

int main(int argc, char *argv[])
{
    AlsaSink sink;
    UdpWorker2 udpWorker(sink);

    std::thread thread([&]() {
        udpWorker.m_ioService.run();
    });

    while(true) {
        sleep(1);
    }

    return 1;
}

int main1(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    bool isStarted = false;

    AudioBuffer buffer(3+5+1152);
    AlsaSink sink;

    QUdpSocket socket;
    socket.bind(QHostAddress::AnyIPv4, 4010, QUdpSocket::ShareAddress);
    socket.joinMulticastGroup(QHostAddress("239.255.77.77"));

    QTimer timer;
    timer.setSingleShot(false);
    QObject::connect(&timer, &QTimer::timeout, [&]() {
        isStarted = !isStarted;
    });
    timer.start(5000);

    QObject::connect(&socket, &QUdpSocket::readyRead, [&]() {
        while (socket.hasPendingDatagrams()) {
            auto data = buffer.acquire(3+5+1152);
            assert(socket.readDatagram(data+3, 5+1152) == 5+1152);
            buffer.commit(3+5+1152);
            buffer.trimFront(8);

            if (isStarted) {
                sink.process({ AudioCodec::RawInt16, SampleRate::Rate44100, Channels::Stereo }, buffer);
            } else {
                buffer.clear();
            }
        }
    });

    return a.exec();
}

int main2(int argc, char *argv[])
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

