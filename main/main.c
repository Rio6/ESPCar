#include <driver/mcpwm.h>

#include "car.h"

void app_main(void) {
    led_init();
    wifi_init();

    led_start(500);
}
