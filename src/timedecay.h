/* timedecay.h
 *
 * TODO: 8, 16, or 32 bit representations of time instead of time_t
 *       - i think i can do this by subtracting start time from current time,
 *         checking if this is greater than max value of uint8_t, uint16_t, etc.
 *         if so, it is expired. otherwise start time % (now - start time)
 *         should yield a counter using less space than a time_t. This would
 *         be useful for filters requiring less precision than however many
 *         seconds the filter requires
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
bool timedecay_lookup_time(const timedecay, void *, const size_t, const size_t);
bool timedecay_save(timedecay, const char *);
bool timedecay_load(timedecay *, const char *);

#endif /* TIMEDECAY_H */
