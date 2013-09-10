#ifndef _MULMOD64_H_INCLUDED
#define _MULMOD64_H_INCLUDED

#include <stdint.h>

#if 0
static inline uint64_t mulmod64(uint64_t a, uint64_t b, uint64_t n)
{
	return (((unsigned __int128)a) * b) % n;
}
#else
// if a*b / c > 2^64, floating point exception will occur
static inline uint64_t mulmod64(uint64_t a, uint64_t b, uint64_t c)
{
	uint64_t d;
	uint64_t unused; // dummy output, unused, to tell GCC that RAX register is modified by this snippet
	asm ("mulq %3\n\t"
		 "divq %4"
		 :"=a"(unused), "=&d"(d)
		 :"a"(a), "rm"(b), "rm"(c)
		 :"cc");
    return d;
}
#endif

#endif // _MULMOD64_H_INCLUDED
