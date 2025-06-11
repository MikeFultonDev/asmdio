#ifndef __MSG_H__
  #define __MSG_H__ 1
  
  #include "dbgopts.h"

  /*
   * info prints informational messages (controlled by the verbose flag in DBG_Opts).
   * If the verbose flag is on, the message is printed. If it is off, it is suppressed.
   * The parameters that follow are standard 'printf' style parameters.
   */
  int info(const DBG_Opts* opts, const char* fmt, ...);

  /*
   * debug prints debug messages (controlled by the debug flag in DBG_Opts).
   * If the debug flag is on, the message is printed. If it is off, it is suppressed.
   * The parameters that follow are standard 'printf' style parameters.
   */  
  int debug(const DBG_Opts* opts, const char* fmt, ...);

  /*
   * errmsg prints error messages.
   * If opts->msg_buffer is not NULL, the message is written to the provided buffer.
   * Otherwise, the message is printed to stderr.
   * The parameters that follow are standard 'printf' style parameters.
   */  
  int errmsg(const DBG_Opts* opts, const char* fmt, ...);
#endif
