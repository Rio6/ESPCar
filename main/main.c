#include <esp_log.h>
#include <esp_event.h>
#include <esp_err.h>
#include <nvs_flash.h>
#include <mdns.h>
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

    // MDNS
    ESP_ERROR_CHECK(mdns_init());
    mdns_hostname_set("espcar");
    mdns_instance_name_set("ESP32 Car");

    led_init();
    led_start(500);
    wifi_init();

    //ota_print_version();
    //if(ota_is_factory()) {
    //    ota_init();
    //} else {
    //    camera_init();
    //    // Wait for some time then mark ota as valid
    //    vTaskDelay(3000 / portTICK_PERIOD_MS);
    //    esp_ota_mark_app_valid_cancel_rollback();
    //}

    motor_init();
    control_init();
    camera_init();
    //mic_init();
}
