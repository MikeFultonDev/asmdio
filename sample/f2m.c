#define _XOPEN_SOURCE
#define _ISOC99_SOURCE
#define _POSIX_SOURCE
#define _OPEN_SYS_FILE_EXT
#include <glob.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

#include "asmdiocommon.h"

#include "util.h"
#include "dio.h"
#include "iosvcs.h"
#include "fm.h"
#include "fmopts.h"
#include "msg.h"
#include "filemap.h"
#include "bpamio.h"

static void syntax(FILE* stream)
{
  fprintf(stream, 
"usage: f2m [OPTION]... <directory> <dataset> [file ...]\n\
\n\
Options:\n\
  -h, --help          Print out this help.\n\
  -v, --verbose       Provide verbose output.\n\
  -d, --debug         Provide debug output.\n\
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
\n\
Examples:\n\
\n\
Copy files in the src directory to the IBMUSER.PROJ23.SRC dataset pattern\n\
Assume there are files with extensions .c, .h, .lst\n\
and the datasets IBMUSER.PROJ23.SRC.C, IBMUSER.PROJ23.SRC.H, IBMUSER.PROJ23.SRC.LST\n\
have already been pre-allocated as PDSE's (or PDS's):\n\
\n\
  f2m src ibmuser.proj23.src '*.*'\n\
\n\
will copy:\n\
  the .c files to corresponding dataset members IBMUSER.PROJ23.SRC.C\n\
  the .h files to corresponding dataset members IBMUSER.PROJ23.SRC.H\n\
  the .lst files to corresponding dataset members IBMUSER.PROJ23.SRC.LST\n\
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
 * Scan the buffer looking for the first newline from the current offset to
 * length of bytes read into buffer.
 * If found, return the offset.
 * If not found, return -1.
 */
static ssize_t scan_buffer(FM_FileHandle* fh, FM_FileBuffer* fb, const FM_Opts* opts)
{
  int i;
  debug(&opts->dbg, "scan record from %d to %d looking for newline character 0x%x\n", 
    fb->record_offset, fb->data_length, fh->newline_char);
  for (i = fb->record_offset; i < fb->data_length; ++i) {
    if (fb->data[i] == fh->newline_char) {
      return i;
    }
  }
  return -1;
}

/*
 * Slightly tricky code.
 * We want to avoid doing data copy so we maintain 2 data buffers.
 * When the last part of the buffer is scanned, there will be no
 * newline found, and we will need to then read data into the 
 * other buffer and mark that some of the record is in the first
 * (active) buffer and the rest is in the second (inactive) buffer.
 * Since the target is a dataset that has a maximum record length,
 * we can know that we'll hit a newline somewhere in a given buffer
 * unless it is the very last line, in which case there may be no
 * newline.
 * This code presumes it is working with 'text'. Binary files will
 * need to drive a different routine.
 */
static int get_record(FM_FileHandle* fh, const FM_Opts* opts)
{
  ssize_t newline_offset;
  int more_data = 1;

  debug(&opts->dbg, "get_record\n");
  newline_offset = scan_buffer(fh, &fh->active, opts);

  if (newline_offset >= 0) {
    fh->active.record_length = newline_offset - fh->active.record_offset;
    fh->inactive.record_offset = 0;
    fh->inactive.record_length = 0;
    debug(&opts->dbg, "newline_offset: %d full line: active_offset: %d active_length: %d\n", 
      newline_offset, fh->active.record_offset, fh->active.record_length);  
  } else {
    fh->active.record_length = fh->active.data_length - fh->active.record_offset;
    /*
     * Partial line (record) in the buffer.
     */
    debug(&opts->dbg, "partial line was read.\n");
    int rc = read(fh->fd, fh->inactive.data, REC_LEN);
    if (rc <= 0) {
      debug(&opts->dbg, "... end of file reached\n");
      /*
       * Fall through here so that this partial line
       * at the end of the file will properly be processed.
       */
      if (fh->active.record_offset > fh->active.data_length) {
        more_data = 0;
      }
    }
    fh->inactive.data_length = rc;
    fh->inactive.record_offset = 0;

    newline_offset = scan_buffer(fh, &fh->inactive, opts);
    if (newline_offset < 0) {
      /*
       * File ends without a newline
       */
      debug(&opts->dbg, "... file ends without a newline\n");
      fh->inactive.record_length = fh->inactive.data_length;
    } else {
      fh->inactive.record_length = newline_offset;
    }
    debug(&opts->dbg, "scattered line: newline_offset:%d active_offset: %d active_length: %d and inactive_offset: %d inactive_length:%d\n", 
      newline_offset, fh->active.record_offset, fh->active.record_length, fh->inactive.record_offset, fh->inactive.record_length);  

  }
  return more_data; 
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

static int read_line(FM_FileHandle* fh, const FM_Opts* opts)
{
  ssize_t rc;
  debug(&opts->dbg, "readline. record_offset:%d\n", fh->active.record_offset);
  if (fh->active.record_offset == 0 && fh->active.record_length == 0) {
    /*
     * Buffer is empty - read in the first buffer and determine the file
     * tagging information
     */
    rc = read(fh->fd, fh->active.data, REC_LEN);
    if (rc <= 0) {
      return 0;
    }
    fh->active.data_length = rc;
    if (fh->newline_char == 0) {
      calc_tag(fh, opts);
    }
  } else {
    fh->active.record_offset += (fh->active.record_length + 1);
  }
  return get_record(fh, opts);
}

/*
 * open_debug_file, write_debug_line, close_debug_file 
 * are functions to make it easier to debug problems in the
 * code that reads in files using buffers for performance.
 * By default, the code is not active. If you specify '-d'
 * as an option, it will write out files of the form: /tmp/<dataset>.<member>
 * that should be the same as the original file read (but with a
 * different name). 
 * The file may or may not have the right file tag on it, so to read
 * it, you may need to issue: chtag -tcXXXXX /tmp/<dataset>.<member> to set
 * the file tag before looking at it.
 * Yes - the global variable is ugly and should be fixed.
 */
static FILE* debug_fp = NULL;
static void open_debug_file(const char* dataset, const char* member, const FM_Opts* opts)
{
  if (!opts->dbg.debug) {
    return;
  }
  const char debug_fmt[] = "/tmp/%s.%s";

  char debug_filename[sizeof(debug_fmt) + DS_MAX + MEM_MAX + 1];
  sprintf(debug_filename, debug_fmt, dataset, member);

  remove(debug_filename);
  debug_fp = fopen(debug_filename, "wb");
  if (!debug_fp) {
    fprintf(stderr, "Unable to open debug file: %s for write\n", debug_filename);
    return;
  }
}

static void write_debug_line(FM_BPAMHandle* bh, const FM_FileHandle* fh, const FM_Opts* opts)
{
  if (!opts->dbg.debug) {
    return;
  }
  if (fh->active.record_length > 0) {
    fwrite(&fh->active.data[fh->active.record_offset], fh->active.record_length, 1, debug_fp);
  }
  if (fh->inactive.record_length > 0) {
    fwrite(&fh->inactive.data[fh->inactive.record_offset], fh->inactive.record_length, 1, debug_fp);
  }
  fwrite(&fh->newline_char, 1, 1, debug_fp);
}

static void close_debug_file(const FM_Opts* opts)
{
  if (!opts->dbg.debug) {
    return;
  }
  fclose(debug_fp);
  debug_fp = NULL;
}

static int copy_at_most(void* dest, void* src, size_t length, size_t max)
{
  if (length > max) {
    length = max;
  }
  if (length == 0) {
    return length;
  }

  memcpy(dest, src, length);
  return length;
}

/*
 * add_line and read_line need to work together with their state because
 * they are sharing a common set of buffers for efficiency. 
 * This means that read_line is responsible for setting up the active and
 * inactive buffer state information and add_line is responsible for 
 * cleaning up the state after it has processed the information.
 */
static int add_line(FM_BPAMHandle* bh, FM_FileHandle* fh, const FM_Opts* opts)
{
  int truncated = 0;
  debug(&opts->dbg, "Add Line. Active (%d,%d) Inactive (%d,%d) bytes_used:%d\n", 
    fh->active.record_offset, fh->active.record_length, 
    fh->inactive.record_offset, fh->inactive.record_length,
    bh->bytes_used
  );
  write_debug_line(bh, fh, opts);
 
  char* block_char = (char*) (bh->block);
  int rec_hdr_size;
  unsigned short rec_len;
  if (bh->dcb->dcbexlst.dcbrecfm & dcbrecv) {
    unsigned short* next_rec;
    rec_hdr_size = sizeof(unsigned int);
    if (bh->bytes_used == 0) {
      /*
       * First word is block length - clear it to 0 for now
       * Set the block size to be the full block size since the 
       * WRITE routine knows to get the logical block size from the
       * first word.
       */
      unsigned int* start = (unsigned int*) (bh->block);
      start[0] = 0;
      bh->bytes_used += sizeof(unsigned int);
    }
    /*
     * Write out length of record for variable length record format
     */
    next_rec = (unsigned short*) (&block_char[bh->bytes_used]);
    rec_len = (fh->active.record_length + fh->inactive.record_length + sizeof(unsigned int));
    if (rec_len > bh->dcb->dcblrecl) {
      rec_len = bh->dcb->dcblrecl;
    }
    next_rec[0] = rec_len;
    next_rec[1] = 0;
    bh->bytes_used += rec_hdr_size;

    debug(&opts->dbg, "Record length:%d bytes used:%d\n", next_rec[0], bh->bytes_used);
  } else {
    rec_hdr_size = 0;
    rec_len = (fh->active.record_length + fh->inactive.record_length);
  }
  if (rec_len > bh->dcb->dcblrecl) {
    info(&opts->dbg, "Long record encountered on line %d and truncated. Maximum %d expected but record is %d bytes\n", fh->line_num, bh->dcb->dcblrecl, rec_len);
    truncated = 1;
  }

  int max_bytes = (bh->dcb->dcblrecl - rec_hdr_size);
  int copied_active_bytes;
  int copied_inactive_bytes;

  /*
   * Write out the raw data of the record which could span the active and inactive data blocks
   */
  copied_active_bytes = copy_at_most(&block_char[bh->bytes_used], &fh->active.data[fh->active.record_offset], fh->active.record_length, max_bytes);
  bh->bytes_used += copied_active_bytes;
  max_bytes -= copied_active_bytes;
  copied_inactive_bytes = copy_at_most(&block_char[bh->bytes_used], &fh->inactive.data[fh->inactive.record_offset], fh->inactive.record_length, max_bytes);
  bh->bytes_used += copied_inactive_bytes;


  if (bh->dcb->dcbexlst.dcbrecfm & dcbrecf) {
    /*
     *  If the record is FIXED, then pad the record out with blanks
     */
    int pad_length = bh->dcb->dcblrecl - (copied_active_bytes + copied_inactive_bytes);
    debug(&opts->dbg, "Pad record %d by %d blanks (active bytes:%d inactive_bytes:%d\n", fh->line_num, pad_length, copied_active_bytes, copied_inactive_bytes);
    if (pad_length > 0) {
      memset(&block_char[bh->bytes_used], fh->space_char, pad_length);
    }
    bh->bytes_used += pad_length;
  }

  /*
   * Now that the buffers have been copied out, if the inactive offsets are non-zero
   * copy them over to the active offsets and swap the buffers.
   */

  if (fh->inactive.record_length > 0) {
    char* tmp_buff;
    tmp_buff = fh->active.data;
    fh->active.data = fh->inactive.data;
    fh->inactive.data = tmp_buff;

    fh->active.record_offset = fh->inactive.record_offset;
    fh->active.record_length = fh->inactive.record_length; 
    fh->active.data_length = fh->inactive.data_length;
    fh->inactive.record_offset = 0;
    fh->inactive.record_length = 0;

    debug(&opts->dbg, "Buffers swapped. active record_offset:%s record_length:%d\n", fh->active.record_offset, fh->active.record_length);
  }
  return truncated;
}

static int can_add_line(FM_FileHandle* fh, FM_BPAMHandle* bh, const FM_Opts* opts)
{
  int line_length;
  if (bh->dcb->dcbexlst.dcbrecfm & dcbrecv) {
    const int hdr_size = sizeof(unsigned int);
    line_length = fh->active.record_length + fh->inactive.record_length + hdr_size;
  } else if (bh->dcb->dcbexlst.dcbrecfm & dcbrecf) {
    line_length = bh->dcb->dcblrecl;
  }
  int rc = (line_length + bh->bytes_used <= bh->block_size);
  debug(&opts->dbg, "Can Add Line:%c (active record length:%d inactive record length:%d bytes_used:%d block_size:%d lrecl:%d)\n", 
    rc == 1 ? 'Y' : 'N', fh->active.record_length, fh->inactive.record_length, bh->bytes_used, bh->block_size, bh->dcb->dcblrecl);
  return rc;
}

/*
 * Open the file, read lines, and add the lines to the block until the block is full
 * at which point, write the block out and repeat.
 * The 'ttr' needs to be marked as unknown and the underlying write_block will establish
 * it on first block write.
 * Once all the blocks are written, write the directory entry for the dataset member
 * and then close off the file.
 */
static int write_member(FM_BPAMHandle* bh, const char* dataset, const char* filename, const char* member, const FM_Opts* opts)
{
  FM_FileHandle fh;
  int rc;
  int memrc;
  int truncated = 0;

  open_debug_file(dataset, member, opts);
  if (!open_file(filename, &fh, opts)) {
    return 4;
  }

  bh->line_num = 1;
  fh.line_num = 1;
  bh->ttr_known = 0;
  while (read_line(&fh, opts)) {
    if (can_add_line(&fh, bh, opts)) {
      truncated |= add_line(bh, &fh, opts);
    } else {
      rc = write_block(bh, &opts->dbg);
      truncated |= add_line(bh, &fh, opts);
    }
    fh.line_num++;
  }
  rc = write_block(bh, &opts->dbg);
  
  memrc = write_member_dir_entry(bh, &fh, dataset, member, &opts->dbg);

  rc = close_file(&fh, opts);

  close_debug_file(opts);

  if (memrc > 0) {
    rc = memrc;
  }
  if (truncated) {
    fprintf(stderr, "One or more lines were truncated copying %s to %s(%s). Specify -v for more details\n", 
      filename, dataset, member);
  }
  return rc;
}

static int copy_file_to_member(FM_BPAMHandle* bh, const char* dataset, const char* filename, const char* member, const FM_Opts* opts)
{
  int rc;
  rc = write_member(bh, dataset, filename, member, opts);
  return rc;
}

static int copy_files(const FM_Table* table, int entries, const FM_FileTable* ext_entry, FM_BPAMHandle* bh, const char* dataset, const FM_Opts* opts)
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
      info(&opts->dbg, "Copy file %s to dataset member %s(%s)\n", filename, dataset, member);
      if (copy_file_to_member(bh, dataset, filename, member, opts)) {
        /*
         * If a member copy fails - just return
         */
        fprintf(stderr, "File %s could not be copied to %s(%s). No further copies to this dataset will be attempted.\n", filename, dataset, member);
        return rc;
      }
    }
  }
  return rc;
}

