/* tdbloom.h
 * TODO: calculate used/remaining capacity of filters
 * TODO: zero out/clear filters
 */
#ifndef TDBLOOM_H
#define TDBLOOM_H

#include <time.h>
#include <stdint.h>
#include <stdbool.h>

/*
 */
typedef enum {
	TDBF_SUCCESS,
	TDBF_INVALIDTIMEOUT,
	TDBF_OUTOFMEMORY,
	// counter
	TDBF_ERRORCOUNT
} tdbloom_error_t;

const char *tdbloom_errors[] = {
	"Success",
	"Invalid timeout value",
	"Out of memory"
};

/* tdbloom -- time-decaying bloom filter structure
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
} tdbloom;

/* function definitions
 */
tdbloom_error_t  tdbloom_init(tdbloom *,
							  const size_t,
							  const float,
							  const size_t);
void             tdbloom_destroy(tdbloom);
void             tdbloom_add(tdbloom *, void *, const size_t);
void             tdbloom_add_string(tdbloom, const char *);
bool             tdbloom_lookup(const tdbloom, void *, const size_t);
bool             tdbloom_lookup_string(const tdbloom, const char *);
bool             tdbloom_save(tdbloom, const char *);
bool             tdbloom_load(tdbloom *, const char *);
const char      *tdbloom_strerror(tdbloom_error_t);

#endif /* TDBLOOM_H */
