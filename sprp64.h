#ifndef _SPRP64_H_INCLUDED
#define _SPRP64_H_INCLUDED

#include <stdint.h>

#include "mulmod64.h"

#if 0 && (defined(_WIN64) || defined(__amd64__))
static inline uint64_t mont_prod64(const uint64_t a, const uint64_t b, const uint64_t n, const uint64_t npi)
{
	const unsigned __int128 t = (unsigned __int128)a*b;
	const uint64_t m = (uint64_t)((uint64_t)t*npi);
	const uint64_t u = (t + (unsigned __int128)m*n) >> 64; // (t + m*n may overflow)

#ifndef SPRP64_ONE_FREE_BIT
	// overflow fix
	if (u < (t >> 64))
		return (uint64_t)(u-n);
#endif

	return u >= n ? (uint64_t)(u-n) : u;
}
#else
static inline uint64_t mont_prod64(uint64_t a, uint64_t b, uint64_t n, uint64_t npi)
{
	uint64_t t_hi, t_lo, m, mn_hi, mn_lo, u;
	int carry;

	// t_hi * 2^64 + t_lo = a*b
	t_lo = mul128(a, b, &t_hi);
	
	m = t_lo * npi;
	
	// mn_hi * 2^64 + mn_lo = m*n
	mn_lo = mul128(m, n, &mn_hi);

	carry = t_lo + mn_lo < t_lo ? 1 : 0;

	u = t_hi + mn_hi + carry;
	if (u < t_hi) return u-n;
	
	return u >= n ? u-n : u;
}
#endif

static inline uint64_t mont_square64(const uint64_t a, const uint64_t n, const uint64_t npi)
{
	return mont_prod64(a, a, n, npi);
}

// WARNING: a must be odd
// returns -a^-1 mod 2^64
// method from B. Arazi 'On Primality Testing Using Purely Divisionless Operations'
// The Computer Journal (1994) 37 (3): 219-222, Procedure 5.
// modified to process 4 bits at a time
static inline uint64_t modular_inverse64(const uint64_t a)
{
	uint64_t S = 1, J = 0;
	int i;

	static const char mask[8][16] = {{0, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1},
		{0, 5, 10, 15, 4, 9, 14, 3, 8, 13, 2, 7, 12, 1, 6, 11},
		{0, 3, 6, 9, 12, 15, 2, 5, 8, 11, 14, 1, 4, 7, 10, 13},
		{0, 9, 2, 11, 4, 13, 6, 15, 8, 1, 10, 3, 12, 5, 14, 7},
		{0, 7, 14, 5, 12, 3, 10, 1, 8, 15, 6, 13, 4, 11, 2, 9},
		{0, 13, 10, 7, 4, 1, 14, 11, 8, 5, 2, 15, 12, 9, 6, 3},
		{0, 11, 6, 1, 12, 7, 2, 13, 8, 3, 14, 9, 4, 15, 10, 5},
		{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15}};

	const char *amask = mask[(a >> 1) & 7];

	for (i = 0; i < 16; i++) {
		int index = amask[S & 15];

		J |= (uint64_t)index << (4 * i);

		S = (S + a * index) >> 4;
	}

	return J;
}

// returns 2^64 mod n
static inline uint64_t compute_modn64(const uint64_t n)
{
	if (n <= (1ULL << 63)) {
		uint64_t res = ((1ULL << 63) % n) << 1;
		
		return res < n ? res : res-n;
	} else
		return -n;
}

static inline uint64_t compute_a_times_2_64_mod_n(const uint64_t a, const uint64_t n, const uint64_t r)
{
#if defined(_WIN64) || defined(__amd64__)
#if 0
	return ((unsigned __int128)a<<64) % n;
#else
	return mulmod64(a, r, n);
#endif
#else
	uint64_t res = a % n;
	int i;
	for (i=64; i>0; i--) {
		if (res < (1ULL << 63)) {
			res <<= 1; if (res >= n) res -= n;
		} else { // n > res >= 2^63
			res <<= 1;
			if (res >= n) res -= n;
		
			// res2 = (res + r) % n
			uint64_t res2 = res + r;
			if (res2 < res) res2 -= n;
			
			res = res2;
		}
	}

	return res;
#endif
}

#define PRIME 1
#define COMPOSITE 0

static inline int efficient_mr64(const uint64_t bases[], const int cnt, const uint64_t n)
{
	const uint64_t npi = modular_inverse64(n);
	const uint64_t r = compute_modn64(n);

	uint64_t u=n-1;
	const uint64_t nr = n-r;	

	int t=0, j;

#ifndef _MSC_VER
	t = __builtin_ctzll(u);
	u >>= t;
#else
	while (!(u&1)) { // while even
		t++;
		u >>= 1;
	}
#endif

	for (j=0; j<cnt; j++) {
		const uint64_t a = bases[j];

		uint64_t A=compute_a_times_2_64_mod_n(a, n, r); // a * 2^64 mod n
		uint64_t d=r, u_copy=u;
		int i;

		if (!A) continue; // PRIME in subtest

		// compute a^u mod n
		do {
			if (u_copy & 1) d=mont_prod64(d, A, n, npi);
			A=mont_square64(A, n, npi);
		} while (u_copy>>=1);

		if (d == r || d == nr) continue; // PRIME in subtest

		for (i=1; i<t; i++) {
			d=mont_square64(d, n, npi);
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

#endif // _SPRP64_H_INCLUDED
