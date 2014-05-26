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
// modified to process 4 or 8 bits at a time
static inline uint64_t modular_inverse64(const uint64_t a)
{
#if 0
	uint64_t S = 1, J = 0;
	int i;

	static const char mask[8] = {15, 5, 3, 9, 7, 13, 11, 1};

	const char amask = mask[(a >> 1) & 7];

	for (i = 0; i < 16; i++) {
		int index = (amask * (S & 15)) & 15;

		J |= (uint64_t)index << (4 * i);

		S = (S + a * index) >> 4;
	}

	return J;
#else
	uint64_t S = 1;

	static const char mask[128] = {255,85,51,73,199,93,59,17,15,229,195,89,215,237,203,33,31,117,83,105,231,125,91,49,47,5,227,121,247,13,235,65,63,149,115,137,7,157,123,81,79,37,3,153,23,45,11,97,95,181,147,169,39,189,155,113,111,69,35,185,55,77,43,129,127,213,179,201,71,221,187,145,143,101,67,217,87,109,75,161,159,245,211,233,103,253,219,177,175,133,99,249,119,141,107,193,191,21,243,9,135,29,251,209,207,165,131,25,151,173,139,225,223,53,19,41,167,61,27,241,239,197,163,57,183,205,171,1};

	const char amask = mask[(a >> 1) & 127];

	/* the code below is the following loop unrolled manually:
	for (i = 0; i < 8; i++) {
		int index = (amask * (S & 255)) & 255;
		J |= (uint64_t)index << (8 * i);
		S = (S + a * index) >> 8;
	} */

	int index = (amask * (S & 255)) & 255;
	uint64_t J = index;
	S = (S + a * index) >> 8;

	index = (amask * (S & 255)) & 255;
	J |= (uint64_t)index << 8;
	S = (S + a * index) >> 8;

	index = (amask * (S & 255)) & 255;
	J |= (uint64_t)index << 16;
	S = (S + a * index) >> 8;

	index = (amask * (S & 255)) & 255;
	J |= (uint64_t)index << 24;
	uint32_t S2 = (S + a * index) >> 8;

	index = (amask * (S2 & 255)) & 255;
	J |= (uint64_t)index << 32;
	S2 = (S2 + a * index) >> 8;

	index = (amask * (S2 & 255)) & 255;
	J |= (uint64_t)index << 40;
	S2 = (S2 + a * index) >> 8;

	index = (amask * (S2 & 255)) & 255;
	J |= (uint64_t)index << 48;
	S2 = (S2 + a * index) >> 8;

	index = (amask * (S2 & 255)) & 255;
	J |= (uint64_t)index << 56;

	return J;
#endif
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
