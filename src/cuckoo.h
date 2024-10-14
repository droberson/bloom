#ifndef CUCKOO_H
#define CUCKOO_H

#include <stdint.h>
#include <stdbool.h>

typedef struct {
	uint64_t fingerprint;
} cuckoobucket;

typedef struct {
	cuckoobucket *buckets;
	size_t num_buckets;
	size_t bucket_size; // 2, 4, or 8
	size_t max_kicks;
} cuckoofilter;

// TODO remove parameter names, const where appropriate
bool cuckoo_init(cuckoofilter *, size_t, size_t, size_t);
void cuckoo_destroy(cuckoofilter);
bool cuckoo_add(cuckoofilter cf, const uint8_t *key, size_t len);
bool cuckoo_lookup(cuckoofilter cf, const uint8_t *key, size_t len);
bool cuckoo_remove(cuckoofilter cf, const uint8_t *key, size_t len);
bool cuckoo_save(cuckoofilter cf, const char *path);
bool cuckoo_load(cuckoofilter *cf, const char *path);

#endif /* CUCKOO_H */
