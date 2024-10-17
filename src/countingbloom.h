/* countingbloom.h
 */
#ifndef COUNTINGBLOOM_H
#define COUNTINGBLOOM_H

#include <stdint.h>
#include <stdbool.h>

/* countingbloomfilter -- structure for a counting bloom filter
 */
typedef struct {
	size_t size;                /* size of counting bloom filter */
	size_t hashcount;           /* number of hashes per element */
	size_t countermap_size;     /* size of map */
	uint8_t *countermap;        /* map of counting bloom filter */
} countingbloomfilter;

/* function declarations
 */
bool   countingbloom_init(countingbloomfilter *, const size_t, const float);
void   countingbloom_destroy(countingbloomfilter);
size_t countingbloom_count(const countingbloomfilter, void *, size_t);
size_t countingbloom_count_string(const countingbloomfilter, char *);
bool   countingbloom_lookup(const countingbloomfilter, void *, const size_t);
bool   countingbloom_lookup_string(const countingbloomfilter, const char *);
void   countingbloom_add(countingbloomfilter, void *, const size_t);
void   countingbloom_add_string(countingbloomfilter, const char *);
void   countingbloom_remove(countingbloomfilter, void *, const size_t);
void   countingbloom_remove_string(countingbloomfilter, const char *);
bool   countingbloom_save(countingbloomfilter, const char *);
bool   countingbloom_load(countingbloomfilter *, const char *);

#endif /* COUNTINGBLOOM_H */
