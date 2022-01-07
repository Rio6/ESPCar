#include <esp_log.h>
#include <esp_ota_ops.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <lwip/sockets.h>

#define BUFF_SIZE 512

static const char *TAG = "OTA";

static void ota_server_task(void *args) {
    int ret;

    int lfd = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
    assert(lfd >= 0);

    int opt = 1;
    ret = setsockopt(lfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    assert(ret >= 0);

    struct sockaddr_in listen_addr = {
        .sin_family        = AF_INET,
        .sin_addr.s_addr   = htonl(INADDR_ANY),
        .sin_port          = htons(8266),
    };
    ret = bind(lfd, (struct sockaddr*) &listen_addr, sizeof(listen_addr));
    assert(ret >= 0);

    ret = listen(lfd, 1);
    assert(ret >= 0);

    while(1) {
        int fd = accept(lfd, NULL, NULL);
        esp_err_t err = ESP_OK;
        esp_ota_handle_t ota_handle;
        const esp_partition_t *partition = esp_ota_get_next_update_partition(NULL);

        uint32_t ota_size;
        for(size_t read = 0; read < sizeof(ota_size);) {
            read += recv(fd, (uint8_t*) &ota_size + read, sizeof(ota_size) - read, 0);
        }
        ota_size = ntohl(ota_size);
        ESP_LOGI(TAG, "Receiving OTA with size: %u", ota_size);

        err = esp_ota_begin(partition, ota_size, &ota_handle);

        uint8_t *buff = malloc(BUFF_SIZE);
        if(!buff) {
            err = ESP_ERR_NO_MEM;
        }

        size_t received = 0;
        while(received < ota_size && err == ESP_OK) {
            ssize_t len = recv(fd, buff, BUFF_SIZE, 0);

            if(len == 0) break;

            if(len < 0) {
                err = ESP_FAIL;
                break;
            }

            err = esp_ota_write(ota_handle, buff, len);
            received += len;
        }

        free(buff);

        if(err == ESP_OK) {
            err = esp_ota_end(ota_handle);
        } else {
            esp_ota_abort(ota_handle); // ignore abort error
        }

        if(err == ESP_OK) {
            esp_ota_set_boot_partition(partition);
        }

        const char *msg = esp_err_to_name(err);
        send(fd, msg, strlen(msg) + 1, 0);
        close(fd);

        if(err == ESP_OK) {
            esp_restart();
        }
    }
}

void ota_init(void) {
    static StaticTask_t buff;
    static StackType_t stack[5300];
    xTaskCreateStatic(ota_server_task, "OTA", sizeof(stack), NULL, tskIDLE_PRIORITY, stack, &buff);

    const esp_partition_t *running_partition = esp_ota_get_running_partition();
    esp_app_desc_t app_desc;
    ESP_ERROR_CHECK(esp_ota_get_partition_description(running_partition, &app_desc));

    if(running_partition->subtype == ESP_PARTITION_SUBTYPE_APP_FACTORY) {
        ESP_LOGI(TAG, "Current firmware: %s-%s (factory), built on %s %s", app_desc.project_name, app_desc.version, app_desc.date, app_desc.time);
    } else {
        ESP_LOGI(TAG, "Current firmware: %s-%s (ota), built on %s %s", app_desc.project_name, app_desc.version, app_desc.date, app_desc.time);
        esp_ota_mark_app_valid_cancel_rollback();
    }
}
