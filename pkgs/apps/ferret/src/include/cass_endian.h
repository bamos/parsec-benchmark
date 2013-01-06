#ifndef CASS_ENDIAN
#define CASS_ENDIAN

/* Detect endianness of machine
 * Returns true if machine uses little endian format and
 * false if it uses big endian format
 */
static inline int isLittleEndian() {
  union {
    uint16_t word;
    uint8_t byte;
  } endian_test;

  endian_test.word = 0x00FF;
  return (endian_test.byte == 0xFF);
}

/* Convert 32-bit integer and floats between the two endianness
 * formats. Note that you *must* use the float version for
 * floating point variables because the compiler inserts
 * additional `magic' for accesses to floating point variables.
 */
union __float_and_int {
  uint32_t i;
  float    f;
};

#define bswap_float(x)                                                      \
     ({ union __float_and_int __x; __x.f = x; __x.i =               \
      ((__x.i & 0xff000000) >> 24) | ((__x.i & 0x00ff0000) >>  8) | \
      ((__x.i & 0x0000ff00) <<  8) | ((__x.i & 0x000000ff) << 24);__x.f;})

#define bswap_int32(x)                                                      \
      ( (((x) & 0xff000000) >> 24) | (((x) & 0x00ff0000) >>  8) | \
        (((x) & 0x0000ff00) <<  8) | (((x) & 0x000000ff) << 24)   )

#endif
