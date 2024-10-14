#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <math.h>

#include "mmh3.h"
#include "countingbloom.h"

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

bool countingbloom_init(countingbloomfilter *cbf, const uint32_t expected, const float accuracy) {
	cbf->size            = ideal_size(expected, accuracy);
	cbf->hashcount       = (cbf->size / expected) * log(2);
	cbf->countermap      = calloc(cbf->size, sizeof(uint8_t));
	if (cbf->countermap == NULL) {
		return false;
	}
	return true;
}

void countingbloom_destroy(countingbloomfilter cbf) {
	free(cbf.countermap);
}

bool countingbloom_lookup(const countingbloomfilter cbf, const uint8_t *element, const size_t len) {
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


bool countingbloom_lookup_string(const countingbloomfilter cbf, const char *element) {
	return countingbloom_lookup(cbf, (uint8_t *)element, strlen(element));
}


void countingbloom_add(countingbloomfilter cbf, const uint8_t *element, const size_t len) {
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

void countingbloom_add_string(countingbloomfilter cbf, const char *element) {
	countingbloom_add(cbf, (uint8_t *)element, strlen(element));
}

void countingbloom_remove(countingbloomfilter cbf, const uint8_t *element, const size_t len) {
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

void countingbloom_remove_string(countingbloomfilter cbf, const char *element) {
	countingbloom_remove(cbf, (uint8_t *)element, strlen(element));
}

/*bool countingbloom_save(countingbloomfilter cbf, const char *path) {
}
bool countingbloom_load(countingbloomfilter *cbf, const char *path) {
}
*/
