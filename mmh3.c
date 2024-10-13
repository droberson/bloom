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
	return mmh3((uint8_t *)element, strlen((char *)element), seed);
}

/* mmh3_128() -- calculate 128 bit mmh3 hash.
 *
 * Args:
 *     key
 *     len
 *     seed
 *     out
 *
 * Returns:
 *     Nothing. Output is written to 'out'.
 */
void mmh3_128(const void *key, const size_t len, const uint64_t seed, uint64_t *out) {
    const uint64_t  c1 = 0x87c37b91114253d5;
    const uint64_t  c2 = 0x4cf5ad432745937f;
    const uint8_t   *data = (const uint8_t *)key;
    const size_t    nblocks = len / 16;
    uint64_t        h1 = seed;
    uint64_t        h2 = seed;
    const uint64_t  *blocks = (const uint64_t *)(data);

    // Process the data in 128-bit chunks
    for (size_t i = 0; i < nblocks; i++) {
        uint64_t k1 = blocks[i * 2 + 0];
        uint64_t k2 = blocks[i * 2 + 1];

        k1 *= c1; k1 = (k1 << 31) | (k1 >> 33); k1 *= c2; h1 ^= k1;
        h1 = (h1 << 27) | (h1 >> 37); h1 += h2; h1 = h1 * 5 + 0x52dce729;

        k2 *= c2; k2 = (k2 << 33) | (k2 >> 31); k2 *= c1; h2 ^= k2;
        h2 = (h2 << 31) | (h2 >> 33); h2 += h1; h2 = h2 * 5 + 0x38495ab5;
    }

    // Process the remaining bytes
    const uint8_t *tail = (const uint8_t *)(data + nblocks * 16);
    uint64_t k1 = 0;
    uint64_t k2 = 0;

    switch (len & 15) {
        case 15: k2 ^= ((uint64_t)tail[14]) << 48;
        case 14: k2 ^= ((uint64_t)tail[13]) << 40;
        case 13: k2 ^= ((uint64_t)tail[12]) << 32;
        case 12: k2 ^= ((uint64_t)tail[11]) << 24;
        case 11: k2 ^= ((uint64_t)tail[10]) << 16;
        case 10: k2 ^= ((uint64_t)tail[9]) << 8;
        case  9: k2 ^= ((uint64_t)tail[8]) << 0;
                 k2 *= c2; k2 = (k2 << 33) | (k2 >> 31); k2 *= c1; h2 ^= k2;

        case  8: k1 ^= ((uint64_t)tail[7]) << 56;
        case  7: k1 ^= ((uint64_t)tail[6]) << 48;
        case  6: k1 ^= ((uint64_t)tail[5]) << 40;
        case  5: k1 ^= ((uint64_t)tail[4]) << 32;
        case  4: k1 ^= ((uint64_t)tail[3]) << 24;
        case  3: k1 ^= ((uint64_t)tail[2]) << 16;
        case  2: k1 ^= ((uint64_t)tail[1]) << 8;
        case  1: k1 ^= ((uint64_t)tail[0]) << 0;
                 k1 *= c1; k1 = (k1 << 31) | (k1 >> 33); k1 *= c2; h1 ^= k1;
    }

    // Finalization
    h1 ^= len; h2 ^= len;

    h1 += h2;
    h2 += h1;

    h1 ^= h1 >> 33;
    h1 *= 0xff51afd7ed558ccd;
    h1 ^= h1 >> 33;
    h1 *= 0xc4ceb9fe1a85ec53;
    h1 ^= h1 >> 33;

    h2 ^= h2 >> 33;
    h2 *= 0xff51afd7ed558ccd;
    h2 ^= h2 >> 33;
    h2 *= 0xc4ceb9fe1a85ec53;
    h2 ^= h2 >> 33;

    h1 += h2;
    h2 += h1;

    out[0] = h1;
    out[1] = h2;
}
