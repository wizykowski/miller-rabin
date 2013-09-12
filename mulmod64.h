#ifndef _MULMOD64_H_INCLUDED
#define _MULMOD64_H_INCLUDED

#include <stdint.h>

#if defined(_WIN64) || defined(__amd64__)

#if 0
static inline uint64_t mulmod64(uint64_t a, uint64_t b, uint64_t n)
{
	return (((unsigned __int128)a) * b) % n;
}
#else
// if a*b / c > 2^64, floating point exception will occur
static inline uint64_t mulmod64(uint64_t a, uint64_t b, uint64_t n)
{
	uint64_t d;
	uint64_t unused; // dummy output, unused, to tell GCC that RAX register is modified by this snippet
	asm ("mulq %3\n\t"
		 "divq %4"
		 :"=a"(unused), "=&d"(d)
		 :"a"(a), "rm"(b), "rm"(n)
		 :"cc");
	return d;
}
#endif

#else

// requires a, b < n
static inline uint64_t mulmod64(uint64_t a, uint64_t b, uint64_t n)
{
	uint64_t r = 0;

	if (a < b) { uint64_t tmp = a; a = b; b = tmp; } // swap(a, b)

	if (n < (1ULL << 63)) {
		while (b) {
			if (b & 1) {
				r += a;
				if (r >= n) r -= n;
			}
			b >>= 1;
			if (b) {
				a += a;
				if (a >= n) a -= n;
			}
		}
	} else {
	    while (b) {
			if (b & 1) {
				r = (n-r > a) ? r+a : r+a-n;
			}
			b >>= 1;
			if (b) {
				a = (n-a > a) ? a+a : a+a-n;
			}
		}
	}
	return r;
}
#endif

#endif // _MULMOD64_H_INCLUDED
