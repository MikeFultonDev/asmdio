#define _XOPEN_SOURCE
#define _ISOC99_SOURCE
#define _POSIX_SOURCE
#define _OPEN_SYS_FILE_EXT 1
#define _OPEN_SYS_EXT
#define _XOPEN_SOURCE_EXTENDED 1

#include <glob.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

#include "asmdiocommon.h"

#include "util.h"
#include "dio.h"
#include "mem.h"
#include "iosvcs.h"
#include "fm.h"
#include "fmopts.h"
#include "msg.h"
#include "filemap.h"
#include "bpamio.h"
#include "memdir.h"

static void syntax(FILE* stream)
{
  fprintf(stream, 
"usage: m2f [OPTION]... <dataset> <directory> [member ...]\n\
\n\
Options:\n\
  -h, --help          Print out this help.\n\
  -v, --verbose       Provide verbose output.\n\
  -d, --debug         Provide debug output.\n\
  -m, --mapexttollq   Treat the dataset name as a dataset prefix and\n\
                      map low-level qualifier to an extension (default).\n\
  -i, --ignoreext     Do not perform any mapping with the extension\n\
                      and copy all members to the directory with no extension.\n\
\n\
<dataset>             The dataset (or dataset prefix, if -m specified)\n\
                      to copy files from.\n\
<directory>           The directory to copy files to.\n\                      
<member>              One or more members to copy. Wildcards are supported.\n\
                      Put the name in single-quotes if wildcards are used\n\
                      to prevent shell-globbing.\n\
\n\
Examples:\n\
\n\
Copy members in the IBMUSER.PROJ23.MACLIB dataset to the directory /u/ibmuser/proj23/maclib\n\
\n\
  m2f -i ibmuser.proj23.maclib /u/ibmuser/src '*'\n\
\n\
will copy the members from ibmuser.proj23.maclib into the corresponding files /u/ibmuser/src/<member>\n\
where <member> is the member name, in lower-case.\n\
\n\
Copy members in the IBMUSER.PROJ23.* dataset pattern to the directory /u/ibmuser/proj23/src\n\
Assume there are datasets with low-level qualifiers .C, .CPP, .H\n\
e.g. the datasets IBMUSER.PROJ23.SRC.C, IBMUSER.PROJ23.SRC.CPP, IBMUSER.PROJ23.SRC.H\n\
already exist as PDSE's (or PDS's):\n\
\n\
  m2f ibmuser.proj23.src /u/ibmuser/src '*'\n\
\n\
will copy the members from:\n\
  the IBMUSER.PROJ23.SRC.C dataset to corresponding files /u/ibmuser/proj23/src/<member>.c\n\
  the IBMUSER.PROJ23.SRC.CPP dataset to corresponding files /u/ibmuser/proj23/src/<member>.cpp\n\
  the IBMUSER.PROJ23.SRC.H dataset to corresponding files /u/ibmuser/proj23/src/<member>.h\n\ 
where <member> is the member name, in lower-case.\n\ 
");
  return;
}

/*
 * Open the file and initialize the file handle 
 */
static FM_FileHandle* open_file(const char* filename, FM_FileHandle* fh, const FM_Opts* opts)
{
  struct stat stat_info;
  struct f_cnvrt req = {SETCVTOFF, 0, 0};
  int fd = open(filename, O_RDONLY);
  if (fd < 0) {
    return NULL;
  }
  /*
   * Turn auto-convert off
   */
  fcntl(fd, F_CONTROL_CVT, &req);

  memset(fh, 0, sizeof(FM_FileHandle));

  fh->fd = fd;

  if (fstat(fd, &stat_info) < 0) {
    close(fd);
    return NULL;
  }
  fh->tag = stat_info.st_tag;

  fh->active.data = fh->data_a;
  fh->inactive.data = fh->data_b;

  info(&opts->dbg, "Code page of input file:%d\n", fh->tag.ft_ccsid);
  return fh;
}

/*
 * Close the file. Returns zero on success, non-zero otherwise
 */
static int close_file(FM_FileHandle* fh, const FM_Opts* opts)
{
  return close(fh->fd);
}

/*
 * This code will calculate the newline character based
 * on looking at the file tag of the file being copied and,
 * if the file tag isn't specified, it will read the 
 * active data buffer for 'clues'.
 */
