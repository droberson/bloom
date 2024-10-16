#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "timedecay.h"

int main() {
	timedecay tf;

	printf("Creating a time-decaying bloom filter. 10 elements, 2 seconds\n");
	timedecay_init(&tf, 10, 0.01, 2);

	printf("size of filter: %d\n", tf.size);
	printf("time value bytes: %d\n", tf.bytes);
	printf("max time: %d\n", tf.max_time);

	timedecay_add_string(tf, "a");
	timedecay_add(&tf, "b", 1);

	printf("filter hex dump: ");
	for (size_t i = 0; i < tf.size * tf.bytes; i++) {
		printf("%02x ", ((uint8_t *)tf.filter)[i]);
	}
	printf("\n");

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
	tf.start_time -= 3;

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

	timedecay tf2;
	timedecay_init(&tf2, 10, 0.01, 200);
	timedecay_add_string(tf2, "testytesttest");
	printf("sleeping 270 seconds\n");
	tf2.start_time -= 270;

	result = timedecay_lookup(tf2, "testytesttest", strlen("testytesttest"));
	printf("testytesttest: %d\n", result);
	if (result != false) {
		fprintf(stderr, "FAILURE: \"testytesttest\" should NOT be in the filter\n");
		return EXIT_FAILURE;
	}

	tf2.start_time += 270; // reset start time
	timedecay_add_string(tf2, "lol");
	result = timedecay_lookup(tf2, "lol", strlen("lol"));
	printf("lol: %d\n", result);
	if (result != true) {
		fprintf(stderr, "FAILURE: \"lol\" should be in the filter\n");
		return EXIT_FAILURE;
	}

	// Cleanup
	timedecay_destroy(tf);
	timedecay_destroy(tf2);

	return EXIT_SUCCESS;
}
