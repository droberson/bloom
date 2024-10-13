/* mmh3.h
 */
#ifndef MMH3_H
#define MMH3_H

#include <stddef.h>

uint32_t mmh3(const uint8_t *, const uint32_t, const uint32_t);
uint32_t mmh3_string(const char *, const uint32_t);

void mmh3_128(const void *, const size_t, const uint64_t, uint64_t *);
#endif /* MMH3_H */
