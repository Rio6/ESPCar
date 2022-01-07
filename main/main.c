#include <esp_log.h>
#include <esp_event.h>
#include <esp_err.h>
#include <nvs_flash.h>
#include "car.h"

void app_main(void) {
    // NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // Event loop
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    led_init();
    wifi_init();
    ota_init();

    led_start(500);
}
