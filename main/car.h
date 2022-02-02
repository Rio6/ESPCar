#pragma once
#include <freertos/FreeRTOS.h>

void led_init(void);
void led_start(TickType_t interval);
void led_stop(uint32_t level);

void wifi_init(void);
void control_init(void);
void camera_init(void);
void mic_init(void);

void motor_init(void);
void motor_move(int16_t, int16_t);

void ota_init(void);
void ota_print_version(void);
bool ota_is_factory(void);
