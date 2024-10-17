/* timedecay.h
 * TODO: calculate used/remaining capacity of filters
 */
#ifndef TIMEDECAY_H
#define TIMEDECAY_H

#include <time.h>
#include <stdint.h>
#include <stdbool.h>

/* timedecay -- time-decaying bloom filter structure
 */
typedef struct {
	size_t  size;          /* size of time filter */
	size_t  hashcount;     /* number of hashes per element */
	size_t  timeout;       /* number of seconds an element is valid */
	size_t  filter_size;   /* number of time_t values in time filter */
	time_t  start_time;    /* time of filter initialization */
	size_t  expected;      /* expected number of elements */
	float   accuracy;      /* desired margin of error */
	size_t  max_time;      /* maximum value of timestamp */
	int     bytes;         /* byte size of timestamps */
	void   *filter;        /* array of time_t elements */
} timedecay;

/* function definitions
 */
bool timedecay_init(timedecay *, const size_t, const float, const size_t);
void timedecay_destroy(timedecay);
void timedecay_add(timedecay *, void *, const size_t);
void timedecay_add_string(timedecay, const char *);
bool timedecay_lookup(const timedecay, void *, const size_t);
bool timedecay_lookup_string(const timedecay, const char *);
bool timedecay_save(timedecay, const char *);
bool timedecay_load(timedecay *, const char *);

#endif /* TIMEDECAY_H */
