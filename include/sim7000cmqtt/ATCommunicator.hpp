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

class ATParser
{
public:
    enum class Status {
        kCPIN,
        kAPPPDPActive,
        kAPPPDPDeactive,
        kRDY,
        kCFUN,
        kSMSRdy,
        kOk,
        kError,
        kWaitInput,
        kNotValid,
        kNotFullInput,
        kTimeoutError,
        kUnknown
    };

public:
    static Status parse(const uint8_t* str, uint8_t size) noexcept;
};

class ATCommunicator
{
private:
    enum class ATRespType : uint8_t {
        kResponseOnly = 0U,
        kEchoStatus,
        kEchoStatusResponse,
        kEchoResponseStatus
    };

public:
    ATCommunicator(UART_HandleTypeDef* huart);

	void waitInit();

    void CNACT(bool on) noexcept;

    void GNSPWR(bool on) noexcept;
    void GNSINF() noexcept;

    void SMCONF(const std::string& params) noexcept;
    void SMCONF_URL(const std::string& url, const std::string& port) noexcept;
    void SMCONF_CLIENTID(const std::string& id) noexcept;
    void SMCONF_USERNAME(const std::string& username) noexcept;
    void SMCONF_PASSWORD(const std::string& passwd) noexcept;
    void SMCONF_KEEPTIME() noexcept;
    void SMCONN(bool on) noexcept;
    void SMPUB(const std::string& msg) noexcept;

    ATParser::Status rawSend(const std::string& str, uint32_t rx_attempts) noexcept;
    ATParser::Status rawTxRx(const std::string& str, std::string& res, uint32_t rx_attempts) noexcept;
    ATParser::Status waitResponse(uint32_t rx_attempts) noexcept;
    ATParser::Status waitResponse(const std::string& str, uint32_t rx_attempts) noexcept;
    ATParser::Status waitResponse(const std::string& str, std::string& result, uint32_t rx_attempts) noexcept;

private:
    UART_HandleTypeDef* huart_ {};

    uint8_t rx_raw_buffer_[512] {};
};


#endif    // TESTSIM7000C_ATCOMMUNICATOR_HPP
