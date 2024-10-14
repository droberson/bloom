#ifndef BLOOM_H
#define BLOOM_H

#include <stdint.h>
#include <stdbool.h>

// TODO: implement 32, 64 bit functions?
#if UINTPTR_MAX == 0xffffffff
typedef uint32_t (*hash_func32_t)(const void *key, size_t len, uint32_t seed);
#define DEFAULT_HASH_FUNC_BITS
#else
typedef uint64_t (*hash_func64_t)(const void *key, size_t len, uint64_t seed);
#endif /* UINTPTR_MAX */

typedef struct {
	uint32_t size;              /* size of bloom filter */
	uint32_t hashcount;         /* number of hashes per element */
	uint32_t bitmap_size;       /* size of bitmap */
	uint8_t  *bitmap;           /* bitmap of bloom filter */
//	hash_funct_t hash_function; /* hash function to use. default: mmh3 */
} bloomfilter;

bool bloom_init(bloomfilter *, const uint64_t, const float);
void bloom_destroy(bloomfilter);
bool bloom_lookup(const bloomfilter, const uint8_t *, const size_t);
bool bloom_lookup_string(const bloomfilter, const char *);
void bloom_add(bloomfilter, const uint8_t *, const size_t);
void bloom_add_string(bloomfilter, const char *);
bool bloom_save(bloomfilter, const char *);
bool bloom_load(bloomfilter *, const char *);

#endif /* BLOOM_H */
