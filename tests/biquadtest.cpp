#include "../include/TBiquad.h"

#include <chrono>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <random>

using namespace coro;

template <class T>
void runTest(std::string filename, std::uint16_t seconds = 100)
{
    auto begin = std::chrono::steady_clock::now();
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

    std::vector<TBiquad<T>> biquads;
    biquads.push_back( {1,1,44100} ); // HP
    biquads.push_back( {1,1,44100} ); // LP
    biquads[0].setFilter( { coro::FilterType::LowPass, 10000.0, 0.0, 0.707 } );
    biquads[1].setFilter( { coro::FilterType::HighPass, 100.0, 0.0, 0.707 } );

    auto end = std::chrono::steady_clock::now();
    std::chrono::duration<double> diff = end-begin;
    std::cout << "Init time: " << diff.count() << std::endl;

    // Process
    begin = std::chrono::steady_clock::now();
    for (auto & b : biquads) {
        for (auto& s : samples) {
            s = b.process(s);
            //b.process(samples.data(), samples.data(), samples.size()/2, 2, 2);
        }
    }

    end = std::chrono::steady_clock::now();
    diff = end-begin;
    std::cout << "Process time: " << diff.count() << std::endl;

    // Write
    begin = std::chrono::steady_clock::now();
    std::ofstream myfile;
    myfile.open(filename, std::ios::out | std::ios::binary);
    const void* data = samples.data();
    myfile.write(static_cast<const char*>(data), 44100*seconds*sizeof(T));
    end = std::chrono::steady_clock::now();
    diff = end-begin;
    std::cout << "Write time: " << diff.count() << std::endl;
}

int main()
{
    // Init
    std::cout << std::endl << "#### Double test ####" << std::endl;
    runTest<double>("testDouble.raw", 100);
    std::cout << std::endl << "#### Float test ####" << std::endl;
    runTest<float>("testFloat.raw", 100);
    std::cout << std::endl << "#### Int16 test ####" << std::endl;
    runTest<int16_t>("testInt16.raw", 100);
    std::cout << std::endl << "#### Int32 test ####" << std::endl;
    runTest<int32_t>("testInt32.raw", 100);

    return 0;
}
