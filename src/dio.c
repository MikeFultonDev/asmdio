#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "asmdiocommon.h"
#include "dio.h"
#include "stow.h"
#include "util.h"
#include "s99.h"
#include "wrappers.h"

//#define DEBUG 1

const struct s99_rbx s99rbxtemplate = {"S99RBX",S99RBXVR,{0,1,0,0,0,0,0},0,0,0};

static unsigned int complement(unsigned int x) {
  return (~x) + 1;
}

int STOW(union stowlist* PTR32 listp, struct ihadcb* PTR32 dcbp, enum stowtype type)
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
      /* list positive, dcb complement */
      dcb = complement(dcb);
      break;
    case STOW_D:
      /* list complement, dcb positive */
      list = complement(list);
      break;
    case STOW_C:
      /* list complement, dcb complement */
      list = complement(list);
      dcb = complement(list);
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

int S99(struct s99rb* PTR32 s99rb)
{
  return S99A(s99rb);
}
int S99MSG(struct s99_em* PTR32 s99em)
{
  return S99MSGA(s99em);
}

int OPEN(struct opencb* PTR32 opencb)
{
  return OPENA(opencb);
}
int FIND(struct findcb* PTR32 findcb, struct ihadcb* PTR32 dcb)
{
  return FINDA(findcb, dcb);
}
int READ(struct decb* PTR32 decb)
{
  return READA(decb);
}
int WRITE(struct decb* PTR32 decb)
{
  return WRITEA(decb);
}
int CHECK(struct decb* PTR32 decb)
{
  return CHECKA(decb);
}
unsigned NOTE(struct ihadcb* PTR32 dcb)
{
  return NOTEA(dcb);
}
unsigned DESERV(struct desp* PTR32 desp)
{
  return DESERVA(desp);
}
int CLOSE(struct closecb* PTR32 closecb)
{
  return CLOSEA(closecb);
}
void* PTR32 MALLOC24(size_t bytes)
{
  int ptr24;
  ptr24 = MALOC24A(bytes);
  void* PTR32 ptr = (void* PTR32) ptr24;
  if (ptr24 == 0) {
    fprintf(stderr, "Internal Error: Unable to allocate %d bytes below the bar\n", bytes);
  } else {
#ifdef DEBUG
  memset(ptr, 0xFE, bytes);
#else
  memset(ptr, 0x00, bytes);
#endif
  }
  return ptr;
}

int FREE24(void* PTR32 addr, size_t len)
{
  return FREE24A(addr, len);
}

void* PTR32 MALLOC31(size_t bytes)
{
  void* PTR32 p = __malloc31(bytes);
  if (p == 0) {
    fprintf(stderr, "Internal Error: Unable to allocate %d bytes below the line\n", bytes);
  } else {
#ifdef DEBUG
  memset(p, 0xFE, bytes);
#else
  memset(p, 0x00, bytes);
#endif
  }
  return p;
}
void FREE31(void* PTR32 addr)
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
