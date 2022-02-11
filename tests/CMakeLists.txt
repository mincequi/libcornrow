macro(coro_tests)
    foreach(_testname ${ARGN})
        add_executable(${_testname} ${_testname}.cpp)
        target_link_libraries(${_testname} ${PROJECT_NAME})
    endforeach(_testname)
endmacro()

coro_tests(
    airplaytest
    airplay2test
    portaudiotest
    biquadtest
    buffertest
    convertertest
    corotest
    #encodertest
    screamtest
    TcpClientSinkTest
)

#find_package(Qt5 COMPONENTS Multimedia Network)
#if(Qt5_FOUND)
#add_executable(sqreamtest sqreamtest.cpp)
#target_link_libraries(sqreamtest
#    ${PROJECT_NAME}
#    Qt5::Multimedia
#    Qt5::Network
#)
#endif(Qt5_FOUND)

#add_executable(pihdmitest pihdmitest.cpp)
#target_link_libraries(pihdmitest
#    ${PROJECT_NAME}
#)
