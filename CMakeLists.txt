cmake_minimum_required(VERSION 3.3)
project(tradesys)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -fPIC")

set(PYTHON_LIBRARY "/usr/lib/python3.6/")
set(PYTHON_INCLUDE_DIR "/usr/include/python3.6")

set(Python_ADDITIONAL_VERSIONS 3.6)
#FIND_PACKAGE(PythonInterp 3.6 REQUIRED)
#FIND_PACKAGE(PythonLibs 3.6 REQUIRED)

set(Boost_USE_STATIC_LIBS       OFF) # only find static libs
set(Boost_USE_MULTITHREADED      ON)
set(Boost_USE_STATIC_RUNTIME    OFF)
find_package(Boost 1.67.0 REQUIRED COMPONENTS python36 numpy36 date_time filesystem serialization)
if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS} "/usr/include/python3.6")
endif()

set(SOURCE_FILES
    src/signals/base_signal.cpp
    src/signals/base_signal.h
    src/signals/candles.cpp
    src/signals/candles.h
    src/signals/common_types.cpp
    src/signals/common_types.h
    src/signals/constraints.cpp
    src/signals/constraints.h
    src/signals/crosses.cpp
    src/signals/crosses.h
    src/signals/feedback.cpp
    src/signals/feedback.h
    src/signals/gaps.cpp
    src/signals/gaps.h
    src/signals/happened.cpp
    src/signals/happened.h
    src/signals/numeric.cpp
    src/signals/numeric.h
    src/signals/ops.cpp
    src/signals/ops.h
    src/signals/raw.cpp
    src/signals/raw.h
    src/signals/stops.cpp
    src/signals/stops.h
    src/signals/supertrend.cpp
    src/signals/supertrend.h
    src/signals/ta.cpp
    src/signals/ta.h
    src/signals/times.cpp
    src/signals/times.h
    src/signals/timesinarow.cpp
    src/signals/timesinarow.h
    src/unit_tests/signals/test_base_signal.cpp
    src/unit_tests/signals/test_crosses.cpp
    src/unit_tests/signals/test_feedback.cpp
    src/unit_tests/signals/test_gaps.cpp
    src/unit_tests/signals/test_happened.cpp
    src/unit_tests/signals/test_numeric.cpp
    src/unit_tests/signals/test_ops.cpp
    src/unit_tests/signals/test_raw.cpp
    src/unit_tests/signals/test_stops.cpp
    src/unit_tests/signals/test_supertrend.cpp
    src/unit_tests/signals/test_ta.cpp
    src/unit_tests/signals/test_times.cpp
    src/unit_tests/signals/test_timesinarow.cpp
    src/unit_tests/test_backtest_run.cpp
    src/unit_tests/test_backtest_stats.cpp
    src/unit_tests/test_data.cpp
    src/unit_tests/test_datetime.cpp
    src/unit_tests/test_expression.cpp
    src/unit_tests/test_signal_forest.cpp
    src/unit_tests/test_signal_tree.cpp
    src/unit_tests/test_strategy.cpp
    src/unit_tests/test_known_bugs.cpp
    src/backtest_run.cpp
    src/backtest_run.h
    src/backtest_stats.cpp
    src/backtest_stats.h
    src/common.h
    src/data.cpp
    src/data.h
    src/dtime.cpp
    src/dtime.h
    src/sformat.cpp
    src/sformat.h
    src/expression.cpp
    src/expression.h
    src/live.cpp
    src/live.h
    src/main.cpp
    src/python_bindings.cpp
    src/python_bindings.h
    src/signal_forest.cpp
    src/signal_forest.h
    src/signal_tree.cpp
    src/signal_tree.h
    src/strategy.cpp
    src/strategy.h
        src/unit_tests/test_backtest_ticks.cpp src/unit_tests/test_variables.cpp src/signals/embedded.h src/signals/embedded.cpp)

add_executable(tradesys ${SOURCE_FILES})
target_link_libraries(tradesys python3.6m ${Boost_LIBRARIES} gtest gtest_main ta_lib pthread)
