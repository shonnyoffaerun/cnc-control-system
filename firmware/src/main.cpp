/*
    main.cpp - MKS DLC32 V2.1 firmware entry point.

    Responsibilities:
      1. Boot grbl_esp32 (motion planner, homing, $ settings).
      2. Configure the stepper/servo drivers, spindle PWM and encoder readers.
      3. Run the binary ProtocolHandler on UART2 so the host PC can send G-code
         frames, query position (from the servo encoders) and stream telemetry.
      4. Periodically publish TELEMETRY back to the host.

    grbl_esp32 is left on UART0 (USB) for backward-compatible textual control;
    the binary protocol lives on UART2 (GPIO21/22) and is what this host app uses.
*/

#include <Arduino.h>

#include "protocol_handler.h"
#include "encoder_reader.h"
#include "step_driver.h"
#include "spindle_control.h"

#include "grbl.hpp"

// ---- Pin map (mirrors platformio.ini) -------------------------------------
static constexpr uint8_t X_STEP = X_STEP_PIN;
static constexpr uint8_t X_DIR  = X_DIR_PIN;
static constexpr uint8_t X_EN   = X_ENABLE_PIN;
static constexpr uint8_t Y_STEP = Y_STEP_PIN;
static constexpr uint8_t Y_DIR  = Y_DIR_PIN;
static constexpr uint8_t Y_EN   = Y_ENABLE_PIN;
static constexpr uint8_t Z_STEP = Z_STEP_PIN;
static constexpr uint8_t Z_DIR  = Z_DIR_PIN;
static constexpr uint8_t Z_EN   = Z_ENABLE_PIN;
static constexpr uint8_t SPINDLE_PWM = SPINDLE_PWM_PIN;

static constexpr int  PROTO_RX = PROTOCOL_RX_PIN;
static constexpr int  PROTO_TX = PROTOCOL_TX_PIN;
static constexpr long PROTO_BAUD = PROTOCOL_BAUD;

static constexpr int  ENC_X_A = ENC_X_A_PIN, ENC_X_B = ENC_X_B_PIN;
static constexpr int  ENC_Y_A = ENC_Y_A_PIN, ENC_Y_B = ENC_Y_B_PIN;
static constexpr int  ENC_Z_A = ENC_Z_A_PIN, ENC_Z_B = ENC_Z_B_PIN;

// ---- Hardware objects ------------------------------------------------------
static ProtocolHandler   g_proto;
static StepDriver        g_step;
static SpindleControl    g_spindle;
static EncoderReader     g_encX, g_encY, g_encZ;

static uint8_t           g_activeTool = 0;
static uint32_t          g_lastTelemetry = 0;
static constexpr uint32_t TELEMETRY_MS = 200;

static void onGcode(const char* line)
{
    grbl_stream_write(line);
}

static void onToolConfig(const ToolConfigPayload& tool)
{
    g_activeTool = tool.tool_id;
    (void)tool;
}

static void onEmergencyStop()
{
    g_spindle.stop();
    g_step.disableAll();
    serial_reset();
}

static void onHome()
{
    grbl_stream_write("$H\n");
}

static void onReset()
{
    serial_reset();
    g_step.enableAll();
}

void setup()
{
    Serial.begin(115200);
    grbl_init();

    g_step.init(
        {X_STEP, X_DIR, X_EN},
        {Y_STEP, Y_DIR, Y_EN},
        {Z_STEP, Z_DIR, Z_EN});
    g_step.enableAll();

    g_spindle.init(SPINDLE_PWM, 24000);

    g_encX.init({PCNT_UNIT_0, ENC_X_A, ENC_X_B, 400, 8.0f});
    g_encY.init({PCNT_UNIT_1, ENC_Y_A, ENC_Y_B, 400, 8.0f});
    g_encZ.init({PCNT_UNIT_2, ENC_Z_A, ENC_Z_B, 400, 4.0f});

    g_proto.begin(PROTOCOL_UART_NUM, PROTO_RX, PROTO_TX, PROTO_BAUD);
    g_proto.setGcodeHandler(onGcode);
    g_proto.setToolHandler(onToolConfig);
    g_proto.setEmergencyStop(onEmergencyStop);
    g_proto.setHomeHandler(onHome);
    g_proto.setResetHandler(onReset);

    g_proto.sendAck(0, StatusCode::OK);
}

void loop()
{
    grbl_read();

    g_proto.update();

    const uint32_t now = millis();
    if (now - g_lastTelemetry >= TELEMETRY_MS)
    {
        g_lastTelemetry = now;

        uint8_t flags = 0;
        if (g_spindle.isOn()) flags |= StatusFlags::SPINDLE_ON;
        if (g_step.isEnabled()) flags |= StatusFlags::IS_RUNNING;

        g_proto.publishTelemetry(
            g_encX.positionMm(), g_encY.positionMm(), g_encZ.positionMm(),
            g_spindle.currentRpm(), 0,
            flags, g_activeTool);
        g_proto.sendTelemetry();
    }
}
