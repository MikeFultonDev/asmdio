#define _XOPEN_SOURCE
#define _ISOC99_SOURCE
#include <glob.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "util.h"
#include "dio.h"

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

typedef struct {
  int cur_value;
  char* values[0];
} F2M_FileTable;

typedef struct {
  const char* key;
  F2M_FileTable* table;
  int count;
} F2M_Entry;

typedef struct {
  size_t max_size;
  size_t size;
  F2M_Entry* entry;
} F2M_Table;

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

static F2M_Table* allocate_table(size_t num_entries)
{
  /* 
   * This is inefficient, but allocate the table assuming one entry
   * for each extension, so guaranteed to be big enough
   */
  F2M_Table* table = malloc(sizeof(F2M_Table));
  if (!table) {
    return NULL;
  }
  F2M_Entry* entry = calloc(num_entries, sizeof(F2M_Entry));
  if (!entry) {
    free(table);
    return NULL;
  }
  table->max_size = num_entries;
  table->size = 0;
  table->entry = entry;
  return table;
}

static int cmpfn(const void* l, const void* r)
{
  F2M_Entry* le = (F2M_Entry*) l;
  F2M_Entry* re = (F2M_Entry*) r;
  return (strcmp(le->key, re->key));
}

static F2M_Entry* find(const char* key, F2M_Table* table)
{
  F2M_Entry key_entry = { key, 0, 0 };
  F2M_Entry* entry = (F2M_Entry*) bsearch(&key_entry, table->entry, table->size, sizeof(F2M_Entry), cmpfn);
  return entry;
}

/*
 * add: low perf implementation - should use hand-written bsearch if this is 
 * important to find the spot to insert.
 */
static F2M_Entry* add(F2M_Entry* entry, F2M_Table* table)
{
  int i;
  for (i=0; i<table->size; ++i) {
    if (strcmp(table->entry[i].key, entry->key) > 0) {
      break;
    }
  }
  memmove(&table->entry[i+1], &table->entry[i], (table->size-i)*sizeof(F2M_Entry));
  memcpy(&table->entry[i], entry, sizeof(F2M_Entry));
  table->size++;

  return &table->entry[i];
}

static const char* extension(const char* path)
{
  char* dot = strrchr(path, '.');
  if (dot) {
    return dot+1;
  } else {
    return NULL;
  }
}

#define MY_PATH_MAX (1024)
static glob_t* expand_file_patterns(char* argv[], int first, int last, const char* dir, glob_t* globset)
{
  char file_pattern[MY_PATH_MAX+1];
  int i;
  int rc;
  for (i=first; i<last; ++i) {
    int flags;
    rc = snprintf(file_pattern, sizeof(file_pattern), "%s/%s", dir, argv[i]);
    if (rc == MY_PATH_MAX) {
      fprintf(stderr, "File pattern: <%s/%s> is too large\n", dir, argv[i]);
      return NULL;
    }
    if (i == first) {
      flags = 0;
    } else {
      flags = GLOB_APPEND;
    }
    rc = glob(file_pattern, flags, errfunc, globset);
  }

  fprintf(stdout, "%d files to be processed\n", globset->gl_pathc);
  return globset;
}

static F2M_Table* create_table(glob_t* globset)
{
  F2M_Table* table = allocate_table(globset->gl_pathc);
  int i;

  if (!table) {
    return NULL;
  }

  /*
   * First pass is to get a count of the number of files with each
   * extension
   */
  for (i=0;i<globset->gl_pathc; ++i) {
    const char* ext = extension(globset->gl_pathv[i]);
    if (!ext || (ext[0] == '\0')) {
      continue;
    }
    F2M_Entry* entry = find(ext, table);
    if (!entry) {
      F2M_Entry new_entry = { ext, 0, 1 };
      add(&new_entry, table);
    } else {
      entry->count++;
    }
  }
  fprintf(stdout, "%d extensions to be processed\n", table->size);
  for (i=0; i<table->size; ++i) {
    fprintf(stdout, "  [%s] has %d entries\n", table->entry[i].key, table->entry[i].count);
  }
  return table;
}

