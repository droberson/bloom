
#include <stdio.h>
#include <unistd.h>

#include "bloom.h"
#include "timedecay.h"
#include "countingbloom.h"

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
	for (size_t i = 0; i < bf.bitmap_size; i++) {
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


	timedecay tf;

	timedecay_init(&tf, 10, 0.01, 2);

	timedecay_add_string(tf, "a");
	timedecay_add(tf, (uint8_t *)"b", 1);
	printf("a: %d\n", timedecay_lookup_string(tf, "a"));
	printf("c: %d\n", timedecay_lookup(tf, (uint8_t *)"c", 1));

	puts("sleeping...");
	sleep(5);
	printf("a: %d\n", timedecay_lookup(tf, (uint8_t *)"a", 1));
	printf("c: %d\n", timedecay_lookup(tf, (uint8_t *)"c", 1));

	timedecay_destroy(tf);


	countingbloomfilter cbf;
	countingbloom_init(&cbf, 20, 0.01);
	countingbloom_add_string(cbf, "foo");
	countingbloom_add_string(cbf, "bar");

	countingbloom_add_string(cbf, "multi");
	countingbloom_add_string(cbf, "multi");
	printf("cbf foo lookup: %d\n", countingbloom_lookup_string(cbf, "foo"));
	printf("cbf bar lookup: %d\n", countingbloom_lookup_string(cbf, "bar"));
	printf("cbf baz lookup: %d\n", countingbloom_lookup_string(cbf, "baz"));

	for (size_t i = 0; i < cbf.size; i++) {
		printf("%d ", cbf.countermap[i]);
	}
	printf("\n");

	countingbloom_remove_string(cbf, "bar");
	printf("cbf bar lookup: %d\n", countingbloom_lookup_string(cbf, "bar"));
}
