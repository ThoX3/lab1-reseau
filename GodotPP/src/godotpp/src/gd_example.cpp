#include "gd_example.h"
#include "../../protocol.h"
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/classes/resource_loader.hpp>
#include <godot_cpp/classes/texture2d.hpp>

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

    uint8_t buf[1024];
    char sender[64];
    
    while (true) {
        int bytes = net_socket_poll(network_socket, buf, 1024, sender, 64);
        if (bytes <= 0) break; 

        uint8_t packet_type = buf[0];

        if (packet_type == 1 && bytes >= (int)sizeof(SpawnPacket)) {
            SpawnPacket* p = (SpawnPacket*)buf;
            
            if (network_to_local.find(p->net_id) == network_to_local.end()) {
                if (type_registry.count(p->type_id)) {
                    Node2D* new_node = type_registry[p->type_id]();
                    add_child(new_node);
                    new_node->set_position(Vector2(p->x, p->y));
                    new_node->set_name("NetEntity_" + String::num_int64(p->net_id));
                    
                    network_to_local[p->net_id] = new_node;
                    UtilityFunctions::print("[Client] SPAWN REUSSI - ID: ", p->net_id);
                }
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