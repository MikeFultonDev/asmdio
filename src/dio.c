#include "dio.h"
#include "wrappers.h"

int S99(struct s99rb* __ptr32 s99rb)
{
  printf("S99: %p\n", s99rb);
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
void* __ptr32 MALLOC24(int len)
{
  int ptr;
  ptr = MALOC24A(len);
  return (void* __ptr32) ptr;
}
int FREE24(void* __ptr32 addr, int len)
{
  return FREE24A(addr, len);
}
