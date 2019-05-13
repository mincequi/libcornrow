macro(gstdsp_tests)
    foreach(_testname ${ARGN})
        add_executable(${_testname} ${_testname}.cpp)
        target_link_libraries(${_testname} ${PROJECT_NAME})
    endforeach(_testname)
endmacro()

gstdsp_tests(
    allovertest
#    biquadtest
    changepipelinetest
    crossovertest
    loudnesstest
    passthroughtest
    peqtest
    sbcparsetest
)

add_executable(biquadtest
    biquadtest.cpp
)
