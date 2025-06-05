#define _XOPEN_SOURCE
#define _ISOC99_SOURCE
#define _POSIX_SOURCE
#define _OPEN_SYS_FILE_EXT
#include <stdarg.h>
#include <stdio.h>

#include "msg.h"

int info(const DBG_Opts* opts, const char* fmt, ...)
{
  va_list arg_ptr;
  int rc;
  if (!opts->verbose) {
    return 0;
  }

  va_start(arg_ptr, fmt);
  rc = vfprintf(stdout, fmt, arg_ptr);
  va_end(arg_ptr);
  return rc;
}

int debug(const DBG_Opts* opts, const char* fmt, ...)
{
  va_list arg_ptr;
  int rc;

  if (!opts->debug) {
    return 0;
  }

  va_start(arg_ptr, fmt);
  rc = vfprintf(stdout, fmt, arg_ptr);
  va_end(arg_ptr);
  return rc;
}
