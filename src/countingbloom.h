#ifndef COUNTINGBLOOM_H
#define COUNTINGBLOOM_H

#include <stdint.h>
#include <stdbool.h>

// TODO: 16 bit counters?
typedef struct {
	uint64_t size;              /* size of counting bloom filter */
	uint64_t hashcount;         /* number of hashes per element */
	uint64_t countermap_size;   /* size of map */
	uint8_t *countermap;        /* map of counting bloom filter */
} countingbloomfilter;

bool countingbloom_init(countingbloomfilter *, const uint32_t, const float);
void countingbloom_destroy(countingbloomfilter);
bool countingbloom_lookup(const countingbloomfilter, const uint8_t *, const size_t);
bool countingbloom_lookup_string(const countingbloomfilter, const char *);
void countingbloom_add(countingbloomfilter, const uint8_t *, const size_t);
void countingbloom_add_string(countingbloomfilter, const char *);
void countingbloom_remove(countingbloomfilter, const uint8_t *, const size_t);
void countingbloom_remove_string(countingbloomfilter, const char *);
bool countingbloom_save(countingbloomfilter, const char *);
bool countingbloom_load(countingbloomfilter *, const char *);

#endif /* COUNTINGBLOOM_H */
