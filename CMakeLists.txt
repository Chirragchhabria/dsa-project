cmake_minimum_required(VERSION 3.20)
project(soc_alert_correlation_engine VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(soc_engine
    src/SecurityEvent.cpp
    src/EventManager.cpp
    src/EventLinkedList.cpp
    src/SlidingWindow.cpp
    src/EntityGraph.cpp
    src/Incident.cpp
    src/CorrelationEngine.cpp
    src/IncidentPrioritizer.cpp
)
target_include_directories(soc_engine PUBLIC include)
if(MSVC)
    target_compile_options(soc_engine PRIVATE /W4)
else()
    target_compile_options(soc_engine PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_executable(soc_cli src/main.cpp)
target_link_libraries(soc_cli PRIVATE soc_engine)

enable_testing()
add_executable(soc_tests tests/test_main.cpp)
target_link_libraries(soc_tests PRIVATE soc_engine)
add_test(NAME soc_tests COMMAND soc_tests)

find_package(Python3 COMPONENTS Interpreter QUIET)
if(Python3_Interpreter_FOUND)
    add_test(NAME soc_cli_integration
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tests/test_cli.py
                $<TARGET_FILE:soc_cli> ${CMAKE_SOURCE_DIR}/data/presentation_events.csv)
endif()
