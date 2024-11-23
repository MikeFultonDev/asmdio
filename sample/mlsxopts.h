#ifndef __MLSX_OPTS_H__
  #define __MLSX_OPTS_H__

  #include "asmdiocommon.h"
  #include "dbgopts.h"

  typedef struct {
    DBG_Opts dbg;
    int help:1;
    int alias:1;
    int ccsid:1;
    int longform:1;
    int sorttime:1;
    int sortreverse:1;
  } MLSX_Opts;

  void init_opts(MLSX_Opts* opts);
  int process_opt(MLSX_Opts* opts, char* argv[], int entry);
#endif
