/*-
 * THE BEER-WARE LICENSE
 *
 */

#include <sys/bsd_cdefs.h>
//__FBSDID("$FreeBSD$");

#include <sys/bsd_types.h>
#include <sys/bsd_systm.h>

#if 0
void
bcopy(const void *src, void *dst, size_t len)
{
}

void
memcpy(void *dst, const void *src, size_t len)
{
	bcopy(src, dst, len);
}

void
bzero(void *b, size_t len)
{
	char *p = b;

	while (len-- != 0)
		*p++ = 0;
}
#endif

int
copyin(const void *udaddr, void *kaddr, size_t len)
{
    memcpy(kaddr, udaddr, len);
    return 0;
}

int
copyout(const void *kdaddr, void *uaddr, size_t len)
{
    memcpy(uaddr, kdaddr, len);
    return 0;
}

int
copyinstr(const void *udaddr, void *kaddr, size_t len, size_t *done)
{
    return 0;
}

int
subyte(void *addr, int byte)
{
	return (0);
}

