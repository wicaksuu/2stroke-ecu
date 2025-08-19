#pragma once
#include <Arduino.h>
#include "env_types.h"

namespace ecu {
struct SafetyLimits{ float egt_soft_c=780.0f, cht_soft_c=110.0f, duty_max_pc=85.0f; };
inline void applySafety(const EnvInputs& in, Outputs& out, const SafetyLimits& L){
  if(in.egt_c > L.egt_soft_c){ out.fuel_mul *= 1.05f; out.ign_add_deg -= 1.0f; }
  if(in.cht_c > L.cht_soft_c){ out.fuel_mul *= 1.05f; out.ign_add_deg -= 1.0f; }
  if(in.duty_inj_pc > L.duty_max_pc){ /* clamp injector PW in driver if needed */ }
}
} // namespace ecu
