macro(coro_tests)
    foreach(_testname ${ARGN})
        add_executable(${_testname} ${_testname}.cpp)
        target_link_libraries(${_testname} ${PROJECT_NAME})
    endforeach(_testname)
endmacro()

coro_tests(
    alsautiltest
    biquadtest
    convertertest
    corotest
    crossovertest
    loudnesstest
    passthroughtest
)
