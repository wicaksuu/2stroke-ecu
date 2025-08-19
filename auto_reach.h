#pragma once
#include <Arduino.h>
#include "env_types.h"

namespace ecu {

struct AutoReachParams{
  bool use_v_target=true; float v_target_kmh=0; float v_margin_kmh=1.5f;
  float dvds_min=0.03f; float dvdt_min=0.15f; uint16_t win_m=25; uint16_t win_ms=300;
  uint16_t min_push_ms=450; float min_push_m=25.0f;
};

class AutoReach{
public:
  void begin(const AutoReachParams&p){ P=p; reset(); }
  void reset(){ reached_=false; t0_=millis(); s0_=0; }
  void arm(float s0){ s0_=s0; t0_=millis(); reached_=false; }
  void setVtarget(float kmh){ v_tgt_kmh_=kmh; }
  bool reached() const { return reached_; }
  float s_reach() const { return s_reach_; }

  inline void sample(const EnvInputs& in){
    if(reached_) return;
    uint32_t dt=millis()-t0_; float ds=in.distance_m - s0_;
    bool minTime = dt >= P.min_push_ms; bool minDist = ds >= P.min_push_m;
    bool hit=false;
    if(P.use_v_target && v_tgt_kmh_>0.1f){ float vm=P.v_margin_kmh*(1000.f/3600.f);
      hit = in.speed_mps >= (v_tgt_kmh_*(1000.f/3600.f) - vm); }
    bool weak = (in.dv_ds <= P.dvds_min) && (in.dv_dt <= P.dvdt_min);
    if(minTime && minDist && (hit || weak)){ reached_=true; s_reach_=in.distance_m; }
  }
private:
  AutoReachParams P{}; bool reached_=false; float s_reach_=0; uint32_t t0_=0; float s0_=0; float v_tgt_kmh_=0;
};

} // namespace ecu
