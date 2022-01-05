#include <esp_err.h>

#include <driver/gpio.h>

#include <freertos/FreeRTOS.h>
#include <freertos/timers.h>

#include "car.h"

#define LED_PIN 2

static TimerHandle_t blink_timer;

static void blink_timer_handler(TimerHandle_t timer) {
    static int level = 0;
    gpio_set_level(LED_PIN, level);
    level ^= 1;
}

void led_init(void) {
    gpio_pad_select_gpio(LED_PIN);
    ESP_ERROR_CHECK(gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT));

    gpio_pad_select_gpio(4);
    ESP_ERROR_CHECK(gpio_set_direction(4, GPIO_MODE_OUTPUT));

    static StaticTimer_t buff;
    blink_timer = xTimerCreateStatic("LED", 1000 / portTICK_PERIOD_MS, pdTRUE, 0, blink_timer_handler, &buff);

    if(!blink_timer) {
        abort();
    }
}

void led_start(TickType_t interval) {
    xTimerChangePeriod(blink_timer, interval / portTICK_PERIOD_MS, portMAX_DELAY);
    xTimerStart(blink_timer, 0);
}

void led_stop(uint32_t level) {
    xTimerStop(blink_timer, portMAX_DELAY);
    gpio_set_level(LED_PIN, level);
}
