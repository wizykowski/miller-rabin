#ifndef _SPRP32_SF_H_INCLUDED
#define _SPRP32_SF_H_INCLUDED

#include <stdint.h>

// 32-bit straightforward implementation begins

// we could use uint32_t d and A, but this is faster (at least on x86-64)
static inline uint32_t modular_exponentiation32(uint32_t a, uint32_t b, uint32_t n)
{
	uint64_t d=1, A=a;

	do {
		if (b&1)
			d=(d*A)%n;
		A=(A*A)%n;
	} while (b>>=1);

	return (uint32_t)d;
}

static inline uint32_t square_modulo32(uint32_t a, uint32_t n)
{
	return (uint32_t)(((uint64_t)a*a) % n);
}

static inline int straightforward_mr32(const uint32_t bases[], int bases_cnt, uint32_t n)
{
	uint32_t u=n-1;
	int t=0, j;

	while (u % 2 == 0) { // while even
		t++;
		u >>= 1;
	}

	for (j=0; j<bases_cnt; j++) {
		uint32_t a = bases[j], x;
		int i;

		if (a >= n) a %= n;



		if (a == 0) continue;

		x = modular_exponentiation32(a, u, n);

		if (x == 1 || x == n-1) continue;

		for (i=1; i<t; i++) {
			x=square_modulo32(x, n);
			if (x == 1)   return 0;
			if (x == n-1) break;
		}

		// if we didn't break, the number is composite
		if (i == t) return 0;
	}

	return 1;
}

#endif // _SPRP32_SF_H_INCLUDED
