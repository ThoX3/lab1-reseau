#ifndef GDEXAMPLE_H
#define GDEXAMPLE_H

#include <godot_cpp/classes/sprite2d.hpp>
#include <godot_cpp/classes/node2d.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <map>
#include <functional>

extern "C" {
    void* net_socket_create(const char* bind_addr);
    void net_socket_destroy(void* socket_ptr);
    int32_t net_socket_poll(void* socket_ptr, uint8_t* out_data, size_t max_len, char* out_sender, size_t sender_max_len);
    int32_t net_socket_send(void* socket, const char* address, const uint8_t* data, size_t len);
}

namespace godot {

class GDExample : public Node2D {
    GDCLASS(GDExample, Node2D)

private:
    double time_passed;
    void* network_socket;

    std::map<uint32_t, Node2D*> network_to_local;

    using CreatorLambda = std::function<Node2D*()>;
    std::map<uint32_t, CreatorLambda> type_registry;

protected:
    static void _bind_methods();

public:
    GDExample();
    ~GDExample(); 

    void _process(double delta) override; 
    void _physics_process(double delta) override; 
    
    void register_entity_types();
};

}

#endif