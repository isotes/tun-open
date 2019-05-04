/* (C) Copyright 2019 Robert Sauter
 * SPDX-License-Identifier: MIT
 */

/**
 * @file
 * Small library without dependencies to open/create a tun device on Linux and macOS.
 */

#include "tun-open.h"
#include <string.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#if __APPLE__
#include <sys/socket.h>
#include <sys/sys_domain.h>
#include <sys/kern_control.h>
#include <net/if_utun.h>
#else
#include <fcntl.h>
#include <net/if.h>
#include <linux/if_tun.h>
#endif

#if TUN_OPEN_VERBOSE
#include <stdio.h>

#define CHK(var, call) \
	do { \
		var = call; \
		if (var < 0) { \
			fprintf(stderr, "%s:%d: '%s' failed: %s\n", __FUNCTION__, __LINE__, #call, strerror(errno)); \
			return var; \
		} \
	} while(0) \

#else
#define CHK(var, call) do { var = call; (void) var; } while (0)
#endif

int tunOpen(const char* nameHint, TunOpenName* tunName) {
#if __APPLE__
	(void) nameHint;  // unused

	int fd;
	CHK(fd, socket(AF_SYSTEM, SOCK_DGRAM, SYSPROTO_CONTROL));

	struct ctl_info info;
	memset(&info, 0, sizeof(info));
	strncpy(info.ctl_name, UTUN_CONTROL_NAME, sizeof(info.ctl_name));
	int err;
	CHK(err, ioctl(fd, CTLIOCGINFO, &info));

	struct sockaddr_sys addr = {
			.ss_len = sizeof(addr),
			.ss_family = AF_SYSTEM,
			.ss_sysaddr = SYSPROTO_CONTROL,
			.ss_reserved = {info.ctl_id},
	};
	CHK(err, connect(fd, (const struct sockaddr *) &addr, sizeof(addr)));

	if (tunName) {
		socklen_t optlen = sizeof(tunName);
		CHK(err, getsockopt(fd, SYSPROTO_CONTROL, UTUN_OPT_IFNAME, tunName, &optlen));
	}

	return fd;
#else /* __APPLE__ */
	size_t nameHintLen = nameHint ? strlen(nameHint) : 0;
	if (nameHintLen >= sizeof(tunName->name) || nameHintLen >= IFNAMSIZ) {
		errno = EINVAL;
		return -1;
	}
	int fd;
	CHK(fd, open("/dev/net/tun", O_RDWR));
	struct ifreq ifr;
	memset(&ifr, 0, sizeof(ifr));
	ifr.ifr_flags = IFF_TUN;
	memcpy(ifr.ifr_name, nameHint, nameHintLen);

	int err;
	CHK(err, ioctl(fd, TUNSETIFF, (void *) &ifr));

	if (tunName) {
		strncpy(tunName->name, ifr.ifr_name, sizeof(tunName->name));
	}

	return fd;
#endif /* __APPLE__ */
}
