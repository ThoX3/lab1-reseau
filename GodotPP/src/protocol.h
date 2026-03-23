#ifndef PROTOCOL_H
#define PROTOCOL_H
#include <stdint.h>

#pragma pack(push, 1)

struct SpawnPacket {
    uint8_t  packet_type = 1;
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

struct InputPacket {
    uint32_t sequence;
    uint8_t keys;  
};

struct InputHistoryPacket {
    uint8_t packet_type = 4;
    InputPacket history[20]; 
};

// Cybersécurité : Ces paquets sont destinés à l'authentification et à l'établissement d'une connexion sécurisée entre le client et le serveur. 
// Ils font partie d'un protocole de handshake qui utilise une cryptographie à clé publique pour échanger des clés de session de manière sécurisée.

struct ClientHelloPacket {
    uint8_t packet_type = 5;
    uint8_t public_key[32]; 
};

struct ServerHelloPacket {
    uint8_t packet_type = 6;
    uint8_t public_key[32]; // Clé publique X25519 (ECDH) du serveur
};

struct EncryptedPacket {
    uint8_t packet_type = 7;
    uint8_t nonce[12];     
    uint8_t tag[16];        
    int32_t payload_length;
    uint8_t payload[1024];  
};

#pragma pack(pop)
#endif