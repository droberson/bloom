/* countingbloom.h
 */
#ifndef COUNTINGBLOOM_H
#define COUNTINGBLOOM_H

#include <stdint.h>
#include <stdbool.h>

/* counter_size -- used for setting appropriately-sized counters, which can
                   result in a reduced memory footprint if smaller counts are
				   expected.
*/
typedef enum {
	COUNTER_8BIT,
	COUNTER_16BIT,
	COUNTER_32BIT,
	COUNTER_64BIT
} counter_size;

/* countingbloomfilter -- structure for a counting bloom filter
 */
typedef struct {
	uint64_t      size;              /* size of counting bloom filter */
	uint64_t      hashcount;         /* number of hashes per element */
	uint64_t      countermap_size;   /* size of map */
	counter_size  csize;             /* size of counter: 8, 16, 32, 64 bit */
	void         *countermap;        /* map of counting bloom filter */
} countingbloomfilter;

/* function declarations
 */
bool   countingbloom_init(countingbloomfilter *, const size_t, const float, counter_size);
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
