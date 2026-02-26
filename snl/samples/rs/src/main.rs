use snl::GameSocket;
use std::{thread, time::Duration};

fn main() {
    println!("[Rust] Starting UDP Client...");

    // Create the socket
    let socket = match GameSocket::new("127.0.0.1:8080") {
        Ok(s) => s,
        Err(e) => {
            eprintln!("[Rust] Failed to bind socket: {}", e);
            return;
        }
    };
    
    // Send a "Hello" packet to the "server"
    let target_address = "127.0.0.1:8080";
    let message = b"Hello from Rust!";

    match socket.send(target_address, message) {
        Ok(_) => println!("[Rust] Sent packet to {}", target_address),
        Err(e) => eprintln!("[Rust] Failed to send packet: {}", e),
    }

    // The Game Loop (Polling)
    println!("[Rust] Entering game loop (listening for 5 seconds)...");
    
    let mut buffer = [0u8; 1024]; // Allocate buffer on the stack
    
    // Run for ~5 seconds (50 iterations * 100ms)
    for _ in 0..50 {
        // Poll for new data
        match socket.poll(&mut buffer) {
            Some((size, sender)) => {
                // Convert bytes to string (lossy handles invalid UTF-8 gracefully)
                let msg_str = String::from_utf8_lossy(&buffer[..size]);
                println!("[Rust] Received {} bytes from {}: {}", size, sender, msg_str);
            },
            None => {
                // No data this frame
            }
        }

        // Simulate a 60 FPS frame time (approx 16ms)
        thread::sleep(Duration::from_millis(16));
    }

    println!("[Rust] Finished.");
}