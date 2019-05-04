/* SPDX-License-Identifier: Unlicense */

/**
 * @file
 * Example/test for test-tun
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include "tun-open.h"

#define CHK(var, call) \
	do { \
		var = call; \
		if (var < 0) { \
			fprintf(stderr, "%s:%d: '%s' failed: %s\n", __FUNCTION__, __LINE__, #call, strerror(errno)); \
			return var; \
		} \
	} while(0) \


static int systemEcho(const char *format, ...) __attribute__ ((format (__printf__, 1, 2)));

static int systemEcho(const char *format, ...) {
	char cmd[256];
	va_list args;
	va_start(args, format);
	vsnprintf(cmd, sizeof(cmd), format, args);
	va_end(args);
	printf("== %s\n", cmd);
	int err = system(cmd);
	if (err != 0) {
		fprintf(stderr, "system command failed with %d: %s\n", err, cmd);
	}
	return err;
}


static int readLoop(int tunfd, int pings) {
	unsigned char buf[2048];
	while (pings) {
		ssize_t n = read(tunfd, buf, sizeof(buf));
		if (n < 0) {
			perror("Read failed");
			return -1;
		}
		unsigned char *p = buf + TUN_OPEN_PACKET_OFFSET;
		printf("%4zd", n);
		if (TUN_OPEN_IS_IP6(buf)) {
			printf(" IPv6");
			unsigned char nextHeader = p[6];
			unsigned char *srcAddr = p + 8;
			unsigned char *dstAddr = p + 8 + 16;
			switch (nextHeader) {
				case 58: {
					unsigned char *icmpType = p + 8 + 16 + 16;
					printf(" ICMPv6");
					switch (*icmpType) {
						case 128: {
							printf(" EchoRequest");
							*icmpType = 129;
							unsigned char addr[16];
							memcpy(addr, srcAddr, 16);
							memcpy(srcAddr, dstAddr, 16);
							memcpy(dstAddr, addr, 16);
							unsigned char *checksum = icmpType + 2;
							int newChecksum = (checksum[0] << 8 | checksum[1]) - 0x0100;
							checksum[0] = newChecksum >> 8;
							checksum[1] = newChecksum;
							ssize_t nwrite = write(tunfd, buf, n);
							if (nwrite < 0) {
								perror("Read failed");
								return -1;
							}
							pings -= 1;
							break;
						}
						case 129:
							printf(" EchoReply");
							break;
						case 133:
							printf(" RouterSolicitation");
							break;
						default:
							printf(" Type=%d", p[0]);
							break;
					}
					break;
				}
				default:
					printf(" NextHeader=%d", p[0]);
					break;
			}
		} else if (TUN_OPEN_IS_IP4(buf)) {
			printf(" IPv4");
		} else {
			printf(" UNKNOWN");
		}
		printf("\n");
	}
	return 0;
}

// example for the Readme
static void readmeExample() {
	TunOpenName tunName;
	int fd = tunOpen(NULL, &tunName);
	if (fd < 0) {
		perror("Creating a tun device failed");
		exit(1);
	}
	printf("Created tun device %s\n", tunName.name);
	// device configuration (address, ...) required
	unsigned char buf[2048];
	while (1) {
		ssize_t n = read(fd, buf, sizeof(buf));
		if (n < 0) {
			printf("Reading from tun device failed");
			break;
		}
		if (TUN_OPEN_IS_IP6(buf)) {
			printf("IPv6 packet with %zu bytes\n", n - TUN_OPEN_PACKET_OFFSET);
		} else if (TUN_OPEN_IS_IP4(buf)) {
			printf("IPv4 packet with %zu bytes\n", n - TUN_OPEN_PACKET_OFFSET);
		} else {
			printf("Unknown packet with %zu bytes\n", n - TUN_OPEN_PACKET_OFFSET);
		}
	}
	close(fd);
}

int main(int argc, char **argv) {
	const char *tunNameHint = NULL;
	const char *address = "fddf:face:face::5555";
	int pings = 3;
	for (int i = 1; i < argc; i++) {
		if (strchr(argv[i], ':')) {
			address = argv[i];
		} else if (strcmp(argv[i], "--readme") == 0) {
			readmeExample();
			return 0;
		} else if (atoi(argv[i]) > 0) {
			pings = atoi(argv[i]);
		} else {
			tunNameHint = argv[i];
		}
	}
	TunOpenName tunName;
	int tunfd;
	CHK(tunfd, tunOpen(tunNameHint, &tunName));
	systemEcho("ip addr add %s/64 dev %s", address, tunName.name);
	systemEcho("ip link set dev %s up", tunName.name);
	systemEcho("ip addr show dev %s", tunName.name);
	systemEcho("ip -6 route");
	char *destAddr = strdup(address);
	destAddr[strlen(destAddr) - 1] ^= 0x01;
	systemEcho("ping6 -c %u -i 1 %s &", pings, destAddr);
	return readLoop(tunfd, pings);
}
