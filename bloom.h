#ifndef BLOOM_H
#define BLOOM_H

#include <time.h>
#include <stdint.h>
#include <stdbool.h>

struct bloom {
	uint32_t size;          /* size of bloom filter */
	uint32_t hashcount;     /* number of hashes per element */
	uint32_t bitmap_size;   /* size of bitmap */
	uint8_t  *bitmap;       /* bitmap of bloom filter */
};

struct timefilter {
	uint32_t size;          /* size of time filter */
	uint32_t hashcount;     /* number of hashes per element */
	uint32_t timeout;       /* number of seconds an element is valid */
	uint32_t filter_size;   /* number of time_t values in time filter*/
	time_t   *filter;       /* array of time_t elements */
};

typedef struct bloom bloomfilter;
typedef struct timefilter timefilter;

bool bloom_init(bloomfilter *, const uint32_t, const float);
void bloom_destroy(bloomfilter);
bool bloom_lookup(const bloomfilter, const uint8_t *, const size_t);
bool bloom_lookup_string(const bloomfilter, const char *);
void bloom_add(bloomfilter, const uint8_t *, const size_t);
void bloom_add_string(bloomfilter, const char *);
bool bloom_save(bloomfilter, const char *);
bool bloom_load(bloomfilter *, const char *);

bool timefilter_init(timefilter *, const uint32_t, const float, const uint32_t);
void timefilter_destroy(timefilter);
void timefilter_add(timefilter, const uint8_t *, const size_t);
void timefilter_add_string(timefilter, const char *);
bool timefilter_lookup(timefilter, const uint8_t *, const size_t);
bool timefilter_lookup_time(timefilter, const uint8_t *, const size_t, const size_t);
bool timefilter_save(timefilter, const char *);
bool timefilter_load(timefilter *, const char *);

#endif /* BLOOM_H */
