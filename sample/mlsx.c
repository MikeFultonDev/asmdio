#include "mlsxopts.h"
#include "memdir.h"
#include <stdio.h>


static void syntax(FILE* stream)
{
  fprintf(stream,
"usage: mlsx [OPTION]... <dataset>\n\
\n\
Options:\n\
  -h, --help          Print out this help.\n\
  -v, --verbose       Provide verbose output.\n\
  -d, --debug         Provide debug output.\n\
  -a, --alias         Print aliases.\n\
  -l, --long          Provide long-form output.\n\
\n\
<dataset>             The dataset to list members from.\n\
\n\
Examples:\n\
\n\
List the members and aliases for the PDSE IBMUSER.PROJ23.SRC:\n\
\n\
  mlsx -a ibmuser.proj23.src\n\
\n\
List the timestamps and CCSIDs for the members of PDSE IBMUSER.PROJ23.SRC:\n\
\n\
  mlsx -l ibmuser.proj23.src\n\
\n\
");
  return;
}

int main(int argc, char* argv[])
{

  MLSX_Opts opts;
  int i, first_arg;

  /*
   * Process command-line options
   */
  init_opts(&opts);

  for (i=1; i<argc; ++i) {
    if (argv[i][0] == '-') {
      i += process_opt(&opts, argv, i);
    } else {
      break;
    }
  }
  if (opts.help) {
    syntax(stdout);
    return 0;
  }

  first_arg = i;

  if (argc - first_arg < 1) {
    syntax(stderr);
    return 4;
  }

  const char* ds = argv[first_arg];

  MEMDIR* md = openmemdir(ds, &opts.dbg);
  struct mement* me;
  if (!md) {
    fprintf(stderr, "Unable to open dataset %s\n", ds);
    return 4;
  }
  while (me = readmemdir(md, &opts.dbg)) {
    struct mstat mem;
    if (!mstat(me, &mem, &opts.dbg)) {
      fprintf(stderr, "Unable to retrieve information for member in %s\n", ds);
      return 8;
    }
  }
  if (closememdir(md, &opts.dbg)) {
    fprintf(stderr, "Error closing memdir for %s\n", ds);
    return 12;
  }
  return 0;
}
