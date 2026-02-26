#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <cstring>
#include "snl.h"

int main() {
    std::cout << "[C++] Starting UDP Socket..." << std::endl;

    // Create the socket (Bind to port 8080)
    // In a real game, client might bind to "0.0.0.0:0" to get a random port
    GameSocket* sock = net_socket_create("127.0.0.1:8080");
    
    if (!sock) {
        std::cerr << "Failed to bind socket!" << std::endl;
        return -1;
    }

    std::cout << "[C++] Socket bound to 127.0.0.1:8080" << std::endl;

    // Send a "Hello" packet to ourselves
    const char* target = "127.0.0.1:8080";
    const char* msg = "Hello from C++!";
    net_socket_send(sock, target, (const uint8_t*)msg, strlen(msg));

    // The Game Loop
    char sender_buf[64];
    uint8_t data_buf[1024];

    for (int i = 0; i < 5; i++) {
        // Poll for data
        int bytes = net_socket_poll(sock, data_buf, 1024, sender_buf, 64);

        if (bytes > 0) {
            // Null-terminate manually just to be safe for printing
            data_buf[bytes] = '\0'; 
            std::cout << "[C++] Received " << bytes << " bytes from " << sender_buf 
                      << ": " << (char*)data_buf << std::endl;
        } else {
            std::cout << "." << std::flush;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    // Cleanup
    std::cout << "\n[C++] Destroying socket." << std::endl;
    net_socket_destroy(sock);

    return 0;
}