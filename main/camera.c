#include <esp_camera.h>
#include <esp_err.h>

void camera_init(void) {
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
