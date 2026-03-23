#include <iostream>
#include <vector>
#include <string>
#include <set>
#include <map>
#include <algorithm> 
#include <cstddef>
#include "../../protocol.h"
#include "../../../openssl/include/crypto_utils.h"

const uint8_t TEST_SECRET_KEY[32] = {
    1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16,
    17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32
};

extern "C" {
    void* net_socket_create(const char* bind_addr);
    int32_t net_socket_poll(void* socket, uint8_t* out_data, size_t max_len, char* out_sender, size_t sender_max_len);
    int32_t net_socket_send(void* socket, const char* address, const uint8_t* data, size_t len);
}

struct ServerEntity {
    uint32_t net_id;
    uint32_t type_id;
    float x, y;
    uint32_t last_received_sequence = 0;
    std::string address;
};

enum class ConnectionState {
    HANDSHAKING, 
    CONNECTED    
};

struct ClientSession {
    ConnectionState state;
    uint8_t shared_aes_key[32];
    uint32_t net_id; 
};

std::map<std::string, ClientSession> active_sessions;

void send_encrypted_packet(void* socket, const char* address, const uint8_t* data, size_t len) {
    EncryptedPacket secure_pkt;
    secure_pkt.packet_type = 7;
    
    secure_pkt.payload_length = encrypt_data(
        data, len,
        TEST_SECRET_KEY,
        secure_pkt.nonce,
        secure_pkt.payload,
        secure_pkt.tag
    );

    if (secure_pkt.payload_length > 0) {
        size_t total_size = sizeof(uint8_t) + 12 + 16 + sizeof(int32_t) + secure_pkt.payload_length;
        net_socket_send(socket, address, (uint8_t*)&secure_pkt, total_size);
    } else {
        std::cerr << "[Serveur] Erreur lors du chiffrement pour " << address << std::endl;
    }
}

