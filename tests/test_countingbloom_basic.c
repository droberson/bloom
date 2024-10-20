#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "countingbloom.h"

int main() {
	countingbloomfilter cbf;
	bool                result;

	printf("initializing counting bloom filter expecting 20 elements\n");
	result = countingbloom_init(&cbf, 20, 0.01, COUNTER_8BIT);
	if (result != true) {
		fprintf(stderr, "FAILURE: failed creating 8 bit filter\n");
		return EXIT_FAILURE;
	}

	printf("\tsize: %d\n", cbf.size);
	printf("\thash count: %d\n", cbf.hashcount);
	printf("\tcountermap size: %d\n", cbf.countermap_size);
	printf("\tcounter size (bits): %d\n", (size_t)pow(2, (cbf.csize + 3)));

	// add some data
	countingbloom_add_string(cbf, "foo");
	countingbloom_add_string(cbf, "bar");
	// add a string twice to test removal
	countingbloom_add_string(cbf, "multi");
	countingbloom_add_string(cbf, "multi");

	result = countingbloom_lookup_string(cbf, "foo");
	printf("cbf foo lookup: %d\n", result);
	if (result != true) {
		fprintf(stderr, "FAILURE: \"foo\" should be in filter\n");
		return EXIT_FAILURE;
	}

	result = countingbloom_lookup_string(cbf, "bar");
	printf("cbf bar lookup: %d\n", result);
	if (result != true) {
		fprintf(stderr, "FAILURE: \"bar\" should be in filter\n");
		return EXIT_FAILURE;
	}

	result = countingbloom_lookup_string(cbf, "baz");
	printf("cbf baz lookup: %d\n", result);
	if (result != false) {
		fprintf(stderr, "FAILURE: \"bar\" should NOT be in filter\n");
		return EXIT_FAILURE;
	}

	size_t count = countingbloom_count_string(cbf, "multi");
	printf("count \"multi\": %d\n", count);
	if (count != 2) {
		fprintf(stderr, "FAILURE: count should be 2\n");
		return EXIT_FAILURE;
	}

	count = countingbloom_count_string(cbf, "fizzbuzz");
	printf("count \"fizzbuzz\": %d\n", count);
	if (count != 0) {
		fprintf(stderr, "FAILURE: count should be 0\n");
		return EXIT_FAILURE;
	}

	// test removal
	printf("removing elements from filter\n");
	countingbloom_remove_string(cbf, "bar");
	countingbloom_remove_string(cbf, "multi");

	result = countingbloom_lookup_string(cbf, "bar");
	printf("cbf bar lookup: %d\n", result);
	if (result != false) {
		fprintf(stderr, "FAILURE: \"bar\" should NOT be in filter\n");
		return EXIT_FAILURE;
	}

	result = countingbloom_lookup_string(cbf, "multi");
	printf("cbf multi lookup: %d\n", result);
	if (result != true) {
		fprintf(stderr, "FAILURE: \"multi\" should be in filter\n");
		return EXIT_FAILURE;
	}

	printf("Testing saving and loading from disk\n");
	result = countingbloom_save(cbf, "/tmp/countingbloom");
	if (result != true) {
		fprintf(stderr, "FAILURE: failed to save 8 bit counting bloom filter file to /tmp/countingbloom\n");
		return EXIT_FAILURE;
	}

	countingbloomfilter newcbf;
	countingbloom_load(&newcbf, "/tmp/countingbloom");
	printf("\tsize: %d\n", newcbf.size);
	printf("\thash count: %d\n", newcbf.hashcount);
	printf("\tcountermap size: %d\n", newcbf.countermap_size);
	printf("\tcounter size (bits): %d\n", (size_t)pow(2, (newcbf.csize + 3)));

	result = countingbloom_lookup_string(newcbf, "multi");
	printf("cbf multi lookup: %d\n", result);
	if (result != true) {
		fprintf(stderr, "FAILURE: \"multi\" should be in filter\n");
		return EXIT_FAILURE;
	}

	result = countingbloom_lookup_string(newcbf, "bar");
	printf("cbf bar lookup: %d\n", result);
	if (result != false) {
		fprintf(stderr, "FAILURE: \"bar\" should NOT be in filter\n");
		return EXIT_FAILURE;
	}

	// test creation of 16, 32, 64 bit filters
	countingbloomfilter cbf16;
	countingbloomfilter cbf32;
	countingbloomfilter cbf64;

	result = countingbloom_init(&cbf16, 20, 0.01, COUNTER_16BIT);
	if (result != true) {
		fprintf(stderr, "FAILURE: creation of 16 bit counter\n");
		return EXIT_FAILURE;
	}
	printf("16 bit:\n");
	printf("\tsize: %d\n", cbf16.size);
	printf("\thash count: %d\n", cbf16.hashcount);
	printf("\tcountermap size: %d\n", cbf16.countermap_size);
	printf("\tcounter size (bits): %d\n", (size_t)pow(2, (cbf16.csize + 3)));
	countingbloom_destroy(cbf16);

	// 32 bit
	result = countingbloom_init(&cbf32, 20, 0.01, COUNTER_32BIT);
	if (result != true) {
		fprintf(stderr, "FAILURE: creation of 32 bit counter\n");
		return EXIT_FAILURE;
	}
	printf("32 bit:\n");
	printf("\tsize: %d\n", cbf32.size);
	printf("\thash count: %d\n", cbf32.hashcount);
	printf("\tcountermap size: %d\n", cbf32.countermap_size);
	printf("\tcounter size (bits): %d\n", (size_t)pow(2, (cbf32.csize + 3)));

	countingbloom_add_string(cbf32, "the last metroid is in captivity");
	countingbloom_add_string(cbf32, "the galaxy is at peace.");
	countingbloom_add_string(cbf32, "blap");
	countingbloom_add_string(cbf32, "blap");

	result = countingbloom_lookup_string(cbf32, "the last metroid is in captivity");
	if (result != true) {
		fprintf(stderr, "FAILURE: \"the last metroid is in captivity\" should be in filter\n");
		return EXIT_FAILURE;
	}

	result = countingbloom_save(cbf32, "/tmp/cbf32");
	if (result != true) {
		fprintf(stderr, "FAILURE: unable to save 32 bit counter to disk\n");
		return EXIT_FAILURE;
	}
	countingbloom_destroy(cbf32);
	result = countingbloom_load(&cbf32, "/tmp/cbf32");
	if (result != true) {
		fprintf(stderr, "FAILURE: unable to load 32 bit filter from disk\n");
		return EXIT_FAILURE;
	}
	printf("32 bit loaded from disk:\n");
	printf("\tsize: %d\n", cbf32.size);
	printf("\thash count: %d\n", cbf32.hashcount);
	printf("\tcountermap size: %d\n", cbf32.countermap_size);
	printf("\tcounter size (bits): %d\n", (size_t)pow(2, (cbf32.csize + 3)));

	result = countingbloom_lookup_string(cbf32, "the last metroid is in captivity");
	if (result != true) {
		fprintf(stderr, "FAILURE: \"the last metroid is in captivity\" should be in filter\n");
		return EXIT_FAILURE;
	}

	// 64 bit
	result = countingbloom_init(&cbf64, 20, 0.01, COUNTER_64BIT);
	if (result != true) {
		fprintf(stderr, "FAILURE: creation of64 bit counter\n");
		return EXIT_FAILURE;
	}
	printf("64 bit:\n");
	printf("\tsize: %d\n", cbf64.size);
	printf("\thash count: %d\n", cbf64.hashcount);
	printf("\tcountermap size: %d\n", cbf64.countermap_size);
	printf("\tcounter size (bits): %d\n", (size_t)pow(2, (cbf64.csize + 3)));
	countingbloom_destroy(cbf64);

	// cleanup
	remove("/tmp/countgbloom");

	countingbloom_destroy(cbf);
	countingbloom_destroy(newcbf);

	return EXIT_SUCCESS;
}
