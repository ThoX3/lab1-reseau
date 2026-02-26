#ifndef SNL_LIB_H
#define SNL_LIB_H

#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

struct GameSocket;

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

const char *net_get_version(void);

struct GameSocket *net_socket_create(const char *bind_addr);

void net_socket_destroy(struct GameSocket *socket_ptr);

int32_t net_socket_send(struct GameSocket *socket_ptr,
                        const char *address,
                        const uint8_t *data,
                        uintptr_t len);

int32_t net_socket_poll(struct GameSocket *socket_ptr,
                        uint8_t *out_data,
                        uintptr_t max_len,
                        char *out_sender,
                        uintptr_t sender_max_len);

#ifdef __cplusplus
}  // extern "C"
#endif  // __cplusplus

#endif  /* SNL_LIB_H */
