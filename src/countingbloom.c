#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <math.h>

#include "mmh3.h"
#include "countingbloom.h"


/* ideal_size() -- calculate ideal size of a filter
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

/* countingbloom_init() -- initialize counting bloom filter
 *
 * Args:
 *     cbf      - filter to initialize
 *     expected - expected number of elements in filter
 *     accuracy - margin of acceptable error. ex: 0.01 is "99.99%" accurate
 *
 * Returns:
 *     true on success, false on failure
 */
bool countingbloom_init(countingbloomfilter *cbf, const size_t expected, const float accuracy) {
	cbf->size            = ideal_size(expected, accuracy);
	cbf->hashcount       = (cbf->size / expected) * log(2);
	cbf->countermap      = calloc(cbf->size, sizeof(uint8_t));
	if (cbf->countermap == NULL) {
		return false;
	}
	return true;
}

/* countingbloom_destroy() -- free memory allocated by countingbloom_init()
 *
 * Args:
 *     cbf - filter to free
 *
 * Returns:
 *     Nothing
 */
void countingbloom_destroy(countingbloomfilter cbf) {
	free(cbf.countermap);
}

/* countingbloom_lookup() -- check if an element is likely in the filter
 *
 * Args:
 *     cbf     - filter to use
 *     element - element to look up
 *     len     - length of element in bytes
 *
 * Returns:
 *     true if element is probably in the filter
 *     false if element is definitely not in the filter
 */
bool countingbloom_lookup(const countingbloomfilter cbf, void *element, const size_t len) {
	uint64_t hash[2];
	uint64_t position;

	for (int i = 0; i < cbf.hashcount; i++) {
		mmh3_128(element, len, i, hash);
		position = ((hash[0] % cbf.size) + (hash[1] % cbf.size)) % cbf.size;

		if (cbf.countermap[position] == 0) {
			return false;
		}
	}

	return true;
}

/* countingbloom_lookup_string() -- helper function for looking up strings
 *
 * Args:
 *     cbf     - filter to use
 *     element - string to look up
 *
 * Returns:
 *     true if element is probably in the set
 *     false if element is definitely not in the set
 */
bool countingbloom_lookup_string(const countingbloomfilter cbf, const char *element) {
	return countingbloom_lookup(cbf, (uint8_t *)element, strlen(element));
}

/* countingbloom_add() -- add an element to a counting bloom filter
 *
 * Args:
 *     cbf     - filter to use
 *     element - element to add
 *     len     - length of element in bytes
 *
 * Returns:
 *     Nothing
 */
void countingbloom_add(countingbloomfilter cbf, void *element, const size_t len) {
	uint64_t hash[2];
	uint64_t position;

	for (int i = 0; i < cbf.hashcount; i++) {
		mmh3_128(element, len, i, hash);
		position = ((hash[0] % cbf.size) + (hash[1] % cbf.size)) % cbf.size;
		if (cbf.countermap[position] != 255) {
			cbf.countermap[position] += 1;
		}
	}
}

/* countingbloom_add_string() -- helper function for adding strings
 *
 * Args:
 *     cbf     - filter to use
 *     element - string to add to the filter
 *
 * Returns:
 *     Nothing
 */
void countingbloom_add_string(countingbloomfilter cbf, const char *element) {
	countingbloom_add(cbf, (uint8_t *)element, strlen(element));
}

/* countingbloom_remove() -- remove an element from a counting bloom filter
 *
 * Args:
 *     cbf     - filter to use
 *     element - element to remove
 *     len     - length of element in bytes
 *
 * Returns:
 *     Nothing
 */
void countingbloom_remove(countingbloomfilter cbf, void *element, const size_t len) {
	uint64_t hash[2];
	uint64_t position;

	bool shouldremove = true;
	for (int i = 0; i < cbf.hashcount; i++) {
		mmh3_128(element, len, i, hash);
		position = ((hash[0] % cbf.size) + (hash[1] % cbf.size)) % cbf.size;
		if (cbf.countermap[position] == 0) {
			shouldremove = false;
			break;
		}
	}

	// TODO refactor to avoid hashing twice.
	if (shouldremove) {
		for (int i = 0; i < cbf.hashcount; i++) {
			mmh3_128(element, len, i, hash);
			position = ((hash[0] % cbf.size) + (hash[1] % cbf.size)) % cbf.size;
			cbf.countermap[position] -= 1;
		}
	}
}

/* countingbloom_remove_string() -- helper function to remove strings
 *
 * Args:
 *     cbf     - filter to use
 *     element - string to remove
 *
 * Returns:
 *     Nothing
 */
void countingbloom_remove_string(countingbloomfilter cbf, const char *element) {
	countingbloom_remove(cbf, (uint8_t *)element, strlen(element));
}

/* TODO
bool countingbloom_save(countingbloomfilter cbf, const char *path) {
}

bool countingbloom_load(countingbloomfilter *cbf, const char *path) {
}
*/