static void calc_tag(FM_FileHandle* fh, const FM_Opts* opts)
{
  if (fh->tag.ft_ccsid == 819) {
    fh->newline_char = 0x0A; /* ASCII newline */
    fh->space_char = 0x20;   /* ASCII space   */
  } else if (fh->tag.ft_ccsid == 1047) {
    fh->newline_char = 0x15; /* EBCDIC newline */
    fh->space_char = 0x40;   /* EBCDIC space   */
  } else {
    /* msf: this needs to be fleshed out */
    fh->newline_char = 0x15; /* default to EBCDIC right now */
    fh->space_char = 0x40;   /* default to EBCDIC right now */
  }
}

static int copy_members_to_files(const char* dataset_pattern, const char* dir, FM_Opts* opts)
{
  int rc = 0;
  int ext;
  FM_BPAMHandle bh;
  int sorttime = 0;
  int sortreverse = 0;

  char dataset_buffer[DS_MAX+1];
  const char* dataset;

  strcpy(dataset_buffer, dataset_pattern);
  uppercase(dataset_buffer);
  dataset = dataset_buffer;

  if (open_pds_for_read(dataset, &bh, &opts->dbg)) {
    fprintf(stderr, "Unable to allocate DDName for dataset %s. Files not copied.\n", dataset);
    return 4;
  }

  MEMDIR* md = openmemdir(dataset_buffer, sorttime, sortreverse, &opts->dbg);
  struct mstat* me;
  if (!md) {
    fprintf(stderr, "Unable to open memdir for dataset %s\n", dataset);
    return 4;
  }
  
  struct mstat* mem;
  while (mem = readmemdir(md, &opts->dbg)) {
    if (mem->is_alias) {
      printf("create symbolic link %s to member: %s\n", mem->alias_name, mem->name);
    } else {
      printf("copy member: %s\n", mem->name);
      if (find_member(&bh, mem->name, &opts->dbg)) {
        fprintf(stderr, "Unable to locate PDS member %s\n", mem->name);
        continue;
      }
      int blocks_read = 0;
      while (!read_block(&bh, &opts->dbg)) {
        blocks_read++;
        while (next_record(&bh, &opts->dbg)) {
          printf("record <%.*s>\n", bh.next_record_len, bh.next_record_start);
        }
      }
      fprintf(stderr, "Read %d blocks from member %s\n", blocks_read, mem->name);
    }
  }
  if (closememdir(md, &opts->dbg)) {
    fprintf(stderr, "Error closing memdir for %s\n", dataset);
    return 12;
  }

  if (close_pds(&bh, &opts->dbg)) {
    fprintf(stderr, "Unable to free DDName for dataset %s.\n", dataset);
    rc |= 8;
  }
  return rc;
}


/*
 * main program:
 * -process options, dataset_pattern, directory, and then expand the member patterns 
 *  storing the expanded set of members into globset via 'expand_member_patterns'
 * -create a table, with one entry for each extension in the list of files, e.g
 *  if there are .h, .c, .cpp datasets there would be one entry for each of .h, .c, and .cpp
 * -fill the table by going through each entry and adding in all the members that match that
 *  extension.
 * -copy all the members to the directory, typically by mapping LLQ's to extensions
 *  of datasets (this is why we went to the trouble of organizing the table with extension entries)
 */ 
int main(int argc, char* argv[])
{
  glob_t globset;
  FM_Opts opts;
  int i, first_arg;
  char* dir;
  char* dataset_pattern;
  int first_member_pattern;
  int rc;
  FM_Table* table;

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
  if (opts.map == 1) {
    fprintf(stderr, "-m is not supported yet.\n");
    return 4;
  }

  first_arg = i;

  if (argc - first_arg < 3) {
    syntax(stderr);
    return 4;
  }

  dataset_pattern = argv[first_arg];
  dir = argv[first_arg + 1];

  first_member_pattern = first_arg + 2;

  if (strlen(dataset_pattern) > DS_MAX) {
    fprintf(stderr, "Dataset %s is invalid (too long).\n", dataset_pattern);
    return 8;
  }

  /*
   * Simple approach of just a straight copy of members from dataset to file.
   * No wildcards supported yet.
   */
  if (first_member_pattern != argc-1) {
    fprintf(stderr, "Multiple member patterns specified. Only one pattern of '*' is supported at this time.\n");
    return 4;
  }

  if (strcmp(argv[first_member_pattern], "'*'") && strcmp(argv[first_member_pattern], "*")) {
    fprintf(stderr, "Only the pattern of '*' is supported at this time.\n");
    return 4;
  }


  /*
   * Copy members to files
   */
  rc = copy_members_to_files(dataset_pattern, dir, &opts);
  return 0;
}

