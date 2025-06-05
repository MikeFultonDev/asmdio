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
#include <time.h>

#include "util.h"
#include "fm.h"
#include "fmopts.h"
#include "msg.h"
#include "filemap.h"
#include "bpamio.h"
#include "memdir.h"
#include "fsio.h"

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

static const char* filename_from_member(char* file_name, size_t file_name_len, const char* dir, const char* umem, const char* ext)
{
  int rc;
  char mem[MEM_MAX+1];

  /*
   * Change blank padded name to name with no padded blanks.
   */
  strcpy(mem, umem);

  char* first_blank = strchr(mem, IBM1047_SPACE);
  if (first_blank) {
    *first_blank = '\0';
  }
  if (ext == NULL) {
    rc = snprintf(file_name, file_name_len, "%s/%s", dir, mem);
  } else {
    rc = snprintf(file_name, file_name_len, "%s/%s.%s", dir, mem, ext);
  }
  if (rc == file_name_len) {
    fprintf(stderr, "File name: <%s/%s> is too large\n", dir, mem);
    return NULL;
  }
  return file_name;
}

#define MY_PATH_MAX (1024)

static int copy_members_to_files(const char* dataset_pattern, const char* dir, FM_Opts* opts)
{
  int rc = 0;
  int ext;
  FM_BPAMHandle* bh;
  int sorttime = 0;
  int sortreverse = 0;

  char dataset_buffer[DS_MAX+1];
  const char* dataset;
  char file_name[MY_PATH_MAX+1];

  strcpy(dataset_buffer, dataset_pattern);
  uppercase(dataset_buffer);
  dataset = dataset_buffer;

  clock_t start;
  clock_t finish;

  unsigned long long dir_time = 0;
  unsigned long long read_openclose_time = 0;
  unsigned long long write_openclose_time = 0;

  unsigned long long find_time = 0;
  unsigned long long read_time = 0;
  unsigned long long write_time = 0;

  start = clock();
  MEMDIR* md = openmemdir(dataset_buffer, sorttime, sortreverse, &opts->dbg);
  struct mstat* me;
  if (!md) {
    fprintf(stderr, "Unable to open memdir for dataset %s\n", dataset);
    return 4;
  }
  finish = clock();
  dir_time = finish - start;

  start = clock();

  if (!(bh = open_pds_for_read(dataset, &opts->dbg))) {
    fprintf(stderr, "Unable to allocate DDName for dataset %s. Files not copied.\n", dataset);
    return 4;
  }  
  finish = clock();
  read_openclose_time = finish - start;  
  
  struct mstat* mem;
  while (mem = readmemdir(md, &opts->dbg)) {
    if (mem->is_alias) {
      printf("create symbolic link %s to member: %s\n", mem->alias_name, mem->name);
    } else {
      FM_FileHandle fh;
      unsigned short ccsid = mem->ext_ccsid;
      const char* ext = NULL; /* msf - for now - no fancy extension generation */
      const char* filenamep = filename_from_member(file_name, sizeof(file_name), dir, mem->name, ext);
      debug(&opts->dbg, "copy member %s to file %s\n", mem->name, filenamep);
      start = clock();
      if (!open_file_create(filenamep, &fh, ccsid, opts)) {
        fprintf(stderr, "Unable to create file %s\n", filenamep);
        return 12;
      }
      finish = clock();
      write_openclose_time += (finish - start);
      start = clock();
      if (find_member(bh, mem->name, &opts->dbg)) {
        fprintf(stderr, "Unable to locate PDS member %s\n", mem->name);
        continue;
      }
      finish = clock();
      find_time += (finish - start);

      start = clock();
      char* rec;
      size_t rec_len;
      while (read_record_direct(bh, &rec, &rec_len, &opts->dbg) >= 0) {
        finish = clock();
        read_time += (finish - start);

        buffer_write(&fh, rec, rec_len);
        buffer_write(&fh, &fh.newline_char, 1);
        finish = clock();
        write_time += (finish - start);
        start = clock();
      }
      start = clock();
      write_file(&fh, opts);
      finish = clock();
      write_time += (finish - start);   
      start = clock();   
      close_file(&fh, opts);
      finish = clock();
      write_openclose_time += (finish - start);
    }
  }
  if (closememdir(md, &opts->dbg)) {
    fprintf(stderr, "Error closing memdir for %s\n", dataset);
    return 12;
  }

  start = clock();
  if (close_pds(bh, &opts->dbg)) {
    fprintf(stderr, "Unable to free DDName for dataset %s.\n", dataset);
    rc |= 8;
  }
  finish = clock();
  read_openclose_time += (finish - start);


  printf("Relative Timings:\n");
  printf(" Directory:        %llu\n", dir_time);
  printf(" Find:             %llu\n", find_time);
  printf(" Read:             %llu\n", read_time);
  printf(" Write:            %llu\n", write_time);
  printf(" Read Open/Close:  %llu\n", read_openclose_time);
  printf(" Write Open/Close: %llu\n", write_openclose_time);


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

