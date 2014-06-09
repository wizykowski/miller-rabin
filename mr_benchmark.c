#include <stdio.h>
#include <time.h>
#include <stdint.h>
#include <stdlib.h>

#if !defined(_MSC_VER) || _MSC_VER >= 1800 // Visual Studio 2013 is the first version with inttypes.h
	#include <inttypes.h>
#else
	#define PRIu64 "llu"
	#define PRIu32 "u"
#endif

#ifdef _MSC_VER
	#define inline __inline
#endif

#include "myrand.h"
#include "mytime.h"

#include "sprp32.h"
#include "sprp32_sf.h"

#include "sprp64.h"
#include "sprp64_sf.h"

#define BENCHMARK_ITERATIONS 100000

// With hashing only 3 bases are enough to test all numbers up to 2^64
// and 1 base to test all up to 2^32
// see http://probableprime.org/download/example-primality.c
#define BASES_CNT64 3
#define BASES_CNT32 3
#define BASES_CNT_MAX BASES_CNT64

// found by Jim Sinclair
const uint64_t bases64[] = {2, 325, 9375, 28178, 450775, 9780504, 1795265022};

// found by Gerhard Jaeschke
const uint32_t bases32[] = {2, 7, 61};
// see http://miller-rabin.appspot.com

#define SIZES_CNT32 4
#define SIZES_CNT64 4
#define SIZES_CNT_MAX SIZES_CNT64

static const char bits32[SIZES_CNT32] = {8,16,24,32};
static const char bits64[SIZES_CNT64] = {40,48,56,64};
static const uint32_t mask32[SIZES_CNT32] = {0xFFU,0xFFFFU,0xFFFFFFU,0xFFFFFFFFU};
static const uint64_t mask64[SIZES_CNT64] = {0xFFFFFFFFFFULL,0xFFFFFFFFFFFFULL,0xFFFFFFFFFFFFFFULL,0xFFFFFFFFFFFFFFFFULL};
static uint32_t n32[SIZES_CNT32][BENCHMARK_ITERATIONS];
static uint64_t n64[SIZES_CNT64][BENCHMARK_ITERATIONS];

#define WHEEL_PRODUCT 105
// wheel contains only odd numbers
static const unsigned char distancewheel[WHEEL_PRODUCT] = 
	{0,8,6,4,2,0,0,2,0,0,2,0,4,2,0,0,4,2,0,2,0,0,2,0,4,2,0,4,2,0,0,4,2,0,2,
	 0,0,4,2,0,2,0,4,2,0,6,4,2,0,2,0,0,2,0,0,2,0,6,4,2,0,4,2,0,2,0,4,2,0,0,
	 2,0,4,2,0,0,4,2,0,4,2,0,2,0,0,2,0,4,2,0,0,4,2,0,2,0,0,2,0,0,8,6,4,2,0};
static const unsigned char wheeladvance[WHEEL_PRODUCT] = 
	{10,0,0,0,0,2,4,0,2,4,0,6,0,0,2,6,0,0,4,0,2,4,0,6,0,0,6,0,0,2,6,0,0,4,0,
	 2,6,0,0,4,0,6,0,0,8,0,0,0,4,0,2,4,0,2,4,0,8,0,0,0,6,0,0,4,0,6,0,0,2,4,
	 0,6,0,0,2,6,0,0,6,0,0,4,0,2,4,0,6,0,0,2,6,0,0,4,0,2,4,0,2,10,0,0,0,0,2};

static void set_nprimes()
{
	int i, j;

	myseed();

	for (i = 0; i < SIZES_CNT32; i++)
		// simple PRIMEINC method - uniformity isn't important
		for (j = 0; j < BENCHMARK_ITERATIONS; j++) {
			uint32_t n = (myrand32() & mask32[i]) | 1;
			n += distancewheel[(n >> 1) % WHEEL_PRODUCT];
			if (n < 5) n = 5;
			while (!efficient_mr32(bases32, 3, n))
				n += wheeladvance[(n >> 1) % WHEEL_PRODUCT];
			n32[i][j] = n;
		}
	
	for (i = 0; i < SIZES_CNT64; i++)
		for (j = 0; j < BENCHMARK_ITERATIONS; j++) {
			uint64_t n = (myrand64() & mask64[i]) | 1;
			n += distancewheel[(n >> 1) % WHEEL_PRODUCT];
			if (n < 5) n = 5;
			while (!efficient_mr64(bases64, 7, n))
				n += wheeladvance[(n >> 1) % WHEEL_PRODUCT];
			n64[i][j] = n;
		}
}

