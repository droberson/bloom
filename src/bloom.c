#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>
#include <unistd.h>

#include "mmh3.h"
#include "bloom.h"

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

// TODO: specify which hashing algorithm to use.
// TODO: add comment/documentation
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

// TODO: comment/documentation
void bloom_destroy(bloomfilter bf) {
	free(bf.bitmap);
}

// TODO comment/documentation
bool bloom_lookup(const bloomfilter bf, const uint8_t *element, const size_t len) {
	uint64_t hash[2];
	uint64_t result;
	uint64_t bytepos;
	uint64_t bitpos;

	for (int i = 0; i < bf.hashcount; i++) {
		mmh3_128(element, len, i, hash);
		result = ((hash[0] % bf.size) + (hash[1] % bf.size)) % bf.size;

		bytepos = ceil(result / 8);
		bitpos = result % 8;

		if ((bf.bitmap[bytepos] & (0x01 << bitpos)) == 0) {
			return false;
		}
	}

	return true;
}

// TODO comment/documenatation
bool bloom_lookup_string(const bloomfilter bf, const char *element) {
	return bloom_lookup(bf, (uint8_t *)element, strlen(element));
}

// TODO comment/documentation
void bloom_add(bloomfilter bf, const uint8_t *element, const size_t len) {
	uint64_t  hash[2];
	uint64_t  result;
	uint64_t  bytepos;
	uint64_t  bitpos;

	for (int i = 0; i < bf.hashcount; i++) {
		mmh3_128(element, len, i, hash);
		result = ((hash[0] % bf.size) + (hash[1] % bf.size)) % bf.size;

		bytepos = ceil(result / 8);
		bitpos  = result % 8;
		bf.bitmap[bytepos] |= (0x01 << bitpos);
	}
}

// TODO comment/documentation
void bloom_add_string(bloomfilter bf, const char *element) {
	bloom_add(bf, (uint8_t *)element, strlen(element));
}

// TODO comment/documentation
bool bloom_save(bloomfilter bf, const char *path) {
	FILE	*fp;

	fp = fopen(path, "w");
	if (fp == NULL) {
		return false;
	}

	fwrite(&bf, sizeof(bloomfilter), 1, fp);
	fwrite(bf.bitmap, bf.bitmap_size, 1, fp);

	fclose(fp);

	return true;
}

// TODO comment/documentation
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
