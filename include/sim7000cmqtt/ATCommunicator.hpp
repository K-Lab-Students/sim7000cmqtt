//
// Created by vadim on 6/1/24.
//

#ifndef TESTSIM7000C_ATCOMMUNICATOR_HPP
#define TESTSIM7000C_ATCOMMUNICATOR_HPP

#include <string>
#include <queue>
#include <vector>
#include <functional>

#include "usart.h"

class ATCommunicator {
public:
    enum class Status {
        kOk,
        kError,
        kTimeout
    };

public:
    ATCommunicator(UART_HandleTypeDef* huart);

    ATCommunicator::Status rawSend(const char* str, char* resp, uint32_t timeout) noexcept;
    ATCommunicator::Status waitResponse(char* resp, uint32_t timeout) noexcept;

private:
    UART_HandleTypeDef* huart_ {};

    uint8_t rx_raw_buffer_[512] {};
};

#endif    // TESTSIM7000C_ATCOMMUNICATOR_HPP
