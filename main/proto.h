#pragma once

#define MAX_PACKET_SIZE 400

enum frametype {
   FRAMETYPE_VIDEO = 0x0,
   FRAMETYPE_AUDIO = 0x1,
} __attribute__((__packed__));

#define HEADER_SIZE sizeof(struct frameheader)
struct frameheader {
   enum frametype type;
   uint16_t seq;
   uint8_t rsd;
} __attribute__((packed));
