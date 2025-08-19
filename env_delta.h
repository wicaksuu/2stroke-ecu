#pragma once
#include <Arduino.h>
#include "env_types.h"

namespace ecu {

struct EDParams { float iat_per10c_pc=0.03f, baro_per10kpa_pc=0.06f, rh_per20pc_pc=-0.01f, max_fuel_pc=0.15f, max_ign_deg=0.5f; };

class EnvDelta{
public:
  void begin(const EDParams&p){ P=p; }
  inline void compute(const EnvInputs& in, float& fuel_mul, float& ign_add_deg) const {
    float fuel_pc=0.0f;
    fuel_pc += P.iat_per10c_pc   * ((25.0f - in.iat_c)/10.0f);
    fuel_pc += P.baro_per10kpa_pc* ((in.baro_kpa - 101.0f)/10.0f);
    fuel_pc += P.rh_per20pc_pc   * ((in.rh_pc - 40.0f)/20.0f);
    fuel_pc = constrain(fuel_pc, -P.max_fuel_pc, P.max_fuel_pc);
    fuel_mul *= (1.0f + fuel_pc);
    float ign=0.0f;
    if(in.iat_c<20 && in.baro_kpa>101) ign+=0.3f; // dense cool air
    if(in.iat_c>35 || in.baro_kpa<95)  ign-=0.3f; // hot / thin air
    ign = constrain(ign, -P.max_ign_deg, P.max_ign_deg);
    ign_add_deg += ign;
  }
private: EDParams P{}; };

} // namespace ecu
