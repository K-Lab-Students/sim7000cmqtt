//
// Created by vadim on 5/26/24.
//

#include "sim7000cmqtt/SIM7000MQTT.hpp"
#include "sim7000cmqtt/ATCommands.hpp"

SIM7000MQTT::SIM7000MQTT(UART_HandleTypeDef *huart, URL url, Port port,
						 CliendID client_id, Username username, Password password) :
		comm_(huart),
		url_(std::move(url)),
		port_(std::move(port)),
		client_id_(std::move(client_id)),
		username_(std::move(username)),
		password_(std::move(password))
{
	setup_mqtt_cmds_ = std::vector<std::string>{
			AT AT_ENDL,
			AT_SMCONF_URL + url_ + "," + port_ + AT_ENDL,
			AT_SMCONF_CLIENTID + client_id_ + AT_ENDL,
			AT_SMCONF_USERNAME + username_ + AT_ENDL,
			AT_SMCONF_PASSWORD + password_ + AT_ENDL,
			AT_SMCONF_KEEPTIME_60 AT_ENDL
	};

	enable_mqtt_cmds_ = std::vector<std::string>{
			AT_CNACT_ON AT_ENDL,
			AT_SMCONN AT_ENDL
	};

	disable_mqtt_cmds_ = std::vector<std::string>{
			AT_SMDISC AT_ENDL,
			AT_CNACT_OFF AT_ENDL
	};
}

void SIM7000MQTT::waitInit() noexcept
{
	uint8_t wait_sim_init_flags = 0;
	do {
		parser_status_ = comm_.waitResponse(1);
		switch (parser_status_) {
			case ATParser::Status::kCPIN:
				wait_sim_init_flags |= 0b0001;
				break;
			case ATParser::Status::kRDY:
				wait_sim_init_flags |= 0b1000;
				break;
			case ATParser::Status::kCFUN:
				wait_sim_init_flags |= 0b0010;
				break;
			case ATParser::Status::kSMSRdy:
				wait_sim_init_flags |= 0b0100;
				break;
			default:
				break;
		}
	}
	while (wait_sim_init_flags < 0b0111);
}

void SIM7000MQTT::setupMQTT() noexcept
{
	for (uint8_t i = 0; i < setup_mqtt_cmds_.size();) {
		HAL_Delay(300);
		auto s = comm_.rawSend(setup_mqtt_cmds_[i], 1);
		if (s == ATParser::Status::kOk)
			++i;
	}
}

void SIM7000MQTT::enableMQTT() noexcept
{

	for (uint8_t i = 0; i < enable_mqtt_cmds_.size();) {
		HAL_Delay(300);
		auto s = comm_.rawSend(enable_mqtt_cmds_[i], 5);
		if (s == ATParser::Status::kOk) {
			if (i == 0) {
				if (comm_.waitResponse(5) == ATParser::Status::kAPPPDPActive) {
					++i;
				}
			} else if (i == 1) {
				++i;
			}
		}
	}
}

void SIM7000MQTT::disableMQTT() noexcept
{
	for (uint8_t i = 0; i < disable_mqtt_cmds_.size();) {
		HAL_Delay(300);
		auto s = comm_.rawSend(disable_mqtt_cmds_[i], 5);
		if (s == ATParser::Status::kOk) {
			if (i == 0) {
				++i;
			} else if (i == 1) {
				if (comm_.waitResponse(5) == ATParser::Status::kAPPPDPDeactive) {
					++i;
				}
			}
		}
	}
}

void SIM7000MQTT::setupGNSS(const SIM7000MQTT::Topic& topic, uint32_t timeout) noexcept
{
}

void SIM7000MQTT::publishMessage(const SIM7000MQTT::Topic& topic, const std::string& message) noexcept
{
	publish_message_cmds_[0] = AT_SMPUB"\"" + topic + "\"," + std::to_string(message.size()) + ",1,1" + AT_ENDL;
	publish_message_cmds_[1] = message + AT_ENDL;
	for (uint8_t i = 0; i < publish_message_cmds_.size();) {
		HAL_Delay(300);
		auto s = comm_.rawSend(publish_message_cmds_[i], 5);
		if (i == 0) {
			if (s == ATParser::Status::kWaitInput) {
				++i;
			}
		} else if (i == 1) {
			if (s == ATParser::Status::kOk) {
				++i;
			} else {
				i = 0;
			}
		}
	}
}
