#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>

#include "sprp64.h"
#include "sprp32.h"

// WARNING: a must be odd
// returns a^-1 mod 2^64
static inline uint64_t my_modular_inverse64(const uint64_t a)
{
	uint64_t u, x, w, z, q;

	x = 1; z = a;

	q = (-z) / z + 1; // = 2^64 / z
	u = -q; // = -q * x
	w = -q * z; // = b - q * z = 2^64 - q * z

	// after first iteration all variables are 64-bit

	while (w) {
		if (w < z) {
			q = u; u = x; x = q; // swap(u, x)
			q = w; w = z; z = q; // swap(w, z)
		}
		q = w / z;
		u -= q * x;
		w -= q * z;
	}

	return x;
}

// WARNING: a must be odd
// returns a^-1 mod 2^32
static inline uint32_t my_modular_inverse32(const uint32_t a)
{
	uint32_t u, x, w, z, q;

	x = 1; z = a;

	q = (-z) / z + 1; // = 2^32 / z
	u = -q; // = -q * x
	w = -q * z; // = b - q * z = 2^32 - q * z

	// after first iteration all variables are 32-bit

	while (w) {
		if (w < z) {
			q = u; u = x; x = q; // swap(u, x)
			q = w; w = z; z = q; // swap(w, z)
		}
		q = w / z;
		u -= q * x;
		w -= q * z;
	}

	return x;
}

int test_modular_inverse64()
{
	uint64_t a;
	for (a=3; a < 500; a+=2) {
		uint64_t received = modular_inverse64(a);
		uint64_t expected = my_modular_inverse64(-((int64_t)a));

		if (expected != received) {
			printf("expected: %" PRIu64 ", received: %" PRIu64 "\n", expected, received);
			return 1;
		}
	}

	for (a = UINT64_MAX; a > UINT64_MAX - 500; a -= 2) {
		uint64_t received = modular_inverse64(a);
		uint64_t expected = my_modular_inverse64(-a);

		if (expected != received) {
			printf("expected: %" PRIu64 ", received: %" PRIu64 "\n", expected, received);
			return 1;
		}
	}

	return 0;
}

int test_modular_inverse32()
{
	uint32_t a;
	for (a = 3; a < 500; a += 2) {
		uint32_t received = modular_inverse32(a);
		uint32_t expected = my_modular_inverse32(-((int32_t)a));

		if (expected != received) {
			printf("expected: %" PRIu32 ", received: %" PRIu32 "\n", expected, received);
			return 1;
		}
	}

	for (a = UINT32_MAX; a > UINT32_MAX - 500; a -= 2) {
		uint32_t received = modular_inverse32(a);
		uint32_t expected = my_modular_inverse32(-a);

		if (expected != received) {
			printf("expected: %" PRIu32 ", received: %" PRIu32 "\n", expected, received);
			return 1;
		}
	}

	return 0;
}

int main()
{
	int res = test_modular_inverse64();
	res |= test_modular_inverse32();

	if (res == 0) {
		printf("All tests completed successfully - no errors.");
	} else {
		printf("Error running tests.");
	}

	return 0;
}
