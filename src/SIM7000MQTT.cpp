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
    RUN_OPERATION(ATsmconf_("URL", url_.c_str(), port_));
    RUN_OPERATION(ATsmconf_("CLIENTID", client_id_.c_str()));
    RUN_OPERATION(ATsmconf_("USERNAME", username_.c_str()));
    RUN_OPERATION(ATsmconf_("PASSWORD", password_.c_str()));
    RUN_OPERATION(ATsmconf_("KEEPTIME", "60"));
    return Status::kOk;
}

SIM7000MQTT::Status SIM7000MQTT::enableWirelessConnection() noexcept
{
    RUN_OPERATION(ATcnact_(true));
    return Status::kOk;
}

SIM7000MQTT::Status SIM7000MQTT::disableWirelessConnection() noexcept
{
    RUN_OPERATION(ATcnact_(false));
    return Status::kOk;
}

SIM7000MQTT::Status SIM7000MQTT::enableMQTT() noexcept
{
    RUN_OPERATION(ATsmconn_(true));
    return Status::kOk;
}

SIM7000MQTT::Status SIM7000MQTT::disableMQTT() noexcept
{
    RUN_OPERATION(ATsmconn_(false));
    return Status::kOk;
}

SIM7000MQTT::Status SIM7000MQTT::enableGNSS() noexcept
{
    RUN_OPERATION(ATgnspwr_(true));
    return Status::kOk;
}

SIM7000MQTT::Status SIM7000MQTT::disableGNSS() noexcept
{
    RUN_OPERATION(ATgnspwr_(false));
    return Status::kOk;
}

SIM7000MQTT::Status SIM7000MQTT::getGNSS(SIM7000MQTT::GPS& data) noexcept
{
    char nmeabuff[256];
    RUN_OPERATION(ATgnsinf_(nmeabuff));

    // parse +CGNSINF: 1,1,20240704144820.000,45.020006,39.030315,94.600,0.00,153.0,1,,0.8,1.1,0.8,,8,7,,,47,,

    // skip GPS run status
    char* tok = strtok(nmeabuff, ",");
    if (!tok) {
        return Status::kError;
    }

    // skip fix status
    tok = strtok(NULL, ",");
    if (!tok) {
        return Status::kError;
    }

    // skip date
    tok = strtok(NULL, ",");
    if (!tok) {
        return Status::kError;
    }

    // grab the latitude
    char* latp = strtok(NULL, ",");
    if (!latp) {
        return Status::kError;
    }
    // grab longitude
    char* longp = strtok(NULL, ",");
    if (!longp) {
        return Status::kError;
    }

    data.latitude = atof(latp);
    data.longitude = atof(longp);

    return Status::kOk;
}

SIM7000MQTT::Status SIM7000MQTT::publishMessage(const SIM7000MQTT::Topic& topic, const std::string& message) noexcept
{
    RUN_OPERATION(ATsmpub_(topic.c_str(), message.c_str()));
    return Status::kOk;
}

SIM7000MQTT::Status SIM7000MQTT::ATsmconf_(const char* parameter, const char* value) noexcept
{
    char msg[100] {};
    sprintf(msg, "AT+SMCONF=\"%s\",\"%s\"\r", parameter, value);

    auto comm_status = comm_.rawSend(msg, resp_, 500);

    if (comm_status != ATCommunicator::Status::kOk || checkResp_(resp_, "OK") != Status::kOk)
        return Status::kError;

    return Status::kOk;
}

SIM7000MQTT::Status SIM7000MQTT::ATsmconf_(const char* parameter, const char* value, uint16_t port) noexcept
{
    char msg[100] {};
    sprintf(msg, "AT+SMCONF=\"%s\",\"%s\",%i\r", parameter, value, port);

    auto comm_status = comm_.rawSend(msg, resp_, 500);

    if (comm_status != ATCommunicator::Status::kOk || checkResp_(resp_, "OK") != Status::kOk)
        return Status::kError;

    return Status::kOk;
}

SIM7000MQTT::Status SIM7000MQTT::ATcnact_(bool status) noexcept
{
    char msg[100] {};
    sprintf(msg, "AT+CNACT=%d\r", status ? 1 : 0);

    while (true) {
        auto comm_status = comm_.rawSend(msg, resp_, 1000);
        if (comm_status != ATCommunicator::Status::kOk || checkResp_(resp_, "OK") != Status::kOk)
            return Status::kError;

        comm_status = comm_.waitResponse(resp_, 32000);
        if (comm_status == ATCommunicator::Status::kTimeout || comm_status == ATCommunicator::Status::kError)
            return Status::kError;

        if (checkResp_(resp_, status ? "ACTIVE" : "DEACTIVE") == Status::kOk)
            break;
    }

    return Status::kOk;
}

SIM7000MQTT::Status SIM7000MQTT::ATsmconn_(bool status) noexcept
{
    auto comm_status = comm_.rawSend(status ? "AT+SMCONN\r" : "AT+SMDISC\r", resp_, 5000);

    if (comm_status != ATCommunicator::Status::kOk || checkResp_(resp_, "OK") != Status::kOk)
        return Status::kError;

    return Status::kOk;
}

SIM7000MQTT::Status SIM7000MQTT::ATgnspwr_(bool status) noexcept
{
    char msg[100] {};
    sprintf(msg, "AT+CGNSPWR=%d\r", status ? 1 : 0);

    auto comm_status = comm_.rawSend(msg, resp_, 5000);

    if (comm_status != ATCommunicator::Status::kOk || checkResp_(resp_, "OK") != Status::kOk)
        return Status::kError;

    return Status::kOk;
}

SIM7000MQTT::Status SIM7000MQTT::ATsmpub_(const char* topic, const char* message) noexcept
{
    char msg[100] {};
    sprintf(msg, "AT+SMPUB=\"%s\",%d,1,1\r", topic, strlen(message));

    auto comm_status = comm_.rawSend(msg, resp_, 2000);

    if (comm_status != ATCommunicator::Status::kOk || checkResp_(resp_, ">") != Status::kOk)
        return Status::kError;

    sprintf(msg, "%s\r", message);
    comm_status = comm_.rawSend(message, resp_, 5000);
    if (comm_status != ATCommunicator::Status::kOk || checkResp_(resp_, "OK") != Status::kOk)
        return Status::kError;

    return Status::kOk;
}

SIM7000MQTT::Status SIM7000MQTT::ATgnsinf_(char* reply) noexcept
{
    auto comm_status = comm_.rawSend("AT+CGNSINF\r", reply, 5000);

    if (comm_status != ATCommunicator::Status::kOk)
        return Status::kError;

    comm_status = comm_.waitResponse(nullptr, 5000);

    return Status::kOk;
}

SIM7000MQTT::Status SIM7000MQTT::checkResp_(const char* resp, const char* except) noexcept
{
    return strstr(resp, except) == 0 ? Status::kError : Status::kOk;
}