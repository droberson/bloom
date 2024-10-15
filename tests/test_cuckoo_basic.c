#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cuckoo.h"

int main() {
	cuckoofilter cf;

	printf("initializing cuckoo filter\n");
	cuckoo_init(&cf, 1000, 4, 500);

	// add elements to filter
	cuckoo_add(cf, "foo", strlen("foo"));
	cuckoo_add(cf, "bar", strlen("bar"));

	bool result;
	result = cuckoo_lookup(cf, "foo", strlen("foo"));
	printf("cuckoo foo lookup: %d\n", result);
	if (result != true) {
		fprintf(stderr, "FATAL: \"foo\" should be in filter\n");
		return EXIT_FAILURE;
	}

	result = cuckoo_lookup(cf, "bar", strlen("bar"));
	printf("cuckoo bar lookup: %d\n", result);
	if (result != true) {
		fprintf(stderr, "FATAL: \"foo\" should be in filter\n");
		return EXIT_FAILURE;
	}

	result = cuckoo_lookup(cf, "baz", strlen("baz"));
	printf("cuckoo baz lookup: %d\n", result);
	if (result != false) {
		fprintf(stderr, "FATAL: \"baz\" should be in filter\n");
		return EXIT_FAILURE;
	}

	// test removal
	cuckoo_remove(cf, "foo", strlen("foo"));
	result = cuckoo_lookup(cf, "foo", strlen("foo"));
	printf("cuckoo foo lookup: %d\n", result);
	if (result != false) {
		fprintf(stderr, "FATAL: \"foo\" should NOT be in filter\n");
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
