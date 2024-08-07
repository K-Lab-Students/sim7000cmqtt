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
    using Port = uint16_t;
    using CliendID = std::string;
    using Username = std::string;
    using Password = std::string;

    using Topic = std::string;

    using Message = std::pair<Topic, std::string>;

    enum class Status : bool {
        kError,
        kOk
    };

    struct GPS {
        float latitude;
        float longitude;
    };

public:
    SIM7000MQTT(UART_HandleTypeDef* huart, URL url, Port port, CliendID client_id, Username username,
                Password password);

    void waitInit() noexcept;

    SIM7000MQTT::Status enableWirelessConnection() noexcept;
    SIM7000MQTT::Status disableWirelessConnection() noexcept;

    SIM7000MQTT::Status setupMQTT() noexcept;
    SIM7000MQTT::Status enableMQTT() noexcept;
    SIM7000MQTT::Status disableMQTT() noexcept;

    SIM7000MQTT::Status enableGNSS() noexcept;
    SIM7000MQTT::Status disableGNSS() noexcept;
    SIM7000MQTT::Status getGNSS(SIM7000MQTT::GPS& data) noexcept;

    SIM7000MQTT::Status publishMessage(const Topic& topic, const std::string& message) noexcept;

private:
    SIM7000MQTT::Status ATsmconf_(const char* parameter, const char* value) noexcept;
    SIM7000MQTT::Status ATsmconf_(const char* parameter, const char* value, uint16_t port) noexcept;
    SIM7000MQTT::Status ATcnact_(bool status) noexcept;
    SIM7000MQTT::Status ATsmconn_(bool status) noexcept;
    SIM7000MQTT::Status ATgnspwr_(bool status) noexcept;
    SIM7000MQTT::Status ATsmpub_(const char* topic, const char* msg) noexcept;
    SIM7000MQTT::Status ATgnsinf_(char* reply) noexcept;

    SIM7000MQTT::Status checkResp_(const char* resp, const char* except) noexcept;

private:
    ATCommunicator comm_;

    URL url_;
    Port port_;
    CliendID client_id_;
    Username username_;
    Password password_;

    char resp_[256] {};
};

#endif    // TESTSIM7000C_SIM7000MQTT_HPP
