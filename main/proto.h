#pragma once

#define MAX_PACKET_SIZE 400

enum frametype {
   FRAMETYPE_VIDEO  = 'V',
   FRAMETYPE_AUDIO  = 'A',
   FRAMETYPE_STATUS = 'S',
} __attribute__((__packed__));

enum commandtype {
   MOVE       = 'M',
   DISCONNECT = 'X',
} __attribute__((__packed__));

#define HEADER_SIZE sizeof(struct frameheader)
struct frameheader {
   uint8_t size;
   enum frametype type;
} __attribute__((packed));

#define STATUS_SIZE sizeof(struct status)
struct status {
   struct frameheader header;
   int16_t x; // big endian
   int16_t y; // big endian
} __attribute__((packed));

#define COMMAND_SIZE sizeof(struct command)
struct command {
   enum commandtype type;
   union {
      uint8_t data[4];
      struct {
         uint16_t x; // big endian
         uint16_t y; // big endian
      };
   };
} __attribute__((packed));

enum conn_state {
    DISCONNECTED = 0,
    CONNECTED,
};

#define conn_infos_len (sizeof(conn_infos) / sizeof(conn_infos[0]))
struct conn_info {
    enum conn_state state;
    ip_addr_t addr;
    uint32_t port;
    uint64_t last_recv_time;
};
