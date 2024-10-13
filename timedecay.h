/* timedecay.h
 */
#ifndef TIMEDECAY_H
#define TIMEDECAY_H

typedef struct {
	uint32_t size;          /* size of time filter */
	uint32_t hashcount;     /* number of hashes per element */
	uint32_t timeout;       /* number of seconds an element is valid */
	uint32_t filter_size;   /* number of time_t values in time filter*/
	time_t   *filter;       /* array of time_t elements */
} timedecay;

bool timedecay_init(timedecay *, const uint32_t, const float, const uint32_t);
void timedecay_destroy(timedecay);
void timedecay_add(timedecay, const uint8_t *, const size_t);
void timedecay_add_string(timedecay, const char *);
bool timedecay_lookup(const timedecay, const uint8_t *, const size_t);
bool timedecay_lookup_string(const timedecay, const char *);
bool timedecay_lookup_time(const timedecay, const uint8_t *, const size_t, const size_t);
bool timedecay_save(timedecay, const char *);
bool timedecay_load(timedecay *, const char *);

#endif /* TIMEDECAY_H */
