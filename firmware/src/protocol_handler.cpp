#include "protocol_handler.h"

#include <Arduino.h>

ProtocolHandler::ProtocolHandler() = default;

void ProtocolHandler::begin(uint8_t uart_num, int rx_pin, int tx_pin, uint32_t baud)
{
    m_uart_num = uart_num;
    m_uart = (uart_num == 1) ? &Serial1 : &Serial2;
    m_uart->begin(baud, SERIAL_8N1, rx_pin, tx_pin);
}

void ProtocolHandler::update()
{
    if (m_uart == nullptr)
    {
        return;
    }
    while (m_uart->available() > 0)
    {
        processByte(static_cast<uint8_t>(m_uart->read()));
    }
}

void ProtocolHandler::processByte(uint8_t b)
{
    if (m_rx_len == 0 && b != PROTOCOL_MAGIC && b != PROTOCOL_MAGIC_ACK)
    {
        return;
    }
    if (m_rx_len >= sizeof(m_rx_buffer))
    {
        m_rx_len = 0;
        return;
    }

    m_rx_buffer[m_rx_len++] = b;

    if (m_rx_len >= PROTOCOL_HEADER_SIZE)
    {
        const uint8_t length = m_rx_buffer[2];
        const size_t  total  = PROTOCOL_HEADER_SIZE + length + PROTOCOL_CRC_SIZE;
        if (m_rx_len >= total)
        {
            ProtocolFrame frame{};
            if (protocol_deserialize(m_rx_buffer, m_rx_len, frame))
            {
                dispatch(frame);
            }
            else
            {
                sendAck(frame.header.seq_id, StatusCode::ERR_CHECKSUM);
            }
            m_rx_len = 0;
        }
    }
}

void ProtocolHandler::dispatch(const ProtocolFrame& frame)
{
    switch (static_cast<CmdType>(frame.header.type))
    {
        case CmdType::GCODE:
        {
            if (frame.header.length > 0 && m_onGcode)
            {
                char line[PROTOCOL_MAX_PAYLOAD_SIZE + 1];
                const size_t n = frame.header.length < PROTOCOL_MAX_PAYLOAD_SIZE
                                     ? frame.header.length : PROTOCOL_MAX_PAYLOAD_SIZE;
                memcpy(line, frame.payload, n);
                line[n] = '\0';
                m_onGcode(line);
            }
            sendAck(frame.header.seq_id, StatusCode::OK);
            break;
        }
        case CmdType::TOOL_CONFIG:
        {
            ToolConfigPayload payload{};
            if (frame.header.length >= sizeof(payload) && m_onTool)
            {
                memcpy(&payload, frame.payload, sizeof(payload));
                m_onTool(payload);
            }
            sendAck(frame.header.seq_id, StatusCode::OK);
            break;
        }
        case CmdType::QUERY_POS:
        case CmdType::QUERY_STATUS:
        {
            sendTelemetry();
            sendAck(frame.header.seq_id, StatusCode::OK);
            break;
        }
        case CmdType::EMERGENCY_STOP:
        {
            if (m_onEstop) m_onEstop();
            sendAck(frame.header.seq_id, StatusCode::OK);
            break;
        }
        case CmdType::HOME_AXIS:
        {
            if (m_onHome) m_onHome();
            sendAck(frame.header.seq_id, StatusCode::OK);
            break;
        }
        case CmdType::RESET_CTRL:
        {
            if (m_onReset) m_onReset();
            sendAck(frame.header.seq_id, StatusCode::OK);
            break;
        }
        default:
        {
            sendAck(frame.header.seq_id, StatusCode::ERR_INVALID_CMD);
            break;
        }
    }
}

void ProtocolHandler::sendAck(uint8_t seq_id, StatusCode code)
{
    ProtocolFrame frame{};
    frame.header.magic  = PROTOCOL_MAGIC_ACK;
    frame.header.type   = (code == StatusCode::OK)
                              ? static_cast<uint8_t>(CmdType::ACK)
                              : static_cast<uint8_t>(CmdType::NACK);
    frame.header.seq_id = seq_id;
    frame.header.length = 1;
    frame.payload[0]    = static_cast<uint8_t>(code);

    uint8_t buffer[PROTOCOL_MAX_PACKET_SIZE];
    const size_t written = protocol_serialize(frame, buffer, sizeof(buffer));
    if (written > 0 && m_uart)
    {
        m_uart->write(buffer, static_cast<int>(written));
    }
}

void ProtocolHandler::publishTelemetry(float x, float y, float z,
                                       uint16_t spindle_rpm, uint16_t feed_rate,
                                       uint8_t flags, uint8_t active_tool)
{
    m_telemetry.pos_x        = x;
    m_telemetry.pos_y        = y;
    m_telemetry.pos_z        = z;
    m_telemetry.spindle_rpm  = spindle_rpm;
    m_telemetry.feed_rate    = feed_rate;
    m_telemetry.flags        = flags;
    m_telemetry.active_tool  = active_tool;
}

void ProtocolHandler::sendTelemetry()
{
    ProtocolFrame frame{};
    frame.header.magic  = PROTOCOL_MAGIC;
    frame.header.type   = static_cast<uint8_t>(CmdType::TELEMETRY);
    frame.header.seq_id = protocol_next_seq(m_seq);

    memcpy(frame.payload, &m_telemetry, sizeof(m_telemetry));
    frame.header.length = static_cast<uint8_t>(sizeof(m_telemetry));

    uint8_t buffer[PROTOCOL_MAX_PACKET_SIZE];
    const size_t written = protocol_serialize(frame, buffer, sizeof(buffer));
    if (written > 0 && m_uart)
    {
        m_uart->write(buffer, static_cast<int>(written));
    }
}
