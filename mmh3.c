#include <string.h>
#include <stdint.h>

#include "mmh3.h"

/* mmh3() - murmur3 hash.
 *
 * MurmurHash3 hash function (32-bit). This was dapted from a code snippet
 * found on Stack Overflow. I am not able to find the source. If anyone knows
 * who wrote this, please let me know so I can credit them.
 *
 * Original MurmurHash3 algorithm by Austin Appleby.
 * See: https://github.com/aappleby/smhasher for details.
 *
 * Args:
 *      key  - bytes to hash
 *      len  - length of key
 *      seed - seed value for the hash function
 *
 * Returns:
 *      32 bit unsigned integer hash value of `key`
 */
uint32_t mmh3(const uint8_t *key, const uint32_t len, const uint32_t seed) {
	uint32_t        c1 = 0xcc9e2d51;
	uint32_t        c2 = 0x1b873593;
	uint32_t        r1 = 15;
	uint32_t        r2 = 13;
	uint32_t        m = 5;
	uint32_t        n = 0xe6546b64;
	uint32_t        h = seed;
	uint32_t        k = 0;
	uint8_t         *d = (uint8_t *)key; // 32 bit extract from `key'
	const uint32_t  *chunks = NULL;
	const uint8_t   *tail = NULL; // tail - last 8 bytes
	int             i = 0;
	int             l = len / 4; // chunk length


	chunks = (const uint32_t *) (d + l * 4); // body
	tail = (const uint8_t *) (d + l * 4); // last 8 byte chunk of `key'

	// for each 4 byte chunk of `key'
	for (i = -l; i != 0; ++i) {
		// next 4 byte chunk of `key'
		k = chunks[i];

		// encode next 4 byte chunk of `key'
		k *= c1;
		k = (k << r1) | (k >> (32 - r1));
		k *= c2;

		// append to hash
		h ^= k;
		h = (h << r2) | (h >> (32 - r2));
		h = h * m + n;
	}

	k = 0;

	// remainder
	switch (len & 3) {
	case 3:
		k ^= (tail[2] << 16);
	case 2:
		k ^= (tail[1] << 8);
	case 1:
		k ^= tail[0];
		k *= c1;
		k = (k << r1) | (k >> (32 - r1));
		k *= c2;
		h ^= k;
	}

	h ^= len;

	h ^= (h >> 16);
	h *= 0x85ebca6b;
	h ^= (h >> 13);
	h *= 0xc2b2ae35;
	h ^= (h >> 16);

	return h;
}

/* mmh3_string() -- calculate mmh3 hash of a string.
 *
 * Args:
 *     element - string to hash
 *     seed    - seed value to use with mmh3
 *
 * Returns:
 *     uint32_t containing calculated mmh3 hash
 */
uint32_t mmh3_string(const char *element, const uint32_t seed) {
	return mmh3(element, strlen((char *)element), seed);
}
