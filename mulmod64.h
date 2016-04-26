#ifndef _MULMOD64_H_INCLUDED
#define _MULMOD64_H_INCLUDED

#include <stdint.h>

#if !defined(_WIN64) && !defined(__amd64__)
// returns lower 64-bit part of a*b ; higher part in hi
static inline uint64_t mul128(uint64_t a, uint64_t b, uint64_t *hi)
{
	uint32_t AH = a >> 32;
	uint32_t AL = (uint32_t)a;

	uint32_t BH = b >> 32;
	uint32_t BL = (uint32_t)b;

	uint64_t AHBH = (uint64_t)AH * BH;
	uint64_t ALBL = (uint64_t)AL * BL;
	uint64_t AHBL = (uint64_t)AH * BL;
	uint64_t ALBH = (uint64_t)AL * BH;
	
	// take care of integer overflow
	
	uint64_t middle = AHBL + ALBH;

	if (middle < AHBL) AHBH += (1ULL << 32);

	uint64_t res_lo = ALBL + (middle << 32);
	if (res_lo < ALBL) AHBH++;

	AHBH += middle >> 32;

	*hi = AHBH;

	return res_lo;
}
#elif !defined(_MSC_VER)
static inline uint64_t mul128(uint64_t a, uint64_t b, uint64_t *hi)
{
	uint64_t lo;
	asm("mulq %3" : "=a"(lo), "=d"(*hi) : "a"(a), "rm"(b));
	return lo;
}
#else
	#include <intrin.h>

	#define mul128(a, b, hi) _umul128(a, b, hi)
#endif

#if (defined(_WIN64) || defined(__amd64__)) && !defined(_MSC_VER)

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

#elif defined(_WIN64) && defined(_MSC_VER)

// according to documentation future Visual C++ versions
// may use different registers to store parameters
//
// it works at least with Visual C++ 2012 and Visual C++ 2015

// arguments passed in RCX, RDX, R8
// return value in RAX

#pragma section("mulmod64", read, execute)
__declspec(allocate("mulmod64"))
unsigned char mulmod64_code[] = {
	0x48, 0x89, 0xC8, // mov rax,rcx
	0x48, 0xF7, 0xE2, // mul rdx
	0x49, 0xF7, 0xF0, // div r8
	0x48, 0x89, 0xD0, // mov rax,rdx
	0xC3              // ret
};

uint64_t (__fastcall *mulmod64)(uint64_t, uint64_t, uint64_t) =
	(uint64_t (__fastcall *)(uint64_t, uint64_t, uint64_t))(void *)mulmod64_code;

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

#endif // if 64-bit architecture

#endif // _MULMOD64_H_INCLUDED
