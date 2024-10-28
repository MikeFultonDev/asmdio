#include <string.h>
#include <stdio.h>
#include "asmdiocommon.h"
#include "mlsxopts.h"

void init_opts(MLSX_Opts* opts) 
{
  opts->help = 0;
  opts->dbg.verbose = 0;
  opts->dbg.debug = 0;
  opts->longform  = 0;
  opts->alias  = 0;
}

int process_opt(MLSX_Opts* opts, char* argv[], int entry)
{
  if (!strcmp(argv[entry], "-h") || !strcmp(argv[entry], "--help")) {
    opts->help = 1;
  } else if (!strcmp(argv[entry], "-v") || !strcmp(argv[entry], "--verbose")) {
    opts->dbg.verbose = 1;
  } else if (!strcmp(argv[entry], "-d") || !strcmp(argv[entry], "--debug")) {
    opts->dbg.debug = 1;
    opts->dbg.verbose = 1;
  } else if (!strcmp(argv[entry], "-l") || !strcmp(argv[entry], "--long")) {
    opts->longform = 1;
  } else if (!strcmp(argv[entry], "-a") || !strcmp(argv[entry], "--alias")) {
    opts->alias = 1;
  } else {
    fprintf(stderr, "Option %s not recognized. Option ignored.\n", argv[entry]);
  }
  return 0;
}
