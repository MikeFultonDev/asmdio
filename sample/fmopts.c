#include <string.h>
#include <stdio.h>
#include "asmdiocommon.h"
#include "fmopts.h"

void init_opts(FM_Opts* opts) 
{
  opts->help = 0;
  opts->verbose = 0;
  opts->map  = 1;
}

int process_opt(FM_Opts* opts, char* argv[], int entry)
{
  if (!strcmp(argv[entry], "-h") || !strcmp(argv[entry], "--help")) {
    opts->help = 1;
  } else if (!strcmp(argv[entry], "-v") || !strcmp(argv[entry], "--verbose")) {
    opts->verbose = 1;
  } else if (!strcmp(argv[entry], "-d") || !strcmp(argv[entry], "--debug")) {
    opts->debug = 1;
    opts->verbose = 1;
  } else if (!strcmp(argv[entry], "-m") || !strcmp(argv[entry], "--mapexttollq")) {
    opts->map = 1;
  } else if (!strcmp(argv[entry], "-i") || !strcmp(argv[entry], "--ignoreext")) {
    opts->map = 0;
  } else {
    fprintf(stderr, "Option %s not recognized. Option ignored.\n", argv[entry]);
  }
  return 0;
}
