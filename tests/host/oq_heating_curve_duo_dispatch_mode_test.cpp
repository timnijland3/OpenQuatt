#include <assert.h>

#include "../../openquatt/includes/control/oq_heating_curve_logic.h"

int main() {
  using oq_curve::DuoDispatchMode;
  using oq_curve::duo_dispatch_mode;
  using oq_curve::duo_enable_thresholds;

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

    const auto maintain = duo_enable_thresholds(DuoDispatchMode::SEQUENTIAL, 9, level_cap, false, 2);
    assert(maintain.single_search_max_level == 9);
    assert(maintain.single_saturated_level == 8);  // matches the "level 8 of 10" field-observed trigger point.
    assert(maintain.enable_min_u == 0.80f);
    assert(maintain.disable_max_u == 0.55f);

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

    const auto maintain = duo_enable_thresholds(DuoDispatchMode::SHARE_LOAD, 10, level_cap, false, 4);
    assert(maintain.enable_min_u == 0.40f);
    assert(maintain.disable_max_u == 0.15f);

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
    assert(floored.disable_max_u == 0.05f);  // clamped up from 0.0f.

    // Ceiling: an out-of-range start level is clamped below level_cap so the
    // lead ODU is never allowed to run the whole range alone in this mode.
    const auto capped = duo_enable_thresholds(DuoDispatchMode::SHARE_LOAD, 10, level_cap, true, 15);
    assert(capped.single_search_max_level == level_cap - 1);
  }
  return 0;
}
