#include <stdio.h>
#include <time.h>
#include <stdint.h>
#include <inttypes.h>
#include <string.h>

#include "myrand.h"

#include "sprp32.h"
#include "sprp32_sf.h"

#if defined(_WIN64) || defined(__amd64__)
	#include "sprp64.h"
	#include "sprp64_sf.h"
#endif

#define BENCHMARK_ITERATIONS 1000000

typedef struct timespec time_point;

time_point get_time()
{
	struct timespec res;

	clock_gettime(CLOCK_MONOTONIC, &res);

	return res;
}

uint64_t elapsed_time(const time_point start)
{
	struct timespec end;
	clock_gettime(CLOCK_MONOTONIC, &end);

    long seconds  = end.tv_sec  - start.tv_sec;
    long nseconds = end.tv_nsec - start.tv_nsec;
    
	return seconds * 1000000000ULL + nseconds;
}

#if defined(_WIN64) || defined(__amd64__)
	// found by Jim Sinclair
	const uint64_t bases64[] = {2, 325, 9375, 28178, 450775, 9780504, 1795265022};
	#define BASES_CNT64 7
#endif

// found by Gerhard Jaeschke
const uint32_t bases32[] = {2, 7, 61};
#define BASES_CNT32 3
// see http://miller-rabin.appspot.com

#if defined(_WIN64) || defined(__amd64__)
	#define BASES_CNT_MAX BASES_CNT64
#else
	#define BASES_CNT_MAX BASES_CNT32
#endif	

void dummy_loops()
{
	int val = 0, i;
	
	myseed();
	for (i=0; i<1000000; i++) {
		val += efficient_mr32(bases32, 3, myrand32());
	}

	printf("val = %d\n", val);
}

void print_results(const int bits, const int cnt_limit, uint64_t time_vals[][2])
{
	int i;

	printf("%d-bit integer\n & bases & efficient & straightforward\n", bits);

	for (i=0; i<cnt_limit; i++) {
		const int cnt = i+1;

		printf(" & %d base%s", cnt, (cnt != 1 ? "s" : ""));
		printf(" & %" PRIu64 " ns", time_vals[i][0] / BENCHMARK_ITERATIONS);
		printf(" & %" PRIu64 " ns \\\\\n", time_vals[i][1] / BENCHMARK_ITERATIONS);
	}
}

int main()
{
	// stress processor to make it switch to full clock speed
	dummy_loops();
	
	printf("Starting benchmark...\n");

	uint64_t time_vals[BASES_CNT_MAX][2];

	myseed();
	int valeff = 0, cnt;
	for (cnt=1; cnt<=BASES_CNT32; cnt++) {
		time_point start = get_time();

		uint32_t n;
		
		int j;
		for (j=0; j<BENCHMARK_ITERATIONS; j++) {
			n = myrand32() | 1; if (n < 5) n = 5;
			
			valeff += efficient_mr32(bases32, cnt, n);
		}
		
		time_vals[cnt-1][0] = elapsed_time(start);
	}

	myseed();
	int valsf = 0;
	for (cnt=1; cnt<=BASES_CNT32; cnt++) {
		time_point start = get_time();

		uint32_t n;

		int j;
		for (j=0; j<BENCHMARK_ITERATIONS; j++) {
			n = myrand32() | 1; if (n < 5) n = 5;
			
			valsf += straightforward_mr32(bases32, cnt, n);
		}
		
		time_vals[cnt-1][1] = elapsed_time(start);
	}

	if (valsf != valeff) {
		fprintf(stderr, "valsf = %d, valeff = %d\n", valsf, valeff);
		return 1;
	}

	// we need to subtract time used for random input generation
	myseed();
	for (cnt=1; cnt<=BASES_CNT32; cnt++) {
		time_point start = get_time();

		uint32_t n;
		int j;
		for (j=0; j<BENCHMARK_ITERATIONS; j++) {
			n = myrand32() | 1; if (n < 5) n = 5;
		}
		
		long t = elapsed_time(start);
		time_vals[cnt-1][0] -= t;
		time_vals[cnt-1][1] -= t;
	}

	print_results(32, BASES_CNT32, time_vals);

#if defined(_WIN64) || defined(__amd64__)
	myseed();
	valeff = 0;
	for (cnt=1; cnt<=BASES_CNT64; cnt++) {
		time_point start = get_time();

		uint64_t n;

		int j;
		for (j=0; j<BENCHMARK_ITERATIONS; j++) {
			n = myrand64() | 1; if (n < 5) n = 5;
			
			valeff += efficient_mr64(bases64, cnt, n);
		}
		
		time_vals[cnt-1][0] = elapsed_time(start);
	}

	myseed();
	valsf = 0;
	for (cnt=1; cnt<=BASES_CNT64; cnt++) {
		time_point start = get_time();

		uint64_t n;

		int j;
		for (j=0; j<BENCHMARK_ITERATIONS; j++) {
			n = myrand64() | 1; if (n < 5) n = 5;
			
			valsf += straightforward_mr64(bases64, cnt, n);
		}
		
		time_vals[cnt-1][1] = elapsed_time(start);
	}

	if (valsf != valeff) {
		fprintf(stderr, "valsf = %d, valeff = %d\n", valsf, valeff);
		return 1;
	}

	// we need to subtract time used for random input generation
	myseed();
	for (cnt=1; cnt<=BASES_CNT64; cnt++) {
		time_point start = get_time();

		uint64_t n;
		int j;
		for (j=0; j<BENCHMARK_ITERATIONS; j++) {
			n = myrand64() | 1; if (n < 5) n = 5;
		}
		
		long t = elapsed_time(start);
		time_vals[cnt-1][0] -= t;
		time_vals[cnt-1][1] -= t;
	}

	print_results(64, BASES_CNT64, time_vals);
#endif

	return 0;
}

