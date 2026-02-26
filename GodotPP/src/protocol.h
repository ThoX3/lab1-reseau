#ifndef PROTOCOL_H
#define PROTOCOL_H
#include <stdint.h>

#pragma pack(push, 1)

struct SpawnPacket {
    uint8_t  packet_type;
    uint32_t net_id;
    uint32_t type_id;
    float    x;
    float    y;
};

struct DestroyPacket {
    uint8_t  packet_type = 2;
    uint32_t net_id;
};

struct DisconnectPacket {
    uint8_t packet_type = 3;
};

#pragma pack(pop)
#endif