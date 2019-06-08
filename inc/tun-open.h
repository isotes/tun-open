/* (C) Copyright 2019 Robert Sauter
 * SPDX-License-Identifier: MIT
 */

#ifdef __cplusplus
extern "C" {
#endif

/// The name of the tun device
typedef struct TunOpenNameS {
	char name[16];
} TunOpenName;


/**
 * Open/create the tun device with the specified name as a hint
 * @attention on macOS, the must be NULL or follow the format 'utun<N>' (N being a unsigned integer)
 * @param name the name of the tun device (may be NULL, must be less than 16 characters)
 * @param[out] tunName the actual name of the tun device (may be NULL)
 * @return return the new file descriptor or -1 if an error occurred (in which case, errno is set appropriately)
 */
int tunOpen(const char* name, TunOpenName* tunName);

/// The offset of the packet in the buffer of a read/write call
#define TUN_OPEN_PACKET_OFFSET 4

#if __APPLE__
#include <sys/socket.h>
#define TUN_OPEN_IP4_HEADER 0x00, 0x00, 0x00, AF_INET
#define TUN_OPEN_IS_IP4(buf) (((const unsigned char*) buf)[3] == AF_INET)
#define TUN_OPEN_IP6_HEADER 0x00, 0x00, 0x00, AF_INET6
#define TUN_OPEN_IS_IP6(buf) (((const unsigned char*) buf)[3] == AF_INET6)
#else
#define TUN_OPEN_IP4_HEADER 0x00, 0x00, 0x08, 0x00
#define TUN_OPEN_IS_IP4(buf) ((((const unsigned char*) buf)[2] == 0x08) && (((const unsigned char*) buf)[3] == 0x00))
#define TUN_OPEN_IP6_HEADER 0x00, 0x00, 0x86, 0xdd
#define TUN_OPEN_IS_IP6(buf) ((((const unsigned char*) buf)[2] == 0x86) && (((const unsigned char*) buf)[3] == 0xdd))
#endif

#ifdef  __cplusplus
}
#endif
