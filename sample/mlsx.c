#define _XOPEN_SOURCE
#define _ISOC99_SOURCE
#define _POSIX_SOURCE
#define _OPEN_SYS_FILE_EXT
#define _XOPEN_SOURCE_EXTENDED 1

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

#include "dio.h"
#include "mem.h"
#include "memdir.h"
#include "mlsxopts.h"
#include "util.h"

/**
 * @brief Display syntax help.
 *
 * @param stream File stream for output.
 */
static void syntax(FILE* stream)
{
  fprintf(stream,
"usage: mlsx [OPTION]... <dataset>\n\
\n\
Options:\n\
  -a, --alias         Print aliases.\n\
  -d, --debug         Provide debug output.\n\
  -h, --help          Print out this help.\n\
  -l, --long          Provide long-form output.\n\
  -r, --reverse       Reverse the sort.\n\
  -t, --time          Sort by time file last changed.\n\
  -T, --tag           Print CCSID.\n\
  -v, --verbose       Provide verbose output.\n\
\n\
<dataset>             The dataset to list members from.\n\
\n\
Examples:\n\
\n\
List the members and aliases for the PDSE IBMUSER.PROJ23.SRC:\n\
\n\
  mlsx -a ibmuser.proj23.src\n\
\n\
List the last changed user and timestamp and CCSIDs for the members of PDSE IBMUSER.PROJ23.SRC:\n\
\n\
  mlsx -l -T ibmuser.proj23.src\n\
\n\
List the members and aliases, sorted by reverse time, for the members of PDSE SYS1.PROCLIB:\n\
\n\
  mlsx -l -r -t sys1.proclib\n\
\n\
");
  return;
}

static void print_mem(struct mstat* mstat, MLSX_Opts* opts)
{
  char crttime_buff[4+1+2+1+2+1];                /* YYYY/MM/DD          */
  char modtime_buff[4+1+2+1+2+1+2+1+2+1+2+1];    /* YYYY/MM/DD HH:MM:SS */
  char exttime_buff[4+1+2+1+2+1+2+1+2+1+2+1];    /* YYYY/MM/DD HH:MM:SS */

  unsigned int memid = mstat->mem_id;
  const char* name = (mstat->name) ? mstat->name : ".";
  const char* alias_name = (mstat->alias_name) ? mstat->alias_name : ".";;

  char* ext_id = (mstat->ext_id) ? mstat->ext_id : NULL;
  unsigned short ccsid = mstat->ext_ccsid;
  if (mstat->has_ext) {
    struct tm* exttime = localtime(&mstat->ext_changed);
    strftime(exttime_buff, sizeof(exttime_buff), "%Y/%m/%d %H:%M:%S", exttime);
  } else {
    strcpy(exttime_buff, ".");
  }
  short ver_num = mstat->ispf_version;
  short mod_num = mstat->ispf_modification;
  int cur = mstat->ispf_current_lines;
  int init = mstat->ispf_initial_lines;
  int mod = mstat->ispf_modified_lines;
  if (mstat->ispf_stats) {
    struct tm* ispf_created_time = localtime(&mstat->ispf_created);
    struct tm* ispf_changed_time = localtime(&mstat->ispf_changed);
    strftime(crttime_buff, sizeof(crttime_buff), "%Y/%m/%d", ispf_created_time);
    strftime(modtime_buff, sizeof(modtime_buff), "%Y/%m/%d %H:%M:%S", ispf_changed_time);
  } else {
    strcpy(crttime_buff, ".");
    strcpy(modtime_buff, ".");
  }
  char* ispf_id = (mstat->ispf_id) ? mstat->ispf_id : NULL;

  char* user_id = (ext_id != NULL) ? ext_id : ispf_id;
  if (user_id == NULL) {
    user_id = ".";
  }

  char* chgtime_buff = (mstat->has_ext) ? exttime_buff : modtime_buff;

  if (opts->ccsid) {
    printf("%8x ", ccsid);
  }
  if (opts->longform) {
    if (mstat->is_alias) {
      printf("%8s %10d %21s %s -> %s\n", user_id, cur, chgtime_buff, alias_name, name);
    } else {
      printf("%8s %10d %21s %s\n", user_id, cur, chgtime_buff, name);
    }
  } else if (opts->alias) {
    if (mstat->is_alias) {
      printf("%s -> %s\n", alias_name, name);
    } else {
      printf("%s\n", name);
    }
  } else {
    printf("%s\n", name);
  }
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

  if (strlen(ds) > DS_MAX) {
    fprintf(stderr, "Dataset %s is invalid (too long).\n", ds);
    return 8;
  }

  char dataset_buffer[DS_MAX+1];

  strcpy(dataset_buffer, ds);
  uppercase(dataset_buffer);

  MEMDIR* md = openmemdir(dataset_buffer, opts.sorttime, opts.sortreverse, &opts.dbg);
  struct mstat* me;
  if (!md) {
    fprintf(stderr, "Unable to open dataset %s\n", ds);
    return 4;
  }
  struct mstat* mem;
  while (mem = readmemdir(md, &opts.dbg)) {
    print_mem(mem, &opts);
  }
  if (closememdir(md, &opts.dbg)) {
    fprintf(stderr, "Error closing memdir for %s\n", ds);
    return 12;
  }
  return 0;
}
