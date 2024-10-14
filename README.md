# bloom

![License](https://img.shields.io/github/license/droberson/bloom)
![GitHub Issues](https://img.shields.io/github/issues/droberson/bloom)


This project includes implementations of various probabilistic data
structures in C:

## Classic, vanilla bloom filters

Bloom filters are space-efficient, probabilistic data structures that
provide a mechanism to quickly determine if an element is _likely_ a
member or _definitely not_ a member of a large dataset.

False positives are possible with bloom filters, but false negatives
are not. Bloom filters can be used to represent large datasets in a
small amount of space, with fast lookups and insertions of elements.

Bloom filters have been used to implement caches, spell checkers, and
more.

https://en.wikipedia.org/wiki/Bloom_filter


## Time-decaying bloom filters

Time-decaying bloom filters are bloom filters with a time
condition. By representing elements with timestamps rather than bits,
a developer can use time-decaying bloom filters to determine when an
element was added to a dataset and make decisions or answer questions
such as:

"If I have seen this data in the last N seconds, do this. Otherwise,
do that."

"Is this element in the set? If so, when was it added?"

## Counting bloom filters

These are like bloom filters, but rather than storing binary bits to
represent an element, a counter is used. This provides the developer
with the ability to remove elements from a set at the expense of
larger filter sizes.

At the moment, this uses a `uint8_t` (8 bits, maximum count 255) as
counters. If larger counters are needed, this can be modified to use
`uint16_t` to increase counter capacity to 65535, however this doubles
the memory and storage required to represent a filter. `uint32_t`
would provide even larger counters, but at quadruple the size.

## Cuckoo filters

Cuckoo filters are a similar concept to bloom filters, but implemented
with a different strategy. In some cases, they may be more
space-efficient or performant than a bloom filter. Cuckoo filters also
support deletion, whereas bloom filters do not.

# Building

This is developed on Debian Linux, and untested on anything else.

Build this with CMake:

Make a directory;
```
mkdir build && cd build
```

Run cmake:
```
cmake ..
```

Build:
```
make
```

Install:
```
make install
```

You may need to run `ldconfig` after installing.


