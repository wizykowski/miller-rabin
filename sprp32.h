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
// returns a^-1 mod 2^32
static inline uint32_t modular_inverse32(const uint32_t a)
{
	uint32_t u,x,w,z,q;

	x = 1; z = a;

	q = (-z)/z + 1; // = 2^32 / z
	u = - q; // = -q * x
	w = - q * z; // = b - q * z = 2^32 - q * z

	// after first iteration all variables are 32-bit

	while (w) {
		if (w < z) {
			q = u; u = x; x = q; // swap(u, x)
			q = w; w = z; z = q; // swap(w, z)
		}
		q = w / z;
		u -= q * x;
		w -= q * z;
	}
	
	return x;
}

// returns 2^32 mod n
static inline uint32_t compute_modn32(const uint32_t n)
{
#if 1
	return 4294967296ULL % n; // for 32-bit integer this version is faster
#else
	if (n <= (1U << 31))
		return (((1U << 31) % n) << 1) % n;
	else
		return -n;
#endif
}

#define PRIME 1
#define COMPOSITE 0

static inline int efficient_mr32(const uint32_t bases[], const int cnt, const uint32_t n)
{
	const unsigned npi = modular_inverse32(-((int)n));
	const unsigned r = compute_modn32(n);

	uint32_t u=n-1;
	const uint32_t nr = n-r;

	int t=0;

#ifndef _MSC_VER
	t = __builtin_ctz(u);
	u >>= t;
#else
	while (!(u&1)) { // while even
		t++;
		u >>= 1;
	}
#endif

	int j;
	for (j=0; j<cnt; j++) {
		const uint32_t a = bases[j];

		uint32_t A=((uint64_t)a<<32) % n;
		if (!A) continue; // PRIME in subtest

		uint32_t d=r;

		// compute a^u mod n
		uint32_t u_copy = u;
		do {
			if (u_copy & 1) d=mont_prod32(d, A, n, npi);
			A=mont_square32(A, n, npi);
		} while (u_copy>>=1);

		if (d == r || d == nr) continue; // PRIME in subtest

		int i;
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
