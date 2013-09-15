#ifndef _MYTIME_H_INCLUDED
#define _MYTIME_H_INCLUDED

#ifdef _MSC_VER
#include <windows.h>

typedef LARGE_INTEGER time_point;

static inline time_point get_time()
{
	LARGE_INTEGER res;
	QueryPerformanceCounter(&res);
	return res;
}

uint64_t diff_time(const LARGE_INTEGER t2, const LARGE_INTEGER t1)
{
	LARGE_INTEGER cycles;
	double cycles_per_ns;
	QueryPerformanceFrequency(&cycles);

	cycles_per_ns = cycles.QuadPart / 1000000000.0;

	return (double)(t2.QuadPart-t1.QuadPart) / cycles_per_ns;
}

uint64_t elapsed_time(const time_point start)
{
	LARGE_INTEGER end;
	QueryPerformanceCounter(&end);

	return diff_time(end, start);
}
#else
typedef struct timespec time_point;

static inline time_point get_time()
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
#endif


#endif // _MYTIME_H_INCLUDED
