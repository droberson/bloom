#ifndef BLOOM_H
#define BLOOM_H

#include <time.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct {
	uint32_t size;          /* size of bloom filter */
	uint32_t hashcount;     /* number of hashes per element */
	uint32_t bitmap_size;   /* size of bitmap */
	uint8_t  *bitmap;       /* bitmap of bloom filter */
} bloomfilter;

bool bloom_init(bloomfilter *, const uint32_t, const float);
void bloom_destroy(bloomfilter);
bool bloom_lookup(const bloomfilter, const uint8_t *, const size_t);
bool bloom_lookup_string(const bloomfilter, const char *);
void bloom_add(bloomfilter, const uint8_t *, const size_t);
void bloom_add_string(bloomfilter, const char *);
bool bloom_save(bloomfilter, const char *);
bool bloom_load(bloomfilter *, const char *);

#endif /* BLOOM_H */
