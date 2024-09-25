#ifndef __CLOSECB__
#define __CLOSECB__ 1

#include "asmdiocommon.h"

#pragma pack(packed)
struct closecb {
  int last_entry:1;
  int opts:7;
  int reserved:24;
  void* PTR32 dcb24;
};

#pragma pack(pop)

#endif
