#include "shannon.h"

#include <math.h>
#include <stdint.h>
#include <string.h>

static size_t _freqs[UINT8_MAX];

void shannon_init(void)
{
	memset(_freqs, 0x00, sizeof(size_t) * UINT8_MAX);
}

void shannon_update(void *src, size_t len)
{
	for (size_t i = 0; i < len; ++i)
		++_freqs[*((uint8_t *)src + i)];
}

double shannon_entropy(void)
{
	size_t n = 0;
	double p, e = 0.0;

	for (uint_fast16_t i = 0; i < UINT8_MAX; ++i)
		n += _freqs[i];

	for (uint_fast16_t i = 0; i < UINT8_MAX; ++i) {
		if (_freqs[i]) {
			p = (double)_freqs[i] / (double)n;
			e -= p * log2(p);
		}
	}

	return e;
}
