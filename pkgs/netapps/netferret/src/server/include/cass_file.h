#ifndef __CASS_FILE__
#define __CASS_FILE__

#include <stdio.h>
#include <cass_endian.h>

typedef FILE CASS_FILE;

static inline CASS_FILE *cass_open (const char *p, const char *m)
{
	return fopen(p, m);
}

static inline void cass_close (CASS_FILE *h)
{
	fclose(h);
}

static inline cass_size_t cass_read (void *p, cass_size_t size, cass_size_t nmemb, CASS_FILE *f)
{
    if (!isLittleEndian()) {
        assert(0);
    }
	return fread(p, size, nmemb, f);
}

static inline cass_size_t cass_write (const void *p, cass_size_t size, cass_size_t nmemb, CASS_FILE *f)
{
    if (!isLittleEndian()) {
        assert(0);
    }
	return fwrite(p, size, nmemb, f);
}

#define GEN_CASS_IO(type,name)	\
static inline int cass_read_##name (type *buf, size_t nmemb, CASS_FILE *in) \
{ \
	return fread(buf, sizeof (type), nmemb, in); \
} \
\
static inline int cass_write_##name (const type *buf, size_t nmemb, CASS_FILE *out) \
{ \
	return fwrite(buf, sizeof (type), nmemb, out); \
} \

char *cass_read_pchar (CASS_FILE *in);
int cass_write_pchar (const char *buf, CASS_FILE *in);

#define cass_printf	fprintf

/*
GEN_CASS_IO(int32_t, int32)
GEN_CASS_IO(uint32_t, uint32)
GEN_CASS_IO(uint64_t, uint64)
GEN_CASS_IO(cass_size_t, size)
GEN_CASS_IO(float, float)
GEN_CASS_IO(double, double) */
GEN_CASS_IO(char, char)

static inline int cass_read_int32 (int32_t *buf, size_t nmemb, CASS_FILE *in) {
    int n = fread(buf, sizeof(int32_t), nmemb, in);
    if (!isLittleEndian()) {
        int i;
        for (i = 0; i < n; ++i) {
            buf[i] = bswap_int32(buf[i]);
        }
    }
    return n;
}

static inline int cass_write_int32 (int32_t *buf, size_t nmemb, CASS_FILE *out) {
    assert(0);
    return -1;
}

static inline int cass_read_uint32 (uint32_t *buf, size_t nmemb, CASS_FILE *in) {
    int n = fread(buf, sizeof(uint32_t), nmemb, in);
    if (!isLittleEndian()) {
        int i;
        for (i = 0; i < n; ++i) {
            buf[i] = bswap_int32(buf[i]);
        }
    }
    return n;
}

static inline int cass_write_uint32 (uint32_t *buf, size_t nmemb, CASS_FILE *out) {
    assert(0);
    return -1;
}

#define cass_read_size cass_read_uint32
#define cass_write_size cass_write_uint32

static inline int cass_read_float (float *buf, size_t nmemb, CASS_FILE *in) {
    int n = fread(buf, sizeof(float), nmemb, in);
    if (!isLittleEndian()) {
        int i;
        for (i = 0; i < n; ++i) {
            buf[i] = bswap_float(buf[i]);
        }
    }
    return n;
}

static inline int cass_write_float (float *buf, size_t nmemb, CASS_FILE *out) {
    assert(0);
    return -1;
}

#endif
