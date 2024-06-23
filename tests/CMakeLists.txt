set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_VERSION 1)
cmake_minimum_required(VERSION 3.23)

project(sim7000cmqtt_test)

set(TESTS_SOURCES
        main.cpp
)

add_executable(${PROJECT_NAME} ${TESTS_SOURCES})

target_link_libraries(${PROJECT_NAME} PUBLIC sim7000cmqtt)
target_link_libraries(${PROJECT_NAME} PUBLIC gtest gtest_main gmock gmock_main)

