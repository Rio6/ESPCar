#include <esp_event.h>
#include <esp_netif.h>
#include <esp_wifi.h>
#include <esp_http_server.h>
#include <esp_camera.h>
#include <esp_err.h>

#include <driver/gpio.h>
#include <driver/mcpwm.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/timers.h>

#include <stdio.h>

#include "secret.h"

static TimerHandle_t blink_timer;

static void blink_timerr(TimerHandle_t timer) {
    static int level = 0;
    ESP_ERROR_CHECK(gpio_set_level(2, level));
    level ^= 1;
}

static void init_gpio(void) {
    const gpio_num_t outputs[] = { 2, 4, 12, 13, 14, 15 };
    for(size_t i = 0; i < sizeof(outputs) / sizeof(outputs[0]); i++) {
        gpio_pad_select_gpio(outputs[i]);
        ESP_ERROR_CHECK(gpio_set_direction(outputs[i], GPIO_MODE_OUTPUT));
    }
}

static void wifi_event_handler(void *arg, esp_event_base_t base, int32_t event, void *data) {
    switch(event) {
        case WIFI_EVENT_STA_DISCONNECTED:
            vTaskDelay(3000 / portTICK_PERIOD_MS);
            // fall through
        case WIFI_EVENT_STA_START:
            esp_wifi_connect();
            break;
    }
}

static void ip_event_handler(void *arg, esp_event_base_t base, int32_t event, void *data) {
    switch(event) {
        case IP_EVENT_STA_GOT_IP:
            xTimerStop(blink_timer, 0);
            gpio_set_level(2, 1);
            break;
        case IP_EVENT_STA_LOST_IP:
            xTimerStart(blink_timer, 0);
            break;
    }
}

static void init_wifi(void) {
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    ESP_ERROR_CHECK(esp_netif_init());
    esp_netif_create_default_wifi_sta();

    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, *wifi_event_handler, NULL, NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, ESP_EVENT_ANY_ID, *ip_event_handler, NULL, NULL));

    wifi_init_config_t wifi_init_config = WIFI_INIT_CONFIG_DEFAULT();
    wifi_init_config.nvs_enable = false;
    ESP_ERROR_CHECK(esp_wifi_init(&wifi_init_config));

    wifi_config_t wifi_config = {
        .sta = {
            .ssid       = WIFI_SSID,
            .password   = WIFI_PASS,
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
            .pmf_cfg.capable = true,
        },
    };
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
}

static void init_camera(void) {
    camera_config_t camera_config = {
        .pin_pwdn     = 32,
        .pin_reset    = -1,
        .pin_xclk     = 0,
        .pin_sscb_sda = 26,
        .pin_sscb_scl = 27,

        .pin_d7       = 35,
        .pin_d6       = 34,
        .pin_d5       = 39,
        .pin_d4       = 36,
        .pin_d3       = 21,
        .pin_d2       = 19,
        .pin_d1       = 18,
        .pin_d0       = 5,
        .pin_vsync    = 25,
        .pin_href     = 23,
        .pin_pclk     = 22,

        .xclk_freq_hz = 10000000,
        .ledc_timer   = LEDC_TIMER_0,
        .ledc_channel = LEDC_CHANNEL_0,

        .pixel_format = PIXFORMAT_JPEG,
        .frame_size   = FRAMESIZE_SXGA,

        .jpeg_quality = 12,
        .fb_count     = 2,
        .grab_mode    = CAMERA_GRAB_LATEST,
    };
    ESP_ERROR_CHECK(esp_camera_init(&camera_config));
}

static void init_mcpwm(void) {
    mcpwm_pin_config_t mcpwm_pin_config = {
        .mcpwm0a_out_num = 13,
        .mcpwm0b_out_num = 14,
        .mcpwm1a_out_num = 15,
        .mcpwm1b_out_num = 12,
    };
    ESP_ERROR_CHECK(mcpwm_set_pin(MCPWM_UNIT_0, &mcpwm_pin_config));

    mcpwm_config_t mcpwm_config = {
        .frequency   = 1000,
        .counter_mode = MCPWM_UP_COUNTER,
    };
    ESP_ERROR_CHECK(mcpwm_init(MCPWM_UNIT_0, MCPWM_TIMER_0, &mcpwm_config));
    ESP_ERROR_CHECK(mcpwm_init(MCPWM_UNIT_0, MCPWM_TIMER_1, &mcpwm_config));
}

void app_main(void) {
    init_gpio();
    init_wifi();
    init_mcpwm();
    init_camera();

    static StaticTimer_t timer;
    blink_timer = xTimerCreateStatic(NULL, 500 / portTICK_PERIOD_MS, pdTRUE, 0, blink_timerr, &timer);
    xTimerStart(blink_timer, 0);

    //mcpwm_set_signal_low(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_B);
    //mcpwm_set_signal_low(MCPWM_UNIT_0, MCPWM_TIMER_1, MCPWM_OPR_B);
    //mcpwm_set_duty_type(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, MCPWM_DUTY_MODE_0);
    //mcpwm_set_duty_type(MCPWM_UNIT_0, MCPWM_TIMER_1, MCPWM_OPR_A, MCPWM_DUTY_MODE_0);
}
