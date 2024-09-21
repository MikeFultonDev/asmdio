#include "msg.h"
#include "fm.h"
#include <stdarg.h>
#include <stdio.h>

int info(const FM_Opts* opts, const char* fmt, ...)
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

int debug(const FM_Opts* opts, const char* fmt, ...)
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
