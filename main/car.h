#include <freertos/FreeRTOS.h>

void led_init(void);
void led_start(TickType_t interval);
void led_stop(uint32_t level);

void wifi_init(void);
void motor_init(void);
void camera_init(void);

void ota_init(void);
bool ota_is_factory(void);