static void set_nintegers()
{
	int i, j;

	myseed();
	for (i = 0; i < SIZES_CNT32; i++)
		for (j = 0; j < BENCHMARK_ITERATIONS; j++) {
			uint32_t n = (myrand32() & mask32[i]) | 1;
			if (n < 5) n = 5;
			n32[i][j] = n;
		}

	for (i = 0; i < SIZES_CNT64; i++)
		for (j = 0; j < BENCHMARK_ITERATIONS; j++) {
			uint64_t n = (myrand64() & mask64[i]) | 1;
			if (n < 5) n = 5;
			n64[i][j] = n;
		}
}

void print_results(const char *bits_array, const int bits_limit, const int cnt_limit, uint64_t time_vals[][3][2])
{
	int i, j;

	printf("         ");
	for (i = 0; i < bits_limit; i++) {
		printf("|    %2d-bit integer   ", bits_array[i]);
	}
	printf("\n  bases  ");
	for (i = 0; i < bits_limit; i++) {
		printf("|  effcnt  |  simple  ");
	}
	printf("\n");

	for (i = 0; i < cnt_limit; i++) {
		const int cnt = i + 1;

		printf(" %d base%s", cnt, (cnt != 1 ? "s" : " "));

		for (j = 0; j < bits_limit; j++) {
			printf(" | %5" PRIu64 " ns", time_vals[j][i][0] / BENCHMARK_ITERATIONS);
			printf(" | %5" PRIu64 " ns", time_vals[j][i][1] / BENCHMARK_ITERATIONS);
		}
		printf("\n");
	}
	printf("\n");
}

void run_benchmark()
{
	int i, j, cnt, valsf, valeff;
	uint64_t time_vals[SIZES_CNT_MAX][BASES_CNT_MAX][2];

	printf("Starting benchmark...\n");

	for (i = 0; i < SIZES_CNT32; i++) {
		valsf = 0;
		valeff = 0;
		for (cnt = 1; cnt <= BASES_CNT32; cnt++) {
			time_point start = get_time();
			for (j = 0; j < BENCHMARK_ITERATIONS; j++)
				valeff += efficient_mr32(bases32, cnt, n32[i][j]);
			time_vals[i][cnt - 1][0] = elapsed_time(start);
		}
		for (cnt = 1; cnt <= BASES_CNT32; cnt++) {
			time_point start = get_time();
			for (j = 0; j < BENCHMARK_ITERATIONS; j++)
				valsf += straightforward_mr32(bases32, cnt, n32[i][j]);
			time_vals[i][cnt - 1][1] = elapsed_time(start);
		}
		if (valsf != valeff) {
			fprintf(stderr, "valsf = %d, valeff = %d\n", valsf, valeff);
			exit(1);
		}
	}
	print_results(bits32, SIZES_CNT32, BASES_CNT32, time_vals);

	for (i = 0; i < SIZES_CNT64; i++) {
		valsf = 0;
		valeff = 0;
		for (cnt = 1; cnt <= BASES_CNT64; cnt++) {
			time_point start = get_time();
			for (j = 0; j < BENCHMARK_ITERATIONS; j++)
				valeff += efficient_mr64(bases64, cnt, n64[i][j]);
			time_vals[i][cnt - 1][0] = elapsed_time(start);
		}
		for (cnt = 1; cnt <= BASES_CNT64; cnt++) {
			time_point start = get_time();
			for (j = 0; j < BENCHMARK_ITERATIONS; j++)
				valsf += straightforward_mr64(bases64, cnt, n64[i][j]);
			time_vals[i][cnt - 1][1] = elapsed_time(start);
		}
		if (valsf != valeff) {
			fprintf(stderr, "valsf = %d, valeff = %d\n", valsf, valeff);
			exit(1);
		}
	}
	print_results(bits64, SIZES_CNT64, BASES_CNT64, time_vals);
}

int main()
{
#ifdef _WIN32
	system("mode CON: COLS=98");
#endif

	printf("Setting random primes...\n");
	set_nprimes();

	run_benchmark();

	printf("Setting random odd integers...\n");
	set_nintegers();

	run_benchmark();

	return 0;
}
