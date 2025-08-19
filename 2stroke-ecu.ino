#include <Arduino.h>
#include "env_types.h"
#include "env_delta.h"
#include "auto_reach.h"
#include "drag_controller.h"
#include "safety.h"
#include "hardware_io.h"

using namespace ecu;

// ---------- User Presets (edit here) ----------
static ProfileParams PROF = {
  Profile::RACE_200,   // profile
  200.0f,              // goal_distance_m
  150.0f,              // v_target_top_kmh (0 = adapt later)
  0.90f,               // frontload_gain
  0.60f,               // frontload_window_pc
  0.025f,              // plateau_fuel_pc (+2.5%)
  0.4f                 // plateau_ign_deg (+0.4°)
};

static AutoReachParams ARP = {
  true, 150.0f, 1.5f,   // use_v_target, v_target_kmh, v_margin_kmh
  0.03f, 0.15f,         // dvds_min, dvdt_min
  25, 300,              // win_m, win_ms (simple use in detector)
  450, 25.0f            // min_push_ms, min_push_m
};

static DragTuning TUNE = {
  0.12f, 0.7f,          // max_push_fuel, max_push_ign_deg
  100, 80,              // ae_tps_ms, ae_rpm_ms
  25.0f, 1500.0f        // dTPS_thr_ps, dRPM_thr_ps
};

static EDParams EDP = { 0.03f, 0.06f, -0.01f, 0.15f, 0.5f };
static SafetyLimits SAFE = { 780.0f, 110.0f, 85.0f };

// ---------- System objects ----------
static EnvInputs IN;
static Outputs   OUT;
static EnvDelta      ED;
static AutoReach     AR;
static DragController DC;

// Base map placeholders (replace with your real base calculations)
static float BASE_FUEL_MS = 3.0f;
static float BASE_IGN_DEG = 18.0f;

void setup(){
  io_init();
  log_begin(115200); // logs will print only if serial is really attached

  ED.begin(EDP);
  AR.begin(ARP);
  DC.begin(PROF, ARP, TUNE);
  DC.attachModules(&ED, &AR);
  DC.setVtargetInit(PROF.v_target_top_kmh);

  // Arm distance origin
  DC.arm(0.0f);

  log_printf("ECU READY: profile=%d goal=%.0fm vTgt=%.0fkmh\n", (int)PROF.profile, PROF.goal_distance_m, PROF.v_target_top_kmh);
}

void loop(){
  // 1) Read sensors (mock or real inside hardware_io)
  io_read(IN);

  // 2) Compute control outputs
  OUT = {1.0f, 0.0f};
  DC.tick(IN, OUT);
  applySafety(IN, OUT, SAFE);

  // 3) Combine with base map
  float fuel_ms = baseFuelMsEstimate(IN, BASE_FUEL_MS) * OUT.fuel_mul;
  float ign_deg = baseIgnDegEstimate(IN, BASE_IGN_DEG) + OUT.ign_add_deg;

  // 4) Drive actuators
  applyInjection(fuel_ms);
  applyIgnition(ign_deg);

  // 5) HUD/log (prints only if serial attached)
  log_status(IN, OUT, DC.state());
}
