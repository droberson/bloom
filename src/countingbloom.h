/* countinbloom.h
 */
#ifndef COUNTINGBLOOM_H
#define COUNTINGBLOOM_H

#include <stdint.h>
#include <stdbool.h>

// TODO: 16 bit counters?
typedef struct {
	size_t size;                /* size of counting bloom filter */
	size_t hashcount;           /* number of hashes per element */
	size_t countermap_size;     /* size of map */
	uint8_t *countermap;        /* map of counting bloom filter */
} countingbloomfilter;

/* function declarations
 * TODO add countingbloom_count(countingbloomfilter, element) -- returns count
 */
bool countingbloom_init(countingbloomfilter *, const size_t, const float);
void countingbloom_destroy(countingbloomfilter);
bool countingbloom_lookup(const countingbloomfilter, void *, const size_t);
bool countingbloom_lookup_string(const countingbloomfilter, const char *);
void countingbloom_add(countingbloomfilter, void *, const size_t);
void countingbloom_add_string(countingbloomfilter, const char *);
void countingbloom_remove(countingbloomfilter, void *, const size_t);
void countingbloom_remove_string(countingbloomfilter, const char *);
bool countingbloom_save(countingbloomfilter, const char *);
bool countingbloom_load(countingbloomfilter *, const char *);

#endif /* COUNTINGBLOOM_H */
