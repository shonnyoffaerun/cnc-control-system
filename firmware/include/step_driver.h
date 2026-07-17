#pragma once

#include <cstdint>

/*
    StepDriver - outputs step/direction/enable signals to the bare CNC's stepper
    drivers. The MKS DLC32 V2.1 sinks these into external A4988/TMC drivers that
    move the gantry in 3 axes. Pulse generation is left to grbl_esp32's stepper
    ISR; this class only owns the GPIO configuration and enable/disable gating.
*/

struct AxisPinMap
{
    uint8_t step;
    uint8_t dir;
    uint8_t enable;   // active-low on most drivers
};

class StepDriver
{
public:
    StepDriver();
    ~StepDriver();

    bool init(const AxisPinMap& x, const AxisPinMap& y, const AxisPinMap& z);

    void enableAll();
    void disableAll();

    void setAxisEnabled(char axis, bool enabled);

    bool isEnabled() const { return m_enabled; }

private:
    static void configureAxis(const AxisPinMap& axis);

    AxisPinMap m_x{}, m_y{}, m_z{};
    bool       m_initialized{false};
    bool       m_enabled{false};
};