/*
 * copy_files_to_multiple_dataset_members processes all the files to be copied for one dataset at a time.
 * e.g. all *.c files will be copied into a dataset with .c as the LLQ.
 * By performing the copy in this way, we can open the pds (or pdse) for write ONE TIME and then subsequently
 * write a member at a time into the dataset using BPAM I/O. This is far more efficient than if we were to
 * open the pds, write the member, then close the member, which is how it would work normally using standard C fopen/fwrite/fclose
 * routines.
 */
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
    if (open_pds_for_write(dataset, &dd, &opts->dbg)) {
      fprintf(stderr, "Unable to open dataset %s for write. Extension skipped\n", dataset);
      rc |= 4;
      continue;
    }
    rc |= copy_files(table, table->entry[ext].count, table->entry[ext].table, &dd, dataset, opts);

    if (close_pds(dataset, &dd, &opts->dbg)) {
      fprintf(stderr, "Unable to free DDName for dataset %s.\n", dataset);
      rc |= 8;
      continue;
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
  if (open_pds_for_write(dataset, &dd, &opts->dbg)) {
    fprintf(stderr, "Unable to allocate DDName for dataset %s. Files not copied.\n", dataset);
    return 4;
  }
  for (ext=0; ext < table->size; ext++) {
    rc |= copy_files(table, table->entry[ext].count, table->entry[ext].table, &dd, dataset, opts);
  }
  if (close_pds(dataset, &dd, &opts->dbg)) {
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

/*
 * main program:
 * -process options, directory, dataset_pattern, and then expand the file patterns 
 *  storing the expanded set of files into globset via 'expand_file_patterns'
 * -create a table, with one entry for each extension in the list of files, e.g
 *  if there are .h, .c, .lst files there would be one entry for each of .h, .c, and .lst
 * -fill the table by going through each entry and adding in all the files that match that
 *  extension.
 * -copy all the files to corresponding datasets, typically by mapping extensions to LLQ's
 *  of datasets (this is why we went to the trouble of organizing the table with extension entries)
 */ 
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