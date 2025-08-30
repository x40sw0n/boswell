#include "app_intercom.h"
#include "config.h"

void app_main(void) {
    intercom_init();
    while (1) {
        intercom_task();
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}
