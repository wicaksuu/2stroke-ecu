#pragma once
#include <Arduino.h>
#include "env_types.h"
#include "drag_controller.h"

// Minimal I/O stubs so sketch compiles and can simulate motion
namespace ecu {
void io_init();
void io_read(EnvInputs& in);
float baseFuelMsEstimate(const EnvInputs& in, float baseMs);
float baseIgnDegEstimate(const EnvInputs& in, float baseDeg);
void applyInjection(float fuel_ms);
void applyIgnition(float ign_deg);

// logging helpers (print only if serial attached)
void log_begin(uint32_t baud);
void log_printf(const char* fmt, ...);
void log_status(const EnvInputs& in, const Outputs& out, ecu::DragState st);
}
