#ifndef _SPRP64_SF_H_INCLUDED
#define _SPRP64_SF_H_INCLUDED

#include "mulmod64.h"

static inline uint64_t modular_exponentiation64(uint64_t a, uint64_t b, uint64_t n)
{
	uint64_t d=1, A=a;

	do {
		if (b&1)
			d=mulmod64(d, A, n);
		A=mulmod64(A, A, n);
	} while (b>>=1);

	return (uint64_t)d;
}

static inline int straightforward_mr64(const uint64_t bases[], int bases_cnt, uint64_t n)
{
	uint64_t u=n-1;

#ifdef _MSC_VER
	// u will be even, as n is required to be odd
	int t=1, j; u >>= 1;

	while (u % 2 == 0) { // while even
		t++;
		u >>= 1;
	}
#else
	int t = __builtin_ctzll(u), j;
	u >>= t;
#endif

	for (j=0; j<bases_cnt; j++) {
		uint64_t a = bases[j], x;
		int i;

		if (a >= n) a %= n;

		if (a == 0) continue;

		x = modular_exponentiation64(a, u, n);

		if (x == 1 || x == n-1) continue;

		for (i=1; i<t; i++) {
			x=mulmod64(x, x, n);
			if (x == 1)   return 0;
			if (x == n-1) break;
		}

		// if we didn't break, the number is composite
		if (i == t) return 0;
	}

	return 1;
}

#endif // _SPRP64_SF_H_INCLUDED
