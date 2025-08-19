#include <Arduino.h>
#include <stdarg.h>
#include <math.h>
#include "env_types.h"
#include "drag_controller.h"
#include "hardware_io.h"  // ensure declarations match implementations

namespace ecu {
static uint32_t t0=0; static bool serial_ready=false; static uint32_t last_log=0;

void log_begin(uint32_t baud){
  Serial.begin(baud);
  delay(150); // allow serial port to settle
  serial_ready = Serial && (Serial.availableForWrite()>0); // log only if host present
}

void log_printf(const char* fmt, ...){
  if(!serial_ready) return;
  char buf[196];
  va_list ap; va_start(ap, fmt);
  vsnprintf(buf, sizeof(buf), fmt, ap);
  va_end(ap);
  Serial.print(buf);
}

void io_init(){ t0=millis(); }

void io_read(EnvInputs& in){
  // SIMPLE SIMULATOR so you can see state transitions without sensors
  static uint32_t last=millis();
  uint32_t now=millis();
  float dt=(now-last)/1000.0f; if(dt<=0) dt=0.01f; last=now;

  // Assume WOT after 300 ms
  if((now - t0) > 300) in.tps_pc = 100.0f; else in.tps_pc = 0.0f;

  // Mock RPM rise (asymptotically approaches ~10k)
  float t=(now - t0)/1000.0f;
  int rpm_prev=in.rpm;
  in.rpm = 3000 + (int)(7000.0f*(1.0f - expf(-t*0.9f)));
  in.dRPM_dt = (in.rpm - rpm_prev) / max(0.001f, dt);

  // Mock speed & distance
  float v_prev=in.speed_mps;
  in.speed_mps = 2.0f + 40.0f*(1.0f - expf(-t*0.8f));
  in.dv_dt = (in.speed_mps - v_prev)/max(0.001f, dt);
  in.distance_m += in.speed_mps * dt;
  in.dv_ds = (in.speed_mps - v_prev)/max(0.01f, in.speed_mps*dt);

  // Thermal & supply (safe nominal)
  in.egt_c = 720.0f + 10.0f*sinf(t*1.3f);
  in.cht_c = 95.0f;
  in.fuel_rail_bar = 3.2f;
  in.duty_inj_pc = 60.0f;
  in.batt_v = 12.4f;

  // Environment (change to test ED)
  in.iat_c = 30.0f; in.baro_kpa=100.0f; in.rh_pc=50.0f;
}

float baseFuelMsEstimate(const EnvInputs& in, float baseMs){ (void)in; return baseMs; }
float baseIgnDegEstimate(const EnvInputs& in, float baseDeg){ (void)in; return baseDeg; }

void applyInjection(float fuel_ms){ (void)fuel_ms; /* hook injector driver here */ }
void applyIgnition(float ign_deg){ (void)ign_deg; /* hook CDI/TCI timing here */ }

void log_status(const EnvInputs& in, const Outputs& out, ecu::DragState st){
  if(!serial_ready) return;
  if(millis()-last_log<200) return;
  last_log=millis();
  Serial.printf("st=%d v=%.1fkmh s=%.1fm rpm=%d fuel×=%.3f ign+=%.2f EGT=%.0f CHT=%.0f\n",
    (int)st, in.speed_mps*3.6f, in.distance_m, in.rpm, out.fuel_mul, out.ign_add_deg, in.egt_c, in.cht_c);
}

} // namespace ecu
