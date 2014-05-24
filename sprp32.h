#ifndef _SPRP32_H_INCLUDED
#define _SPRP32_H_INCLUDED

#include <stdint.h>

static inline uint32_t mont_prod32(const uint32_t a, const uint32_t b, const uint32_t n, const uint32_t npi)
{
	const uint64_t t = (uint64_t)a*b;
	const uint32_t m = (uint32_t)((uint32_t)t*npi);
	const uint32_t u = (t + (uint64_t)m*n) >> 32; // (t + m*n may overflow)

#ifndef SPRP32_ONE_FREE_BIT
	// overflow fix
	if (u < (t >> 32))
		return (uint32_t)(u-n);
#endif

	return u >= n ? (uint32_t)(u-n) : u;
}

static inline uint32_t mont_square32(const uint32_t a, const uint32_t n, const uint32_t npi)
{
	return mont_prod32(a, a, n, npi);
}

// WARNING: a must be odd
// returns -a^-1 mod 2^32
// method from B. Arazi 'On Primality Testing Using Purely Divisionless Operations'
// The Computer Journal (1994) 37 (3): 219-222, Procedure 5.
// modified to process 4 bits at a time
static inline uint32_t modular_inverse32(const uint32_t a)
{
	uint32_t S = 1, J = 0;
	int i;

	static const char mask[8] = {15, 5, 3, 9, 7, 13, 11, 1};

	const char amask = mask[(a >> 1) & 7];

	for (i = 0; i < 8; i++) {
		int index = (amask * (S & 15)) & 15;

		J |= (uint32_t)index << (4 * i);

		S = (S + a * index) >> 4;
	}

	return J;
}

// returns 2^32 mod n
static inline uint32_t compute_modn32(const uint32_t n)
{
	if (n <= (1U << 31)) {
		uint32_t res = ((1U << 31) % n) << 1;
		
		return res < n ? res : res-n;
	} else
		return -n;
}

#define PRIME 1
#define COMPOSITE 0

static inline int efficient_mr32(const uint32_t bases[], const int cnt, const uint32_t n)
{
	const unsigned npi = modular_inverse32(n);
	const unsigned r = compute_modn32(n);

	uint32_t u=n-1;
	const uint32_t nr = n-r;

	int t=0, j;

#ifndef _MSC_VER
	t = __builtin_ctz(u);
	u >>= t;
#else
	while (!(u&1)) { // while even
		t++;
		u >>= 1;
	}
#endif

	for (j=0; j<cnt; j++) {
		const uint32_t a = bases[j];
		uint32_t d=r, u_copy = u;

		uint32_t A=((uint64_t)a<<32) % n;
		int i;

		if (!A) continue; // PRIME in subtest

		// compute a^u mod n

		do {
			if (u_copy & 1) d=mont_prod32(d, A, n, npi);
			A=mont_square32(A, n, npi);
		} while (u_copy>>=1);

		if (d == r || d == nr) continue; // PRIME in subtest

		for (i=1; i<t; i++) {
			d=mont_square32(d, n, npi);
			if (d == r) return COMPOSITE;
			if (d == nr) break; // PRIME in subtest
		}

		if (i == t)
			return COMPOSITE;
	}

	return PRIME;
}
#undef PRIME
#undef COMPOSITE

#endif // _SPRP32_H_INCLUDED
