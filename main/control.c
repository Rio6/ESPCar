#include <esp_log.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <lwip/api.h>
#include <lwip/ip_addr.h>

#include "car.h"
#include "proto.h"
#include "sdkconfig.h"

static const char *TAG = "CONTROL";

struct conn_info conn_infos[CONFIG_MAX_CONNECTION] = {0};

static struct conn_info *find_conn(ip_addr_t *addr) {
    uint64_t now = xTaskGetTickCount() * portTICK_PERIOD_MS;
    struct conn_info *empty = NULL;
    for(unsigned i = 0; i < conn_infos_len; i++) {
        if(now - conn_infos[i].last_recv_time > CONFIG_CONNECTION_TIMEOUT) {
            conn_infos[i].state = DISCONNECTED;
        }

        if(addr && ip_addr_cmp(&conn_infos[i].addr, addr)) {
            return &conn_infos[i];
        }

        if(!empty && conn_infos[i].state == DISCONNECTED) {
            empty = &conn_infos[i];
        }
    }
    return empty;
}

static void control_server(void *params) {
    struct netconn *conn = netconn_new(NETCONN_UDP);
    assert(conn != NULL);

    err_t err = netconn_bind(conn, IP4_ADDR_ANY, CONFIG_CONTROL_PORT);
    assert(err == ERR_OK);

    while(1) {
        struct netbuf *buff = NULL;

        err_t err = netconn_recv(conn, &buff);
        if(err != ERR_OK) goto loop_end;

        ip_addr_t *addr = netbuf_fromaddr(buff);
        struct conn_info *conn_info = find_conn(addr);
        if(!conn_info) goto loop_end;

        uint64_t now = xTaskGetTickCount() * portTICK_PERIOD_MS;

        if(conn_info->state == DISCONNECTED) {
            ESP_LOGI(TAG, "new connection from %s", ipaddr_ntoa(addr));
            ip_addr_set(&conn_info->addr, addr);
            conn_info->port = netbuf_fromport(buff);
            conn_info->state = CONNECTED;
        }

        conn_info->last_recv_time = now;

        void *data = NULL;
        uint16_t len = 0;
        netbuf_data(buff, &data, &len);

        for(uint16_t i = 0; i + COMMAND_SIZE <= len; i += COMMAND_SIZE) {
            struct command *cmd = data + i;
            switch(cmd->type) {
                case 'X':
                    conn_info->state = DISCONNECTED;
                    break;
                case 'M':
                    {
                        float x = (float) ((int16_t) ntohs(cmd->x)) / INT16_MAX * 100;
                        float y = (float) ((int16_t) ntohs(cmd->y)) / INT16_MAX * 100;
                        motor_move(y + x, y - x);
                        break;
                    }
                default:
                    break;
            }
        }

loop_end:
        netbuf_delete(buff);
    }
}

void control_init(void) {
    xTaskCreate(control_server, TAG, 2048, NULL, tskIDLE_PRIORITY+1, NULL);
}

struct conn_info **control_get_clients(void) {
    static struct conn_info *clients[conn_infos_len + 1];
    uint64_t now = xTaskGetTickCount() * portTICK_PERIOD_MS;

    size_t c = 0;
    for(size_t i = 0; i < conn_infos_len; i++) {
        if(now - conn_infos[i].last_recv_time > CONFIG_CONNECTION_TIMEOUT) {
            conn_infos[i].state = DISCONNECTED;
        } else if(conn_infos[i].state == CONNECTED) {
            clients[c++] = &conn_infos[i];
        }
    }
    clients[c] = NULL;
    return clients;
}
