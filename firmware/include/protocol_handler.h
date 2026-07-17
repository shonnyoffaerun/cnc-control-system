#pragma once

#include <cstdint>
#include <functional>

#include "protocol.h"

/*
    ProtocolHandler - firmware-side counterpart of the host protocol. It listens on
    a dedicated UART (UART2 on the MKS DLC32) for binary ProtocolFrames, validates
    them with CRC, dispatches to the machine via callbacks, and streams periodic
    TELEMETRY frames back to the host (position from encoders, spindle rpm, flags).
*/

class ProtocolHandler
{
public:
    using GcodeFn      = std::function<void(const char* line)>;
    using ToolFn       = std::function<void(const ToolConfigPayload&)>;
    using SimpleFn     = std::function<void(void)>;

    ProtocolHandler();

    void begin(uint8_t uart_num, int rx_pin, int tx_pin, uint32_t baud);
    void update();

    void setGcodeHandler(GcodeFn fn)   { m_onGcode = std::move(fn); }
    void setToolHandler(ToolFn fn)     { m_onTool  = std::move(fn); }
    void setEmergencyStop(SimpleFn fn) { m_onEstop = std::move(fn); }
    void setHomeHandler(SimpleFn fn)   { m_onHome  = std::move(fn); }
    void setResetHandler(SimpleFn fn)  { m_onReset = std::move(fn); }

    void publishTelemetry(float x, float y, float z,
                          uint16_t spindle_rpm, uint16_t feed_rate,
                          uint8_t flags, uint8_t active_tool);

private:
    void        processByte(uint8_t b);
    void        dispatch(const ProtocolFrame& frame);
    void        sendAck(uint8_t seq_id, StatusCode code);
    void        sendTelemetry();

    HardwareSerial*           m_uart{nullptr};
    uint8_t                   m_uart_num{2};

    uint8_t                   m_rx_buffer[PROTOCOL_MAX_PACKET_SIZE];
    size_t                    m_rx_len{0};
    bool                      m_escaped{false};

    uint8_t                   m_seq{0};

    TelemetryPayload          m_telemetry{};

    GcodeFn                   m_onGcode;
    ToolFn                    m_onTool;
    SimpleFn                  m_onEstop;
    SimpleFn                  m_onHome;
    SimpleFn                  m_onReset;
};
