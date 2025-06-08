CROSS_COMPILE=
CROSS_CC=/home/hengjt/Downloads/arm-gnu-toolchain-13.3.rel1-x86_64-aarch64-none-linux-gnu/bin/aarch64-none-linux-gnu-gcc
CC=${CROSS_COMPILE}gcc
default:findermake
findermake:writer.c
	${CC} -o writer writer.c -I.
clean:
	rm -f writer *.o
