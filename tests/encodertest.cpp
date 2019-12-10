#define protected public
#include <coro/audio/AlsaSink.h>
#include <coro/audio/AudioEncoderFfmpeg.h>
#include <coro/audio/FileSink.h>
#undef protected
#include <coro/audio/AudioTypes.h>

#include <chrono>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <random>

using namespace coro;
using namespace coro::audio;

template <class T>
std::vector<T> generateWhiteNoise(std::uint16_t seconds = 100)
{
    std::random_device rd;
    std::mt19937 gen(rd());

    std::vector<T> samples(2*44100*seconds);
    std::uniform_real_distribution<> dist(-1.0, 1.0);
    if (typeid(T) == typeid(int16_t)) {
        for (auto & s : samples) {
            s = 32767.0*dist(gen);
        }
    } else if (typeid(T) == typeid(int32_t)) {
        for (auto & s : samples) {
            s = 2147483647.0*dist(gen);
        }
    } else {
        for (auto & s : samples) {
            s = dist(gen);
        }
    }

    return samples;
}

template <class T>
void runTest(std::string filename, std::uint16_t seconds = 100, int cycles = 1)
{
    // Read
    auto begin = std::chrono::steady_clock::now();
    auto samples = generateWhiteNoise<T>(seconds);

    auto end = std::chrono::steady_clock::now();
    std::chrono::duration<double> diff = end-begin;
    std::cout << "Init time: " << diff.count() << std::endl;

    // Process
    audio::AudioEncoderFfmpeg encoder;
    audio::AlsaSink sink;
    sink.setDevice("iec958:CARD=sndrpihifiberry,DEV=0");
    //sink.setFileName(filename);
    //sink.start();
    audio::Node::link(encoder, sink);
    encoder.start({ Codec::RawFloat32, SampleRate::Rate44100, Channels::Stereo });

    for (int i = 0; i < cycles; ++i) {
        AudioBuffer buffer((char*)samples.data(), samples.size()*4);
        encoder.process({ Codec::RawFloat32, SampleRate::Rate44100, Channels::Stereo }, buffer);
    }

    sink.stop();
    end = std::chrono::steady_clock::now();
    diff = end-begin;
    std::cout << "Process time: " << diff.count() << std::endl;
}

int main()
{
    // Init
    std::cout << std::endl << "#### AC3 test ####" << std::endl;
    runTest<float>("testFloatFloat.ac3", 1, 10);
    //runTest<int32_t,int64_t>("testInt32Int64.raw", 100);

    return 0;
}
