#include "led_status.h"
#include "config.h"
#include "driver/gpio.h"

void led_status_init(void) {
    gpio_set_direction(CALL_LED, GPIO_MODE_OUTPUT);
    gpio_set_direction(DND_LED, GPIO_MODE_OUTPUT);
}

void led_status_set_call(bool on) {
    gpio_set_level(CALL_LED, on);
}

void led_status_set_dnd(bool on) {
    gpio_set_level(DND_LED, on);
}
