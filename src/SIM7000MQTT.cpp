//
// Created by vadim on 5/26/24.
//

#include "SIM7000MQTT.hpp"
#include "ATCommands.hpp"

#include <cstring>

#define RUN_OPERATION(OPERATION)           \
    do {                                   \
        if (OPERATION == Status::kError) { \
            return Status::kError;         \
        }                                  \
    } while (0)

SIM7000MQTT::SIM7000MQTT(UART_HandleTypeDef* huart, URL url, Port port, CliendID client_id, Username username,
                         Password password) :
    comm_(huart),
    url_(std::move(url)),
    port_(std::move(port)),
    client_id_(std::move(client_id)),
    username_(std::move(username)),
    password_(std::move(password))
{
}

void SIM7000MQTT::waitInit() noexcept
{
    uint8_t success_cnt = 0;
    uint8_t timeout = HAL_GetTick();
    char resp[256] {};
    do {
        if (HAL_GetTick() - timeout >= 15000) {
            break;
        }

        auto comm_status = comm_.waitResponse(resp, 500);

        if (comm_status == ATCommunicator::Status::kOk) {
            success_cnt++;
        }

    } while (success_cnt < 3);

    comm_.rawSend("ATE0\r", resp, 500);
}

SIM7000MQTT::Status SIM7000MQTT::setupMQTT() noexcept
{
    RUN_OPERATION(smconf_("URL", url_.c_str(), port_));
    RUN_OPERATION(smconf_("CLIENTID", client_id_.c_str()));
    RUN_OPERATION(smconf_("USERNAME", username_.c_str()));
    RUN_OPERATION(smconf_("PASSWORD", password_.c_str()));
    RUN_OPERATION(smconf_("KEEPTIME", "60"));
    return Status::kOk;
}

SIM7000MQTT::Status SIM7000MQTT::wirelessConnectionOn() noexcept
{
    RUN_OPERATION(cnact_(true));
    return Status::kOk;
}

SIM7000MQTT::Status SIM7000MQTT::wirelessConnectionOff() noexcept
{
    RUN_OPERATION(cnact_(false));
    return Status::kOk;
}

SIM7000MQTT::Status SIM7000MQTT::enableMQTT() noexcept
{
    RUN_OPERATION(smconn_(true));
    return Status::kOk;
}

SIM7000MQTT::Status SIM7000MQTT::disableMQTT() noexcept
{
    RUN_OPERATION(smconn_(false));
    return Status::kOk;
}

void SIM7000MQTT::setupGNSS(const SIM7000MQTT::Topic& topic, uint32_t timeout) noexcept
{
}

SIM7000MQTT::Status SIM7000MQTT::publishMessage(const SIM7000MQTT::Topic& topic, const std::string& message) noexcept
{
    RUN_OPERATION(smpub_(topic.c_str(), message.c_str()));
    return Status::kOk;
}

SIM7000MQTT::Status SIM7000MQTT::process(const char* message) noexcept
{
    RUN_OPERATION(wirelessConnectionOn());
    RUN_OPERATION(enableMQTT());
    RUN_OPERATION(publishMessage("test/test_stm", message));
    RUN_OPERATION(disableMQTT());
    RUN_OPERATION(wirelessConnectionOff());
    return Status::kOk;
}

SIM7000MQTT::Status SIM7000MQTT::smconf_(const char* parameter, const char* value) noexcept
{
    char msg[100] {};
    char resp[256] {};
    sprintf(msg, "AT+SMCONF=\"%s\",\"%s\"\r", parameter, value);

    auto comm_status = comm_.rawSend(msg, resp, 500);

    if (comm_status != ATCommunicator::Status::kOk || checkResp_(resp, "OK") != Status::kOk)
        return Status::kError;

    return Status::kOk;
}

SIM7000MQTT::Status SIM7000MQTT::smconf_(const char* parameter, const char* value, uint16_t port) noexcept
{
    char msg[100] {};
    char resp[256] {};
    sprintf(msg, "AT+SMCONF=\"%s\",\"%s\",%i\r", parameter, value, port);

    auto comm_status = comm_.rawSend(msg, resp, 500);

    if (comm_status != ATCommunicator::Status::kOk || checkResp_(resp, "OK") != Status::kOk)
        return Status::kError;

    return Status::kOk;
}

SIM7000MQTT::Status SIM7000MQTT::cnact_(bool status) noexcept
{
    char msg[100] {};
    char resp[256] {};
    sprintf(msg, "AT+CNACT=%d\r", status ? 1 : 0);

    while (true) {
        auto comm_status = comm_.rawSend(msg, resp, 1000);
        if (comm_status != ATCommunicator::Status::kOk || checkResp_(resp, "OK") != Status::kOk)
            return Status::kError;

        comm_status = comm_.waitResponse(resp, 32000);
        if (comm_status == ATCommunicator::Status::kTimeout || comm_status == ATCommunicator::Status::kError)
            return Status::kError;

        if (checkResp_(resp, status ? "ACTIVE" : "DEACTIVE") == Status::kOk)
            break;
    }

    return Status::kOk;
}

SIM7000MQTT::Status SIM7000MQTT::smconn_(bool status) noexcept
{
    char msg[100] {};
    char resp[256] {};
    sprintf(msg, status ? "AT+SMCONN\r" : "AT+SMDISC\r");

    auto comm_status = comm_.rawSend(msg, resp, 5000);

    if (comm_status != ATCommunicator::Status::kOk || checkResp_(resp, "OK") != Status::kOk)
        return Status::kError;

    return Status::kOk;
}

SIM7000MQTT::Status SIM7000MQTT::smpub_(const char* topic, const char* message) noexcept
{
    char msg[100] {};
    char resp[256] {};
    sprintf(msg, "AT+SMPUB=\"%s\",%d,1,1\r", topic, strlen(message));

    auto comm_status = comm_.rawSend(msg, resp, 2000);

    if (comm_status != ATCommunicator::Status::kOk || checkResp_(resp, ">") != Status::kOk)
        return Status::kError;

    sprintf(msg, "%s\r", message);
    comm_status = comm_.rawSend(message, resp, 5000);
    if (comm_status != ATCommunicator::Status::kOk || checkResp_(resp, "OK") != Status::kOk)
        return Status::kError;

    return Status::kOk;
}

SIM7000MQTT::Status SIM7000MQTT::checkResp_(const char* resp, const char* except) noexcept
{
    return strstr(resp, except) == 0 ? Status::kError : Status::kOk;
}