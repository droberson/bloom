/* timedecay.c
 */

#include <time.h>
#include <math.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

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
static uint32_t ideal_size(const uint32_t expected, const float accuracy) {
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
 */
bool timedecay_init(timedecay *tf, const uint32_t expected, const float accuracy, const uint32_t timeout) {
	tf->size      = ideal_size(expected, accuracy);
	tf->hashcount = (tf->size / expected) * log(2);
	tf->timeout   = timeout;
	tf->filter    = malloc(tf->size * sizeof(time_t));
	if (tf->filter == NULL) {
		return false;
	}

	/* Initialize filter with all zero values */
	for (int i = 0; i < tf->size; i++) {
		tf->filter[i] = 0;
	}

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
void timedecay_add(timedecay tf, const uint8_t *element, const size_t len) {
	uint64_t    result;
	uint64_t    hash[2];
	time_t      now = get_monotonic_time();

	for (int i = 0; i < tf.hashcount; i++) {
		mmh3_128(element, len, i, hash);
		result = ((hash[0] % tf.size) + (hash[1] % tf.size)) % tf.size;
		tf.filter[result] = now;
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
	timedecay_add(tf, (uint8_t *)element, strlen(element));
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
bool timedecay_lookup(const timedecay tf, const uint8_t *element, const size_t len) {
	uint64_t    result;
	uint64_t    hash[2];
	time_t      now = get_monotonic_time();

	for (int i = 0; i < tf.hashcount; i++) {
		mmh3_128(element, len, i, hash);
		result = ((hash[0] % tf.size) + (hash[1] % tf.size)) % tf.size;

		if (((now - tf.filter[result]) > tf.timeout) || (tf.filter[result] == 0)) {
			return false;
		}
	}

	return true;
}

// TODO add comment/documentation
bool timedecay_lookup_string(const timedecay tf, const char *element) {
	return timedecay_lookup(tf, (uint8_t *)element, strlen(element));
}

// TODO add comment/documentation
bool timedecay_lookup_time(const timedecay tf, const uint8_t *element, const size_t len, const size_t timeout) {
	uint64_t    result;
	uint64_t    hash[2];
	time_t      now = get_monotonic_time();

	for (int i = 0; i < tf.hashcount; i++) {
		mmh3_128(element, len, i, hash);
		result = ((hash[0] % tf.size) + (hash[1] % tf.size)) % tf.size;

		if (((now - tf.filter[result]) > timeout) || (tf.filter[result] == 0)) {
			return false;
		}
	}

	return true;
}

// TODO add comment/documentation
bool timedecay_save(timedecay tf, const char *path) {
	FILE *fp;

	fp = fopen(path, "w");
	if (fp == NULL) {
		return false;
	}

	fwrite(&tf, sizeof(timedecay), 1, fp);
	fwrite(tf.filter, tf.filter_size, 1, fp);

	fclose(fp);

	return true;
}

// TODO add comment/documentation
bool timedecay_load(timedecay *tf, const char *path) {
	FILE *fp;

	fp = fopen(path, "r");
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
