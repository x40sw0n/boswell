#include "gpio_buttons.h"
#include "config.h"
#include "driver/gpio.h"

void gpio_buttons_init(void) {
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL<<INTERCOM_BUTTON) | (1ULL<<DND_BUTTON),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&io_conf);
}

bool gpio_button_pressed(int gpio_num) {
    return gpio_get_level(gpio_num) == 0;
}
