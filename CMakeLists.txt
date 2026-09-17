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
target_compile_options(soc_engine PRIVATE -Wall -Wextra -Wpedantic)

add_executable(soc_cli src/main.cpp)
target_link_libraries(soc_cli PRIVATE soc_engine)

enable_testing()
add_executable(soc_tests tests/test_main.cpp)
target_link_libraries(soc_tests PRIVATE soc_engine)
add_test(NAME soc_tests COMMAND soc_tests)

