#define LONG(p)		(((p)[0]<<24)|((p)[1]<<16)|((p)[2]<<8)|((p)[3]))
#define PLONG(p, l)	(((p)[0]=(l)>>24),((p)[1]=(l)>>16),\
	 		((p)[2]=(l)>>8),((p)[3]=((uchar)(l))))
#define SHORT(p)	(((p)[0]<<8)|(p)[1])
#define PSHORT(p,l)	(((p)[0]=(l)>>8),((p)[1]=((uchar)(l))))

#ifndef ctassert		/* Allow lint to override */
#define	ctassert(x)		_ctassert(x, __LINE__)
#define	_ctassert(x, y)		__ctassert(x, y)
#define	__ctassert(x, y)	typedef char __assert ## y[(x) ? 1 : -1]
#endif
