#include <esp_event.h>
#include <esp_netif.h>
#include <esp_wifi.h>
#include <esp_err.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "car.h"
#include "sdkconfig.h"

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
            led_stop(1);
            break;
        case IP_EVENT_STA_LOST_IP:
            led_start(500);
            break;
    }
}

void wifi_init(void) {
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
            .ssid       = CONFIG_WIFI_SSID,
            .password   = CONFIG_WIFI_PASS,
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
            .pmf_cfg.capable = true,
        },
    };
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
}
