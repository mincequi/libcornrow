#include "../include/TBiquad.h"

#include <chrono>
#include <iostream>
#include <fstream>
#include <random>

using namespace GstDsp;

int main(int argc, char** argv)
{
    // Init
    auto begin = std::chrono::steady_clock::now();
    std::random_device rd;
    std::mt19937 gen;
    std::normal_distribution<> dist;
    std::uint8_t duration = 10;

    std::vector<double> samples(2*44100*duration);
    for (auto & s : samples) {
        s = dist(gen);
    }

    std::vector<TBiquad<double>> biquads;
    biquads.push_back( {2,2,44100} ); // HP
    biquads.push_back( {2,2,44100} ); // LP
    biquads.push_back( {2,1,44100} ); // P1
    biquads.push_back( {2,1,44100} ); // P2
    biquads.push_back( {2,1,44100} ); // P3
    biquads.push_back( {2,1,44100} ); // P4
    biquads.push_back( {2,1,44100} ); // P5
    biquads.push_back( {2,1,44100} ); // P6
    biquads.push_back( {2,1,44100} ); // P7
    biquads.push_back( {2,1,44100} ); // P8
    biquads.push_back( {2,1,44100} ); // P9
    biquads.push_back( {2,1,44100} ); // P10

    biquads[0].setFilter( { GstDsp::FilterType::HighPass, 80.0, 0.0, 0.707 } );
    biquads[1].setFilter( { GstDsp::FilterType::LowPass, 12000.0,  0.0, 0.707 } );
    biquads[3].setFilter( { GstDsp::FilterType::Peak, 150.0, -6.0, 2.0 } );
    biquads[4].setFilter( { GstDsp::FilterType::Peak, 200.0, +0.5, 2.0 } );
    biquads[5].setFilter( { GstDsp::FilterType::Peak, 300.0, -6.0, 2.0 } );
    biquads[6].setFilter( { GstDsp::FilterType::Peak, 500.0, +0.5, 2.0 } );
    biquads[7].setFilter( { GstDsp::FilterType::Peak, 750.0, -6.0, 2.0 } );
    biquads[8].setFilter( { GstDsp::FilterType::Peak, 1200.0, +0.5, 2.0 } );
    biquads[9].setFilter( { GstDsp::FilterType::Peak, 2000.0, -6.0, 2.0 } );
    biquads[10].setFilter( { GstDsp::FilterType::Peak, 3000.0, +0.5, 2.0 } );
    biquads[11].setFilter( { GstDsp::FilterType::Peak, 5000.0, -6.0, 2.0 } );
    biquads[2].setFilter( { GstDsp::FilterType::Peak,  7500.0, +0.5, 2.0 } );
    auto end = std::chrono::steady_clock::now();
    std::chrono::duration<double> diff = end-begin;
    std::cout << "Init time: " << diff.count() << std::endl;

    // Process
    begin = std::chrono::steady_clock::now();
    for (auto & b : biquads) {
        b.process(samples.data(), samples.data(), 44100*duration, 2, 2);
    }
    end = std::chrono::steady_clock::now();
    diff = end-begin;
    std::cout << "Process time: " << diff.count() << std::endl;

    // Write
    begin = std::chrono::steady_clock::now();
    std::ofstream myfile;
    myfile.open ("test.raw", std::ios::out | std::ios::binary);
    const void* data = samples.data();
    myfile.write(static_cast<const char*>(data), 2*44100*duration*8);
    end = std::chrono::steady_clock::now();
    diff = end-begin;
    std::cout << "Write time: " << diff.count() << std::endl;

    return 0;
}
