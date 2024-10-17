/* timedecay.c
 */

#include <time.h>
#include <math.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <limits.h>

#include "timedecay.h"
#include "mmh3.h"

/* ideal_size() - calculate ideal size of a filter
 *
 * Args:
 *     expected - maximum expected number of elements
 *     accuracy - margin of error. ex: use 0.01 if you want 99.99% accuracy
 *
 * Returns:
 *     unsigned integer
 */
static uint64_t ideal_size(const uint64_t expected, const float accuracy) {
	return -(expected * log(accuracy) / pow(log(2.0), 2));
}

/* get_monotonic_time() - get monotonic time.
 *
 * This helps account for clock changes on the local system. Relying on time()
 * will ruin the filter if the clock is adjusted on the system.
 *
 * Args:
 *     None
 *
 * Returns:
 *     time_t holding CLOCK_MONOTONIC value
 */
static time_t get_monotonic_time() {
	struct timespec ts;

	clock_gettime(CLOCK_MONOTONIC, &ts);

	return ts.tv_sec;
}

/* timedecay_init() - initialize a time filter
 *
 * Args:
 *     tf       - pointer to timedecay structure
 *     expected - maximum expected number of elements
 *     accuracy - acceptable false positive rate. ex: 0.01 == 99.99% accuracy
 *     timeout  - number of seconds an element is valid
 *
 * Returns:
 *     true on success, false on failure
 *
 * TODO: report reason for error.
 */
bool timedecay_init(timedecay *tf, const size_t expected, const float accuracy, const size_t timeout) {
	tf->size       = ideal_size(expected, accuracy);
	tf->hashcount  = (tf->size / expected) * log(2);
	tf->timeout    = timeout;
	tf->expected   = expected;
	tf->accuracy   = accuracy;
	tf->start_time = get_monotonic_time();

	// decide which datatype to use for storing timestamps
	int bytes;
	if (sizeof(time_t) == 4 && timeout > UINT32_MAX) {
		return false;
	}

	if      (timeout < UINT8_MAX)  { bytes = 1; tf->max_time = UINT8_MAX; }
	else if (timeout < UINT16_MAX) { bytes = 2; tf->max_time = UINT16_MAX; }
	else if (timeout < UINT32_MAX) { bytes = 4; tf->max_time = UINT32_MAX; }
	else if (timeout < UINT64_MAX) { bytes = 8; tf->max_time = UINT64_MAX; }

	tf->bytes = bytes;

	tf->filter = calloc(tf->size, bytes);
	if (tf->filter == NULL) {
		return false;
	}

	// calculate filter size
	tf->filter_size = tf->size * tf->bytes;

	return true;
}

/* timedecay_destroy() - uninitialize a time filter
 *
 * Args:
 *     tf - filter to destroy
 *
 * Returns:
 *     Nothing
 */
void timedecay_destroy(timedecay tf) {
	free(tf.filter);
}

/* timedecay_add() - add an element to a time filter
 *
 * Args:
 *     tf      - time filter to add element to
 *     element - element to add to filter
 *     len     - length of element in bytes
 *
 * Returns:
 *     Nothing
 */
void timedecay_add(timedecay *tf, void *element, const size_t len) {
	uint64_t    result;
	uint64_t    hash[2];
	time_t      now = get_monotonic_time();
	size_t      ts = ((now - tf->start_time) % tf->max_time) + 1;

	for (int i = 0; i < tf->hashcount; i++) {
		mmh3_128(element, len, i, hash);
		result = ((hash[0] % tf->size) + (hash[1] % tf->size)) % tf->size;
		switch(tf->bytes) {
		case 1:	((uint8_t *)tf->filter)[result] = ts; break;
		case 2:	((uint16_t *)tf->filter)[result] = ts; break;
		case 4:	((uint32_t *)tf->filter)[result] = ts; break;
		case 8: ((uint64_t *)tf->filter)[result] = ts; break;
		}
	}
}

/* timedecay_add_string() - add a string element to a time filter
 *
 * Args:
 *     tf      - time filter to add element to
 *     element - element to add to filter
 *
 * Returns:
 *     Nothing
 */
void timedecay_add_string(timedecay tf, const char *element) {
	timedecay_add(&tf, (uint8_t *)element, strlen(element));
}

/* timedecay_lookup() - check if element exists within timedecay
 *
 * Args:
 *     tf      - time filter to perform lookup against
 *     element - element to search for
 *     len     - length of element to search (bytes)
 *
 * Returns:
 *     true if element is in filter
 *     false if element is not in filter
 */
bool timedecay_lookup(const timedecay tf, void *element, const size_t len) {
	uint64_t    result;
	uint64_t    hash[2];
	time_t      now = get_monotonic_time();
	size_t      ts = ((now - tf.start_time) % tf.max_time) + 1;

	if ((now - tf.start_time) > tf.max_time) { return false; }

	for (int i = 0; i < tf.hashcount; i++) {
		mmh3_128(element, len, i, hash);
		result = ((hash[0] % tf.size) + (hash[1] % tf.size)) % tf.size;

		size_t value;
		switch(tf.bytes) {
		case 1:	value = ((uint8_t *)tf.filter)[result];	break;
		case 2:	value = ((uint16_t *)tf.filter)[result]; break;
		case 4:	value = ((uint32_t *)tf.filter)[result]; break;
		case 8:	value = ((uint64_t *)tf.filter)[result]; break;
		}

		if (((ts - value) > tf.timeout) || (value == 0)) {
			return false;
		}
	}

	return true;
}

/* timedecay_lookup_string() -- helper function to handle string lookups
 *
 * Args:
 *     tf      - filter to use
 *     element - string element to lookup
 *
 * Returns:
 *     true if element is likely in the filter
 *     false if element is definitely not in the filter
 */
bool timedecay_lookup_string(const timedecay tf, const char *element) {
	return timedecay_lookup(tf, (uint8_t *)element, strlen(element));
}


/* timedecay_save() -- save a time-decaying bloom filter to disk
 *
 * Format of these files on disk is:
 *    +------------------+
 *    | timedecay struct |
 *    +------------------+
 *    |      bitmap      |
 *    +------------------+
 *
 * Args:
 *     tf   - filter to save
 *     path - file path to save filter
 *
 * Returns:
 *      true on success, false on failure
 *
 * TODO: test timedecay_save()
 */
bool timedecay_save(timedecay tf, const char *path) {
	FILE *fp;

	fp = fopen(path, "wb");
	if (fp == NULL) {
		return false;
	}

	fwrite(&tf, sizeof(timedecay), 1, fp);
	fwrite(tf.filter, tf.filter_size, 1, fp);

	fclose(fp);

	return true;
}

/* timedecay_load() -- load a time-decaying bloom filter from disk
 *
 * Args:
 *     tf   - timedecay struct of new filter
 *     path - location of filter on disk
 *
 * Returns:
 *     true on success, false on failure
 *
 * TODO: test timedecay_save()
 */
bool timedecay_load(timedecay *tf, const char *path) {
	FILE *fp;

	fp = fopen(path, "rb");
	if (fp == NULL) {
		return false;
	}

	fread(tf, sizeof(timedecay), 1, fp);

	tf->filter = malloc(tf->filter_size);
	if (tf->filter == NULL) {
		fclose(fp);
		return false;
	}

	fread(tf->filter, tf->filter_size, 1, fp);

	fclose(fp);

	return true;
}
