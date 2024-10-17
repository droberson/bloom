/* cuckoo.c
 */
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <time.h>

#include "cuckoo.h"
#include "mmh3.h"

bool cuckoo_init(cuckoofilter *cf, size_t num_buckets, size_t bucket_size,
				 size_t max_kicks) {
	cf->num_buckets = num_buckets;
	cf->bucket_size = bucket_size;
	cf->max_kicks   = max_kicks;
	cf->prng_state  = (uint32_t)time(NULL);
	cf->buckets     = (cuckoobucket *)calloc(num_buckets * bucket_size, sizeof(cuckoobucket));
	if (cf->buckets == NULL) {
		return false;
	}

	return true;
}

void cuckoo_destroy(cuckoofilter cf) {
	free(cf.buckets);
}

static uint32_t xorshift32(uint32_t *state) {
	uint32_t x = *state;
	x ^= x << 13;
	x ^= x >> 17;
	x ^= x << 5;
	*state = x;

	return x;
}

bool cuckoo_add(cuckoofilter cf, void *key, size_t len) {
	uint32_t hash          = mmh3_32(key, len, 0);
	uint16_t fingerprint   = (uint16_t)(hash & 0xffff); // lower 16 bits
	size_t   i1            = hash % cf.num_buckets;
	size_t   i2            = (hash >> 16) % cf.num_buckets; // upper 16 bits

	size_t   i1_offset     = i1 * cf.bucket_size;
	size_t   i2_offset     = i2 * cf.bucket_size;

	for (size_t b = 0; b < cf.bucket_size; b++) {
		if (cf.buckets[i1_offset + b].fingerprint == 0) {
			cf.buckets[i1_offset + b].fingerprint = fingerprint;
			return true;
		}
		if (cf.buckets[i2_offset + b].fingerprint == 0) {
			cf.buckets[i2_offset + b].fingerprint = fingerprint;
			return true;
		}
	}

	// Eviction
	size_t index        = (xorshift32(&cf.prng_state) % 2) ? i1 : i2;
	size_t index_offset = index * cf.bucket_size;

	for (size_t kick = 0; kick < cf.max_kicks; kick++) {
		size_t b = xorshift32(&cf.prng_state) % cf.bucket_size;
		uint32_t evicted = cf.buckets[index_offset + b].fingerprint;
		cf.buckets[index_offset + b].fingerprint = fingerprint;
		fingerprint = evicted;
		index = (index ^ fingerprint) % cf.num_buckets;

		// re-insert into new bucket
		for (size_t b = 0; b < cf.bucket_size; b++) {
			if (cf.buckets[index_offset + b].fingerprint == 0) {
				cf.buckets[index_offset + b].fingerprint = fingerprint;
				return true;
			}
		}
	}

	return false; // max kicks reached; insertion failed.
}

bool cuckoo_add_string(cuckoofilter cf, char *key) {
	return cuckoo_add(cf, (uint8_t *)key, strlen(key));
}

bool cuckoo_lookup(cuckoofilter cf, void *key, size_t len) {
	uint32_t hash = mmh3_32(key, len, 0);
	uint16_t fingerprint = (uint16_t)(hash & 0xffff);
	size_t i1 = hash % cf.num_buckets;
	size_t i2 = (hash >> 16) % cf.num_buckets;

	size_t i1_offset = i1 * cf.bucket_size;
	size_t i2_offset = i2 * cf.bucket_size;

	for (size_t b = 0; b < cf.bucket_size; b++) {
		if (cf.buckets[i1_offset + b].fingerprint == fingerprint ||
			cf.buckets[i2_offset + b].fingerprint == fingerprint) {
			return true;
		}
	}

	return false;
}

bool cuckoo_lookup_string(cuckoofilter cf, char *key) {
	return cuckoo_lookup(cf, (uint8_t *)key, strlen(key));
}

bool cuckoo_remove(cuckoofilter cf, void *key, size_t len) {
	uint32_t hash        = mmh3_32(key, len, 0);
	uint16_t fingerprint = (uint16_t)(hash & 0xffff);
	size_t   i1          = hash % cf.num_buckets;
	size_t   i2          = (hash >> 16) % cf.num_buckets;

	size_t   i1_offset = i1 * cf.bucket_size;
	size_t   i2_offset = i2 * cf.bucket_size;

	for (size_t b = 0; b < cf.bucket_size; b++) {
		if (cf.buckets[i1_offset + b].fingerprint == fingerprint) {
			cf.buckets[i1_offset + b].fingerprint = 0;
			return true;
		}
		if (cf.buckets[i2_offset + b].fingerprint == fingerprint) {
			cf.buckets[i2_offset + b].fingerprint = 0;
			return true;
		}
	}

	return false; // probably not in cuckoo filter; remove failed.
}

bool cuckoo_remove_string(cuckoofilter cf, char *key) {
	return cuckoo_remove(cf, (uint8_t *)key, strlen(key));
}

/* TODO
bool cuckoo_save(cuckoofilter cf, const char *path);
bool cuckoo_load(cuckoofilter *cf, const char *path);
*/
