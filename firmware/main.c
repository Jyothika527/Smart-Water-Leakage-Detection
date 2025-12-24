#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/i2c.h"
#include "hardware/irq.h"
#include "hardware/timer.h"
#include "bmp180.h"

/* =======================
   PIN DEFINITIONS
======================= */
#define FLOW_PIN        15
#define VALVE_PIN       16
#define BUZZER_PIN      17

#define I2C_PORT        i2c0
#define I2C_SDA         0
#define I2C_SCL         1

/* =======================
   FLOW SENSOR VARIABLES
======================= */
volatile uint32_t pulse_count = 0;
absolute_time_t last_time;

/* =======================
   FLOW SENSOR ISR
======================= */
void flow_pulse_handler(uint gpio, uint32_t events) {
    pulse_count++;
}

/* =======================
   FLOW RATE CALCULATION
======================= */
float calculate_flow_rate() {
    absolute_time_t now = get_absolute_time();
    int64_t diff = absolute_time_diff_us(last_time, now);

    if (diff >= 1000000) { // 1 second
        float flow_rate = pulse_count / 7.5f; // L/min
        pulse_count = 0;
        last_time = now;
        return flow_rate;
    }
    return -1.0f;
}

/* =======================
   LEAK DETECTION LOGIC
======================= */
bool detect_leak(float flow, int32_t pressure) {
    if (pressure < 95000) return true;
    if (flow > 10.0f) return true;

    if (!gpio_get(VALVE_PIN) && flow > 0.5f)
        return true;

    return false;
}

/* =======================
   MAIN FUNCTION
======================= */
int main() {
    stdio_init_all();

    /* GPIO INIT */
    gpio_init(VALVE_PIN);
    gpio_set_dir(VALVE_PIN, GPIO_OUT);
    gpio_put(VALVE_PIN, 1);  // Valve OPEN

    gpio_init(BUZZER_PIN);
    gpio_set_dir(BUZZER_PIN, GPIO_OUT);

    gpio_init(FLOW_PIN);
    gpio_set_dir(FLOW_PIN, GPIO_IN);
    gpio_pull_up(FLOW_PIN);

    /* INTERRUPT SETUP */
    gpio_set_irq_enabled_with_callback(
        FLOW_PIN,
        GPIO_IRQ_EDGE_FALL,
        true,
        &flow_pulse_handler
    );

    /* I2C INIT */
    i2c_init(I2C_PORT, 100 * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    bmp180_t bmp;
    bmp180_init(&bmp, I2C_PORT);

    last_time = get_absolute_time();

    printf("Smart Water Leakage System Started\n");

    while (true) {
        float flow = calculate_flow_rate();
        int32_t pressure = bmp180_read_pressure(&bmp);

        if (flow >= 0) {
            printf("Flow: %.2f L/min | Pressure: %ld Pa\n", flow, pressure);

            bool leak = detect_leak(flow, pressure);

            if (leak) {
                printf("🚨 LEAK DETECTED!\n");
                gpio_put(VALVE_PIN, 0);   // Close valve
                gpio_put(BUZZER_PIN, 1);  // Buzzer ON
            } else {
                gpio_put(VALVE_PIN, 1);
                gpio_put(BUZZER_PIN, 0);
            }
        }

        sleep_ms(500);
    }
}