int main() {
    void* socket = net_socket_create("0.0.0.0:12345");
    if (!socket) return -1;

    uint32_t next_id = 100;
    
    std::set<std::string> clients;
    std::vector<ServerEntity> active_entities;
    std::map<std::string, uint32_t> ip_to_id;

    std::cout << "[Server] Waiting for clients..." << std::endl;

    while (true) {
        uint8_t buffer[1024];
        char sender[64];
        int bytes = net_socket_poll(socket, buffer, 1024, sender, 64);

        if (bytes > 0) {
            std::string sender_addr(sender);
            uint8_t packet_type = buffer[0];

            if (clients.find(sender_addr) == clients.end()) {
                clients.insert(sender_addr);
                std::cout << "[Server] New Client: " << sender_addr << std::endl;

                for (const auto& ent : active_entities) {
                    SpawnPacket p = {1, ent.net_id, ent.type_id, ent.x, ent.y};
                    send_encrypted_packet(socket, sender_addr.c_str(), (uint8_t*)&p, sizeof(p));
                }

                ServerEntity new_ent;
                new_ent.net_id = next_id++;
                new_ent.type_id = 1; 
                new_ent.address = sender_addr;
                new_ent.x = 100.0f + (active_entities.size() * 150.0f);
                new_ent.y = 200.0f;
                
                active_entities.push_back(new_ent); 
                
                ip_to_id[sender_addr] = new_ent.net_id;

                SpawnPacket p = {1, new_ent.net_id, new_ent.type_id, new_ent.x, new_ent.y};
                for (const auto& client : clients) {
                    send_encrypted_packet(socket, client.c_str(), (uint8_t*)&p, sizeof(p));           
                }
                
                std::cout << "[Server] Spawned ID " << p.net_id << std::endl;
            }
            
            else if (packet_type == 3) {
                if (ip_to_id.count(sender_addr)) {
                    uint32_t id_to_remove = ip_to_id[sender_addr];
                    
                    std::cout << "[Server] Disconnect from " << sender_addr << " (ID " << id_to_remove << ")" << std::endl;

                    ip_to_id.erase(sender_addr);
                    
                    clients.erase(sender_addr);

                    auto it = std::remove_if(active_entities.begin(), active_entities.end(), 
                        [id_to_remove](const ServerEntity& e) { return e.net_id == id_to_remove; });
                    active_entities.erase(it, active_entities.end());

                    DestroyPacket p = {2, id_to_remove};
                    for (const auto& client : clients) {
                        send_encrypted_packet(socket, client.c_str(), (uint8_t*)&p, sizeof(p));            
                    }
                }
            }
            else if (packet_type == 4 && bytes >= (int)sizeof(InputHistoryPacket)) {
                InputHistoryPacket* pkt = (InputHistoryPacket*)buffer;
                std::string sender_addr(sender);

                if (ip_to_id.count(sender_addr)) {
                    uint32_t target_id = ip_to_id[sender_addr];

                    for (auto& ent : active_entities) {
                        if (ent.net_id == target_id) {
                            
                            for (int i = 19; i >= 0; --i) {
                                if (pkt->history[i].sequence > ent.last_received_sequence) {
                                    if (pkt->history[i].keys & (1 << 0)) ent.y -= 10.0f; // Haut
                                    if (pkt->history[i].keys & (1 << 1)) ent.y += 10.0f; // Bas
                                    if (pkt->history[i].keys & (1 << 2)) ent.x -= 10.0f; // Gauche
                                    if (pkt->history[i].keys & (1 << 3)) ent.x += 10.0f; // Droite
                                    ent.last_received_sequence = pkt->history[i].sequence;
                                }
                            }

                            SpawnPacket update = {1, ent.net_id, ent.type_id, ent.x, ent.y};
                            for (const auto& client : clients) {
                                send_encrypted_packet(socket, client.c_str(), (uint8_t*)&update, sizeof(update));
                            }
                            break;
                        }
                    }
                }
            } else if (packet_type == 7 && bytes >= (int)offsetof(EncryptedPacket, payload)) {
                EncryptedPacket* secure_pkt = (EncryptedPacket*)buffer;
                uint8_t decrypted_payload[1024];

                // --- LES MOUCHARDS DE DEBUG ---
                std::cout << "[DEBUG] --- NOUVEAU PAQUET TYPE 7 ---" << std::endl;
                std::cout << "[DEBUG] Taille totale reçue sur le réseau : " << bytes << " octets" << std::endl;
                std::cout << "[DEBUG] Payload_length déclaré par le client : " << secure_pkt->payload_length << " octets" << std::endl;

                // Sécurité anti-crash au cas où la taille serait absurde
                if (secure_pkt->payload_length < 0 || secure_pkt->payload_length > 1024) {
                    std::cout << "[DEBUG] ERREUR : Payload length aberrant !" << std::endl;
                    continue; 
                }

                int decrypted_len = decrypt_data(
                    secure_pkt->payload, secure_pkt->payload_length,
                    secure_pkt->tag, secure_pkt->nonce,
                    TEST_SECRET_KEY,
                    decrypted_payload
                );

                std::cout << "[DEBUG] Résultat de decrypt_data : " << decrypted_len << std::endl;

                if (decrypted_len > 0) {
                    uint8_t real_packet_type = decrypted_payload[0];
                    std::cout << "[DEBUG] SUCCÈS ! Vrai type caché : " << (int)real_packet_type << std::endl;
                    
                    if (real_packet_type == 4 && decrypted_len >= (int)sizeof(InputHistoryPacket)) {
                        InputHistoryPacket* pkt = (InputHistoryPacket*)decrypted_payload;

                        if (ip_to_id.count(sender_addr)) {
                            uint32_t target_id = ip_to_id[sender_addr];

                            for (auto& ent : active_entities) {
                                if (ent.net_id == target_id) {
                                    for (int i = 19; i >= 0; --i) {
                                        if (pkt->history[i].sequence > ent.last_received_sequence) {
                                            if (pkt->history[i].keys & (1 << 0)) ent.y -= 10.0f;
                                            if (pkt->history[i].keys & (1 << 1)) ent.y += 10.0f;
                                            if (pkt->history[i].keys & (1 << 2)) ent.x -= 10.0f;
                                            if (pkt->history[i].keys & (1 << 3)) ent.x += 10.0f;
                                            ent.last_received_sequence = pkt->history[i].sequence;
                                        }
                                    }

                                    SpawnPacket update = {1, ent.net_id, ent.type_id, ent.x, ent.y};
                                    for (const auto& client : clients) {
                                        send_encrypted_packet(socket, client.c_str(), (uint8_t*)&update, sizeof(update)); 
                                    }
                                    break;
                                }
                            }
                        }
                    }
                } else {
                    std::cout << "[Sécurité] ALERTE : La signature AES a été rejetée !" << std::endl;
                }
            }
        }
    }
    return 0;
}