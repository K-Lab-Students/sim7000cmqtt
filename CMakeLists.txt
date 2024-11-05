set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_VERSION 1)
cmake_minimum_required(VERSION 3.22)

project(sim7000cmqtt)

option(SIM7000CMQTT_DEBUG_ENABLED "" OFF)

set(SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/src/SIM7000MQTT.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/ATCommunicator.cpp
        include/sim7000cmqtt/SIM7000MQTT.hpp
)

add_library(${PROJECT_NAME} STATIC ${SOURCES})

target_link_libraries(${PROJECT_NAME} PUBLIC stm32cubemx)

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include/${PROJECT_NAME})

if (SIM7000CMQTT_DEBUG_ENABLED)
    target_compile_definitions(${PROJECT_NAME} PRIVATE 
        SIM7000CMQTT_DEBUG_ENABLED
    )
endif()