static F2M_Table* fill_table(glob_t* globset, F2M_Table* table)
{
  int i;
  /*
   * Second pass is to allocate each file table now that the number of entries is known.
   */
  for (i=0; i<table->size; ++i) {
    int values = table->entry[i].count;
    F2M_FileTable* file_table = malloc(sizeof(F2M_FileTable) + (sizeof(char*)*values));
    if (!file_table) {
      return NULL;
    }
    file_table->cur_value = 0;
    table->entry[i].table = file_table;
  }
  for (i=0;i<globset->gl_pathc; ++i) {
    const char* ext = extension(globset->gl_pathv[i]);
    if (!ext || (ext[0] == '\0')) {
      continue;
    }
    F2M_Entry* entry = find(ext, table);
    if (entry) {
      entry->table->values[entry->table->cur_value++] = globset->gl_pathv[i];
    } else {
      /* 
       * Error - should have been added already
       */
       fprintf(stderr, "Internal Error: file %s not in table\n", globset->gl_pathv[i]);
    }
  }
  return table;
}

static int map_file_to_member(const char* file, char* member, const F2M_Opts* opts)
{
  const char* slash = strrchr(file, '/');
  if (!slash || slash[1] == '\0') {
    return 1;
  }

  /*
   * msf - add in length checks (truncate if option to truncate specified)
   * msf - add in checks to map invalid characters if option specified to do so
   */
  const char* dot = strrchr(file, '.');
  if (!dot || (dot < slash)) {
    dot = &file[strlen(file)];
  }
  size_t memlen = dot-slash-1;
  if (memlen > MEM_MAX) {
    return 8;
  }

  memcpy(member, &slash[1], memlen);
  member[memlen] = 0;
  uppercase(member);

  return 0;
}

static int map_ext_to_dataset(const char* dataset_pattern, const char* ext, char* dataset, const F2M_Opts* opts)
{
  /*
   * msf - add in length checks (truncate if option to truncate specified)
   * msf - add in checks to map invalid characters if option specified to do so
   * msf - if option not specified to incorporate extension into llq, then don't
   */

  if (strlen(dataset_pattern) + 1 + strlen(ext) > DS_MAX) {
    return 8;
  }

  sprintf(dataset, "%s.%s", dataset_pattern, ext);
  uppercase(dataset);

  return 0;
}

static int copy_files_to_members(F2M_Table* table, const char* dataset_pattern, const F2M_Opts* opts)
{
  int rc = 0;
  int ext;
  int file;
  char member[MEM_MAX+1];
  char dataset[DS_MAX+1];
  for (ext=0; ext < table->size; ext++) {
    for (file=0; file < table->entry[ext].count; ++file) {
      const char* filename = table->entry[ext].table->values[file];
      if (map_file_to_member(filename, member, opts)) {
        fprintf(stderr, "File %s could not be mapped to a valid member. File skipped\n", file);
        rc |= 1;
      } else {
        const char* extname = table->entry[ext].key;
        if (map_ext_to_dataset(dataset_pattern, extname, dataset, opts)) {
          fprintf(stderr, "File %s, dataset pattern %s, and extension %s could not be mapped to a valid dataset. File skipped\n", filename, dataset_pattern, extname);
          rc |= 2;
        } else {
          printf("Copy file %s to dataset member %s(%s)\n", filename, dataset, member);
        }
      }
    }
  }
  return rc;
}

int main(int argc, char* argv[])
{
  glob_t globset;
  F2M_Opts opts;
  int i, first_arg;
  char* dir;
  char* dataset_pattern;
  int first_file_pattern;
  int rc;
  F2M_Table* table;

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
  first_arg = i;

  if (argc - first_arg < 3) {
    syntax(stderr);
    return 4;
  }
  dir = argv[first_arg];
  dataset_pattern = argv[first_arg + 1];
  first_file_pattern = first_arg + 2;

  /*
   * Expand the file patterns and store them into globset
   */
  if (expand_file_patterns(argv, first_file_pattern, argc, dir, &globset) == NULL) {
    return 8;
  }

  /*
   * Create an entry in a sorted table for each extension
   */
  table = create_table(&globset);
  if (!table) {
    return 12;
  }

  /*
   * Fill in all the file entries into the table
   */
  table = fill_table(&globset, table);
  if (!table) {
    return 16;
  }

  /*
   * Copy files to members
   */
  rc = copy_files_to_members(table, dataset_pattern, &opts);
  return 0;
}
