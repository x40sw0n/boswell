#include "app_intercom.h"
#include "sip_wrapper.h"
#include "audio_i2s.h"
#include "gpio_buttons.h"
#include "led_status.h"
#include "config.h"

static bool dnd_enabled = false;

void intercom_init(void) {
    gpio_buttons_init();
    led_status_init();
    audio_i2s_init();
    sip_init();
}

void intercom_task(void) {
    if (gpio_button_pressed(INTERCOM_BUTTON)) {
        if (!dnd_enabled) {
            sip_make_call(SIP_PEER_URI);
            led_status_set_call(true);
        }
    }
    if (gpio_button_pressed(DND_BUTTON)) {
        dnd_enabled = !dnd_enabled;
        sip_set_dnd(dnd_enabled);
        led_status_set_dnd(dnd_enabled);
    }
}
