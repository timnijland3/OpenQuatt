#include <assert.h>
#include <math.h>

#include "../../openquatt/includes/control/oq_flow_meter_logic.h"

namespace {

bool near(float actual, float expected, float tolerance = 0.001f) { return fabsf(actual - expected) <= tolerance; }

void test_custom_linear_profile() {
  assert(near(oq_flow_meter::linear_lph(0.0f, 476.0f), 0.0f));
  assert(near(oq_flow_meter::linear_lph(476.0f, 476.0f), 60.0f));
  assert(near(oq_flow_meter::linear_lph(4760.0f, 476.0f), 600.0f));
  assert(isnan(oq_flow_meter::linear_lph(500.0f, 0.0f)));
  assert(isnan(oq_flow_meter::linear_lph(NAN, 500.0f)));
}

}  // namespace

int main() {
  test_custom_linear_profile();
  return 0;
}
