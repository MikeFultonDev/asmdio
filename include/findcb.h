#ifndef __FINDCB_H__
#define __FINDCB_H__

#include "asmdiocommon.h"

#pragma pack(1)
struct findcb {
  //unsigned short cb_len;
  //unsigned short filler;
  //unsigned int mname_len;
  char mname[8];
  //unsigned int gen_num;
};

#pragma pack(pop)

#endif
