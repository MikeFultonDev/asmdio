#define _XOPEN_SOURCE
#define _ISOC99_SOURCE
#include <glob.h>
#include <string.h>
#include <stdio.h>

static void syntax(FILE* stream)
{
  fprintf(stream, 
"usage: f2m [OPTION]... <directory> <dataset> [file ...]\n\
\n\
Options:\n\
  -h, --help          Print out this help.\n\
  -m, --mapexttollq   Treat the dataset name as a dataset prefix and\n\
                      map the extension to a a low-level qualifier.\n\
\n\
<directory>           The directory to copy files from.\n\
<dataset>             The dataset (or dataset prefix, if -m specified)\n\
                      to copy files to.\n\
<file>                One or more files to copy. Wildcards are supported.\n\
                      Put the name in single-quotes if wildcards are used\n\
                      to prevent shell-globbing.\n\
");
  return;
}
  
typedef struct {
  int help:1;
  int map:1;
} F2M_Opts;

static void init_opts(F2M_Opts* opts) {
  opts->help = 0;
  opts->map  = 0;
}

static int process_opt(F2M_Opts* opts, char* argv[], int entry)
{
  return 0;
}

static int errfunc(const char *epath, int eerrno)
{
  fprintf(stderr, "%s", strerror(eerrno));
  fprintf(stderr, "Path %s is in error. No processing performed\n");
  return -1;
}

#define MY_PATH_MAX (1024)
int main(int argc, char* argv[])
{
  char file_pattern[MY_PATH_MAX+1];
  glob_t globset;
  F2M_Opts opts;
  int i, first_arg;
  char* dir;
  char* dataset_pattern;
  int first_file_pattern;
  int rc;

  init_opts(&opts);

  for (i=1; i<argc; ++i) {
    if (argv[i][0] == '-') {
      i += process_opt(&opts, argv, i);
    } else {
      break;
    }
  }
  first_arg = i;

  if (argc - first_arg < 3) {
    syntax(stderr);
    return 4;
  }
  dir = argv[first_arg];
  dataset_pattern = argv[first_arg + 1];
  first_file_pattern = first_arg + 2;

  for (i=first_file_pattern; i<argc; ++i) {
    int flags;
    rc = snprintf(file_pattern, sizeof(file_pattern), "%s/%s", dir, argv[i]);
    if (rc == MY_PATH_MAX) {
      fprintf(stderr, "File pattern: <%s/%s> is too large\n", dir, argv[i]);
      return 8;
    }
    if (i == first_file_pattern) {
      flags = 0;
    } else {
      flags = GLOB_APPEND;
    }
    rc = glob(file_pattern, flags, errfunc, &globset);
  }

  fprintf(stdout, "%d files to be processed\n", globset.gl_pathc);
  return 0;
}
