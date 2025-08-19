#pragma once
#include <Arduino.h>

namespace ecu {

enum class Profile : uint8_t { HARIAN=0, RACE_200, RACE_300, RACE_400, RACE_500, RACE_1000 };

struct ProfileParams{
  Profile profile; float goal_distance_m; float v_target_top_kmh;
  float frontload_gain; float frontload_window_pc; float plateau_fuel_pc; float plateau_ign_deg;
};

struct EnvInputs{
  // Vehicle kinematics (from GPS/IMU fusion or stubbed)
  float speed_mps=0; float distance_m=0; float dv_dt=0; float dv_ds=0;
  // Engine
  int rpm=0; float dRPM_dt=0; float tps_pc=0; float dTPS_dt=0;
  // Thermal & supply
  float egt_c=0; float cht_c=0; float fuel_rail_bar=3.0f; float duty_inj_pc=0; float batt_v=12.5f;
  // Environment
  float iat_c=25; float baro_kpa=101; float rh_pc=40;
};

struct Outputs{ float fuel_mul=1.0f; float ign_add_deg=0.0f; };

} // namespace ecu
