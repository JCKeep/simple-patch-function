all:
	arm-linux-gnueabihf-gcc  -marm -mcpu=cortex-a9  -O0 -g -Wall -fvisibility=hidden --static -o main main.c && qemu-arm ./main
	aarch64-linux-gnu-gcc -O0 -g -Wall -fvisibility=hidden --static -o main main.c && qemu-aarch64 ./main
	gcc -O0 -Wall -fvisibility=hidden -o main main.c && ./main
