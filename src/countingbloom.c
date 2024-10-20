/* countingbloom.c
 */
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <sys/stat.h>

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
 *     csize    - size of counter: COUNTER_8BIT, _16BIT, _32BIT, _64BIT, ...
 *
 * Returns:
 *     true on success, false on failure
 */
bool countingbloom_init(countingbloomfilter *cbf, const size_t expected, const float accuracy, counter_size csize) {
	cbf->size      = ideal_size(expected, accuracy);
	// add 0.5 to round up/down
	cbf->hashcount = (uint64_t)((cbf->size / expected) * log(2) + 0.5);
	cbf->csize     = csize;

	switch (csize) {
	case COUNTER_8BIT:
		cbf->countermap_size = cbf->size * sizeof(uint8_t);
		break;
	case COUNTER_16BIT:
		cbf->countermap_size = cbf->size * sizeof(uint16_t);
		break;
	case COUNTER_32BIT:
		cbf->countermap_size = cbf->size * sizeof(uint32_t);
		break;
	case COUNTER_64BIT:
		cbf->countermap_size = cbf->size * sizeof(uint64_t);
		break;
	default: // invalid counter size
		return false;
	}

	cbf->countermap = calloc(1, cbf->countermap_size);
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

/* get_counter, inc_counter, dec_counter -- helper functions used to handle
 *     different sized counters.
 */
static uint64_t get_counter(const countingbloomfilter *cbf, uint64_t position) {
	switch (cbf->csize) {
	case COUNTER_8BIT:	return  ((uint8_t *)cbf->countermap)[position];
	case COUNTER_16BIT:	return ((uint16_t *)cbf->countermap)[position];
	case COUNTER_32BIT:	return ((uint32_t *)cbf->countermap)[position];
	case COUNTER_64BIT:	return ((uint64_t *)cbf->countermap)[position];
	default:
		return 0; // shouldn't get here
	}
}

static void inc_counter(countingbloomfilter *cbf, uint64_t position) {
	switch (cbf->csize) {
	case COUNTER_8BIT:
		if (((uint8_t *)cbf->countermap)[position] != UINT8_MAX) {
			((uint8_t *)cbf->countermap)[position]++;
		}
		break;
	case COUNTER_16BIT:
		if (((uint16_t *)cbf->countermap)[position] != UINT16_MAX) {
			((uint16_t *)cbf->countermap)[position]++;
		}
		break;
	case COUNTER_32BIT:
		if (((uint32_t *)cbf->countermap)[position] != UINT32_MAX) {
			((uint32_t *)cbf->countermap)[position]++;
		}
		break;
	case COUNTER_64BIT:
		if (((uint64_t *)cbf->countermap)[position] != UINT64_MAX) {
			((uint64_t *)cbf->countermap)[position]++;
		}
		break;
	default:
		return; // shouldn't get here
	}
}

static void dec_counter(countingbloomfilter *cbf, uint64_t position) {
	switch (cbf->csize) {
		case COUNTER_8BIT:
		if (((uint8_t *)cbf->countermap)[position] > 0) {
			((uint8_t *)cbf->countermap)[position]--;
		}
		break;
	case COUNTER_16BIT:
		if (((uint16_t *)cbf->countermap)[position] > 0) {
			((uint16_t *)cbf->countermap)[position]--;
		}
		break;
	case COUNTER_32BIT:
		if (((uint32_t *)cbf->countermap)[position] > 0) {
			((uint32_t *)cbf->countermap)[position]--;
		}
		break;
	case COUNTER_64BIT:
		if (((uint64_t *)cbf->countermap)[position] > 0) {
			((uint64_t *)cbf->countermap)[position]--;
		}
		break;
	default:
		return; // shouldn't get here
	}
}

/* countingbloom_count() -- get approximate count of an element in the filter
 *
 * Args:
 *     cbf     - filter to use
 *     element - element to count
 *     len     - length of element
 *
 * Returns:
 *     size_t representing the approximate count of 'element' in the filter
 */
size_t countingbloom_count(const countingbloomfilter cbf, void *element, size_t len) {
	uint64_t hash[2];
	uint64_t position;
	uint64_t count = UINT64_MAX;

	for (int i = 0; i < cbf.hashcount; i++) {
		mmh3_128(element, len, i, hash);
		position = ((hash[0] % cbf.size) + (hash[1] % cbf.size)) % cbf.size;

		uint64_t current_count = get_counter(&cbf, position);
		if (current_count < count) {
			count = current_count;
		}
	}

	return count;
}

/* countingbloom_count_string() -- helper function to get approximate count of
 *                                 a string element in the filter.
 *
 * Args:
 *     cbf     - filter to use
 *     element - element to count
 *
 * Returns:
 *     size_t representing the approximate count of 'element' in the filter
 */

size_t countingbloom_count_string(const countingbloomfilter cbf, char *element) {
	return countingbloom_count(cbf, (uint8_t *)element, strlen(element));
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

		// if any counter is zero, element isn't in the filter
		if (get_counter(&cbf,position) == 0) {
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
		inc_counter(&cbf, position);
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
	uint64_t positions[cbf.hashcount];

	bool shouldremove = true;
	for (int i = 0; i < cbf.hashcount; i++) {
		mmh3_128(element, len, i, hash);
		positions[i] = ((hash[0] % cbf.size) + (hash[1] % cbf.size)) % cbf.size;
		if (get_counter(&cbf, positions[i]) == 0) {
			shouldremove = false;
			break;
		}
	}

	if (shouldremove) {
		for (int i = 0; i < cbf.hashcount; i++) {
			dec_counter(&cbf, positions[i]);
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

/* countingbloom_save() -- save a counting bloom filter to disk
 *
 * Format of these files on disk:
 *    +------------------------------+
 *    | counting bloom filter struct |
 *    +------------------------------+
 *    |             data             |
 *    +------------------------------+
 *
 * Args:
 *     cbf  - filter to save
 *     path - path to save filter
 *
 * Returns:
 *     true on success, false on failure
 */
bool countingbloom_save(countingbloomfilter cbf, const char *path) {
	FILE        *fp;
	struct stat  sb;

	fp = fopen(path, "wb");
	if (fp == NULL) {
		return false;
	}

	if (fwrite(&cbf, sizeof(countingbloomfilter), 1, fp) != 1 ||
		fwrite(cbf.countermap, cbf.countermap_size, 1, fp) != 1) {
		fclose(fp);
		return false;
	}

	fclose(fp);
	return true;
}

/* countingbloom_load() -- load a counting bloom filter from disk
 *
 * Args:
 *     cbf  - counting bloom filter struct to populate
 *     path - path on disk to counting bloom filter file
 *
 * Returns:
 *     true on success, false on failure
 */
bool countingbloom_load(countingbloomfilter *cbf, const char *path) {
	FILE        *fp;
    struct stat  sb;

	fp = fopen(path, "rb");
	if (fp == NULL) {
		return false;
	}

	if (fstat(fileno(fp), &sb) == -1) {
		fclose(fp);
		return false;
	}

	if (fread(cbf, sizeof(countingbloomfilter), 1, fp) != 1) {
		fclose(fp);
		free(cbf->countermap);
		return false;
	}

	// basic sanity check. should fail if the file isn't valid
	if (sizeof(countingbloomfilter) + cbf->countermap_size != sb.st_size) {
		fclose(fp);
		return false;
	}

	cbf->countermap = malloc(cbf->countermap_size);
	if (cbf->countermap == NULL) {
		fclose(fp);
		return false;
	}

	fread(cbf->countermap, cbf->size, 1, fp);

	fclose(fp);

	return true;
}
