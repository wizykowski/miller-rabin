CC = gcc
CFLAGS = -Wall -Wextra -O2
ifdef ComSpec
	# we are compiling under Windows
	LDFLAGS = 
else
	LDFLAGS = -lrt
endif

all: mr_benchmark

mr_benchmark: mr_benchmark.c sprp64_sf.h sprp32_sf.h sprp64.h sprp32.h mulmod64.h myrand.h
	$(CC) $(CFLAGS) $@.c -o $@ $(LDFLAGS)

