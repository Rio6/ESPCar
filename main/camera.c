#include <esp_camera.h>
#include <esp_err.h>
#include <esp_log.h>

#include <lwip/sockets.h>

static const char *TAG = "camera";

#define MAX_PACKET_SIZE 400
#define MIN_FRAME_TICK 1000 / CONFIG_CAMERA_FPS_CAP / portTICK_PERIOD_MS

static int min(size_t a, size_t b) {
    return a < b ? a : b;
}

static void camera_task(void *args) {
    int fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    assert(fd >= 0);

    while(1) {
        TickType_t start = xTaskGetTickCount();

        camera_fb_t *fb = esp_camera_fb_get();
        if(!fb) {
            ESP_LOGE(TAG, "Error getting frame buffer");
            goto loop_end;
        }

        struct sockaddr_in dest_addr = {
            .sin_family        = AF_INET,
            .sin_port          = htons(3232),
        };
        if(inet_pton(AF_INET, "192.168.0.30", &dest_addr.sin_addr) < 0) {
            goto loop_end;
        }

        size_t sent = 0;
        while(sent < fb->len) {
            ssize_t len = sendto(fd, fb->buf + sent, min(fb->len - sent, MAX_PACKET_SIZE),
                    0, (struct sockaddr*) &dest_addr, sizeof(dest_addr));
            if(len < 0) {
                break;
            }
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
