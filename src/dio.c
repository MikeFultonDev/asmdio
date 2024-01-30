#include "dio.h"
#include "wrappers.h"

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
  void* __ptr32 ptr;
  ptr = MALOC24A(len);
  return ptr;
}
int FREE24(void* __ptr32 addr, int len)
{
  return FREE24A(addr, len);
}
