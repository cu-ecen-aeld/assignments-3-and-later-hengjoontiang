CROSS_COMPILE=
CROSS_CC=/home/hengjt/Downloads/arm-gnu-toolchain-13.3.rel1-x86_64-aarch64-none-linux-gnu/bin/aarch64-none-linux-gnu-gcc
CC=${CROSS_COMPILE}gcc
default:all
all:aesdsocket.c
	${CC} -o aesdsocket aesdsocket.c ThreadSafeArrayList.c -I.
clean:
	rm -f aesdsocket *.o
