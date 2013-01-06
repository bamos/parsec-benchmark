/* Wrapper file to include correct header

*/
//#ifdef ARCH_ppc64
//
//#ifdef PARTYPE_null
//#include "rt_null_ppc64.h"
//#elif PARTYPE_pthread
//#include "rt_pthread_ppc64.h"
//#elif PARTYPE_shq
//#include "rt_shq_ppc64.h"
//#endif

//#elif ARCH_i386

//#ifdef PARTYPE_null
//#include "rt_null_i386.h"
//#elif PARTYPE_pthread
#include "rt_pthread_i386.h"
//#elif PARTYPE_shq
//#include "rt_shq_i386.h"
//#endif

//#endif
