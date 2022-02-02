#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/event_groups.h>

#include <lwip/api.h>
#include <lwip/pbuf.h>

#include "car.h"
#include "sdkconfig.h"

enum conn_state {
    DISCONNECTED = 0,
    CONNECTED,
};

struct conn {
    struct netconn *netconn;
    enum conn_state state;
} connections[CONFIG_MAX_CONNECTION] = {0};
#define connections_len (sizeof(connections) / sizeof(connections[0]))

static EventGroupHandle_t conn_event_bits;

static struct conn *find_free_conn(void) {
    for(unsigned i = 0; i < connections_len; i++) {
        if(connections[i].state == DISCONNECTED) {
            return &connections[i];
        }
    }
    return NULL;
}

static int16_t bytes_to_int16(uint8_t *data) {
    return ntohs(*((uint16_t*) data));
}

static void control_server(void *params) {
    struct netconn *server_conn = netconn_new(NETCONN_TCP);
    assert(server_conn != NULL);

    netconn_bind(server_conn, IP4_ADDR_ANY, 3232);
    netconn_listen(server_conn);

    while(1) {
        struct netconn *netconn;
        netconn_accept(server_conn, &netconn);

        struct conn *conn = find_free_conn();
        if(!conn) {
            netconn_close(netconn);
            continue;
        }

        conn->netconn = netconn;
        conn->state = CONNECTED;
        xEventGroupSetBits(conn_event_bits, 1);
    }
}

static void control_handler(void *params) {
    while(1) {
        xEventGroupWaitBits(conn_event_bits, 1, false, false, portMAX_DELAY);

        int conn_count = 0;
        for(unsigned i = 0; i < connections_len; i++) {
            if(connections[i].state != CONNECTED) continue;
            struct netconn *conn = connections[i].netconn;

            struct pbuf *buff = NULL;
            err_t err = netconn_recv_tcp_pbuf_flags(conn, &buff, NETCONN_DONTBLOCK);

            if(err == ERR_WOULDBLOCK) {
                continue;
            }

            if(err != ERR_OK) {
                netconn_close(conn);
                connections[i].netconn = NULL;
                connections[i].state = DISCONNECTED;
                continue;
            }

            conn_count++;

            for(size_t i = 0; buff && i < buff->len; i++) {
                uint8_t *data = buff->payload + i;
                size_t len = buff->len - i;
                switch(data[0]) {
                    case 'M':
                        if(len < 5) break;
                        motor_move(bytes_to_int16(data + 1), bytes_to_int16(data + sizeof(int16_t) + 1));
                        i += 4;
                        break;
                    default:
                        break;
                }
            }

            pbuf_free(buff);
        }

        if(conn_count == 0) {
            xEventGroupClearBits(conn_event_bits, 1);
        }
    }
}

void control_init(void) {
    conn_event_bits = xEventGroupCreate();
    xTaskCreate(control_server, "control server", 8192, NULL, tskIDLE_PRIORITY+1, NULL);
    xTaskCreate(control_handler, "control handler", 8192, NULL, tskIDLE_PRIORITY+2, NULL);
}
