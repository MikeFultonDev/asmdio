#ifndef __MSG_H__
  #define __MSG_H__ 1
  
  #include "dbgopts.h"

  int info(const DBG_Opts* opts, const char* fmt, ...);
  int debug(const DBG_Opts* opts, const char* fmt, ...);
#endif
