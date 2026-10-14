#pragma once

#include <stdint.h>

namespace PowerTelemetry
{
struct BusPower
{
    float battery;
    float vehicleLoad;
};

// iConverter is positive when the DCDC charges the capacitor from the bus.
inline BusPower measureBusPower(float busVoltage, float iBattery, float iConverter)
{
    return {busVoltage * iBattery, busVoltage * (iBattery - iConverter)};
}

// Preserve the existing 1/64 W wire format. Saturate before unsigned conversion.
inline uint16_t encodePower(float watts)
{
    if (watts <= -256.0f)
        return 0U;
    if (watts >= 767.984375f)
        return UINT16_MAX;
    return static_cast<uint16_t>(watts * 64.0f + 16384.0f);
}
} // namespace PowerTelemetry
