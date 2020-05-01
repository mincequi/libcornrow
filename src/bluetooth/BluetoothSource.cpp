#include <coro/bluetooth/BluetoothSource.h>

#include <loguru/loguru.hpp>

namespace coro {
namespace bluetooth {

BluetoothSource::BluetoothSource()
{
}

BluetoothSource::~BluetoothSource()
{
}

const char* BluetoothSource::name() const
{
    return "BluetoothSource";
}

} // namespace bluetooth
} // namespace coro
