#include <coro/audio/ScreamSource.h>

#include <thread>
#include <asio/io_service.hpp>
#include <asio/placeholders.hpp>
#include <asio/steady_timer.hpp>
#include <asio/ip/address.hpp>
#include <asio/ip/multicast.hpp>
#include <asio/ip/udp.hpp>
#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include <boost/system/error_code.hpp>
#include <loguru/loguru.hpp>

namespace coro {
namespace audio {

// 1st byte denotes the sampling rate.
//      Bit 7 specifies the base rate: 0 for 48kHz, 1 for 44,1kHz.
//      Other bits specify the multiplier for the base rate.
// 2nd byte denotes the sampling width, in bits.
// 3rd byte denotes the number of channels being transferred.
// 4th and 5th bytes make up the DWORD dwChannelMask from Microsofts WAVEFORMATEXTENSIBLE structure,
//      describing the mapping of channels to speaker positions.

class ScreamHeader
{
public:
    uint8_t rateMultiplier:7;
    uint8_t baseRate:1;
    uint8_t sampleSize;
    uint8_t channelCount;
    uint16_t channelMask;

    bool isValid() const
    {
        return (rateMultiplier == 1 &&
                sampleSize == 16 &&
                channelCount == 2);
    }
    size_t size() const { return 5; }
};

using namespace asio;
namespace ph = asio::placeholders;

class UdpWorker
{
public:
    UdpWorker(Source& source) :
        m_source(source),
        //m_work(m_ioService),
        m_socket(m_ioService),
        m_localEndpoint(ip::address::from_string("0.0.0.0"), 4010),
        m_timeout(m_ioService, asio::chrono::seconds(1)),
        m_buffer(3+5+(1152*7))  // 3 padding, 5 header, 1152 payload, +2*payload (int to float), +2*2*payload (float to crossover)
    {
        m_socket.open(m_localEndpoint.protocol());
        m_socket.set_option(ip::udp::socket::reuse_address(true));
        m_socket.bind(m_localEndpoint);
        // Join the multicast group.
        m_socket.set_option(asio::ip::multicast::join_group(ip::address::from_string("239.255.77.77")));

        // Call doReceive() to give ioService some work. Otherwise it will return immediatly.
        doReceive();
        m_timeout.expires_after(chrono::seconds(1));
        m_timeout.async_wait(std::bind(&UdpWorker::onTimeout, this));
        m_thread = new std::thread([this] { m_ioService.run(); });
    }

    ~UdpWorker()
    {
        m_ioService.stop();
        m_source.stop();

        m_thread->join();
        delete m_thread;
    }

private:
    void doReceive()
    {
        m_buffer.acquire(3+5+1152);
        m_buffer.commit(3+5+1152);
        m_socket.async_receive_from(
                    asio::buffer(m_buffer.data()+3, m_buffer.size()-3),
                    m_remoteEndpoint,
                    boost::bind(&UdpWorker::onReceive, this, ph::error, ph::bytes_transferred));
    }

    void onReceive(const asio::error_code& error, std::size_t bytesTransferred)
    {
        ScreamHeader* header = nullptr;

        if (!m_source.isStarted()) {
            m_source.setWantsToStart(true);
            if (!m_source.isStarted()) {
                LOG_F(2, "%s not started. Will drop buffer.", m_source.name());
                goto end;
            }
            //LOG_F(INFO, "restart timer");
            //m_timeout.expires_after(chrono::seconds(1));
            //m_timeout.async_wait(std::bind(&UdpWorker::onTimeout, this));
        }

        if (error) {
            LOG_F(ERROR, "error receiving data: %s", error.message().c_str());
            goto end;
        } else if (bytesTransferred != (5+1152)) {
            LOG_F(ERROR, "invalid bytes count: %d", bytesTransferred);
            goto end;
        }

        header = (ScreamHeader*)(m_buffer.data()+3);
        if (!header->isValid()) {
            LOG_F(ERROR, "invalid scream header: we only support 44.1/48 khz, S16, Stereo");
            goto end;
        }

        ++m_bufferCount;
        m_buffer.trimFront(header->size()+3);
        m_source.pushBuffer( { audio::AudioCodec::RawInt16,
                               header->baseRate ? SampleRate::Rate44100 : SampleRate::Rate48000,
                               audio::Channels::Stereo },
                             m_buffer);

    end:
        doReceive();
    }

    void onTimeout()
    {
        // Always restart timer.
        m_timeout.expires_at(m_timeout.expiry() + chrono::seconds(1));
        m_timeout.async_wait(std::bind(&UdpWorker::onTimeout, this));

        if (!m_source.isStarted()) {
            return;
        }

        LOG_F(1, "buffer count: %d", m_bufferCount);
        // 5 time outs without buffer reception, stop.
        if (m_bufferCount < -4) {
            LOG_F(INFO, "Time out. No buffers received for quite a while.");
            m_bufferCount = 0;
            m_source.stop();
            m_source.setWantsToStart(false);
            return;
        }

        // No buffers received since last timeout.
        if (m_bufferCount <= 0) {
            --m_bufferCount;
        } else {
            m_bufferCount = 0;
        }
    }

    Source&             m_source;
    io_service          m_ioService;
    ip::udp::socket     m_socket;
    ip::udp::endpoint   m_localEndpoint;
    ip::udp::endpoint   m_remoteEndpoint;
    std::thread*        m_thread;
    steady_timer        m_timeout;
    int                 m_bufferCount = 0;
    AudioBuffer         m_buffer;
};

ScreamSource::ScreamSource()
    : m_udpWorker(new UdpWorker(*this))
{
}

ScreamSource::~ScreamSource()
{
    delete m_udpWorker;
}

const char* ScreamSource::name() const
{
    return "ScreamSource";
}

} // namespace audio
} // namespace coro
