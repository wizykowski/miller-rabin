#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>

#include "sprp64.h"
#include "sprp32.h"
#include "myrand.h"

int test_modular_inverse64()
{
	uint64_t a;
	int i;

	for (a=3; a < 5000000; a+=2) {
		uint64_t received = modular_inverse64(a);
		int64_t received_minus_one = received * a;

		if (received_minus_one != -1) {
			printf("expected: -1, received: %" PRIu64 ", argument: %" PRIu64 "\n", received_minus_one, a);
			return 1;
		}
	}

	for (a = UINT64_MAX; a > UINT64_MAX - 5000000; a -= 2) {
		uint64_t received = modular_inverse64(a);
		int64_t received_minus_one = received * a;

		if (received_minus_one != -1) {
			printf("expected: -1, received: %" PRIu64 ", argument: %" PRIu64 "\n", received_minus_one, a);
			return 1;
		}
	}

	myseed();
	for (i=0; i < 5000000; i++) {
		a = myrand64() | 1;
		uint64_t received = modular_inverse64(a);
		int64_t received_minus_one = received * a;

		if (received_minus_one != -1) {
			printf("expected: -1, received: %" PRIu64 ", argument: %" PRIu64 "\n", received_minus_one, a);
			return 1;
		}
	}

	return 0;
}

int test_modular_inverse32()
{
	uint32_t a;
	int i;

	for (a = 3; a < 5000000; a += 2) {
		uint32_t received = modular_inverse32(a);
		int32_t received_minus_one = received * a;

		if (received_minus_one != -1) {
			printf("expected: -1, received: %" PRIu32 ", argument: %" PRIu32 "\n", received_minus_one, a);
			return 1;
		}
	}

	for (a = UINT32_MAX; a > UINT32_MAX - 5000000; a -= 2) {
		uint32_t received = modular_inverse32(a);
		int32_t received_minus_one = received * a;

		if (received_minus_one != -1) {
			printf("expected: -1, received: %" PRIu32 ", argument: %" PRIu32 "\n", received_minus_one, a);
			return 1;
		}
	}

	myseed();
	for (i=0; i < 5000000; i++) {
		a = myrand32() | 1;
		uint32_t received = modular_inverse32(a);
		int32_t received_minus_one = received * a;

		if (received_minus_one != -1) {
			printf("expected: -1, received: %" PRIu32 ", argument: %" PRIu32 "\n", received_minus_one, a);
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
		printf("All tests completed successfully - no errors.\n");
	} else {
		printf("Error running tests.\n");
	}

	return res;
}
