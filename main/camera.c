#include <esp_camera.h>
#include <esp_err.h>
#include <esp_log.h>

#include <lwip/api.h>

#include <string.h>

#include "proto.h"

static const char *TAG = "camera";

#define MAX_PACKET_SIZE 400
#define MIN_FRAME_TICK 1000 / CONFIG_CAMERA_FPS_CAP / portTICK_PERIOD_MS

static int min(size_t a, size_t b) {
    return a < b ? a : b;
}

static void camera_task(void *args) {
    struct netconn *conn = netconn_new(NETCONN_UDP);
    assert(conn != NULL);

    while(1) {
        TickType_t start = xTaskGetTickCount();

        camera_fb_t *fb = esp_camera_fb_get();
        if(!fb) {
            ESP_LOGE(TAG, "Error getting frame buffer");
            goto loop_end;
        }

        ip_addr_t dest_addr;
        if(!ipaddr_aton("192.168.0.30", &dest_addr)) {
            goto loop_end;
        }

        for(size_t sent = 0; sent < fb->len;) {
            size_t len = min(fb->len - sent, MAX_PACKET_SIZE /*- HEADER_SIZE*/);
            struct netbuf buf = {0};
            uint8_t *buf_data = netbuf_alloc(&buf, /*HEADER_SIZE + */len);
            if(!buf_data) goto loop_end;

            //buf_data[0] = FRAMETYPE_VIDEO;
            //buf_data[1] = len >> 7 & 0xFF;
            //buf_data[2] = len      & 0xFF;
            memcpy(buf_data /*+ HEADER_SIZE*/, fb->buf + sent, len);

            err_t err = netconn_sendto(conn, &buf, &dest_addr, CONFIG_CAMERA_PORT);
            netbuf_free(&buf);
            if(err != ERR_OK) goto loop_end;

            sent += len;
        }

loop_end:
        esp_camera_fb_return(fb);

        TickType_t elapsed = xTaskGetTickCount() - start;
        if(elapsed < MIN_FRAME_TICK) {
            vTaskDelay(MIN_FRAME_TICK - elapsed);
        }
    }
}

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
        .frame_size   = FRAMESIZE_SVGA,

        .jpeg_quality = 5,
        .fb_count     = 2,
        .grab_mode    = CAMERA_GRAB_LATEST,
    };
    ESP_ERROR_CHECK(esp_camera_init(&camera_config));

    static StaticTask_t buff;
    static StackType_t stack[8192];
    xTaskCreateStaticPinnedToCore(camera_task, TAG, sizeof(stack), NULL, tskIDLE_PRIORITY+1, stack, &buff, 0);
}
