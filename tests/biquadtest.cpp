#include "../include/TBiquad.h"

#include <chrono>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <random>

using namespace coro;

template <class T, class U>
void runTest(std::string filename, std::uint16_t seconds = 100)
{
    auto begin = std::chrono::steady_clock::now();
    std::random_device rd;
    std::mt19937 gen(rd());

    std::vector<T> samples(2*44100*seconds);
    std::uniform_real_distribution<> dist(-0.8, 0.8);
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

    std::vector<TBiquad<T,U>> biquads;
    biquads.push_back( {2,2,44100} ); // HP
    biquads.push_back( {2,2,44100} ); // LP
    biquads.push_back( {2,1,44100} ); // PK1
    biquads.push_back( {2,1,44100} ); // PK2
    biquads.push_back( {2,1,44100} ); // PK3
    /*biquads.push_back( {2,1,44100} ); // PK4
    biquads.push_back( {2,1,44100} ); // PK5
    biquads.push_back( {2,1,44100} ); // PK6
    biquads.push_back( {2,1,44100} ); // PK7
    biquads.push_back( {2,1,44100} ); // PK8
    biquads.push_back( {2,1,44100} ); // PK9
    biquads.push_back( {2,1,44100} ); // PK10 */
    biquads[0].setFilter( { coro::FilterType::LowPass, 10000.0, 0.0, 0.707 } );
    biquads[1].setFilter( { coro::FilterType::HighPass, 100.0, 0.0, 0.707 } );
    biquads[2].setFilter( { coro::FilterType::Peak, 200.0, -9.0, 1.414 } );
    biquads[3].setFilter( { coro::FilterType::Peak, 400.0, -9.0, 1.414 } );
    biquads[4].setFilter( { coro::FilterType::Peak, 800.0, -9.0, 1.414 } );
    /*biquads[5].setFilter( { coro::FilterType::Peak, 1600.0, -3.0, 1.414 } );
    biquads[6].setFilter( { coro::FilterType::Peak, 3200.0, -3.0, 1.414 } );
    biquads[7].setFilter( { coro::FilterType::Peak, 6400.0, -3.0, 1.414 } );
    biquads[8].setFilter( { coro::FilterType::Peak, 12800.0, 3.0, 1.414 } );
    biquads[9].setFilter( { coro::FilterType::Peak, 16000.0, 6.0, 1.414 } );
    biquads[10].setFilter( { coro::FilterType::Peak, 50.0, 6.0, 1.414 } );
    biquads[11].setFilter( { coro::FilterType::Peak, 100.0, 3.0, 1.414 } );*/

    auto end = std::chrono::steady_clock::now();
    std::chrono::duration<double> diff = end-begin;
    std::cout << "Init time: " << diff.count() << std::endl;

    // Process
    begin = std::chrono::steady_clock::now();
    for (auto & b : biquads) {
        b.process(samples.data(), samples.data(), samples.size()/2, 2, 2);
        //for (auto& s : samples) {
        //    s = b.process(s);
        //}
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
    std::cout << std::endl << "#### Float/Float test ####" << std::endl;
    runTest<float,float>("testFloatFloat.raw", 100);

    std::cout << std::endl << "#### Float/Double test ####" << std::endl;
    runTest<float,double>("testFloatDouble.raw", 100);

    std::cout << std::endl << "#### Double test ####" << std::endl;
    runTest<double,double>("testDouble.raw", 100);

    std::cout << std::endl << "#### Int16/Int32 test ####" << std::endl;
    runTest<int16_t,int32_t>("testInt16Int32.raw", 100);

    //std::cout << std::endl << "#### Int16/Int64 test ####" << std::endl;
    //runTest<int16_t,int64_t>("testInt16Int64.raw", 100);

    std::cout << std::endl << "#### Int16/Float test ####" << std::endl;
    runTest<int16_t,float>("testInt16Float.raw", 100);

    std::cout << std::endl << "#### Int16/Double test ####" << std::endl;
    runTest<int16_t,double>("testInt16Double.raw", 100);

    //std::cout << std::endl << "#### Int32/Int64 test ####" << std::endl;
    //runTest<int32_t,int64_t>("testInt32Int64.raw", 100);

    return 0;
}
