#define _XOPEN_SOURCE
#define _ISOC99_SOURCE
#define _POSIX_SOURCE
#define _OPEN_SYS_FILE_EXT
#include <stdarg.h>
#include <stdio.h>
#include <_Nascii.h>

#include "msg.h"

static int _emit_msg(FILE* fallback_stream, const DBG_Opts* opts, const char* fmt, va_list arg_ptr) {
  int rc;

  if (opts && opts->msg_buffer && opts->msg_buffer->buffer && opts->msg_buffer->size > 0) {
    DBG_MsgBuffer* msg_buf = opts->msg_buffer;
    
    rc = vsnprintf(msg_buf->buffer, msg_buf->size, fmt, arg_ptr);

    if (rc > 0) {
      if (__isASCII()) {
        size_t msglen = (rc < msg_buf->size) ? rc : msg_buf->size - 1;
        if (msglen > 0) {
          __e2a_l(msg_buf->buffer, msglen);
        }
      }
    }
    return rc;

  } else {
    // No buffer configured, so fall back to printing to the specified stream.
    static char temp_buffer[4096];

    rc = vsnprintf(temp_buffer, sizeof(temp_buffer), fmt, arg_ptr);

    if (rc < 0) {
      return rc; 
    }

    if (__isASCII()) {
      size_t msglen = (rc < sizeof(temp_buffer)) ? rc : sizeof(temp_buffer) - 1;
      if (msglen > 0) {
        __e2a_l(temp_buffer, msglen);
      }
    }

    fputs(temp_buffer, fallback_stream);
    
    return rc;
  }
}

int info(const DBG_Opts* opts, const char* fmt, ...)
{
  va_list arg_ptr;
  int rc;
  
  // Proceed only if verbose option is set
  if (!opts || !opts->verbose) {
    return 0;
  }

  va_start(arg_ptr, fmt);
  rc = _emit_msg(stdout, opts, fmt, arg_ptr);
  va_end(arg_ptr);
  
  return rc;
}

int debug(const DBG_Opts* opts, const char* fmt, ...)
{
  va_list arg_ptr;
  int rc;

  if (!opts || !opts->debug) {
    return 0;
  }

  va_start(arg_ptr, fmt);
  rc = _emit_msg(stdout, opts, fmt, arg_ptr);
  va_end(arg_ptr);

  return rc;
}

int errmsg(const DBG_Opts* opts, const char* fmt, ...)
{
  va_list arg_ptr;
  int rc;

  va_start(arg_ptr, fmt);
  rc = _emit_msg(stderr, opts, fmt, arg_ptr);
  va_end(arg_ptr);

  return rc;
}

