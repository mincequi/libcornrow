#define protected public
#define private public
#include <coro/audio/AlsaSink.h>
#include <coro/audio/AudioAppSink.h>
#include <coro/audio/AudioDecoderFfmpeg.h>
#include <coro/audio/AudioEncoderFfmpeg.h>
#include <coro/audio/FileSink.h>
#undef protected
#undef private
#include <coro/audio/AudioTypes.h>
#include <Gist.h>

#include <chrono>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <random>

using namespace coro;
using namespace coro::audio;

template <class T>
std::vector<T> generateWhiteNoise(uint32_t numSamples = 44100 * 2)
{
    std::random_device rd;
    std::mt19937 gen(rd());

    std::vector<T> samples(numSamples);
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

void runTest(uint32_t numSamples = 2 * 44100, uint32_t numBuffers = 100)
{
    // Read
    auto begin = std::chrono::steady_clock::now();
    auto inSamples = generateWhiteNoise<float>(numSamples);

    auto end = std::chrono::steady_clock::now();
    std::chrono::duration<double> diff = end-begin;
    std::cout << "Init time: " << diff.count() << std::endl;

    // Process
    audio::AudioEncoderFfmpeg encoder(AudioCodec::Ac3);
    audio::AudioDecoderFfmpeg<AudioCodec::Ac3> decoder;
    audio::AudioAppSink       sink;
    audio::FileSink           fileSink;
    encoder.setBitrate(320);
    fileSink.setFileName("test32float.ac3");
    fileSink.onStart();
    //audio::AlsaSink sink;
    //sink.setDevice("iec958:CARD=sndrpihifiberry,DEV=0");
    audio::AudioNode::link(encoder, decoder);
    audio::AudioNode::link(decoder, sink);

    std::vector<float> outSamples;
    outSamples.reserve(inSamples.size() + 4096);

    sink.setProcessCallback([&](const AudioConf&, const core::Buffer& outBuffer) {
        for (uint i = 0; i < outBuffer.size(); i += 4) {
            outSamples.push_back(*(float*)(outBuffer.data()+i));
        }
    });

    for (uint32_t i = 0; i < 1; ++i) {
        core::Buffer inBuffer((char*)inSamples.data(), inSamples.size()*4);
        encoder.process({ AudioCodec::RawFloat32, SampleRate::Rate48000, Channels::Stereo }, inBuffer);
    }
    encoder.onStop();

    std::vector<float> sum(inSamples.size()/2, 0.0f);
    for (uint i = 0; i < inSamples.size(); i += 2) {
        sum.at(i/2) = inSamples.at(i) - outSamples.at(i+512);
    }

    std::vector<float> avg;

    for (uint s = 0; s < sum.size()-2048; s += 2048) {
        Gist<float> gist(2048, 44100);
        gist.processAudioFrame(sum.data()+s, 2048);
        std::vector<float> magSpec = gist.getMagnitudeSpectrum();
        avg.resize(magSpec.size());
        uint j = 0;
        for (int i = 0; j < magSpec.size(); ++i) {
            for (j = pow(2,i)-1; j < pow(2,i+1)-1; ++j) {
                if (j == pow(2,i)-1) {
                    magSpec[i] = magSpec[j];
                } else {
                    magSpec[i] += magSpec[j];
                }
            }
            magSpec[i] /= pow(2,i);
            magSpec[i] = 20.0f * log10(magSpec[i]);
            avg[i] += magSpec[i];
            avg[i] *= 0.5f;
        }
    }

    end = std::chrono::steady_clock::now();
    diff = end-begin;
    std::cout << "Process time: " << diff.count() << std::endl;
}

int main()
{
    std::cout << std::endl << "#### AC3 test ####" << std::endl;
    runTest();
    return 0;
}
