#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "timedecay.h"

int main() {
	timedecay tf;

	printf("Creating a time-decaying bloom filter. 10 elements, 2 seconds\n");
	timedecay_init(&tf, 10, 0.01, 2);

	timedecay_add_string(tf, "a");
	timedecay_add(tf, (uint8_t *)"b", 1);

	bool result;
	result =  timedecay_lookup_string(tf, "a");
	printf("a: %d\n", result);
	if (result != true) {
		fprintf(stderr, "FAILURE: \"a\" should be in filter\n");
		return EXIT_FAILURE;
	}

	result =  timedecay_lookup_string(tf, "b");
	printf("b: %d\n", result);
	if (result != true) {
		fprintf(stderr, "FAILURE: \"b\" should be in filter\n");
		return EXIT_FAILURE;
	}

	result = timedecay_lookup(tf, (uint8_t *)"c", 1);
	printf("c: %d\n", result);
	if (result != false) {
		fprintf(stderr, "FAILURE: \"c\" should NOT be in the filter\n");
		return EXIT_FAILURE;
	}

	puts("sleeping three seconds to expire results...");
	sleep(3);

	result = timedecay_lookup_string(tf, "a");
	printf("a: %d\n", result);
	if (result != false) {
		fprintf(stderr, "FAILURE: \"a\" should NOT be in filter\n");
		return EXIT_FAILURE;
	}

	result = timedecay_lookup_string(tf, "b");
	printf("b: %d\n", result);
	if (result != false) {
		fprintf(stderr, "FAILURE: \"b\" should NOT be in filter\n");
		return EXIT_FAILURE;
	}

	timedecay_add_string(tf, "c");
	result = timedecay_lookup(tf, (uint8_t *)"c", 1);
	printf("c: %d\n", result);
	if (result != true) {
		fprintf(stderr, "FAILURE: \"c\" should be in the filter\n");
		return EXIT_FAILURE;
	}

	// Cleanup
	timedecay_destroy(tf);

	return EXIT_SUCCESS;
}
