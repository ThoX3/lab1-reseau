#include <iostream>
#include <vector>
#include <string>
#include <set>
#include <map>
#include <algorithm>
#include <cstddef>
#include <chrono>
#include "../../protocol.h"
#include "../../openssl/include/crypto_utils.h"

// Clé partagée pour le chiffrement AES
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

    // Variables pour l'Anti-Cheat
    std::chrono::steady_clock::time_point last_reset_time = std::chrono::steady_clock::now();
    int inputs_this_second = 0;
    int suspicion_score = 0;
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

// Fonction utilitaire pour envoyer un paquet chiffré
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

    std::cout << "[Server] En attente de joueurs sur le port 12345..." << std::endl;

    while (true) {
        uint8_t buffer[1024];
        char sender[64];
        int bytes = net_socket_poll(socket, buffer, 1024, sender, 64);

        if (bytes > 0) {
            std::string sender_addr(sender);
            uint8_t packet_type = buffer[0];

            // =========================================================
            // CAS 1 : NOUVEAU CLIENT (Connexion)
            // =========================================================
            if (clients.find(sender_addr) == clients.end()) {
                clients.insert(sender_addr);
                std::cout << "[Server] Nouveau Client: " << sender_addr << std::endl;

                // Envoi de l'historique au nouveau
                for (const auto& ent : active_entities) {
                    SpawnPacket p = {1, ent.net_id, ent.type_id, ent.x, ent.y};
                    send_encrypted_packet(socket, sender_addr.c_str(), (uint8_t*)&p, sizeof(p));
                }

                // Création du joueur
                ServerEntity new_ent;
                new_ent.net_id = next_id++;
                new_ent.type_id = 1;
                new_ent.address = sender_addr;
                new_ent.x = 100.0f + (active_entities.size() * 150.0f);
                new_ent.y = 200.0f;

                active_entities.push_back(new_ent);
                ip_to_id[sender_addr] = new_ent.net_id;

                // Broadcast à tout le monde
                SpawnPacket p = {1, new_ent.net_id, new_ent.type_id, new_ent.x, new_ent.y};
                for (const auto& client : clients) {
                    send_encrypted_packet(socket, client.c_str(), (uint8_t*)&p, sizeof(p));
                }

                std::cout << "[Server] Spawned ID " << p.net_id << std::endl;
            }

            // =========================================================
            // CAS 2 : DÉCONNEXION (Type 3)
            // =========================================================
            else if (packet_type == 3) {
                if (ip_to_id.count(sender_addr)) {
                    uint32_t id_to_remove = ip_to_id[sender_addr];

                    std::cout << "[Server] Deconnexion de " << sender_addr << " (ID " << id_to_remove << ")" << std::endl;

                    ip_to_id.erase(sender_addr);
                    clients.erase(sender_addr);

                    auto it = std::remove_if(active_entities.begin(), active_entities.end(),
                        [id_to_remove](const ServerEntity& e) { return e.net_id == id_to_remove; });
                    active_entities.erase(it, active_entities.end());

                    // Broadcast destruction
                    DestroyPacket p = {2, id_to_remove};
                    for (const auto& client : clients) {
                        send_encrypted_packet(socket, client.c_str(), (uint8_t*)&p, sizeof(p));
                    }
                }
            }

            // =========================================================
            // CAS 3 : PAQUET CHIFFRÉ (Mouvements, etc.)
            // =========================================================
            else if (packet_type == 7 && bytes >= (int)offsetof(EncryptedPacket, payload)) {
                EncryptedPacket* secure_pkt = (EncryptedPacket*)buffer;
                uint8_t decrypted_payload[1024];

                // Sécurité anti-crash
                if (secure_pkt->payload_length < 0 || secure_pkt->payload_length > 1024) {
                    continue;
                }

                // Déchiffrement AES
                int decrypted_len = decrypt_data(
                    secure_pkt->payload, secure_pkt->payload_length,
                    secure_pkt->tag, secure_pkt->nonce,
                    TEST_SECRET_KEY,
                    decrypted_payload
                );

                if (decrypted_len > 0) {
                    uint8_t real_packet_type = decrypted_payload[0];

                    // --- TRAITEMENT DES INPUTS ET ANTI-CHEAT ---
                    if (real_packet_type == 4 && decrypted_len >= (int)sizeof(InputHistoryPacket)) {
                        InputHistoryPacket* pkt = (InputHistoryPacket*)decrypted_payload;

                        if (ip_to_id.count(sender_addr)) {
                            uint32_t target_id = ip_to_id[sender_addr];

                            for (auto& ent : active_entities) {
                                if (ent.net_id == target_id) {

                                    // 1. CHRONOMÈTRE
                                    auto now = std::chrono::steady_clock::now();
                                    auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - ent.last_reset_time).count();

                                    // Toutes les secondes réelles, on remet à zéro
                                    if (elapsed >= 1) {
                                        ent.inputs_this_second = 0;
                                        ent.last_reset_time = now;

                                        // Le score de suspicion baisse naturellement avec le temps
                                        if (ent.suspicion_score > 0) ent.suspicion_score -= 10;
                                        if (ent.suspicion_score < 0) ent.suspicion_score = 0;
                                    }

                                    // 2. COMPTAGE DES INPUTS
                                    int new_inputs = 0;
                                    for (int i = 19; i >= 0; --i) {
                                        if (pkt->history[i].sequence > ent.last_received_sequence) {
                                            new_inputs++;
                                        }
                                    }
                                    ent.inputs_this_second += new_inputs;

                                    // 3. TRIBUNAL ANTI-CHEAT
                                    if (ent.inputs_this_second > 70) {
                                        ent.suspicion_score += 20;
                                        std::cout << "[Anti-Cheat] Abus detecte ! Inputs/sec : " << ent.inputs_this_second << " | Score : " << ent.suspicion_score << "/100" << std::endl;
                                    }

                                    if (ent.suspicion_score > 100) {
                                        ent.suspicion_score = 100;
                                    }

                                    // 4. SANCTION OU APPLICATION
                                    if (ent.suspicion_score < 100) {
                                        // Joueur réglo : on applique les mouvements
                                        for (int i = 19; i >= 0; --i) {
                                            if (pkt->history[i].sequence > ent.last_received_sequence) {
                                                if (pkt->history[i].keys & (1 << 0)) ent.y -= 10.0f; // Haut
                                                if (pkt->history[i].keys & (1 << 1)) ent.y += 10.0f; // Bas
                                                if (pkt->history[i].keys & (1 << 2)) ent.x -= 10.0f; // Gauche
                                                if (pkt->history[i].keys & (1 << 3)) ent.x += 10.0f; // Droite
                                                ent.last_received_sequence = pkt->history[i].sequence;
                                            }
                                        }
                                    } else {
                                        // Hacker : on ignore ses touches, mais on met à jour la séquence
                                        // pour vider son paquet frauduleux et le bloquer sur place
                                        ent.last_received_sequence = pkt->history[0].sequence;
                                        std::cout << "[Anti-Cheat] ALERTE ! Joueur " << ent.net_id << " bloque sur place !" << std::endl;
                                    }

                                    // 5. BROADCAST DE LA POSITION
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
                    // Si le déchiffrement échoue (mauvaise clé ou paquet corrompu)
                    std::cout << "[Sécurité] ALERTE : La signature AES a été rejetée !" << std::endl;
                }
            }
        }
    }
    return 0;
}
