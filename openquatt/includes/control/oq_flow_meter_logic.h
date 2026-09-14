#pragma once

#include <cmath>

namespace oq_flow_meter {

inline float linear_lph(float pulses_per_minute, float pulses_per_liter) {
  if (!std::isfinite(pulses_per_minute) || !std::isfinite(pulses_per_liter) || pulses_per_liter <= 0.0f) {
    return NAN;
  }
  return pulses_per_minute > 0.0f ? pulses_per_minute * 60.0f / pulses_per_liter : 0.0f;
}

}  // namespace oq_flow_meter
