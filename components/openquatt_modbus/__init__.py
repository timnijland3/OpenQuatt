import esphome.codegen as cg
import esphome.config_validation as cv
from esphome import pins
from esphome.components import modbus, uart
from esphome.const import CONF_FLOW_CONTROL_PIN, CONF_ID
from esphome.cpp_helpers import gpio_pin_expression

AUTO_LOAD = ["modbus"]
DEPENDENCIES = ["uart"]
MULTI_CONF = True

openquatt_modbus_ns = cg.esphome_ns.namespace("openquatt_modbus")
OpenQuattModbusClientHub = openquatt_modbus_ns.class_(
    "OpenQuattModbusClientHub", modbus.ModbusClient
)

CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(OpenQuattModbusClientHub),
            cv.Optional(modbus.CONF_ROLE, default="client"): cv.one_of("client", lower=True),
            cv.Optional(CONF_FLOW_CONTROL_PIN): pins.gpio_output_pin_schema,
            cv.Optional(modbus.CONF_SEND_WAIT_TIME, default="2000ms"): cv.positive_time_period_milliseconds,
            cv.Optional(modbus.CONF_TURNAROUND_TIME, default="600ms"): cv.positive_time_period_milliseconds,
        }
    )
    .extend(cv.COMPONENT_SCHEMA)
    .extend(uart.UART_DEVICE_SCHEMA)
)


async def to_code(config):
    cg.add_global(openquatt_modbus_ns.using)
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await uart.register_uart_device(var, config)

    if CONF_FLOW_CONTROL_PIN in config:
        pin = await gpio_pin_expression(config[CONF_FLOW_CONTROL_PIN])
        cg.add(var.set_flow_control_pin(pin))

    cg.add(var.set_send_wait_time(config[modbus.CONF_SEND_WAIT_TIME]))
    cg.add(var.set_turnaround_time(config[modbus.CONF_TURNAROUND_TIME]))
