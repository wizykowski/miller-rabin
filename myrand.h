#ifndef _MYRAND_H_INCLUDED
#define _MYRAND_H_INCLUDED

// xorshift random number generator by George Marsaglia
// it has period of 2^128 - 1
// see http://en.wikipedia.org/wiki/Xorshift

uint32_t __rand_x = 123456789;
uint32_t __rand_y = 362436069;
uint32_t __rand_z = 521288629;
uint32_t __rand_w = 88675123;

void myseed()
{
	__rand_x = 123456789;
	__rand_y = 362436069;
	__rand_z = 521288629;
	__rand_w = 88675123;
}

uint32_t myrand32()
{
	uint32_t t;

	t = __rand_x ^ (__rand_x << 11);
	__rand_x = __rand_y; __rand_y = __rand_z; __rand_z = __rand_w;
	return __rand_w = __rand_w ^ (__rand_w >> 19) ^ (t ^ (t >> 8));
}

uint64_t myrand64()
{
	return (((uint64_t)myrand32()) << 32) + myrand32();
}

#endif // _MYRAND_H_INCLUDED

