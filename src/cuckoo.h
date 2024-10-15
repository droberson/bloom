/* cuckoo.h
 */
#ifndef CUCKOO_H
#define CUCKOO_H

#include <stdint.h>
#include <stdbool.h>

/* cuckoobucket -- typedef for cuckoo filter bucket
 */
typedef struct {
	uint64_t fingerprint;
} cuckoobucket;

/* cuckoofilter -- cuckoo filter structure
 */
typedef struct {
	cuckoobucket *buckets;
	size_t num_buckets;
	size_t bucket_size; // 2, 4, or 8
	size_t max_kicks;
} cuckoofilter;

/* function definitions
 * TODO cuckoo_add_string, cuckoo_lookup_string, cuckoo_remove_string
 */
bool cuckoo_init(cuckoofilter *, size_t, size_t, size_t);
void cuckoo_destroy(cuckoofilter);
bool cuckoo_add(cuckoofilter, void *, size_t);
bool cuckoo_lookup(cuckoofilter, void *, size_t);
bool cuckoo_remove(cuckoofilter, void *, size_t);
bool cuckoo_save(cuckoofilter, const char *);
bool cuckoo_load(cuckoofilter *, const char *);

#endif /* CUCKOO_H */
