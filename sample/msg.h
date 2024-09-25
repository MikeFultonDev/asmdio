#ifndef __MSG_H__
  #define __MSG_H__ 1
  
  #include "asmdiocommon.h"
  #include "fmopts.h"

  int info(const FM_Opts* opts, const char* fmt, ...);
  int debug(const FM_Opts* opts, const char* fmt, ...);
#endif
