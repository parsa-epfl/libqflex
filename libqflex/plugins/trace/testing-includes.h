#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>
#include <glib.h>

/**
 * extract32:
 * @value: the value to extract the bit field from
 * @start: the lowest bit in the bit field (numbered from 0)
 * @length: the length of the bit field
 *
 * Extract from the 32 bit input @value the bit field specified by the
 * @start and @length parameters, and return it. The bit field must
 * lie entirely within the 32 bit word. It is valid to request that
 * all 32 bits are returned (ie @length 32 and @start 0).
 *
 * Returns: the value of the bit field extracted from the input value.
 */
static inline uint32_t extract32(uint32_t value, int start, int length)
{
    assert(start >= 0 && length > 0 && length <= 32 - start);
    return (value >> start) & (~0U >> (32 - length));
}
