/* cuckoo.c
 */
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <time.h>

#include "cuckoo.h"
#include "mmh3.h"


static uint32_t seed_xorshift32() {
	uint32_t        seed;
	struct timespec ts;

	clock_gettime(CLOCK_MONOTONIC, &ts);

	seed = (uint32_t)(ts.tv_sec ^ ts.tv_nsec);

	return seed;
}

static uint32_t xorshift32(uint32_t *state) {
	uint32_t x = *state;
	x ^= x << 13;
	x ^= x >> 17;
	x ^= x << 5;
	*state = x;

	return x;
}

bool cuckoo_init(cuckoofilter *cf, size_t num_buckets, size_t bucket_size,
				 size_t max_kicks) {
	cf->num_buckets      = num_buckets;
	cf->bucket_size      = bucket_size;
	cf->max_kicks        = max_kicks;
	cf->prng_state       = seed_xorshift32();
	cf->total_insertions = 0;
	cf->evictions        = 0;

	cf->buckets          = (cuckoobucket *)calloc(num_buckets * bucket_size, sizeof(cuckoobucket));
	if (cf->buckets == NULL) {
		return false;
	}

	cf->bucket_insertions = calloc(num_buckets, sizeof(size_t));
	if (cf->bucket_insertions == NULL) {
		free(cf->buckets);
		return false;
	}

	return true;
}

void cuckoo_destroy(cuckoofilter cf) {
	free(cf.bucket_insertions);
	free(cf.buckets);
}

static bool cuckoo_add_fingerprint(cuckoofilter cf, size_t bucket_index, size_t offset, uint16_t fingerprint) {
	for (size_t b = 0; b < cf.bucket_size; b++) {
		if (cf.buckets[offset + b].fingerprint == 0) {
			cf.buckets[offset + b].fingerprint = fingerprint;
			cf.bucket_insertions[bucket_index] += 1;
			cf.total_insertions += 1;
			return true;
		}
	}

	return false;
}

bool cuckoo_add(cuckoofilter cf, void *key, size_t len) {
	uint32_t hash          = mmh3_32(key, len, 0);
	uint16_t fingerprint   = (uint16_t)(hash & 0xffff); // lower 16 bits
	size_t   i1            = hash % cf.num_buckets;
	size_t   i2            = (i1 ^ (fingerprint >> 1)) % cf.num_buckets;
	size_t   i1_offset     = i1 * cf.bucket_size;
	size_t   i2_offset     = i2 * cf.bucket_size;

	if (cuckoo_add_fingerprint(cf, i1, i1_offset, fingerprint) ||
		cuckoo_add_fingerprint(cf, i2, i2_offset, fingerprint)) {
		return true;
	}

	// Eviction
	size_t index        = (xorshift32(&cf.prng_state) % 2) ? i1 : i2;
	size_t index_offset = index * cf.bucket_size;

	for (size_t kick = 0; kick < cf.max_kicks; kick++) {
		size_t b = xorshift32(&cf.prng_state) % cf.bucket_size;
		uint32_t evicted = cf.buckets[index_offset + b].fingerprint;
		cf.buckets[index_offset + b].fingerprint = fingerprint;
		fingerprint = evicted;

		if (cf.bucket_insertions[index] > 0) {
			cf.bucket_insertions[index] -= 1;
		}

		// re-insert into new bucket
		index = (index ^ fingerprint) % cf.num_buckets;
		size_t new_index_offset = index * cf.bucket_size;
		if (cuckoo_add_fingerprint(cf, index, new_index_offset, fingerprint)) {
			return true;
		}
	}

	cf.evictions += 1;
	return false; // max kicks reached; insertion failed.
}

bool cuckoo_add_string(cuckoofilter cf, char *key) {
	return cuckoo_add(cf, key, strlen(key));
}

bool cuckoo_lookup(cuckoofilter cf, void *key, size_t len) {
	uint32_t hash        = mmh3_32(key, len, 0);
	uint16_t fingerprint = (uint16_t)(hash & 0xffff);
	size_t   i1          = hash % cf.num_buckets;
	size_t   i2          = (i1 ^ (fingerprint >> 1)) % cf.num_buckets;

	size_t i1_offset     = i1 * cf.bucket_size;
	size_t i2_offset     = i2 * cf.bucket_size;

	for (size_t b = 0; b < cf.bucket_size; b++) {
		if (cf.buckets[i1_offset + b].fingerprint == fingerprint ||
			cf.buckets[i2_offset + b].fingerprint == fingerprint) {
			return true;
		}
	}

	return false;
}

bool cuckoo_lookup_string(cuckoofilter cf, char *key) {
	return cuckoo_lookup(cf, key, strlen(key));
}

static bool cuckoo_remove_fingerprint(cuckoofilter cf, size_t bucket_index, size_t offset, uint16_t fingerprint) {
	for (size_t b = 0; b < cf.bucket_size; b++) {
		if (cf.buckets[offset + b].fingerprint == fingerprint) {
			cf.buckets[offset + b].fingerprint = 0;

			if (cf.bucket_insertions[bucket_index] > 0) {
				cf.bucket_insertions[bucket_index] -= 1;
			}

			if (cf.total_insertions > 0) {
				cf.total_insertions -= 1;
			}

			return true;
		}
	}

	return false;
}

bool cuckoo_remove(cuckoofilter cf, void *key, size_t len) {
	uint32_t hash        = mmh3_32(key, len, 0);
	uint16_t fingerprint = (uint16_t)(hash & 0xffff);
	size_t   i1          = hash % cf.num_buckets;
	size_t   i2          = (i1 ^ (fingerprint >> 1)) % cf.num_buckets;

	size_t   i1_offset   = i1 * cf.bucket_size;
	size_t   i2_offset   = i2 * cf.bucket_size;

	if (cuckoo_remove_fingerprint(cf, i1, i1_offset, fingerprint) ||
		cuckoo_remove_fingerprint(cf, i2, i2_offset, fingerprint)) {
		return true;
	}

	return false; // probably not in cuckoo filter; remove failed.
}

bool cuckoo_remove_string(cuckoofilter cf, char *key) {
	return cuckoo_remove(cf, key, strlen(key));
}

/* TODO
bool cuckoo_save(cuckoofilter cf, const char *path);
bool cuckoo_load(cuckoofilter *cf, const char *path);
*/
