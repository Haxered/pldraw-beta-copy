cmake_minimum_required(VERSION 3.5)
project(PLDRAW CXX)

# require a C++11 compiler
# use it for all targets
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# configure Qt
set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
find_package(Qt5 COMPONENTS Widgets Core Test REQUIRED)

# make vim auto completion happy 
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(POSTLISP_DEBUG_AST "Enable AST debug printing after parse()" OFF)

if(POSTLISP_DEBUG_AST)
    add_definitions(-DPOSTLISP_DEBUG_AST)
endif()

# EDIT
# add any files you create related to the interpreter here
# excluding unit tests
set(interpreter_src
        tokenizer.hpp tokenizer.cpp
        expression.hpp expression.cpp
        environment.hpp environment.cpp
        interpreter.hpp interpreter.cpp
)

# EDIT
# add any files you create related to the GUI here
# excluding tests
set(gui_src
        qgraphics_arc_item.hpp qgraphics_arc_item.cpp
        message_widget.hpp message_widget.cpp
        canvas_widget.hpp canvas_widget.cpp
        repl_widget.hpp repl_widget.cpp
        qt_interpreter.hpp qt_interpreter.cpp
        main_window.hpp main_window.cpp
)

# EDIT
# add any files you create related to interpreter unit testing here
set(test_src
        catch.hpp
        unittests.cpp
)

# EDIT
# add any files you create related to the postlisp program here
set(postlisp_src
        ${interpreter_src}
        postlisp.cpp
)

# EDIT
# add any files you create related to the pldraw program here
set(pldraw_src
        ${interpreter_src}
        ${gui_src}
        pldraw.cpp
)

# You should not need to edit below this line
#-----------------------------------------------------------------------
#-----------------------------------------------------------------------

set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

# setup testing
set(TEST_FILE_DIR "${CMAKE_SOURCE_DIR}/tests")
configure_file(${CMAKE_SOURCE_DIR}/test_config.hpp.in
        ${CMAKE_BINARY_DIR}/test_config.hpp)
include_directories(${CMAKE_BINARY_DIR})

# EXECUTABLES
add_executable(postlisp ${postlisp_src})
add_executable(pldraw ${pldraw_src})

# SAMPLE
add_executable(test_gui test_gui.cpp ${gui_src} ${interpreter_src})
add_executable(test_message test_message.cpp message_widget.hpp message_widget.cpp)
add_executable(test_interpreter test_interpreter.cpp test_expression.cpp test_tokenize.cpp catch.hpp ${interpreter_src})

# STUDENT
add_executable(unittests ${interpreter_src} ${test_src})
add_executable(unittests_gui unittests_gui.cpp ${gui_src} ${interpreter_src})

# INSTRUCTOR
add_executable(inst_test catch.hpp instructor_test.cpp ${interpreter_src})
add_executable(inst_test_gui instructor_test_gui.cpp ${gui_src} ${interpreter_src})

# EXECUTABLE
target_link_libraries(pldraw Qt5::Widgets)

# SAMPLE
target_link_libraries(test_gui Qt5::Widgets Qt5::Test)
target_link_libraries(test_message Qt5::Widgets Qt5::Test)

# STUDENT
target_link_libraries(unittests_gui Qt5::Widgets Qt5::Test)

# INSTRUCTOR
target_link_libraries(inst_test_gui Qt5::Widgets Qt5::Test)

enable_testing()

add_test(test_gui test_gui)
add_test(test_message test_message)
add_test(test_interpreter test_interpreter)

add_test(unittests unittests)
add_test(unittests_gui unittests_gui)

add_test(inst_test inst_test)
add_test(inst_test_gui inst_test_gui)

# On Linux, using GCC, to enable coverage on tests -DCOVERAGE=TRUE
if (UNIX AND NOT APPLE AND CMAKE_COMPILER_IS_GNUCXX AND COVERAGE)
    message("Enabling Test Coverage")
    SET(GCC_COVERAGE_COMPILE_FLAGS "-g -O0 -fprofile-arcs -ftest-coverage")
    set_target_properties(unittests PROPERTIES COMPILE_FLAGS ${GCC_COVERAGE_COMPILE_FLAGS})
    target_link_libraries(unittests gcov)
    set_target_properties(test_gui PROPERTIES COMPILE_FLAGS ${GCC_COVERAGE_COMPILE_FLAGS})
    target_link_libraries(test_gui Qt5::Widgets Qt5::Test gcov)
    set_target_properties(test_message PROPERTIES COMPILE_FLAGS ${GCC_COVERAGE_COMPILE_FLAGS})
    target_link_libraries(test_message Qt5::Widgets Qt5::Test gcov)
    add_custom_target(coverage-grading
            COMMAND ${CMAKE_COMMAND} -E env "ROOT=${CMAKE_CURRENT_SOURCE_DIR}"
            ${CMAKE_CURRENT_SOURCE_DIR}/scripts/coverage.sh)
endif ()

# On Linux, using GCC, to enable coverage on tests -DMEMORY=TRUE
if (UNIX AND NOT APPLE AND CMAKE_COMPILER_IS_GNUCXX AND MEMORY)
    add_custom_target(memtest
            COMMAND valgrind ${CMAKE_BINARY_DIR}/unittests)
endif ()

# enable clang-tidy with -DTIDY=TRUE
if (UNIX AND NOT APPLE AND CMAKE_COMPILER_IS_GNUCXX AND TIDY)
    add_custom_target(tidy
            COMMAND ${CMAKE_COMMAND} -E env "source=${CMAKE_CURRENT_SOURCE_DIR}" env "build=${CMAKE_CURRENT_BINARY_DIR}"
            ${CMAKE_CURRENT_SOURCE_DIR}/scripts/tidy.sh
    )
endif ()
