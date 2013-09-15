#include <stdio.h>
#include <time.h>
#include <stdint.h>
#include <stdlib.h>

#ifndef _MSC_VER
	#include <inttypes.h>
#else
	#define PRIu64 "llu"
	#define inline __inline
#endif

#include "myrand.h"
#include "mytime.h"

#include "sprp32.h"
#include "sprp32_sf.h"

#include "sprp64.h"
#include "sprp64_sf.h"

#define BENCHMARK_ITERATIONS 10000

// found by Jim Sinclair
const uint64_t bases64[] = {2, 325, 9375, 28178, 450775, 9780504, 1795265022};
#define BASES_CNT64 7

// found by Gerhard Jaeschke
const uint32_t bases32[] = {2, 7, 61};
#define BASES_CNT32 3
// see http://miller-rabin.appspot.com

#define BASES_CNT_MAX BASES_CNT64

static uint32_t n32[BENCHMARK_ITERATIONS];
static uint64_t n64[BENCHMARK_ITERATIONS];

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
	int i;

	myseed();
	// simple PRIMEINC method - uniformity isn't important
	for (i = 0; i < BENCHMARK_ITERATIONS; i++) {
		uint32_t n = myrand32() | 1;
		n += distancewheel[(n >> 1) % WHEEL_PRODUCT];
		if (n < 5) n = 5;
		while (!efficient_mr32(bases32, 3, n))
			n += wheeladvance[(n >> 1) % WHEEL_PRODUCT];
		n32[i] = n;
	}
	
	for (i = 0; i < BENCHMARK_ITERATIONS; i++) {
		uint64_t n = myrand64() | 1;
		n += distancewheel[(n >> 1) % WHEEL_PRODUCT];
		if (n < 5) n = 5;
		while (!efficient_mr64(bases64, 7, n))
			n += wheeladvance[(n >> 1) % WHEEL_PRODUCT];
		n64[i] = n;
	}
}

static void set_nintegers()
{
	int i;

	myseed();
	for (i = 0; i < BENCHMARK_ITERATIONS; i++) {
		uint32_t n = myrand32() | 1;
		if (n < 5) n = 5;
		n32[i] = n;
	}

	for (i = 0; i < BENCHMARK_ITERATIONS; i++) {
		uint64_t n = myrand64() | 1;
		if (n < 5) n = 5;
		n64[i] = n;
	}
}

void print_results(const int bits, const int cnt_limit, uint64_t time_vals[][2])
{
	int i;

	printf("        %d-bit integer\n  bases  |  effcnt  | simple\n", bits);

	for (i=0; i<cnt_limit; i++) {
		const int cnt = i+1;

		printf(" %d base%s", cnt, (cnt != 1 ? "s" : " "));
		printf(" | %5" PRIu64 " ns", time_vals[i][0] / BENCHMARK_ITERATIONS);
		printf(" | %5" PRIu64 " ns\n", time_vals[i][1] / BENCHMARK_ITERATIONS);
	}
	printf("\n");
}

void run_benchmark()
{
	int j, cnt, valsf, valeff;
	uint64_t time_vals[BASES_CNT_MAX][2];

	printf("Starting benchmark...\n");

	valsf = 0;
	valeff = 0;
	for (cnt=1; cnt<=BASES_CNT32; cnt++) {
		time_point start = get_time();
		for (j=0; j<BENCHMARK_ITERATIONS; j++)
			valeff += efficient_mr32(bases32, cnt, n32[j]);
		time_vals[cnt-1][0] = elapsed_time(start);
	}
	for (cnt=1; cnt<=BASES_CNT32; cnt++) {
		time_point start = get_time();
		for (j=0; j<BENCHMARK_ITERATIONS; j++)
			valsf += straightforward_mr32(bases32, cnt, n32[j]);
		time_vals[cnt-1][1] = elapsed_time(start);
	}
	if (valsf != valeff) {
		fprintf(stderr, "valsf = %d, valeff = %d\n", valsf, valeff);
		exit(1);
	}
	print_results(32, BASES_CNT32, time_vals);

	valsf = 0;
	valeff = 0;
	for (cnt=1; cnt<=BASES_CNT64; cnt++) {
		time_point start = get_time();
		for (j=0; j<BENCHMARK_ITERATIONS; j++)
			valeff += efficient_mr64(bases64, cnt, n64[j]);
		time_vals[cnt-1][0] = elapsed_time(start);
	}
	for (cnt=1; cnt<=BASES_CNT64; cnt++) {
		time_point start = get_time();
		for (j=0; j<BENCHMARK_ITERATIONS; j++)
			valsf += straightforward_mr64(bases64, cnt, n64[j]);
		time_vals[cnt-1][1] = elapsed_time(start);
	}
	if (valsf != valeff) {
		fprintf(stderr, "valsf = %d, valeff = %d\n", valsf, valeff);
		exit(1);
	}

	print_results(64, BASES_CNT64, time_vals);
}

int main()
{
	printf("Setting random primes...\n");
	set_nprimes();

	run_benchmark();

	printf("Setting random odd integers...\n");
	set_nintegers();

	run_benchmark();

	return 0;
}
