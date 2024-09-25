#ifndef __FM_OPTS_H__
  #define __FM_OPTS_H__

  #include "asmdiocommon.h"
  typedef struct {
    int help:1;
    int verbose:1;
    int debug:1;
    int map:1;
  } FM_Opts;

  void init_opts(FM_Opts* opts);
  int process_opt(FM_Opts* opts, char* argv[], int entry);
#endif
