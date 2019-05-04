# tun-open
Small library without dependencies to open/create a tun device on Linux and macOS.

## Portability
The library supports only tun (vs. tap) interfaces to be compatible with macOS. Nevertheless, there are a couple of differences to consider:
 - Specifying the name of the device is affects only Linux (the name is always automatically chosen on macOS)
 - Opening existing devices is only possible on Linux

On Linux, the library uses the [tun/tap](https://www.kernel.org/doc/Documentation/networking/tuntap.txt) interface provided by the kernel.

On macOS, the library uses the [utun kernel control](https://github.com/apple/darwin-xnu/blob/master/bsd/net/if_utun.h) interface.

## Use
The implementation consists of 1 header and 1 source file and can be included directly in other projects. Additionally, the project contains a CMake file to build as a library.

## API
There is only one function and a few helper defines. There is no inbuilt support for configuring the created network interface.

```C
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
```

## Configuration
The CMake project provides 2 options that are both disabled by default:
 - `TUN_OPEN_ENABLE_VERBOSE`: enable verbose error output
 - `TUN_OPEN_ENABLE_TEST`: build the test executable

With `TUN_OPEN_ENABLE_VERBOSE`, the library will print an error message to stderr when a system call fails. This is only recommended during development. E.g., when used without root rights on macOS, it prints the following error message:
```
tunOpen:60: 'connect(fd, (const struct sockaddr *) &addr, sizeof(addr))' failed: Operation not permitted
```

## Example
The [test executable](test/test-tun-open.c) provides a more complete example for IPv6:
 - it must run as root or, on Linux, the tun device must already have been created with appropriate permissions
 - it can be called with an address (default: `fddf:face:face::5555`) and/or a device name (provided by the OS by default and ignored on macOS)
 - it opens the device, and assigns the address using the `ip` command
    - the `ip` command is usually installed on Linux systems and is made available for macOS by the [iproute2mac](https://github.com/brona/iproute2mac) project (available on Homebrew)
- it starts the `ping6` command in the background to ping an address in the virtual network
- it runs in a loop, shows some information for each packet, and responds to  ICMPv6-Echo-Request message


## License
[MIT](LICENSE) for the library and [The Unlicense](https://unlicense.org/) for the test/example code.
