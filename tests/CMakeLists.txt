macro(gstdsp_tests)
    foreach(_testname ${ARGN})
        add_executable(${_testname} ${_testname}.cpp)
        #target_include_dirs(${GST_INCLUDE_DIRS})
        target_link_libraries(${_testname} ${PROJECT_NAME})
    endforeach(_testname)
endmacro()

gstdsp_tests(
    crossovertest
    loudnesstest
    peqtest
)
