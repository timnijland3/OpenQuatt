import assert from "node:assert/strict";
import test from "node:test";

globalThis.__OQ_PREVIEW__ = false;
globalThis.window = {
  innerWidth: 1200,
  innerHeight: 800,
  localStorage: { getItem: () => null },
  matchMedia: () => ({ matches: false }),
};
globalThis.localStorage = { getItem: () => null };

const { state } = await import("../js/src/core/state.js");
const { renderOverviewStatCardMarkup } = await import("../js/src/views/overview.js");
const { renderHeatPumpPanel, renderOverviewView, getBoilerPanelModel, renderBoilerCompactPanel } = await import("../js/src/views/heatpump.js");
const { HP_PANEL_CONFIGS } = await import("../js/src/core/config.js");

test("echte KPI-cards dragen flow- en cop-targets", () => {
  const flow = renderOverviewStatCardMarkup({ key: "flowSelected", label: "Flow", value: "16,8 L/min", tone: "sky" });
  assert.ok(flow.includes('data-oq-matrix-target="flow"'));
  const cop = renderOverviewStatCardMarkup({ key: "totalCop", label: "COP", value: "5,4", tone: "green" });
  assert.ok(cop.includes('data-oq-matrix-target="cop"'));
  const eer = renderOverviewStatCardMarkup({ key: "totalEer", label: "COP (EER)", value: "4,1", tone: "green" });
  assert.ok(eer.includes('data-oq-matrix-target="cop"'));
  const other = renderOverviewStatCardMarkup({ key: "totalPower", label: "Elektrisch", value: "450 W", tone: "blue" });
  assert.ok(!other.includes("data-oq-matrix-target"));
});

test("echte HP-panels dragen hp1/hp2-targets", () => {
  const prevEntities = state.entities;
  const prevMode = state.hpVisualMode;
  try {
    state.hpVisualMode = "compact";
    state.entities = {
      hp1Power: { value: 450 },
      hp1Freq: { value: 23 },
      hp2Power: { value: 400 },
      hp2Freq: { value: 23 },
    };
    const hp1 = HP_PANEL_CONFIGS[0];
    assert.ok(renderHeatPumpPanel(hp1.title, hp1.keys, hp1.accent).includes('data-oq-matrix-target="hp1"'));
    const hp2 = HP_PANEL_CONFIGS[1];
    assert.ok(renderHeatPumpPanel(hp2.title, hp2.keys, hp2.accent).includes('data-oq-matrix-target="hp2"'));
  } finally {
    state.entities = prevEntities;
    state.hpVisualMode = prevMode;
  }
});

test("echte boiler-panel draagt boiler-target", () => {
  const prevEntities = state.entities;
  try {
    state.entities = { boilerConnection: { value: "Relais" } };
    assert.ok(renderBoilerCompactPanel(getBoilerPanelModel()).includes('data-oq-matrix-target="boiler"'));
  } finally {
    state.entities = prevEntities;
  }
});

test("echte overview-grid draagt hardware-section-target", () => {
  const prevEntities = state.entities;
  const prevMode = state.hpVisualMode;
  try {
    state.hpVisualMode = "compact";
    state.entities = {
      hp1Power: { value: 450 },
      hp1Freq: { value: 23 },
      flowSelected: { value: 1008 },
      totalCop: { value: 5.4 },
    };
    const html = renderOverviewView();
    assert.ok(html.includes('data-oq-matrix-target="hardware-section"'));
    assert.ok(html.includes('data-oq-matrix-target="hp1"'));
    assert.ok(html.includes('data-oq-matrix-target="flow"'));
    assert.ok(html.includes('data-oq-matrix-target="cop"'));
  } finally {
    state.entities = prevEntities;
    state.hpVisualMode = prevMode;
  }
});
