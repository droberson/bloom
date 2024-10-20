#include <stdio.h>
#include <stdlib.h>

#include "countingbloom.h"

int main() {
	countingbloomfilter cbf;

	printf("initializing counting bloom filter expecting 20 elements\n");
	countingbloom_init(&cbf, 20, 0.01, COUNTER_8BIT);

	// add some data
	countingbloom_add_string(cbf, "foo");
	countingbloom_add_string(cbf, "bar");
	// add a string twice to test removal
	countingbloom_add_string(cbf, "multi");
	countingbloom_add_string(cbf, "multi");

	bool result;
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
		fprintf(stderr, "FAILURE: \"bar\" should not be in filter\n");
		return EXIT_FAILURE;
	}

	result = countingbloom_lookup_string(cbf, "multi");
	printf("cbf multi lookup: %d\n", result);
	if (result != true) {
		fprintf(stderr, "FAILURE: \"multi\" should be in filter\n");
		return EXIT_FAILURE;
	}

	// cleanup
	countingbloom_destroy(cbf);

	return EXIT_SUCCESS;
}
