
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "cuckoo.h"
#include "mmh3.h"

bool cuckoo_init(cuckoofilter *cf,
				 size_t num_buckets,
				 size_t bucket_size,
				 size_t max_kicks) {
	cf->num_buckets = num_buckets;
	cf->bucket_size = bucket_size;
	cf->max_kicks   = max_kicks;
	cf->buckets     = (cuckoobucket *)calloc(num_buckets * bucket_size,
											 sizeof(cuckoobucket));
	if (!cf->buckets) {
		return false;
	}

	return true;
}

void cuckoo_destroy(cuckoofilter cf) {
	free(cf.buckets);
}


bool cuckoo_add(cuckoofilter cf, const uint8_t *key, size_t len) {
	uint64_t hash = mmh3_64(key, len, 0);
	size_t i1 = hash % cf.num_buckets;
	size_t i2 = (i1 ^ (hash >> 32)) % cf.num_buckets;

	for (size_t b = 0; b < cf.bucket_size; b++) {
		if (cf.buckets[i1 * cf.bucket_size + b].fingerprint == 0) {
			cf.buckets[i1 * cf.bucket_size + b].fingerprint = hash;
			return true;
		}
		if (cf.buckets[i2 * cf.bucket_size + b].fingerprint == 0) {
			cf.buckets[i2 * cf.bucket_size + b].fingerprint = hash;
			return true;
		}
	}

	size_t index = (rand() % 2) ? i1 : i2;
	for (size_t kick = 0; kick < cf.max_kicks; kick++) {
		size_t b = rand() % cf.bucket_size;
		uint64_t evicted = cf.buckets[index * cf.bucket_size + b].fingerprint;
		cf.buckets[index * cf.bucket_size + b].fingerprint = hash;
		hash = evicted;
		index = (index ^ (hash >> 32)) % cf.num_buckets;
	}

	return false; // max kicks reached; insertion failed.
}


bool cuckoo_lookup(cuckoofilter cf, const uint8_t *key, size_t len) {
	uint64_t hash = mmh3_64(key, len, 0);
	size_t i1 = hash % cf.num_buckets;
	size_t i2 = (i1 ^ (hash >> 32)) % cf.num_buckets;

	for (size_t b = 0; b < cf.bucket_size; b++) {
		if (cf.buckets[i1 * cf.bucket_size + b].fingerprint == hash ||
			cf.buckets[i2 * cf.bucket_size + b].fingerprint == hash) {
			return true;
		}
	}

	return false;
}


bool cuckoo_remove(cuckoofilter cf, const uint8_t *key, size_t len) {
	uint64_t hash = mmh3_64(key, len, 0);
	size_t i1 = hash % cf.num_buckets;
	size_t i2 = (i1 ^ (hash >> 32)) % cf.num_buckets;

	for (size_t b = 0; b < cf.bucket_size; b++) {
		if (cf.buckets[i1 * cf.bucket_size + b].fingerprint == hash) {
			cf.buckets[i1 * cf.bucket_size + b].fingerprint = 0;
			return true;
		}
		if (cf.buckets[i2 * cf.bucket_size + b].fingerprint == hash) {
			cf.buckets[i2 * cf.bucket_size + b].fingerprint = 0;
			return true;
		}
	}

	return false; // probably not in cuckoo filter; remove failed.
}

/*
bool cuckoo_save(cuckoofilter cf, const char *path);
bool cuckoo_load(cuckoofilter *cf, const char *path);
*/
