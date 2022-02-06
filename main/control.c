#include <esp_log.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <lwip/api.h>
#include <lwip/ip_addr.h>

#include "car.h"
#include "proto.h"
#include "sdkconfig.h"

static const char *TAG = "control";

static struct {
    int16_t x;
    int16_t y;
} current_status = {0};

struct netconn *conn = NULL;
struct conn_info conn_infos[CONFIG_MAX_CONNECTION] = {0};

static struct conn_info *find_conn(ip_addr_t *addr) {
    struct conn_info *empty = NULL;
    for(unsigned i = 0; i < conn_infos_len; i++) {
        if(addr && ip_addr_cmp(&conn_infos[i].addr, addr)) {
            return &conn_infos[i];
        }

        if(!empty && conn_infos[i].state == DISCONNECTED) {
            empty = &conn_infos[i];
        }
    }
    return empty;
}

static void disconnect_conn(struct conn_info *conn_info) {
    if(!conn_info) return;
    ESP_LOGI(TAG, "disconnect from %s", ipaddr_ntoa(&conn_info->addr));
    conn_info->state = DISCONNECTED;
}

struct conn_info **control_get_clients(void) {
    static struct conn_info *clients[conn_infos_len + 1];
    uint64_t now = xTaskGetTickCount() * portTICK_PERIOD_MS;

    size_t c = 0;
    for(size_t i = 0; i < conn_infos_len; i++) {
        if(conn_infos[i].state == CONNECTED) {
            if(now - conn_infos[i].last_recv_time > CONFIG_CONNECTION_TIMEOUT) {
                disconnect_conn(&conn_infos[i]);
            } else {
                clients[c++] = &conn_infos[i];
            }
        }
    }
    clients[c] = NULL;
    return clients;
}

static void control_server(void *params) {
    while(1) {
        uint64_t now = xTaskGetTickCount() * portTICK_PERIOD_MS;

        // remove timeout'd clients and send info
        static uint64_t last_status_time = 0;
        if(now - last_status_time > CONFIG_STATUS_INTERVAL) {
            for(struct conn_info **client = control_get_clients(); *client != NULL; client++) {
                struct netbuf send_buf = {0};
                uint8_t *buf_data = netbuf_alloc(&send_buf, STATUS_SIZE);
                if(!buf_data) goto loop_end;

                struct status *status = (struct status*) buf_data;
                status->header.size = STATUS_SIZE;
                status->header.type = FRAMETYPE_STATUS;
                status->x = htons(current_status.x);
                status->y = htons(current_status.y);

                err_t err = netconn_sendto(conn, &send_buf, &(*client)->addr, (*client)->port);
                if(err != ERR_OK) {
                    ESP_LOGE(TAG, "Error sending status: %s", lwip_strerr(err));
                }

                netbuf_free(&send_buf);
            }
            last_status_time = now;
        }

        // receive commands
        struct netbuf *buff = NULL;
        err_t err = netconn_recv(conn, &buff);
        if(err != ERR_OK) goto loop_end;

        ip_addr_t *addr = netbuf_fromaddr(buff);
        struct conn_info *conn_info = find_conn(addr);
        if(!conn_info) goto loop_end;

        if(conn_info->state == DISCONNECTED) {
            ESP_LOGI(TAG, "new connection from %s", ipaddr_ntoa(addr));
            ip_addr_set(&conn_info->addr, addr);
            conn_info->state = CONNECTED;
        }

        conn_info->port = netbuf_fromport(buff);
        conn_info->last_recv_time = now;

        void *data = NULL;
        uint16_t len = 0;
        netbuf_data(buff, &data, &len);

        for(uint16_t i = 0; i + COMMAND_SIZE <= len; i += COMMAND_SIZE) {
            struct command *cmd = data + i;
            switch(cmd->type) {
                case 'X':
                    disconnect_conn(&conn_infos[i]);
                    break;
                case 'M':
                    {
                        current_status.x = ntohs(cmd->x);
                        current_status.y = ntohs(cmd->y);
                        float x = (float) ((int16_t) current_status.x) / INT16_MAX * 100;
                        float y = (float) ((int16_t) current_status.y) / INT16_MAX * 100;
                        motor_move(y + x, y - x);
                        break;
                    }
                default:
                    break;
            }
        }

loop_end:;
        netbuf_delete(buff);
    }
}

void control_init(void) {
    conn = netconn_new(NETCONN_UDP);
    assert(conn != NULL);

    netconn_set_recvtimeout(conn, 100);

    err_t err = netconn_bind(conn, IP4_ADDR_ANY, CONFIG_CONTROL_PORT);
    assert(err == ERR_OK);

    static StaticTask_t buff;
    static StackType_t stack[2048];
    xTaskCreateStatic(control_server, TAG, sizeof(stack) / sizeof(stack[0]), NULL, tskIDLE_PRIORITY+1, stack, &buff);
}

struct netconn *control_get_conn(void) {
    return conn;
}
