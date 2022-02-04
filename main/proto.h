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

#define COMMAND_SIZE sizeof(struct command)
struct command {
   char type;
   union {
      uint8_t data[4];
      struct {
         uint16_t x;
         uint16_t y;
      };
   };
} __attribute__((packed));
