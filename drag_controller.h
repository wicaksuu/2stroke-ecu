#pragma once
#include <Arduino.h>
#include "env_types.h"
#include "env_delta.h"
#include "auto_reach.h"

namespace ecu {

enum class DragState : uint8_t { ARM=0, LAUNCH, PUSH, HOLD, FINISH };

struct DragTuning{ float max_push_fuel=0.12f, max_push_ign_deg=0.7f; uint16_t ae_tps_ms=100, ae_rpm_ms=80; float dTPS_thr_ps=25.0f, dRPM_thr_ps=1500.0f; };

class DragController{
public:
  void begin(const ProfileParams& prof, const AutoReachParams& arP, const DragTuning& tune){ prof_=prof; tune_=tune; ar_.begin(arP); state_=DragState::ARM; }
  void attachModules(EnvDelta* ed, AutoReach* ar){ ed_=ed; external_ar_=ar; }
  void arm(float s0){ state_=DragState::ARM; ar_.arm(s0); ae_tps_until_=0; ae_rpm_until_=0; }
  void setVtargetInit(float kmh){ v_tgt_kmh_=kmh; }
  DragState state() const { return state_; }

  inline void tick(const EnvInputs& in, Outputs& out){
    out.fuel_mul=1.0f; out.ign_add_deg=0.0f;
    if(ed_) ed_->compute(in, out.fuel_mul, out.ign_add_deg);

    switch(state_){
      case DragState::ARM:
        if(in.tps_pc>=90.0f) state_=DragState::LAUNCH; break;
      case DragState::LAUNCH:
        state_=DragState::PUSH; break;
      case DragState::PUSH: {
        uint32_t now=millis();
        if(in.dTPS_dt>tune_.dTPS_thr_ps) ae_tps_until_=now+tune_.ae_tps_ms;
        if(in.dRPM_dt>tune_.dRPM_thr_ps) ae_rpm_until_=now+tune_.ae_rpm_ms;
        float extra=0.0f; if(now<ae_tps_until_) extra+=0.15f; if(now<ae_rpm_until_) extra+=0.08f;

        float bump=0.0f;
        if(v_tgt_kmh_>0.1f){
          float vt=v_tgt_kmh_*(1000.f/3600.f);
          if(vt>0.1f){ float ev=(vt - in.speed_mps)/vt; if(ev>0){
            float env=1.0f; if(prof_.goal_distance_m>1.0f){ float x=in.distance_m/(prof_.goal_distance_m*max(0.1f,prof_.frontload_window_pc)); env=constrain(1.0f-x,0.0f,1.0f);} bump=ev*prof_.frontload_gain*env; }}
        }
        bump=constrain(bump,0.0f,tune_.max_push_fuel);

        out.fuel_mul *= (1.0f + bump + extra);
        if(tune_.max_push_fuel>1e-6f)
          out.ign_add_deg += tune_.max_push_ign_deg * (bump / tune_.max_push_fuel);

        ar_.setVtarget(v_tgt_kmh_);
        ar_.sample(in);
        if(ar_.reached()) state_=DragState::HOLD;
      } break;
      case DragState::HOLD:
        out.fuel_mul *= (1.0f + prof_.plateau_fuel_pc);
        out.ign_add_deg += prof_.plateau_ign_deg;
        if(prof_.goal_distance_m>1.0f && in.distance_m>=prof_.goal_distance_m) state_=DragState::FINISH;
        break;
      case DragState::FINISH: break;
    }
  }
private:
  ProfileParams prof_{}; DragTuning tune_{}; AutoReach ar_{}; EnvDelta* ed_=nullptr; AutoReach* external_ar_=nullptr; DragState state_=DragState::ARM; uint32_t ae_tps_until_=0, ae_rpm_until_=0; float v_tgt_kmh_=0.0f;
};

} // namespace ecu
