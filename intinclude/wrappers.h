/*
 * wrappers for C 31-bit or 64-bit functions
 */

#include "dio.h"

#if AMODE==31
  #pragma linkage(OPENA, OS)
  #pragma linkage(CLOSEA, OS)

  int OPENA(struct dcb* __ptr32 dcb);
  int CLOSEA(struct dcb* __ptr32 dcb);
  void* __ptr32 MALLOC24(int len);
  void* __ptr32 FREE24(void* __ptr32, int len);

#elif AMODE == 64
  extern int OPENA;
  extern int CLOSEA;
  extern int MALLOC24;
  extern int FREEC24;
  #pragma variable(OPENA, NORENT)
  #pragma variable(CLOSEA,  NORENT)
  #pragma variable(MALLOC24,  NORENT)
  #pragma variable(FREE24,  NORENT)
	#define OPENA(dcb)    call31asm("OPENA", &OPENA, 1, dcb)
	#define CLOSEA(dcb)   call31asm("CLOSEA", &CLOSEA, 1, dcb)
	#define MALLOC24(len) call31asm("MALLOC24", &MALLOC24, 1, len)
	#define FREE24(ptr)   call31asm("FREE24", &FREE24, 2, ptr, len)
#else
	#error "AMODE must be 31 or 64"
#endif
