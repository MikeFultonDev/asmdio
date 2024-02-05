#include <stdio.h>
#include <stdlib.h>

#include "dio.h"
#include "stow.h"
#include "s99.h"
#include "wrappers.h"

int STOW(struct ihadcb* __ptr32 dcbp, union stowlist* __ptr32 listp, enum stowtype type)
{
  /*
   * Need to set the bits on the dcb and list pointers, so make 
   * them 32 bit to ensure C does not try to do anything 'fancy'
   */ 
  unsigned int dcb = (unsigned int) dcbp;
  unsigned int list = (unsigned int) listp;
  return STOWA(dcb, list);
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
int CLOSE(struct opencb* __ptr32 opencb)
{
  return CLOSEA(opencb);
}
void* __ptr32 MALLOC24(size_t len)
{
  int ptr;
  ptr = MALOC24A(len);
#ifdef DEBUG
  memset(p, 0xFE, bytes);
#endif
  return (void* __ptr32) ptr;
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
    if (i % 4 == 0) {
      fprintf(stream, " ");
    }
    fprintf(stream, "%2.2X", buff[i]);
  }
}
