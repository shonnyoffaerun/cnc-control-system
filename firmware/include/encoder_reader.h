#pragma once

#include <cstdint>
#include <driver/pcnt.h>

/*
    EncoderReader - reads quadrature encoder pulses from servo motors using the
    ESP32 PCNT (Pulse Counter) peripheral. One PCNT unit is dedicated to each axis.

    A/B quadrature decoding gives signed position counts. The host requests the
    live count over the binary protocol (QUERY_POS) so closed-loop position can be
    verified against the open-loop step plan computed by grbl_esp32.
*/

struct EncoderConfig
{
    pcnt_unit_t unit;
    int         pin_a;
    int         pin_b;
    int16_t     pulse_per_rev;
    float       mm_per_rev;
};

class EncoderReader
{
public:
    EncoderReader();
    ~EncoderReader();

    bool init(const EncoderConfig& cfg);
    void reset();

    int32_t count() const;

    float positionMm() const;

    bool   hasFault() const;

private:
    EncoderConfig m_cfg{};
    bool          m_initialized{false};
    mutable int16_t m_last_raw{0};
    int32_t       m_accumulator{0};
};
