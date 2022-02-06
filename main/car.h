#pragma once
#include <freertos/FreeRTOS.h>

void led_init(void);
void led_start(TickType_t interval);
void led_stop(uint32_t level);

void wifi_init(void);
void camera_init(void);
void mic_init(void);

struct conn_info;
struct netconn;
void control_init(void);
struct netconn *control_get_conn(void);
struct conn_info **control_get_clients(void);

void motor_init(void);
void motor_move(float, float);

void ota_init(void);
void ota_print_version(void);
bool ota_is_factory(void);
