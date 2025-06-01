/*
 * Perform a write of a member, then a read of the member, then write new member@ with new records on the end.
 * Use services from bpamio.h and memdir.h
 * The dataset needs to already be allocated as a PDS or PDSE of VB or FB record format and at least record length 80.
 *
 */
#define _XOPEN_SOURCE
#define _ISOC99_SOURCE
#define _POSIX_SOURCE
#define _OPEN_SYS_FILE_EXT 1
#define _OPEN_SYS_EXT
#define _XOPEN_SOURCE_EXTENDED 1

#include <stdio.h>
#include <string.h>
#include "util.h"
#include "bpamio.h"
#include "memdir.h"
#include "dbgopts.h"

static ssize_t write_record(FM_BPAMHandle* bh, size_t rec_len, const char* rec, DBG_Opts* opts)
{
  /*
   * Batch up records until there is a full block and write it out 
   */
  fprintf(stderr, "write_record: write some code\n");
  return -1;

  int rc;
  rc = write_block(bh, opts);
  if (rc) {
    return -1;
  }
  return 1;
}

static ssize_t read_record(FM_BPAMHandle* bh, size_t max_rec_len, char* rec, DBG_Opts* opts)
{
  int bytes_read = -1;
  fprintf(stderr, "read_record: write some code\n");
  return bytes_read;

  /*
   * If there is a record in the block that hasn't been processed, send it back, otherwise
   * read another block first.
   */
  int rc = read_block(bh, opts);
  if (rc) {
    return -1;
  }
  return bytes_read;
}

static ssize_t read_member(FM_BPAMHandle* bh, const char* ds, const char* mem_name, char* buffer, size_t buffer_len, DBG_Opts* opts)
{
  int rc = find_member(&bh, mem_name, &opts);
  if (rc) {
    fprintf(stderr, "Unable to find %s(%s) for read. rc:%d\n", ds, mem_name, rc);
    return 8;
  }

  size_t max_bytes_to_read = buffer_len-1;
  char* cur = buffer;
  size_t bytes_read;
  int num_lines = 0;
  while ((bytes_read = read_record(bh, max_bytes_to_read, cur, opts)) >= 0) {
    cur += bytes_read;
    *cur = '\n';
    ++cur;
    max_bytes_to_read -= (bytes_read + 1);
    ++num_lines;
  }
  if (bytes_read < 0) {
    fprintf(stderr, "Unable to read records from %s(%s)\n", ds, mem_name);
  }
  fprintf(stdout, "Read %d lines for member %s(%s)\n", num_lines, ds, mem_name);      

  return bytes_read;
}

static int write_member(FM_BPAMHandle* bh, const char* ds, const char* mem_name, const char* buffer, DBG_Opts* opts)
{
  const char* cur = buffer;
  const char* next;
  int num_lines = 0;

  while ((next = strchr(cur, '\n')) != NULL) {
    size_t rec_len = next - cur;
    ssize_t bytes_written = write_record(bh, rec_len, cur, opts);
    if (bytes_written < 0) {
      fprintf(stderr, "Unable to write record %d for member %s(%s)\n", num_lines, ds, mem_name);
      return -1;    
    }
    ++num_lines;
  }

  struct mstat mstat;
  char userid[USERID_LEN+1];
  char* alias_name = NULL;
  void* ttr = bh->memstart_ttr; /* msf - perhaps the TTR should not be part of the mstat? */

  if (!create_mstat(&mstat, userid, alias_name, mem_name, ttr, num_lines, opts)) {
    fprintf(stderr, "Unable to create member statistics for PDS member %s(%s). Member not written\n", ds, mem_name);
    return 8;
  }

  if (ispf_enq_dataset_member(ds, mem_name)) {
    fprintf(stderr, "Unable to obtain ENQ for PDS member %s(%s). Member not written\n", ds, mem_name);
    return 8;
  }
  if (writememdir_entry(bh, &mstat, &opts)) {
    fprintf(stderr, "Unable to write directory entry for member %s(%s)\n", ds, mem_name);
    return 8;
  }
  if (ispf_deq_dataset_member(ds, mem_name)) {
    fprintf(stderr, "Unable to obtain ENQ for PDS member %s(%s). Member not written\n", ds, mem_name);
    return 8;
  }
  return 0;
}

static const char* ascii_data = 
  "This is the first line of the member being created.\n"
  "This is the second line of the member being created.\n"
  "The following is a line of length 0.\n"
  "\n"
  "This is the last line of the member being created.\n"
;

static const char* ascii_extra_data = 
  "This is the first extra line for the new member.\n"
  "This following is a line of length 0.\n"
  "\n"
  "This is the last extra line of the new member.\n"
;

