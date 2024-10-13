#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>
#include <time.h>
#include <unistd.h>

#include "mmh3.h"

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


/* timefilter_init() - initialize a time filter
 *
 * Args:
 *     tf       - pointer to timefilter structure
 *     expected - maximum expected number of elements
 *     accuracy - acceptable false positive rate. ex: 0.01 == 99.99% accuracy
 *     timeout  - number of seconds an element is valid
 *
 * Returns:
 *     true on success, false on failure
 */
bool timefilter_init(timefilter *tf, const uint32_t expected, const float accuracy, const uint32_t timeout) {
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


/* timefilter_destroy() - uninitialize a time filter
 *
 * Args:
 *     tf - filter to destroy
 *
 * Returns:
 *     Nothing
 */
void timefilter_destroy(timefilter tf) {
	free(tf.filter);
}


/* timefilter_add() - add an element to a time filter
 *
 * Args:
 *     tf      - time filter to add element to
 *     element - element to add to filter
 *     len     - length of element in bytes
 *
 * Returns:
 *     Nothing
 */
void timefilter_add(timefilter tf, const uint8_t *element, const size_t len) {
	int			i;
	uint32_t	result;
	time_t		now = time(NULL);

	for (i = 0; i < tf.hashcount; i++) {
		result  = mmh3(element, len, i) % tf.size; // salt is seed.
		tf.filter[result] = now;
	}
}


/* timefilter_add_string() - add a string element to a time filter
 *
 * Args:
 *     tf      - time filter to add element to
 *     element - element to add to filter
 *
 * Returns:
 *     Nothing
 */
void timefilter_add_string(timefilter tf, const char *element) {
	timefilter_add(tf, element, strlen(element));
}


/* timefilter_lookup() - check if element exists within timefilter
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
bool timefilter_lookup(timefilter tf, const uint8_t *element, const size_t len) {
	int			i;
	uint32_t	result;
	time_t		now = time(NULL);

	for (i = 0; i < tf.hashcount; i++) {
		result = mmh3(element, len, i) % tf.size;

		if (((now - tf.filter[result]) > tf.timeout) || (tf.filter[result] == 0)) {
			return false;
		}
	}

	return true;
}


bool timefilter_lookup_time(timefilter tf, const uint8_t *element, const size_t len, const size_t timeout) {
	int			i;
	uint32_t	result;
	time_t		now = time(NULL);

	for (i = 0; i < tf.hashcount; i++) {
		result = mmh3(element, len, i) % tf.size;

		if (((now - tf.filter[result]) > timeout) || (tf.filter[result] == 0)) {
			return false;
		}
	}

	return true;
}


bool timefilter_save(timefilter tf, const char *path) {
	FILE	*fp;

	fp = fopen(path, "w");
	if (fp == NULL) {
		return false;
	}

	fwrite(&tf, sizeof(timefilter), 1, fp);
	fwrite(tf.filter, tf.filter_size, 1, fp);

	fclose(fp);

	return true;
}


bool timefilter_load(timefilter *tf, const char *path) {
	FILE	*fp;

	fp = fopen(path, "r");
	if (fp == NULL) {
		return false;
	}

	fread(tf, sizeof(timefilter), 1, fp);

	tf->filter = malloc(tf->filter_size);
	if (tf->filter == NULL) {
		fclose(fp);
		return false;
	}

	fread(tf->filter, tf->filter_size, 1, fp);

	fclose(fp);

	return true;
}


bool bloom_init(bloomfilter *bf, const uint32_t expected, const float accuracy) {
	bf->size        = ideal_size(expected, accuracy);
	bf->hashcount   = (bf->size / expected) * log(2);
	bf->bitmap_size = ceil(bf->size / 8);

	bf->bitmap      = malloc(bf->bitmap_size);
	if (bf->bitmap == NULL) {
		return false;
	}

	return true;
}


void bloom_destroy(bloomfilter bf) {
	free(bf.bitmap);
}


bool bloom_lookup(const bloomfilter bf, const uint8_t *element, const size_t len) {
	int      i;
	uint32_t result;
	uint32_t bytepos;
	uint32_t bitpos;

	for (i = 0; i < bf.hashcount; i++) {
		result = mmh3(element, len, i) % bf.size;
		bytepos = ceil(result / 8);
		bitpos = result % 8;

		if ((bf.bitmap[bytepos] & (0x01 << bitpos)) == 0) {
			return false;
		}
	}

	return true;
}


bool bloom_lookup_string(const bloomfilter bf, const char *element) {
	return bloom_lookup(bf, element, strlen(element));
}


void bloom_add(bloomfilter bf, const uint8_t *element, const size_t len) {
	int			i;
	uint32_t	result;
	uint32_t	bytepos;
	uint32_t	bitpos;

	for (i = 0; i < bf.hashcount; i++) {
		result  = mmh3(element, len, i) % bf.size; // salt is seed.
		bytepos = ceil(result / 8);
		bitpos  = result % 8;
		bf.bitmap[bytepos] |= (0x01 << bitpos);
	}
}


void bloom_add_string(bloomfilter bf, const char *element) {
	bloom_add(bf, element, strlen(element));
}


bool bloom_save(bloomfilter bf, const char *path) {
	FILE	*fp;

	fp = fopen(path, "w");
	if (fp == NULL)
		return false;

	fwrite(&bf, sizeof(bloomfilter), 1, fp);
	fwrite(bf.bitmap, bf.bitmap_size, 1, fp);

	fclose(fp);

	return true;
}


bool bloom_load(bloomfilter *bf, const char *path) {
	FILE	*fp;

	fp = fopen(path, "r");
	if (fp == NULL) {
		return false;
	}

	fread(bf, sizeof(bloomfilter), 1, fp);

	bf->bitmap = malloc(bf->bitmap_size);
	if (bf->bitmap == NULL) {
		fclose(fp);
		return false;
	}

	fread(bf->bitmap, bf->bitmap_size, 1, fp);

	fclose(fp);

	return true;
}


int main() {
	bloomfilter bf;

	puts("Initializing filter with 100 expected elements and 99.99% accuracy\n");
	bloom_init(&bf, 100, 0.01);
	printf("size: %d\n", bf.size);
	printf("hashcount: %d\n", bf.hashcount);
	printf("bitmap size: %d\n", bf.bitmap_size);

	// Add some shit to the filter
	bloom_add_string(bf, "asdf");
	bloom_add_string(bf, "bar");
	bloom_add_string(bf, "foo");

	// Look up some stuff
	printf("foo: %d\n", bloom_lookup_string(bf, "foo"));
	printf("bar: %d\n", bloom_lookup_string(bf, "bar"));
	printf("baz: %d\n", bloom_lookup_string(bf, "baz"));
	printf("asdf: %d\n", bloom_lookup_string(bf, "asdf"));

	// Hex dump the bitmap

	int i;
	for (i = 0; i < bf.bitmap_size; i++) {
		printf("%02x ", bf.bitmap[i]);
	}
	printf("\n");


	// Save to file
	bloom_save(bf, "/tmp/bloom");
	bloom_destroy(bf);

	// Load from file
	bloomfilter newbloom;
	bloom_load(&newbloom, "/tmp/bloom");

	printf("foo: %d\n", bloom_lookup_string(newbloom, "foo"));
	printf("bar: %d\n", bloom_lookup_string(newbloom, "bar"));
	printf("baz: %d\n", bloom_lookup_string(newbloom, "baz"));
	printf("asdf: %d\n", bloom_lookup_string(newbloom, "asdf"));


	timefilter tf;

	timefilter_init(&tf, 10, 0.01, 2);

	timefilter_add(tf, "a", 1);
	timefilter_add(tf, "b", 1);
	printf("a: %d\n", timefilter_lookup(tf, "a", 1));
	printf("c: %d\n", timefilter_lookup(tf, "c", 1));

	puts("sleeping...");
	sleep(5);
	printf("a: %d\n", timefilter_lookup(tf, "a", 1));
	printf("c: %d\n", timefilter_lookup(tf, "c", 1));

	timefilter_destroy(tf);
}
