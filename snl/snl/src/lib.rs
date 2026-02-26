use std::ffi::{c_char, CStr, CString};
use std::net::UdpSocket;
use std::ptr;

pub const VERSION_STR: &str = "Stupid Network Library v0.1.0";

pub fn get_version_rust() -> &'static str {
    VERSION_STR
}

const VERSION_C: &[u8] = b"Stupid Network Library v0.1.0\0";

#[unsafe(no_mangle)]
pub extern "C" fn net_get_version() -> *const c_char {
    VERSION_C.as_ptr() as *const c_char
}

pub struct GameSocket {
    socket: UdpSocket,
}

impl GameSocket {
    pub fn new(address: &str) -> Result<Self, String> {
        let socket = UdpSocket::bind(address).map_err(|e| e.to_string())?;
        socket.set_nonblocking(true).map_err(|e| e.to_string())?;

        Ok(Self { socket })
    }

    pub fn send(&self, target: &str, data: &[u8]) -> Result<usize, String> {
        self.socket.send_to(data, target).map_err(|e| e.to_string())
    }

    pub fn poll(&self, buf: &mut [u8]) -> Option<(usize, String)> {
        match self.socket.recv_from(buf) {
            Ok((size, src)) => Some((size, src.to_string())),
            Err(ref e) if e.kind() == std::io::ErrorKind::WouldBlock => None,
            Err(_) => None,
        }
    }
}

#[unsafe(no_mangle)]
pub extern "C" fn net_socket_create(bind_addr: *const c_char) -> *mut GameSocket {
    if bind_addr.is_null() { return ptr::null_mut(); }

    let c_str = unsafe { CStr::from_ptr(bind_addr) };
    let addr_str = match c_str.to_str() {
        Ok(s) => s,
        Err(_) => return ptr::null_mut(),
    };

    match GameSocket::new(addr_str) {
        Ok(s) => Box::into_raw(Box::new(s)), // Move to heap, return pointer
        Err(_) => ptr::null_mut(),
    }
}

#[unsafe(no_mangle)]
pub extern "C" fn net_socket_destroy(socket_ptr: *mut GameSocket) {
    if !socket_ptr.is_null() {
        unsafe { drop(Box::from_raw(socket_ptr)) };
    }
}

#[unsafe(no_mangle)]
pub extern "C" fn net_socket_send(
    socket_ptr: *mut GameSocket,
    address: *const c_char,
    data: *const u8,
    len: usize
) -> i32 {
    let socket = unsafe {
        if socket_ptr.is_null() { return -1; }
        &*socket_ptr
    };

    let c_str = unsafe { CStr::from_ptr(address) };
    let addr_str = match c_str.to_str() {
        Ok(s) => s,
        Err(_) => return -1,
    };

    let payload = unsafe { std::slice::from_raw_parts(data, len) };

    match socket.send(addr_str, payload) {
        Ok(sent) => sent as i32,
        Err(_) => -1,
    }
}

// Poll Data (The "Game Loop" function)
// Returns: bytes read, or 0 if nothing, -1 if error.
// Fills 'out_sender' with the "IP:Port" string of who sent it.
#[unsafe(no_mangle)]
pub extern "C" fn net_socket_poll(
    socket_ptr: *mut GameSocket,
    out_data: *mut u8,
    max_len: usize,
    out_sender: *mut c_char,
    sender_max_len: usize
) -> i32 {
    let socket = unsafe {
        if socket_ptr.is_null() { return -1; }
        &*socket_ptr
    };

    // Create a temporary slice to write directly into C++ memory
    let buf = unsafe { std::slice::from_raw_parts_mut(out_data, max_len) };

    match socket.poll(buf) {
        Some((size, src_addr)) => {
            // Write the sender address back to C++
            let src_c = CString::new(src_addr).unwrap();
            let src_bytes = src_c.as_bytes_with_nul();

            if src_bytes.len() <= sender_max_len {
                unsafe {
                    ptr::copy_nonoverlapping(src_bytes.as_ptr(), out_sender as *mut u8, src_bytes.len());
                }
            }
            size as i32
        },
        None => 0, // No data
    }
}