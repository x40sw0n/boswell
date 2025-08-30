#pragma once
#include <stdbool.h>

void gpio_buttons_init(void);
bool gpio_button_pressed(int gpio_num);
