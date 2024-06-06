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

class ATParser {
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

class ATCommunicator {
public:
	ATCommunicator(UART_HandleTypeDef *huart);

	ATParser::Status rawSend(const std::string& str, uint32_t rx_attempts) noexcept;
	ATParser::Status waitResponse(uint32_t rx_attempts) noexcept;

private:
	UART_HandleTypeDef *huart_{};

	uint8_t rx_raw_buffer_[512]{};
};

#endif //TESTSIM7000C_ATCOMMUNICATOR_HPP
