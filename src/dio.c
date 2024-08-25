#include <stdio.h>
#include <stdlib.h>

#include "dio.h"
#include "stow.h"
#include "s99.h"
#include "wrappers.h"

int STOW(union stowlist* __ptr32 listp, struct ihadcb* __ptr32 dcbp, enum stowtype type)
{
  /*
   * Need to set the bits on the dcb and list pointers, so make 
   * them 32 bit to ensure C does not try to do anything 'fancy'
   *
   * For the default case, the dcb should be NULL since it 
   * should have been stored into the list already
   */ 
  unsigned int list = (unsigned int) listp;
  unsigned int dcb = (unsigned int) dcbp;

  /*
   * Clear the high order bit to get to a 'well known state'
   */
  list &= 0x7FFFFFFF;
  dcb  &= 0x7FFFFFFF;

  switch (type) {
    case STOW_A: 
      /* list and dcb should be positive - nothing required */
      break;
    case STOW_R:
      /* list positive, dcb negative */
      dcb |= 0x80000000;
      break;
    case STOW_D:
      /* list negative, dcb positive */
      list |= 0x80000000;
      break;
    case STOW_C:
      /* list negative, dcb negative */
      list |= 0x80000000;
      dcb |= 0x80000000;
      break;
    case STOW_I:
      /* list should be NULL, dcb positive */
      if (list) {
      #ifdef DEBUG
        fprintf(stderr, "stowlist pointer should be NULL for the INIT stowtype.\n");
      #endif
        return -1;
      }
      break;
    default:
      if (dcb) {
      #ifdef DEBUG
        fprintf(stderr, "DCB pointer should be NULL for the default stowtype - DCB is in stowlist\n");
      #endif
        return -1;
      }
      break;
  }
  return STOWA(list, dcb);
}

int S99(struct s99rb* __ptr32 s99rb)
{
  return S99A(s99rb);
}
int S99MSG(struct s99_em* __ptr32 s99em)
{
  return S99MSGA(s99em);
}

int OPEN(struct opencb* __ptr32 opencb)
{
  return OPENA(opencb);
}
int WRITE(struct decb* __ptr32 decb)
{
  return WRITEA(decb);
}
int CHECK(struct decb* __ptr32 decb)
{
  return CHECKA(decb);
}
unsigned NOTE(struct ihadcb* __ptr32 dcb)
{
  return NOTEA(dcb);
}
unsigned DESERV(struct desp* __ptr32 desp)
{
  return DESERVA(desp);
}
int CLOSE(struct closecb* __ptr32 closecb)
{
  return CLOSEA(closecb);
}
void* __ptr32 MALLOC24(size_t bytes)
{
  int ptr24;
  ptr24 = MALOC24A(bytes);
  void* __ptr32 ptr = (void* __ptr32) ptr24;
#ifdef DEBUG
  memset(ptr, 0xFE, bytes);
#else
  memset(ptr, 0x00, bytes);
#endif
  return ptr;
}

int FREE24(void* __ptr32 addr, size_t len)
{
  return FREE24A(addr, len);
}

void* __ptr32 MALLOC31(size_t bytes)
{
  void* __ptr32 p = __malloc31(bytes);
#ifdef DEBUG
  memset(p, 0xFE, bytes);
#else
  memset(p, 0x00, bytes);
#endif
  return p;
}
void FREE31(void* __ptr32 addr)
{
  return free(addr);
}

void dumpstg(FILE* stream, void* p, size_t len)
{
  char* buff = p;
  size_t i;
  for (i=0; i<len; ++i) {
    if ((i != 0) && (i % 16 == 0)) {
      fprintf(stream, "\n");
    }
    if (i % 4 == 0) {
      fprintf(stream, " ");
    }
    fprintf(stream, "%2.2X", buff[i]);
  }
}
