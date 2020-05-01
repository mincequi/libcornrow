#include <thread>
#include <asio/io_service.hpp>
#include <asio/placeholders.hpp>
#include <asio/steady_timer.hpp>
#include <asio/ip/address.hpp>
#include <asio/ip/multicast.hpp>
#include <asio/ip/udp.hpp>
#include <boost/system/error_code.hpp>
#include <loguru/loguru.hpp>

#include <coro/audio/AudioBuffer.h>
#include <coro/audio/Source.h>

namespace coro {
namespace core {

class UdpSource : public audio::Source
{
public:
    struct Config {
        uint8_t prePadding = 0;
        uint16_t mtu = 1492;
    };

    UdpSource();
    explicit UdpSource(const Config& config);
    ~UdpSource();

    static constexpr std::array<audio::AudioCaps,1> outCaps() {
        return {{ { audio::AudioCodecs::Any,
                    audio::SampleRates::Any,
                    audio::ChannelFlags::Any } }};
    }

private:
    void _start();
    void startTimer();
    void doReceive();
    void onReceive(std::size_t bytesTransferred);
    void onTimeout();

    Config m_config;

    asio::io_service          m_ioService;
    asio::ip::udp::socket     m_socket;
    asio::ip::udp::endpoint   m_localEndpoint;
    asio::ip::udp::endpoint   m_remoteEndpoint;
    std::thread         m_thread;
    asio::steady_timer  m_timeout;
    int                 m_bufferCount = 0;
    bool                m_isReceiving = false;
    audio::AudioBuffer  m_buffer;
    size_t              m_previousBytesTransferred = 0;
};

} // namespace core
} // namespace coro
