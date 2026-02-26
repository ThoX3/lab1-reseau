#include <iostream>
#include <vector>
#include <string>
#include <set>
#include <map>
#include <algorithm> 
#include "../../protocol.h"

extern "C" {
    void* net_socket_create(const char* bind_addr);
    int32_t net_socket_poll(void* socket, uint8_t* out_data, size_t max_len, char* out_sender, size_t sender_max_len);
    int32_t net_socket_send(void* socket, const char* address, const uint8_t* data, size_t len);
}

struct ServerEntity {
    uint32_t net_id;
    uint32_t type_id;
    float x, y;
};

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
                    net_socket_send(socket, sender_addr.c_str(), (uint8_t*)&p, sizeof(p));
                }

                ServerEntity new_ent;
                new_ent.net_id = next_id++;
                new_ent.type_id = 1; 
                new_ent.x = 100.0f + (active_entities.size() * 150.0f);
                new_ent.y = 200.0f;
                
                active_entities.push_back(new_ent); 
                
                ip_to_id[sender_addr] = new_ent.net_id;

                SpawnPacket p = {1, new_ent.net_id, new_ent.type_id, new_ent.x, new_ent.y};
                for (const auto& client : clients) {
                    net_socket_send(socket, client.c_str(), (uint8_t*)&p, sizeof(p));
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
                        net_socket_send(socket, client.c_str(), (uint8_t*)&p, sizeof(p));
                    }
                }
            }
        }
    }
    return 0;
}