#include "PowerTelemetry.hpp"

#include <cassert>
#include <cmath>
#include <iostream>

static void near(float actual, float expected)
{
    assert(std::fabs(actual - expected) < 0.001f);
}

int main()
{
    // Charging consumes some battery power before the shared output branches.
    const auto charging = PowerTelemetry::measureBusPower(24.0f, 25.0f, 5.0f);
    near(charging.battery, 600.0f);
    near(charging.vehicleLoad, 480.0f);

    // Discharging supplements battery output; the total load is higher.
    const auto discharging = PowerTelemetry::measureBusPower(24.0f, 25.0f, -5.0f);
    near(discharging.battery, 600.0f);
    near(discharging.vehicleLoad, 720.0f);

    // Use measured bus voltage, including voltage sag and regenerative flow.
    const auto sag = PowerTelemetry::measureBusPower(20.0f, 25.0f, 0.0f);
    near(sag.battery, 500.0f);
    near(sag.vehicleLoad, 500.0f);
    const auto regen = PowerTelemetry::measureBusPower(24.0f, -2.0f, 3.0f);
    near(regen.battery, -48.0f);
    near(regen.vehicleLoad, -120.0f);

    assert(PowerTelemetry::encodePower(-256.0f) == 0U);
    assert(PowerTelemetry::encodePower(0.0f) == 16384U);
    assert(PowerTelemetry::encodePower(430.0f) == 43904U);
    assert(PowerTelemetry::encodePower(767.984375f) == 65535U);
    assert(PowerTelemetry::encodePower(1000.0f) == 65535U);
    assert(PowerTelemetry::encodePower(-300.0f) == 0U);
    assert(PowerTelemetry::encodePower(0.015625f) == 16385U);
    std::cout << "PASS: bus-power flow, measured voltage, regeneration and encoding boundaries\n";
}
