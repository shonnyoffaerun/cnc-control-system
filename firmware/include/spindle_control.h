#pragma once

#include <cstdint>

/*
    SpindleControl - drives the drill/spindle on the MKS DLC32 via a hardware PWM
    channel. The router is a simple on/off AC spindle on most bare wood CNCs, but
    the DLC32 exposes a PWM output that scales speed for VFD / brushless setups.
*/

class SpindleControl
{
public:
    SpindleControl();
    ~SpindleControl();

    bool init(uint8_t pwm_pin, uint16_t max_rpm = 24000);

    void setRpm(uint16_t rpm);

    uint16_t currentRpm() const { return m_rpm; }
    bool     isOn() const { return m_rpm > 0; }

    void stop();

private:
    uint8_t  m_pwm_pin{0};
    uint16_t m_max_rpm{24000};
    uint16_t m_rpm{0};
    bool     m_initialized{false};
};
