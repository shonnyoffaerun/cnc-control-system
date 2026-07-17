#include "encoder_reader.h"

#include <Arduino.h>

EncoderReader::EncoderReader() = default;

EncoderReader::~EncoderReader()
{
    if (m_initialized)
    {
        pcnt_unit_disable(m_cfg.unit);
        pcnt_del_unit(m_cfg.unit);
    }
}

bool EncoderReader::init(const EncoderConfig& cfg)
{
    m_cfg = cfg;

    pcnt_config_t pcnt{};
    pcnt.unit     = cfg.unit;
    pcnt.channel  = PCNT_CHANNEL_0;
    pcnt.pulse_gpio_num = static_cast<gpio_num_t>(cfg.pin_a);
    pcnt.ctrl_gpio_num  = static_cast<gpio_num_t>(cfg.pin_b);
    pcnt.pos_mode = PCNT_COUNT_INC;
    pcnt.neg_mode = PCNT_COUNT_DEC;
    pcnt.lctrl_mode = PCNT_MODE_KEEP;
    pcnt.hctrl_mode = PCNT_MODE_REVERSE;
    pcnt.counter_l_lim = -32768;
    pcnt.counter_h_lim =  32767;

    if (pcnt_unit_config(&pcnt) != ESP_OK)
    {
        return false;
    }

    pcnt_counter_pause(cfg.unit);
    pcnt_counter_clear(cfg.unit);
    pcnt_counter_resume(cfg.unit);

    m_initialized = true;
    m_last_raw = 0;
    m_accumulator = 0;
    return true;
}

void EncoderReader::reset()
{
    m_accumulator = 0;
    m_last_raw = 0;
    if (m_initialized)
    {
        pcnt_counter_pause(m_cfg.unit);
        pcnt_counter_clear(m_cfg.unit);
        pcnt_counter_resume(m_cfg.unit);
    }
}

int32_t EncoderReader::count() const
{
    if (!m_initialized)
    {
        return 0;
    }
    int16_t raw = 0;
    pcnt_get_counter_value(m_cfg.unit, &raw);

    int16_t delta = static_cast<int16_t>(raw - m_last_raw);
    m_last_raw = raw;
    m_accumulator += delta;
    return m_accumulator;
}

float EncoderReader::positionMm() const
{
    if (m_cfg.pulse_per_rev == 0 || m_cfg.mm_per_rev == 0.0f)
    {
        return 0.0f;
    }
    return (static_cast<float>(count()) / static_cast<float>(m_cfg.pulse_per_rev)) * m_cfg.mm_per_rev;
}

bool EncoderReader::hasFault() const
{
    return !m_initialized;
}
