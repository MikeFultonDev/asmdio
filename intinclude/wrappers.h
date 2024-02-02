/*
 * wrappers for C 31-bit or 64-bit functions
 */

#if AMODE==31
  #pragma linkage(S99A,     OS)
  #pragma linkage(S99MSGA,  OS)
  #pragma linkage(OPENA,    OS)
  #pragma linkage(CLOSEA,   OS)
  #pragma linkage(MALOC24A, OS)
  #pragma linkage(FREE24A,  OS)

  struct s99rb;
  struct opencb;
  struct s99_em;

  int S99A(struct s99rb* __ptr32 addr);
  int S99MSGA(struct s99_em* __ptr32 addr);
  int OPENA(struct opencb* __ptr32 opencb);
  int CLOSEA(struct opencb* __ptr32 opencb);

  int MALOC24A(size_t len);
  int FREE24A(void* __ptr32 addr, size_t len);

#elif AMODE == 64
  extern int S99A;
  extern int S99MSGA;
  extern int OPENA;
  extern int CLOSEA;
  extern int MALOC24A;
  extern int FREE24A;
  #pragma variable(S99A,     NORENT)
  #pragma variable(S99MSGA,  NORENT)
  #pragma variable(OPENA,    NORENT)
  #pragma variable(CLOSEA,   NORENT)
  #pragma variable(MALOC24A, NORENT)
  #pragma variable(FREE24A,  NORENT)
	#define S99A(ptr)         call31asm("S99A", &S99A, 1, ptr)
	#define S99MSGA(ptr)      call31asm("S99MSGA", &S99MSGA, 1, ptr)
	#define OPENA(dcb)        call31asm("OPENA", &OPENA, 1, dcb)
	#define CLOSEA(dcb)       call31asm("CLOSEA", &CLOSEA, 1, dcb)
	#define MALOC24A(len)     call31asm("MALOC24A", &MALOC24A, 1, len)
	#define FREE24A(ptr,len)  call31asm("FREE24A", &FREE24A, 2, ptr, len)
#else
	#error "AMODE must be 31 or 64"
#endif
