CC = gcc
CFLAGS = -Wall -Wextra -O2

all: mr_benchmark

mr_benchmark: mr_benchmark.c sprp64_sf.h sprp32_sf.h sprp64.h sprp32.h mulmod64.h myrand.h
	$(CC) $(CFLAGS) mr_benchmark.c -o mr_benchmark

