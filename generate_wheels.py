#!/usr/bin/env python3

# this code generates distancewheel and wheeladvance
# used by mr_benchmark.c

import functools, operator

wheel_primes = [2,3,5,7]
only_odd = True # we take only odd numbers into wheels

product = functools.reduce(operator.mul, wheel_primes, 1)

is_divisible = [False for i in range(product)]

for prime in wheel_primes:
	for i in range(0, product, prime):
		is_divisible[i] = True

distancewheel = [0 for i in range(product)]

for i in range(product):
	for j in range(product):
		if not is_divisible[(i+j) % product]:
			distancewheel[i] = j
			break


wheeladvance = [0 for i in range(product)]

for i in range(product):
	if distancewheel[i] == 0:
		wheeladvance[i] = distancewheel[(i+1) % product] + 1
	else:
		wheeladvance[i] = 0

if only_odd:
	distancewheel = [distancewheel[i] for i in range(1, product, 2)]
	wheeladvance  = [ wheeladvance[i] for i in range(1, product, 2)]
	product /= 2

print('#define WHEEL_PRODUCT %d' % product);

print('static const unsigned char distancewheel[WHEEL_PRODUCT] = ')
print('\t{' + ','.join(map(str,distancewheel)) + '};')

print('static const unsigned char wheeladvance[WHEEL_PRODUCT] = ')
print('\t{' + ','.join(map(str,wheeladvance)) + '};')

