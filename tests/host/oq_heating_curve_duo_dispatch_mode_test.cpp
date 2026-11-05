#include <assert.h>
#include <cmath>

#include "../../openquatt/includes/control/oq_heating_curve_logic.h"

namespace {
bool approx(float actual, float expected) { return std::fabs(actual - expected) < 0.05f; }
}  // namespace

int main() {
  using oq_curve::duo_dispatch_mode;
  using oq_curve::duo_enable_thresholds;
  using oq_curve::DuoDispatchMode;

  // Mode selection: unknown/empty option (e.g. entity not yet restored) falls
  // back to the historical Sequential behaviour, never to Share Load.
  assert(duo_dispatch_mode("Share Load") == DuoDispatchMode::SHARE_LOAD);
  assert(duo_dispatch_mode("1 Running / 1 Standby") == DuoDispatchMode::SEQUENTIAL);
  assert(duo_dispatch_mode("") == DuoDispatchMode::SEQUENTIAL);
  assert(duo_dispatch_mode("garbage") == DuoDispatchMode::SEQUENTIAL);

  constexpr int level_cap = 10;

  // Sequential must reproduce the exact pre-existing hardcoded thresholds:
  // an uncapped single-ODU search, saturation at max(6, owner_max-1), and the
  // fixed 0.90/0.80 enable and 0.70/0.55 disable utilization gates.
  {
    const auto heat = duo_enable_thresholds(DuoDispatchMode::SEQUENTIAL, 10, level_cap, true, 2);
    assert(heat.single_search_max_level == 10);
    assert(heat.single_saturated_level == 9);
    assert(heat.enable_min_u == 0.90f);
    assert(heat.disable_max_u == 0.70f);
    assert(approx(heat.enable_margin_w, 700.0f));
    assert(approx(heat.disable_margin_w, 250.0f));

    const auto maintain = duo_enable_thresholds(DuoDispatchMode::SEQUENTIAL, 9, level_cap, false, 2);
    assert(maintain.single_search_max_level == 9);
    assert(maintain.single_saturated_level == 8);  // matches the "level 8 of 10" field-observed trigger point.
    assert(maintain.enable_min_u == 0.80f);
    assert(maintain.disable_max_u == 0.55f);
    assert(approx(maintain.enable_margin_w, 450.0f));
    assert(approx(maintain.disable_margin_w, 250.0f));

    // Small owner_max: the saturation floor of 6 is preserved even when it
    // exceeds owner_max - 1, matching the original std::max(6, max - 1).
    const auto small = duo_enable_thresholds(DuoDispatchMode::SEQUENTIAL, 5, level_cap, true, 2);
    assert(small.single_search_max_level == 5);
    assert(small.single_saturated_level == 6);
  }

  // Share Load: the single-ODU search is capped at the configured start
  // level (never above the true owner max), and the utilization gate scales
  // down with it instead of staying pinned at 0.90/0.80.
  {
    const auto heat = duo_enable_thresholds(DuoDispatchMode::SHARE_LOAD, 10, level_cap, true, 4);
    assert(heat.single_search_max_level == 4);
    assert(heat.single_saturated_level == 4);
    assert(heat.enable_min_u == 0.40f);
    assert(heat.disable_max_u == 0.20f);
    // The absolute-power margin must scale down with the level too: an
    // unscaled 700W/450W margin would block the hand-off forever at a low
    // start level even when demand genuinely exceeds what it can deliver.
    assert(approx(heat.enable_margin_w, 280.0f));   // 700 * (4/10)
    assert(approx(heat.disable_margin_w, 100.0f));  // 250 * (4/10)

    const auto maintain = duo_enable_thresholds(DuoDispatchMode::SHARE_LOAD, 10, level_cap, false, 4);
    assert(maintain.enable_min_u == 0.40f);
    assert(maintain.disable_max_u == 0.15f);
    assert(approx(maintain.enable_margin_w, 180.0f));  // 450 * (4/10)
    assert(approx(maintain.disable_margin_w, 100.0f));

    // The configured level can never push the search past the ODU's real
    // maximum, even if the user picks a higher start level than that.
    const auto small_owner = duo_enable_thresholds(DuoDispatchMode::SHARE_LOAD, 3, level_cap, true, 6);
    assert(small_owner.single_search_max_level == 3);
    assert(small_owner.single_saturated_level == 3);

    // Floor: a start level of 1 (or below) is raised to 2, so Share Load
    // never triggers on level-1+level-1.
    const auto floored = duo_enable_thresholds(DuoDispatchMode::SHARE_LOAD, 10, level_cap, true, 1);
    assert(floored.single_search_max_level == 2);
    assert(floored.enable_min_u == 0.20f);
    assert(floored.disable_max_u == 0.05f);           // clamped up from 0.0f.
    assert(approx(floored.enable_margin_w, 140.0f));  // 700 * (2/10)
    assert(approx(floored.disable_margin_w, 50.0f));  // 250 * (2/10)

    // Margin floor: a very fine-grained level scale (hypothetical level_cap)
    // must not scale the margin down to (near) zero, or noise could flap
    // the topology. Real firmware always uses level_cap 10, where the
    // level-2 floor already keeps the ratio above this floor; this exercises
    // the floor itself for a level scale where that is not automatic.
    const auto tiny_margin = duo_enable_thresholds(DuoDispatchMode::SHARE_LOAD, 100, 100, false, 2);
    assert(tiny_margin.enable_margin_w == 60.0f);   // 450 * (2/100) = 9, floored to 60
    assert(tiny_margin.disable_margin_w == 40.0f);  // 250 * (2/100) = 5, floored to 40

    // Ceiling: an out-of-range start level is clamped below level_cap so the
    // lead ODU is never allowed to run the whole range alone in this mode.
    const auto capped = duo_enable_thresholds(DuoDispatchMode::SHARE_LOAD, 10, level_cap, true, 15);
    assert(capped.single_search_max_level == level_cap - 1);
  }

  // Share Load keeps both ODUs within one level of each other, so 3+3 wins
  // over 5+1; Sequential and boot-held (fixed) levels are unrestricted.
  {
    using oq_curve::duo_pair_allowed;
    assert(duo_pair_allowed(DuoDispatchMode::SHARE_LOAD, 3, 3, false));
    assert(duo_pair_allowed(DuoDispatchMode::SHARE_LOAD, 3, 2, false));
    assert(duo_pair_allowed(DuoDispatchMode::SHARE_LOAD, 2, 3, false));
    assert(!duo_pair_allowed(DuoDispatchMode::SHARE_LOAD, 5, 1, false));
    assert(!duo_pair_allowed(DuoDispatchMode::SHARE_LOAD, 1, 3, false));
    assert(duo_pair_allowed(DuoDispatchMode::SHARE_LOAD, 5, 1, true));
    assert(duo_pair_allowed(DuoDispatchMode::SEQUENTIAL, 5, 1, false));
  }

  // Share Load sizes an enabled pair against both units' capacity in every
  // phase; with the single-unit RECOVERY target the pair would stall near one
  // unit's maximum output. Sequential keeps the phase target unchanged.
  {
    using oq_curve::duo_pair_target_power_w;
    using oq_curve::phase_target_power_w;
    constexpr float single_cap_w = 6000.0f;
    constexpr float duo_cap_w = 12000.0f;
    assert(duo_pair_target_power_w(DuoDispatchMode::SHARE_LOAD, true, 1.0f, single_cap_w, duo_cap_w) == 12000.0f);
    assert(duo_pair_target_power_w(DuoDispatchMode::SHARE_LOAD, false, 1.0f, single_cap_w, duo_cap_w) == 12000.0f);
    assert(duo_pair_target_power_w(DuoDispatchMode::SHARE_LOAD, true, 0.5f, single_cap_w, duo_cap_w) == 6000.0f);
    assert(duo_pair_target_power_w(DuoDispatchMode::SEQUENTIAL, true, 1.0f, single_cap_w, duo_cap_w) == 6000.0f);
    assert(duo_pair_target_power_w(DuoDispatchMode::SEQUENTIAL, false, 1.0f, single_cap_w, duo_cap_w) == 12000.0f);
    assert(duo_pair_target_power_w(DuoDispatchMode::SEQUENTIAL, true, 0.5f, single_cap_w, duo_cap_w) ==
           phase_target_power_w(true, 0.5f, single_cap_w, duo_cap_w));
    // No demand means no target in either mode.
    assert(duo_pair_target_power_w(DuoDispatchMode::SHARE_LOAD, true, 0.0f, single_cap_w, duo_cap_w) == 0.0f);
  }
  return 0;
}
