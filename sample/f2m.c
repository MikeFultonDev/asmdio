#define _XOPEN_SOURCE
#define _ISOC99_SOURCE
#include <glob.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include "util.h"
#include "dio.h"
#include "iosvcs.h"

static void syntax(FILE* stream)
{
  fprintf(stream, 
"usage: f2m [OPTION]... <directory> <dataset> [file ...]\n\
\n\
Options:\n\
  -h, --help          Print out this help.\n\
  -v, --verbose       Provide verbose output.\n\
  -m, --mapexttollq   Treat the dataset name as a dataset prefix and\n\
                      map the extension to a a low-level qualifier (default).\n\
  -i, --ignoreext     Do not perform any mapping with the extension\n\
                      and copy all files to the dataset as specified..\n\
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
  int verbose:1;
  int map:1;
} FM_Opts;

typedef struct {
  int cur_value;
  char* values[0];
} FM_FileTable;

typedef struct {
  const char* key;
  FM_FileTable* table;
  int count;
} FM_Entry;

typedef struct {
  size_t max_size;
  size_t size;
  FM_Entry* entry;
} FM_Table;

typedef struct {
  char ddname[DD_MAX+1];
  struct ihadcb* __ptr32 dcb;
  struct opencb* __ptr32 opencb;
  struct decb* __ptr32 decb;
  void* __ptr32 block;
  size_t block_size;
} FM_BPAMHandle;

typedef struct {
  char member[MEM_MAX+1];
  char* filename;
} FM_MemFilePair;

static void init_opts(FM_Opts* opts) {
  opts->help = 0;
  opts->verbose = 0;
  opts->map  = 1;
}

static int info(const FM_Opts* opts, const char* fmt, ...)
{
  va_list arg_ptr;
  int rc;
  va_start(arg_ptr, fmt);
  if (opts->verbose) {
    rc = vfprintf(stdout, fmt, arg_ptr);
  } else {
    rc = 0;
  }
  va_end(arg_ptr);
  return rc;
}

static int process_opt(FM_Opts* opts, char* argv[], int entry)
{
  if (!strcmp(argv[entry], "-h") || !strcmp(argv[entry], "--help")) {
    opts->help = 1;
  } else if (!strcmp(argv[entry], "-v") || !strcmp(argv[entry], "--verbose")) {
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

static int errfunc(const char *epath, int eerrno)
{
  fprintf(stderr, "%s", strerror(eerrno));
  fprintf(stderr, "Path %s is in error. No processing performed\n");
  return -1;
}

static FM_Table* allocate_table(size_t num_entries)
{
  /* 
   * This is inefficient, but allocate the table assuming one entry
   * for each extension, so guaranteed to be big enough
   */
  FM_Table* table = malloc(sizeof(FM_Table));
  if (!table) {
    return NULL;
  }
  FM_Entry* entry = calloc(num_entries, sizeof(FM_Entry));
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
  FM_Entry* le = (FM_Entry*) l;
  FM_Entry* re = (FM_Entry*) r;
  return (strcmp(le->key, re->key));
}

static FM_Entry* find(const char* key, FM_Table* table)
{
  FM_Entry key_entry = { key, 0, 0 };
  FM_Entry* entry = (FM_Entry*) bsearch(&key_entry, table->entry, table->size, sizeof(FM_Entry), cmpfn);
  return entry;
}

/*
 * add: low perf implementation - should use hand-written bsearch if this is 
 * important to find the spot to insert.
 */
static FM_Entry* add(FM_Entry* entry, FM_Table* table)
{
  int i;
  for (i=0; i<table->size; ++i) {
    if (strcmp(table->entry[i].key, entry->key) > 0) {
      break;
    }
  }
  memmove(&table->entry[i+1], &table->entry[i], (table->size-i)*sizeof(FM_Entry));
  memcpy(&table->entry[i], entry, sizeof(FM_Entry));
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
static glob_t* expand_file_patterns(char* argv[], int first, int last, const char* dir, glob_t* globset, const FM_Opts* opts)
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

  info(opts, "%d files to be processed\n", globset->gl_pathc);
  return globset;
}

static FM_Table* create_table(glob_t* globset, const FM_Opts* opts)
{
  FM_Table* table = allocate_table(globset->gl_pathc);
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
    FM_Entry* entry = find(ext, table);
    if (!entry) {
      FM_Entry new_entry = { ext, 0, 1 };
      add(&new_entry, table);
    } else {
      entry->count++;
    }
  }
  info(opts, "%d extensions to be processed\n", table->size);
  for (i=0; i<table->size; ++i) {
    info(opts, "  [%s] has %d entries\n", table->entry[i].key, table->entry[i].count);
  }
  return table;
}

static FM_Table* fill_table(glob_t* globset, FM_Table* table)
{
  int i;
  /*
   * Second pass is to allocate each file table now that the number of entries is known.
   */
  for (i=0; i<table->size; ++i) {
    int values = table->entry[i].count;
    FM_FileTable* file_table = malloc(sizeof(FM_FileTable) + (sizeof(char*)*values));
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
    FM_Entry* entry = find(ext, table);
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

static const char* map_file_to_member(const char* file, char* member, const FM_Opts* opts)
{
  const char* slash = strrchr(file, '/');
  if (!slash || slash[1] == '\0') {
    return NULL;
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
    return NULL;
  }

  memcpy(member, &slash[1], memlen);
  member[memlen] = 0;
  uppercase(member);

  return member;
}

static const char* map_ext_to_dataset(const char* dataset_pattern, const char* ext, char* dataset, const FM_Opts* opts)
{
  /*
   * msf - add in length checks (truncate if option to truncate specified)
   * msf - add in checks to map invalid characters if option specified to do so
   */

  if (strlen(dataset_pattern) + 1 + strlen(ext) > DS_MAX) {
    return NULL;
  }

  sprintf(dataset, "%s.%s", dataset_pattern, ext);
  uppercase(dataset);

  return dataset;
}

static int bpam_open_write(FM_BPAMHandle* handle)
{
  struct ihadcb* __ptr32 dcb;
  struct opencb* __ptr32 opencb;
  struct decb* __ptr32 decb;
  void* __ptr32 block;

  const struct opencb opencb_template = { 1, 0, 0, 0, 0 };
  int rc;

  dcb = dcb_init(handle->ddname);
  if (!dcb) {
    fprintf(stderr, "Unable to obtain storage for OPEN dcb\n");
    return 4;
  }

  /*
   * DCB set to PO, BPAM WRITE and POINT
   */
  dcb->dcbdsgpo = 1;
  dcb->dcbeodad.dcbhiarc.dcbbftek.dcbbfaln = 0x84;
  dcb->dcboflgs = dcbofuex;
  dcb->dcbmacr.dcbmacr2 = dcbmrwrt|dcbmrpt2;

  opencb = MALLOC31(sizeof(struct opencb));
  if (!opencb) {
    fprintf(stderr, "Unable to obtain storage for OPEN cb\n");
    return 4;
  }
  *opencb = opencb_template;
  opencb->dcb24 = dcb;
  opencb->mode = OPEN_OUTPUT;

  rc = OPEN(opencb);
  if (rc) {
    fprintf(stderr, "Unable to perform OPEN. rc:%d\n", rc);
    return rc;
  }

  decb = MALLOC24(sizeof(struct decb));
  if (!decb) {
    fprintf(stderr, "Unable to obtain storage for WRITE decb\n");
    return 4;
  }
  block = MALLOC24(dcb->dcbblksi);
  if (!block) {
    fprintf(stderr, "Unable to obtain storage for WRITE block\n");
    return 4;
  }

  handle->dcb = dcb;
  handle->opencb = opencb;
  handle->decb = decb;
  handle->block = block;
  handle->block_size = dcb->dcbblksi;

  return 0;
}

#define ASCII_A 0x61

static int write_member(const FM_BPAMHandle* bh, const char* member)
{
  const struct stowlist_iff stowlistiff_template = { sizeof(struct stowlist_iff), 0, 0, 0, 0, 0, 0, 0 };
  const struct stowlist_add stowlistadd_template = { "        ", 0, 0, 0, 0 };
  const struct decb decb_template = { 0, 0x8020 };
  union stowlist* stowlist;
  struct stowlist_add* stowlistadd;
  unsigned int ttr;
  size_t memlen = strlen(member);
  int rc;

  if (bh->dcb->dcbexlst.dcbrecfm & dcbrecv) {
    /*
     * write out a variable length record
     */
    memset(bh->block, 0, bh->block_size);
    unsigned short* halfword = (unsigned short*) (bh->block);
    halfword[0] = 8;  /* size of block */
    halfword[2] = 4;  /* size of record */
    bh->dcb->dcbblksi = bh->block_size;
  } else {
    /*
     * write out a record of A's
     */
    bh->dcb->dcbblksi = bh->dcb->dcblrecl;
    memset(bh->block, ASCII_A, bh->dcb->dcbblksi);
  }

  /*
   * Write out a block 
   */
  *(bh->decb) = decb_template;
  SET_24BIT_PTR(bh->decb->dcb24, bh->dcb);
  bh->decb->area = bh->block;

  rc = WRITE(bh->decb);
  if (rc) {
    fprintf(stderr, "Unable to perform WRITE. rc:%d\n", rc);
    return rc;
  }

  rc = CHECK(bh->decb);
  if (rc) {
    fprintf(stderr, "Unable to perform CHECK. rc:%d\n", rc);
    return rc;
  }

  ttr = NOTE(bh->dcb);

  stowlist = MALLOC24(sizeof(struct stowlist_iff));
  stowlistadd = MALLOC24(sizeof(struct stowlist_add));
  if ((!stowlist) || (!stowlistadd)) {
    fprintf(stderr, "Unable to obtain storage for STOW\n");
    return 4;
  }
  stowlist->iff = stowlistiff_template;
  *stowlistadd = stowlistadd_template;
  memcpy(stowlistadd->mem_name, member, memlen);
  STOW_SET_TTR((*stowlistadd), ttr);

  SET_24BIT_PTR(stowlist->iff.dcb24, bh->dcb);
  stowlist->iff.type = STOW_IFF;
  stowlist->iff.direntry = stowlistadd;
  stowlist->iff.ccsid = 819;

  rc = STOW(stowlist, NULL, STOW_IFF);
  if (rc != STOW_IFF_CC_CREATE_OK) {
    fprintf(stderr, "Unable to perform STOW (Does the member already exist?). rc:%d\n", rc);
    return rc;
  } else {
    return 0;
  }
}

static int copy_file_to_member(const FM_BPAMHandle* bh, const char* filename, const char* member)
{
  int rc;
  rc = write_member(bh, member);
  return rc;
}

static int copy_files(const FM_Table* table, int entries, const FM_FileTable* ext_entry, const FM_BPAMHandle* bh, const char* dataset, const FM_Opts* opts)
{
  int file;
  int rc = 0;
  char member_buffer[MEM_MAX+1];
  const char* member;
  for (file=0; file < entries; ++file) {
    const char* filename = ext_entry->values[file];
    const char* member = map_file_to_member(filename, member_buffer, opts);
    if (!member) {
      fprintf(stderr, "File %s could not be mapped to a valid member. File skipped\n", filename);
      rc |= 1;
    } else {
      info(opts, "Copy file %s to dataset member %s(%s)\n", filename, dataset, member);
      if (copy_file_to_member(bh, filename, member)) {
        fprintf(stderr, "File %s could not be copied to %s(%s)\n", filename, dataset, member);
        rc |= 1;
      }
    }
  }
  return rc;
}

static int open_pds_for_write(const char* dataset, FM_BPAMHandle* bpam_handle, const FM_Opts* opts)
{
  struct s99_common_text_unit dsn = { DALDSNAM, 1, 0, 0 };
  struct s99_common_text_unit dd = { DALRTDDN, 1, sizeof(DD_SYSTEM)-1, DD_SYSTEM };
  struct s99_common_text_unit stats = { DALSTATS, 1, 1, { DALSTATS_SHR } };

  int rc = init_dsnam_text_unit(dataset, &dsn);
  if (rc) {
    return rc;
  }
  rc = dsdd_alloc(&dsn, &dd, &stats);
  if (rc) {
    return rc;
  }
  rc = init_dsnam_text_unit(dataset, &dsn);
  if (rc) {
    return 4;
  }
  rc = dsdd_alloc(&dsn, &dd, &stats);
  if (rc) {
    return 4;
  }

  /*
   * Copy system generated DD name into passed in handle
   */
  memcpy(bpam_handle->ddname, dd.s99tupar, dd.s99tulng);
  bpam_handle->ddname[dd.s99tulng] = '\0';

  info(opts, "Allocated DD:%s to %s\n", bpam_handle->ddname, dataset);

  return bpam_open_write(bpam_handle);
}

static int close_pds(const char* dataset, const FM_BPAMHandle* bpam_handle, const FM_Opts* opts)
{
  const struct closecb closecb_template = { 1, 0, 0 };
  struct closecb* __ptr32 closecb;
  int rc;

  struct s99_common_text_unit dd = { DUNDDNAM, 1, 0, 0 };
  int ddname_len = strlen(bpam_handle->ddname);
  dd.s99tulng = ddname_len;
  memcpy(dd.s99tupar, bpam_handle->ddname, ddname_len);

  closecb = MALLOC31(sizeof(struct closecb));
  if (!closecb) {
    fprintf(stderr, "Unable to obtain storage for CLOSE cb\n");
    return 4;
  }
  *closecb = closecb_template;
  closecb->dcb24 = bpam_handle->dcb;

  rc = CLOSE(closecb);
  if (rc) {
    fprintf(stderr, "Unable to perform CLOSE. rc:%d\n", rc);
    return rc;
  }

  rc = ddfree(&dd);
  info(opts, "Free DD:%s\n", bpam_handle->ddname);

  return rc;
}

static int copy_files_to_multiple_dataset_members(const FM_Table* table, const char* dataset_pattern, const FM_Opts* opts)
{
  int rc = 0;
  int ext;
  char dataset_buffer[DS_MAX+1];
  const char* dataset;
  FM_BPAMHandle dd;

  for (ext=0; ext < table->size; ext++) {
    const char* extname = table->entry[ext].key;
    dataset = map_ext_to_dataset(dataset_pattern, extname, dataset_buffer, opts);
    if (!dataset) {
      fprintf(stderr, "Dataset pattern %s, and extension %s could not be mapped to a valid dataset. Extension skipped\n", dataset_pattern, extname);
      rc |= 2;
      continue;
    }
    if (open_pds_for_write(dataset, &dd, opts)) {
      fprintf(stderr, "Unable to allocate DDName for dataset %s. Extension skipped\n", dataset);
      rc |= 4;
      continue;
    }
    rc |= copy_files(table, table->entry[ext].count, table->entry[ext].table, &dd, dataset, opts);

    if (close_pds(dataset, &dd, opts)) {
      fprintf(stderr, "Unable to free DDName for dataset %s.\n", dataset);
      rc |= 8;
      continue;
    }
  }
  return rc;
}

static int cmp_mem_file_pair(const void* l, const void* r)
{
  FM_MemFilePair* lpair = (FM_MemFilePair*) l;
  FM_MemFilePair* rpair = (FM_MemFilePair*) r;

  return (strcmp(lpair->member, rpair->member));
}

static int check_for_duplicate_members(glob_t* globset, const FM_Table* table, const FM_Opts* opts)
{
  FM_MemFilePair* mem_file_pair = calloc(globset->gl_pathc, sizeof(FM_MemFilePair));
  char member_buffer[MEM_MAX+1];
  const char* member;
  int i;
  int num_members = 0;
  int rc = 0;

  /*
   * Create an array of member/file pairs, then sort it by member
   * and then loop through the sorted array and see if there are any
   * duplicates (same member name)
   */
  for (i=0; i<globset->gl_pathc; ++i) {
    member = map_file_to_member(globset->gl_pathv[i], member_buffer, opts);
    if (member) {
      strcpy(mem_file_pair[num_members].member, member);
      mem_file_pair[num_members].filename = globset->gl_pathv[i];
      num_members++;
    }
  }
  qsort(mem_file_pair, num_members, sizeof(FM_MemFilePair), cmp_mem_file_pair);
  for (i=0; i<num_members-1; ++i) {
    if (!strcmp(mem_file_pair[i].member, mem_file_pair[i+1].member)) {
      fprintf(stderr, "Error. File %s and file %s would both be copied to the same member %s\n", 
        mem_file_pair[i].filename, mem_file_pair[i+1].filename, mem_file_pair[i].member); 
      rc = 1;
    }
  }
  return rc;
}

static int copy_files_to_one_dataset_members(glob_t* glob_set, const FM_Table* table, const char* dataset_pattern, const FM_Opts* opts)
{
  int rc = 0;
  int ext;
  FM_BPAMHandle dd;

  char dataset_buffer[DS_MAX+1];
  const char* dataset;

  strcpy(dataset_buffer, dataset_pattern);
  uppercase(dataset_buffer);
  dataset = dataset_buffer;

  if (check_for_duplicate_members(glob_set, table, opts)) {
    fprintf(stderr, "Copy not performed.\n");
    return 8;
  }
  if (open_pds_for_write(dataset, &dd, opts)) {
    fprintf(stderr, "Unable to allocate DDName for dataset %s. Files not copied.\n", dataset);
    return 4;
  }
  for (ext=0; ext < table->size; ext++) {
    rc |= copy_files(table, table->entry[ext].count, table->entry[ext].table, &dd, dataset, opts);
  }
  if (close_pds(dataset, &dd, opts)) {
    fprintf(stderr, "Unable to free DDName for dataset %s.\n", dataset);
    rc |= 8;
  }
  return rc;
}

static int copy_files_to_members(glob_t* globset, const FM_Table* table, const char* dataset_pattern, const FM_Opts* opts)
{
  if (opts->map) {
    return copy_files_to_multiple_dataset_members(table, dataset_pattern, opts);
  } else {
    return copy_files_to_one_dataset_members(globset, table, dataset_pattern, opts);
  }
}

int main(int argc, char* argv[])
{
  glob_t globset;
  FM_Opts opts;
  int i, first_arg;
  char* dir;
  char* dataset_pattern;
  int first_file_pattern;
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
  if (expand_file_patterns(argv, first_file_pattern, argc, dir, &globset, &opts) == NULL) {
    return 8;
  }

  /*
   * Create an entry in a sorted table for each extension
   */
  table = create_table(&globset, &opts);
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
  rc = copy_files_to_members(&globset, table, dataset_pattern, &opts);
  return 0;
}
