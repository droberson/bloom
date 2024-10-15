/* mmh3.h
 */
#ifndef MMH3_H
#define MMH3_H

#include <stddef.h>

/* function definitions
 * TODO: mmh3_64_string()
 */
uint32_t mmh3_32(const uint8_t *, const size_t, const uint32_t);
uint32_t mmh3_32_string(const char *, const uint32_t);
uint64_t mmh3_64(const void *, const size_t, uint64_t);
void     mmh3_128(const void *, const size_t, const uint64_t, uint64_t *);

#endif /* MMH3_H */
