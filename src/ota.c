#include <esp_ota_ops.h>

#include <lwip/sockets.h>

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

        err = esp_ota_begin(partition, OTA_SIZE_UNKNOWN, &ota_handle);

        while(err == ESP_OK) {
            uint8_t buff[128];
            ssize_t len = recv(fd, buff, sizeof(buff), 0);
            if(len == 0) break;
            if(len < 0) err = ESP_FAIL;
            else        err = esp_ota_write(ota_handle, buff, len);
        }

        if(err == ESP_OK) {
            err = esp_ota_end(ota_handle);
        } else {
            esp_ota_abort(ota_handle); // ignore abort error
        }

        if(err == ESP_OK) {
            esp_ota_set_boot_partition(partition);
        }

        const char *msg = esp_err_to_name(err);
        send(fd, msg, strlen(msg), 0);

        if(err == ESP_OK) {
            esp_restart();
        }
    }
}

void ota_init(void) {
}
