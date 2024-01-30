/*
 * wrappers for C 31-bit or 64-bit functions
 */

#if AMODE==31
  #pragma linkage(OPENA,    OS)
  #pragma linkage(CLOSEA,   OS)
  #pragma linkage(MALOC24A, OS)
  #pragma linkage(FREE24A,  OS)

  struct opencb;

  int OPENA(struct opencb* __ptr32 opencb);
  int CLOSEA(struct opencb* __ptr32 opencb);
  void* __ptr32 MALOC24A(int len);
  int FREE24A(void* __ptr32, int len);

#elif AMODE == 64
  extern int OPENA;
  extern int CLOSEA;
  extern int MALOC24A;
  extern int FREE24A;
  #pragma variable(OPENA,    NORENT)
  #pragma variable(CLOSEA,   NORENT)
  #pragma variable(MALOC24A, NORENT)
  #pragma variable(FREE24A,  NORENT)
	#define OPENA(dcb)        call31asm("OPENA", &OPENA, 1, dcb)
	#define CLOSEA(dcb)       call31asm("CLOSEA", &CLOSEA, 1, dcb)
	#define MALOC24A(len)     call31asm("MALOC24A", &MALOC24A, 1, len)
	#define FREE24A(ptr,len)  call31asm("FREE24A", &FREE24A, 2, ptr, len)
#else
	#error "AMODE must be 31 or 64"
#endif
