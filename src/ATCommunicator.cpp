//
// Created by vadim on 6/1/24.
//

#include "ATCommunicator.hpp"
#include "ATCommands.hpp"

#include <cstring>

ATParser::Status ATParser::parse(const uint8_t *str, uint8_t size) noexcept
{
    uint8_t idx {};

    while (str[idx] != '\r' && idx < size)
        idx++;
    if (str[idx + 1] == '\r')
        idx++;

    idx += 2;

    if (idx >= size) {
        return Status::kNotFullInput;
    }

    if (str[idx] == '>') {
        return Status::kWaitInput;
    }
    if (str[idx] == '+') {
        if (str[idx + 1] == 'C') {
            if (str[idx + 2] == 'P') {
                return Status::kCPIN;
            } else if (str[idx + 2] == 'F') {
                return Status::kCFUN;
            } else {
                return Status::kUnknown;
            }
        }
        if (str[idx + 1] == 'A') {
            if (str[idx + 10] == 'A') {
                return Status::kAPPPDPActive;
            } else if (str[idx + 10] == 'D') {
                return Status::kAPPPDPDeactive;
            } else {
                return Status::kUnknown;
            }
        }
        return Status::kUnknown;
    } else if (str[idx] == 'S' && str[idx + 1] == 'M' && str[idx + 2] == 'S') {
        return Status::kSMSRdy;
    } else if (str[idx] == 'R' && str[idx + 1] == 'D' && str[idx + 2] == 'Y') {
        return Status::kRDY;
    } else if (str[idx] == 'O' && str[idx + 1] == 'K') {
        return Status::kOk;
    } else if (str[idx] == 'E' && str[idx + 1] == 'R' && str[idx + 2] == 'R') {
        return Status::kError;
    }

    return Status::kNotFullInput;
}

ATCommunicator::ATCommunicator(UART_HandleTypeDef *huart) : huart_(huart)
{
}

void ATCommunicator::waitInit()
{
}

void ATCommunicator::CNACT(bool on) noexcept
{
    const char *cmd = (on) ? AT_CNACT_ON : AT_CNACT_OFF;
}

void ATCommunicator::GNSPWR(bool on) noexcept
{
    const char *cmd = (on) ? AT_CGNSPWR_ON : AT_CGNSPWR_OFF;
}

void ATCommunicator::GNSINF() noexcept
{
    const char *cmd = AT_CGNSINF;
}

void ATCommunicator::SMCONF(const std::string &params) noexcept
{
}

void ATCommunicator::SMCONF_URL(const std::string &url, const std::string &port) noexcept
{
    SMCONF(AT_SMCONF_URL + url + "," + port);
}

void ATCommunicator::SMCONF_CLIENTID(const std::string &id) noexcept
{
    SMCONF(AT_SMCONF_CLIENTID + id);
}

void ATCommunicator::SMCONF_USERNAME(const std::string &username) noexcept
{
    SMCONF(AT_SMCONF_USERNAME + username);
}

void ATCommunicator::SMCONF_PASSWORD(const std::string &passwd) noexcept
{
    SMCONF(AT_SMCONF_PASSWORD + passwd);
}

void ATCommunicator::SMCONF_KEEPTIME() noexcept
{
    SMCONF(AT_SMCONF_KEEPTIME_60);
}

void ATCommunicator::SMCONN(bool on) noexcept
{
    const char *cmd = (on) ? AT_SMCONN : AT_SMDISC;
}

void ATCommunicator::SMPUB(const std::string &msg) noexcept
{
}

ATParser::Status ATCommunicator::rawSend(const std::string &str, uint32_t rx_attempts) noexcept
{
    HAL_UART_Transmit(huart_, reinterpret_cast<const uint8_t *>(str.c_str()), str.size(), 1000);
    return waitResponse(str, rx_attempts);
}

ATParser::Status ATCommunicator::rawTxRx(const std::string &str, std::string &res, uint32_t rx_attempts) noexcept
{
    HAL_UART_Transmit(huart_, reinterpret_cast<const uint8_t *>(str.c_str()), str.size(), 1000);
    return waitResponse(str, res, rx_attempts);
}

ATParser::Status ATCommunicator::waitResponse(uint32_t rx_attempts) noexcept
{
    uint16_t size, idx {};
    static uint8_t resp[256];
    ATParser::Status res;
    uint8_t attempts {};

    do {
        HAL_UARTEx_ReceiveToIdle(huart_, reinterpret_cast<uint8_t *>(rx_raw_buffer_), sizeof(rx_raw_buffer_), &size,
                                 10000);
        memcpy(&resp[idx], rx_raw_buffer_, size);
        idx += size;
        if (attempts > rx_attempts) {
            res = ATParser::Status::kTimeoutError;
        } else {
            res = ATParser::parse(resp, idx);
        }
        attempts++;
    } while (res == ATParser::Status::kNotFullInput);
    memset(rx_raw_buffer_, 0, sizeof(rx_raw_buffer_));

    HAL_UART_Transmit(&huart3, resp, idx, 100);
    memset(resp, 0, sizeof(resp));

    return res;
}

ATParser::Status ATCommunicator::waitResponse(const std::string &str, uint32_t rx_attempts) noexcept
{
    uint16_t size, idx {};
    static uint8_t resp[256];
    ATParser::Status res;
    uint8_t attempts {};

    do {
        HAL_UARTEx_ReceiveToIdle(huart_, reinterpret_cast<uint8_t *>(rx_raw_buffer_), sizeof(rx_raw_buffer_), &size,
                                 10000);
        memcpy(&resp[idx], rx_raw_buffer_, size);
        idx += size;
        if (attempts > rx_attempts) {
            res = ATParser::Status::kTimeoutError;
        } else {
            res = ATParser::parse(resp, idx);
        }
        attempts++;
    } while (res == ATParser::Status::kNotFullInput);
    memset(rx_raw_buffer_, 0, sizeof(rx_raw_buffer_));

    HAL_UART_Transmit(&huart3, resp, idx, 100);
    for (uint16_t i = 0; i < str.size(); ++i) {
        if (resp[i] != str[i]) {
            res = ATParser::Status::kNotValid;
            break;
        }
    }
    memset(resp, 0, sizeof(resp));

    return res;
}

ATParser::Status ATCommunicator::waitResponse(const std::string &str, std::string &result,
                                              uint32_t rx_attempts) noexcept
{
    uint16_t size, idx {};
    static uint8_t resp[256];
    ATParser::Status res;
    uint8_t attempts {};

    do {
        HAL_UARTEx_ReceiveToIdle(huart_, reinterpret_cast<uint8_t *>(rx_raw_buffer_), sizeof(rx_raw_buffer_), &size,
                                 10000);
        memcpy(&resp[idx], rx_raw_buffer_, size);
        idx += size;
        if (attempts > rx_attempts) {
            res = ATParser::Status::kTimeoutError;
        } else {
            res = ATParser::parse(resp, idx);
        }
        attempts++;
    } while (res == ATParser::Status::kNotFullInput);
    memset(rx_raw_buffer_, 0, sizeof(rx_raw_buffer_));

    HAL_UART_Transmit(&huart3, resp, idx, 100);
    for (uint16_t i = 0; i < str.size(); ++i) {
        if (resp[i] != str[i]) {
            res = ATParser::Status::kNotValid;
            break;
        }
    }

    for (uint16_t i = 0; i < idx; ++i)
        result[i] += resp[i];

    memset(resp, 0, sizeof(resp));

    return res;
}
