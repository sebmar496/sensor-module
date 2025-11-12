#include "leos/log.h"
#include "config.h"
#include "leos/mcp251xfd.h"
#include "leos/cyphal/node.h"
#include "module_setup.h"
#include "pico/stdlib.h"
#include "leos/purpleboard.h"
#include <stdio.h>

#define SDA 16
#define SCL 17

void main() {
    // --- INITIALIZE MODULE ---
    leos_log_init_console(ULOG_INFO_LEVEL);
    MCP251XFD dev;
    leos_cyphal_node_t node;
    if (init_module(&dev, &node) < 0) {
        LOG_ERROR("A critical communications error has occured. This node is offline.");
        return;
    }

    // Your setup code goes here
    leos_purpleboard_t *pb = NULL;
    leos_purpleboard_readings_t pb_readings = {0};
    
    leos_purpleboard_result_t pb_init_res = leos_purpleboard_init(i2c0, SDA, SCL, &pb);

    if (pb_init_res > 0) {
        LOG_ERROR("A critical sensor error has occured upon initialization: error code %d.", pb_init_res);
        // return;
    }

    // After finishing initialization, set our mode to operational
    node.mode.value = uavcan_node_Mode_1_0_OPERATIONAL;
    // Turn on board LED to indicate setup success.
    gpio_put(PICO_DEFAULT_LED_PIN, 1);

    // --- MAIN LOOP ---
    LOG_INFO("Entering main loop...");
    while (true) {
        leos_mcp251xfd_task(&dev);
        leos_cyphal_task(&node);

        // Your looping code goes here
        leos_purpleboard_result_t pb_sensor_res = leos_purpleboard_read(pb, &pb_readings);

        if (pb_sensor_res > 0) {
            LOG_ERROR("A sensor error has occured upon reading data: error code %d.", pb_init_res);
        }

        printf("---- PurpleBoard Sensor Readings ----\n");
        printf("Temperature      : %.2f °C\n", pb_readings.temperature_c);
        printf("Pressure         : %.2f mbar\n", pb_readings.pressure_mb);
        printf("UV Index         : %u\n", pb_readings.uvs);
        printf("PM1.0 (env)      : %u µg/m³\n", pb_readings.pm10_env);
        printf("PM2.5 (env)      : %u µg/m³\n", pb_readings.pm25_env);
        printf("PM10  (env)      : %u µg/m³\n", pb_readings.pm100_env);
        printf("AQI (PM2.5, US)  : %u\n", pb_readings.aqi_pm25_us);
        printf("AQI (PM10, US)   : %u\n", pb_readings.aqi_pm100_us);
        printf("Light Intensity  : %.2f lux\n", pb_readings.light_lux);
        printf("------------------------------------\n");

        sleep_ms(1000);
    }
}