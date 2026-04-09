#include "gd_example.h"
#include "../../protocol.h"
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/classes/resource_loader.hpp>
#include <godot_cpp/classes/texture2d.hpp>
#include <godot_cpp/classes/input.hpp>
#include <godot_cpp/classes/input_event_key.hpp>
#include "../../openssl/include/crypto_utils.h"

const uint8_t TEST_SECRET_KEY[32] = {
    1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16,
    17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32
};

using namespace godot;

void GDExample::_bind_methods() {}

GDExample::GDExample() {
    time_passed = 0.0;
    network_socket = net_socket_create("127.0.0.1:0");
    register_entity_types();

    if (network_socket) {
        UtilityFunctions::print("[Client] Sending HELLO...");
        const char* hello = "HELLO";
        net_socket_send(network_socket, "127.0.0.1:12345", (uint8_t*)hello, 5);
    }
}

GDExample::~GDExample() {
    if (network_socket) {
        DisconnectPacket p;
        p.packet_type = 3;
        net_socket_send(network_socket, "127.0.0.1:12345", (uint8_t*)&p, sizeof(p));
        
        net_socket_destroy(network_socket);
    }
}

void GDExample::register_entity_types() {
    type_registry[1] = []() -> Node2D* {
        Sprite2D* s = memnew(Sprite2D);
        
        Ref<Texture2D> tex = ResourceLoader::get_singleton()->load("res://icon.svg");
        
        if (tex.is_valid()) {
            s->set_texture(tex);
        } else {
            UtilityFunctions::print("[Client] ERREUR: Impossible de charger icon.svg !");
        }
        
        return s;
    };
}

void GDExample::_process(double delta) {
    time_passed += delta;
}

void GDExample::_physics_process(double delta) {
    if (!network_socket) return;

    Input* input_singleton = Input::get_singleton();
    
    // 1. Initialisation COMPLÈTE du paquet courant
    InputPacket current_input;
    current_input.sequence = current_sequence++;
    current_input.keys = 0;

    // 2. Détection des touches
    if (input_singleton->is_key_pressed(KEY_UP) || input_singleton->is_key_pressed(KEY_W)) {
        current_input.keys |= (1 << 0);
    }
    if (input_singleton->is_key_pressed(KEY_DOWN) || input_singleton->is_key_pressed(KEY_S)) {
        current_input.keys |= (1 << 1);
    }
    if (input_singleton->is_key_pressed(KEY_LEFT) || input_singleton->is_key_pressed(KEY_A)) {
        current_input.keys |= (1 << 2);
    }
    if (input_singleton->is_key_pressed(KEY_RIGHT) || input_singleton->is_key_pressed(KEY_D)) {
        current_input.keys |= (1 << 3);
    }

    // 3. Gestion de l'historique
    for (int i = 19; i > 0; --i) {
        input_history.history[i] = input_history.history[i - 1];
    }
    input_history.history[0] = current_input;
    
    input_history.packet_type = 4;

    // 4. Envoi
    EncryptedPacket secure_pkt;
    secure_pkt.packet_type = 7; 

    secure_pkt.payload_length = encrypt_data(
        (uint8_t*)&input_history, sizeof(InputHistoryPacket), 
        TEST_SECRET_KEY, 
        secure_pkt.nonce, 
        secure_pkt.payload, 
        secure_pkt.tag
    );

    if (secure_pkt.payload_length > 0) {
        size_t packet_size = sizeof(uint8_t) + 12 + 16 + sizeof(int32_t) + secure_pkt.payload_length;
        net_socket_send(network_socket, "127.0.0.1:12345", (uint8_t*)&secure_pkt, packet_size);
    } else {
        UtilityFunctions::print("[Client] ERREUR : Le chiffrement a échoué !");
    }

    uint8_t buf[1024];
    char sender[64];
    
    while (true) {
        int bytes = net_socket_poll(network_socket, buf, 1024, sender, 64);
        if (bytes <= 0) break; 

        uint8_t packet_type = buf[0];
        UtilityFunctions::print("[Client DEBUG] Paquet recu : ", bytes, " octets | Type brut : ", packet_type);

        // --- DÉCHIFFREMENT DU SERVEUR ---
        if (packet_type == 7 && bytes >= (int)offsetof(EncryptedPacket, payload)) {
            EncryptedPacket* received_secure_pkt = (EncryptedPacket*)buf;
            uint8_t decrypted_payload[1024];

            int decrypted_len = decrypt_data(
                received_secure_pkt->payload, received_secure_pkt->payload_length,
                received_secure_pkt->tag, received_secure_pkt->nonce,
                TEST_SECRET_KEY,
                decrypted_payload
            );

            UtilityFunctions::print("[Client DEBUG] decrypt_data a renvoye : ", decrypted_len);

            if (decrypted_len > 0) {
                packet_type = decrypted_payload[0];
                memcpy(buf, decrypted_payload, decrypted_len);
                bytes = decrypted_len; 
                UtilityFunctions::print("[Client DEBUG] Succes ! Vrai type : ", packet_type, " | Vraie taille : ", bytes);
            } else {
                UtilityFunctions::print("[Client ERREUR] Signature invalide, hacker detecte !");
                continue; 
            }
        }

        // --- TRAITEMENT DES PAQUETS EN CLAIR ---
        if (packet_type == 1) {
            UtilityFunctions::print("[Client DEBUG] Type 1 detecte. Taille attendue : ", (int)sizeof(SpawnPacket), " | Actuelle : ", bytes);
            
            if (bytes >= (int)sizeof(SpawnPacket)) {
                SpawnPacket* p = (SpawnPacket*)buf;
                UtilityFunctions::print("[Client DEBUG] Tentative de spawn pour l'ID : ", p->net_id);
                
                if (network_to_local.find(p->net_id) == network_to_local.end()) {
                    if (type_registry.count(p->type_id)) {
                        Node2D* new_node = type_registry[p->type_id]();
                        add_child(new_node);
                        new_node->set_position(Vector2(p->x, p->y));
                        new_node->set_name("NetEntity_" + String::num_int64(p->net_id));
                        
                        network_to_local[p->net_id] = new_node;
                        UtilityFunctions::print("[Client] SPAWN TOTALEMENT REUSSI - ID: ", p->net_id);
                    } else {
                        UtilityFunctions::print("[Client ERREUR] type_id inconnu : ", p->type_id);
                    }
                }

                if (network_to_local.count(p->net_id)) {
                    network_to_local[p->net_id]->set_position(Vector2(p->x, p->y));
                }
            } else {
                UtilityFunctions::print("[Client ERREUR] Le paquet est trop petit pour etre un SpawnPacket !");
            }
        }
        
        else if (packet_type == 2 && bytes >= (int)sizeof(DestroyPacket)) {
            DestroyPacket* p = (DestroyPacket*)buf;
            if (network_to_local.count(p->net_id)) {
                Node2D* node_to_delete = network_to_local[p->net_id];
                node_to_delete->queue_free();
                network_to_local.erase(p->net_id);
                UtilityFunctions::print("[Client] DESTROY - ID: ", p->net_id);
            }
        }
    }
}