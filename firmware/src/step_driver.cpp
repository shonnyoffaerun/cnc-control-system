#include "step_driver.h"

#include <Arduino.h>

StepDriver::StepDriver() = default;

StepDriver::~StepDriver()
{
    disableAll();
}

bool StepDriver::init(const AxisPinMap& x, const AxisPinMap& y, const AxisPinMap& z)
{
    m_x = x; m_y = y; m_z = z;

    configureAxis(m_x);
    configureAxis(m_y);
    configureAxis(m_z);

    m_initialized = true;
    disableAll();
    return true;
}

void StepDriver::configureAxis(const AxisPinMap& axis)
{
    pinMode(axis.step, OUTPUT);
    pinMode(axis.dir, OUTPUT);
    pinMode(axis.enable, OUTPUT);
    digitalWrite(axis.step, LOW);
    digitalWrite(axis.dir, LOW);
}

void StepDriver::enableAll()
{
    digitalWrite(m_x.enable, LOW);
    digitalWrite(m_y.enable, LOW);
    digitalWrite(m_z.enable, LOW);
    m_enabled = true;
}

void StepDriver::disableAll()
{
    digitalWrite(m_x.enable, HIGH);
    digitalWrite(m_y.enable, HIGH);
    digitalWrite(m_z.enable, HIGH);
    m_enabled = false;
}

void StepDriver::setAxisEnabled(char axis, bool enabled)
{
    const uint8_t level = enabled ? LOW : HIGH;
    switch (axis)
    {
        case 'X': digitalWrite(m_x.enable, level); break;
        case 'Y': digitalWrite(m_y.enable, level); break;
        case 'Z': digitalWrite(m_z.enable, level); break;
        default: break;
    }
}
