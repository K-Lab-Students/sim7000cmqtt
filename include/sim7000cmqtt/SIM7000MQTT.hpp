//
// Created by vadim on 5/26/24.
//

#ifndef TESTSIM7000C_SIM7000MQTT_HPP
#define TESTSIM7000C_SIM7000MQTT_HPP

#include <cstdint>
#include <string>
#include <vector>
#include <queue>
#include <array>
#include <memory>

#include "usart.h"
#include "ATCommunicator.hpp"

class SIM7000MQTT {
public:
	using URL = std::string;
	using Port = std::string;
	using CliendID = std::string;
	using Username = std::string;
	using Password = std::string;

	using Topic = std::string;

	using Message = std::pair<Topic, std::string>;

	enum class Status {
		kError,
		kOk
	};

public:
	SIM7000MQTT(UART_HandleTypeDef *huart, URL url, Port port,
				CliendID client_id, Username username, Password password);

	void waitInit() noexcept;
	void setupMQTT() noexcept;
	void enableMQTT() noexcept;
	void disableMQTT() noexcept;
	void setupGNSS() noexcept;
	void publishMessage(const Topic& topic, const std::string& message) noexcept;

private:
	ATCommunicator comm_;

	URL url_;
	Port port_;
	CliendID client_id_;
	Username username_;
	Password password_;

	std::vector<std::string> setup_mqtt_cmds_;
	std::vector<std::string> enable_mqtt_cmds_;
	std::vector<std::string> disable_mqtt_cmds_;
	std::vector<std::string> gnss_cmds_;
	std::array<std::string, 2> publish_message_cmds_;

	std::string current_response_;

	bool is_mqtt_enabled_{false};
	bool is_received_{false};

	ATParser::Status parser_status_{ATParser::Status::kOk};
};

#endif //TESTSIM7000C_SIM7000MQTT_HPP
