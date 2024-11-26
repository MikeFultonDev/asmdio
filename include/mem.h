#ifndef __MEM_H__
#define __MEM_H__ 1

  #include <stdio.h>
  #include "asmdiocommon.h"

  void* PTR32 MALLOC24(size_t len);
  int FREE24(void* PTR32 addr, size_t len);
  void* PTR32 MALLOC31(size_t len);
  void FREE31(void* PTR32 addr);

  void dumpstg(FILE* stream, void* p, size_t len);


#endif