int main(int argc, char* argv[])
{
  if (argc != 3) {
    printf("Usage: %s <dataset> <member>\n", argv[0]);
    printf("  Write member X, Read member X, Update member X@, updating ISPF statistics and setting the CCSID (ASCII)\n");
    return 4;
  }

  char* ds = argv[1];
  char* mem = argv[2];
  char newmem[8+1];
  size_t mem_len = strlen(mem);

  DBG_Opts opts = { 0 };

#if 0
  opts.debug = 1;
#endif
  FM_BPAMHandle bh;
  int rc;

  if (strlen(ds) > 44 || mem_len > 7) {
    fprintf(stderr, "dataset or member name too long\n");
    return 8;
  }

  uppercase(ds);
  uppercase(mem);
  memcpy(newmem, mem, mem_len);
  newmem[mem_len] = '@';
  newmem[mem_len+1] = '\0';

  /*
   * Open the PDS/PDSE for write (then read, then update), then close the dataset.
   */

  rc = open_pds_for_write(ds, &bh, &opts);

  if (rc) {
    fprintf(stderr, "Unable to open %s for write. rc:%d\n", ds, rc);
    return 8;
  }

  if (write_member(&bh, ds, mem, ascii_data, &opts)) {
    fprintf(stderr, "Unable to write initial member %s(%s). rc:%d\n", ds, mem, rc);
    return 8;    
  }

  /*
   * Read the member entry and validate it looks right (visual check of time stamp and number of lines in ascii_data)
   */
  struct mstat read_mstat;
  if (readmemdir_entry(&bh, mem, &read_mstat, &opts)) {
    fprintf(stderr, "Unable to read directory entry for member %s(%s)\n", ds, mem);
    return 8;
  }
  fprintf(stdout, "Member %s last modified at %s\n", mem, ctime(&read_mstat.ext_changed));
  fprintf(stdout, "Member %s has ISPF statistics? %c \n", mem, read_mstat.ispf_stats ? 'Y' : 'N');
  if (read_mstat.ispf_stats) {
    fprintf(stdout, "Member %s has %d current lines\n", mem, read_mstat.ispf_current_lines);
  }

  /*
   * Read the member back and compare it to the original buffer written out.
   * It should be the same.
   */
  char* buffer;
  size_t first_file_len = strlen(ascii_data);
  size_t buffer_len = first_file_len + strlen(ascii_extra_data) + 1;
  ssize_t bytes_read;
  buffer = malloc(buffer_len);
  if (!buffer) {
    fprintf(stderr, "Unable to allocate buffer for read/write of member.\n");
    return 8;
  }
  if ((bytes_read = read_member(&bh, ds, mem, buffer, buffer_len, &opts)) < 0 ) {
    fprintf(stderr, "Unable to read back initial member %s(%s). rc:%d\n", ds, mem, rc);
    return 8;    
  }
  if (bytes_read != first_file_len || !memcmp(buffer, ascii_data, first_file_len)) {
    fprintf(stderr, "Expected to read %d bytes with value:\n%s but got %d bytes of value:\n%s", 
      first_file_len, ascii_data, bytes_read, buffer);
  }

  /*
   * Create a new member that has the '@' on the end and has the extra ascii data
   */
  strcpy(&buffer[first_file_len], ascii_extra_data);
  if (write_member(&bh, ds, newmem, buffer, &opts)) {
    fprintf(stderr, "Unable to write initial new member %s(%s). rc:%d\n", ds, newmem, rc);
    return 8;    
  }

  /*
   * Read the new member entry and validate it looks right (visual check of time stamp and number of lines in ascii_data and ascii_extra_data)
   */
  if (readmemdir_entry(&bh, newmem, &read_mstat, &opts)) {
    fprintf(stderr, "Unable to read directory entry for member %s(%s)\n", ds, newmem);
    return 8;
  }
  fprintf(stdout, "Member %s last modified at %s\n", newmem, ctime(&read_mstat.ext_changed));
  fprintf(stdout, "Member %s has ISPF statistics? %c \n", newmem, read_mstat.ispf_stats ? 'Y' : 'N');
  if (read_mstat.ispf_stats) {
    fprintf(stdout, "Member %s has %d current lines\n", newmem, read_mstat.ispf_current_lines);
  }
  if ((bytes_read = read_member(&bh, ds, newmem, buffer, buffer_len, &opts)) < 0 ) {
    fprintf(stderr, "Unable to read back initial member %s(%s). rc:%d\n", ds, newmem, rc);
    return 8;    
  }
  if (bytes_read != buffer_len) {
    fprintf(stderr, "Expected to read %d bytes but got %d bytes", buffer_len, bytes_read);
  }

  rc = close_pds(&bh, &opts);

  return 0;
}




