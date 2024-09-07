#define _XOPEN_SOURCE
#define _ISOC99_SOURCE
#include <glob.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

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
  const char* key;
  char** values;
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

static F2M_Table* create(size_t num_entries)
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
      printf("break at %d comparing %s to %s\n", i, entry->key, table->entry[i].key);
      break;
    }
  }
  printf("i after loop:%d\n", i);
  memmove(&table->entry[i+1], &table->entry[i], (table->size-i)*sizeof(F2M_Entry));
  memcpy(&table->entry[i], entry, sizeof(F2M_Entry));
  table->size++;

  for (i=0; i<table->size; ++i) {
    printf("t[%d]: %s\n", i, table->entry[i].key);
  }
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
  F2M_Table* table;

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

  /*
   * Create an entry in a sorted table for each extension
   * First pass is to get a count of the number of files with each
   * extension
   */

  table = create(globset.gl_pathc);
  for (i=0;i<globset.gl_pathc; ++i) {
    const char* ext = extension(globset.gl_pathv[i]);
    if (!ext || (ext[0] == '\0')) {
      printf("%s has no extension - skipped\n", globset.gl_pathv[i]);
      continue;
    }
    F2M_Entry* entry = find(ext, table);
    if (!entry) {
      F2M_Entry new_entry = { ext, 0, 0 };
      add(&new_entry, table);
    }
  }
  fprintf(stdout, "%d extensions to be processed\n", table->size);
  return 0;
}
