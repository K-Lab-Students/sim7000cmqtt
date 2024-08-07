//
// Created by vadim on 6/1/24.
//

#include "ATCommunicator.hpp"

#include <cstring>

ATCommunicator::ATCommunicator(UART_HandleTypeDef *huart) : huart_(huart)
{
}

ATCommunicator::Status ATCommunicator::rawSend(const char *str, char *resp, uint32_t timeout) noexcept
{
    auto hal_status = HAL_UART_Transmit(huart_, reinterpret_cast<const uint8_t *>(str), strlen(str), 1000);
    if (hal_status == HAL_OK) {
        return waitResponse(resp, timeout);
    }
    return hal_status == HAL_TIMEOUT ? Status::kTimeout : Status::kError;
}

ATCommunicator::Status ATCommunicator::waitResponse(char *resp, uint32_t timeout) noexcept
{
    uint16_t size, idx {};

    auto hal_status = HAL_UARTEx_ReceiveToIdle(huart_, rx_raw_buffer_, sizeof(rx_raw_buffer_), &size, timeout);

    if (resp != nullptr) {
        memcpy(resp, rx_raw_buffer_, size);
    }

    memset(rx_raw_buffer_, 0, sizeof(rx_raw_buffer_));

#ifdef SIM7000CMQTT_DEBUG_ENABLED
    if (resp != nullptr) {
        HAL_UART_Transmit(&huart3, resp, idx, 1000);
    }
#endif

    if (hal_status == HAL_OK) {
        return Status::kOk;
    }
    return hal_status == HAL_TIMEOUT ? Status::kTimeout : Status::kError;
}
