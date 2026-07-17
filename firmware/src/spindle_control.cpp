#include "spindle_control.h"

#include <Arduino.h>
#include <driver/ledc.h>

SpindleControl::SpindleControl() = default;

SpindleControl::~SpindleControl()
{
    stop();
}

bool SpindleControl::init(uint8_t pwm_pin, uint16_t max_rpm)
{
    m_pwm_pin = pwm_pin;
    m_max_rpm = max_rpm;

    ledc_timer_config_t timer{};
    timer.speed_mode      = LEDC_HIGH_SPEED_MODE;
    timer.duty_resolution = LEDC_TIMER_10_BIT;
    timer.timer_num       = LEDC_TIMER_0;
    timer.freq_hz         = 1000;
    ledc_timer_config(&timer);

    ledc_channel_config_t channel{};
    channel.gpio_num   = static_cast<gpio_num_t>(pwm_pin);
    channel.speed_mode = LEDC_HIGH_SPEED_MODE;
    channel.channel    = LEDC_CHANNEL_0;
    channel.timer_sel  = LEDC_TIMER_0;
    channel.duty       = 0;
    channel.hpoint     = 0;
    ledc_channel_config(&channel);

    m_initialized = true;
    stop();
    return true;
}

void SpindleControl::setRpm(uint16_t rpm)
{
    if (!m_initialized)
    {
        return;
    }
    m_rpm = (rpm > m_max_rpm) ? m_max_rpm : rpm;

    uint32_t duty = 0;
    if (m_rpm > 0)
    {
        const float ratio = static_cast<float>(m_rpm) / static_cast<float>(m_max_rpm);
        duty = static_cast<uint32_t>(ratio * 1023.0f);
    }
    ledc_set_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0, duty);
    ledc_update_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0);
}

void SpindleControl::stop()
{
    setRpm(0);
}
