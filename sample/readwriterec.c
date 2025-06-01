/*
 * Perform a write of a member, then a read of the member, then write new member@ with new records on the end.
 * Use services from bpamio.h and memdir.h
 * The dataset needs to already be allocated as a PDS or PDSE of VB or FB record format and at least record length 80.
 * 
 * MSF: Note - right now if you try a VB dataset, it won't work because the directory read will fail because it expects
 * the information for the read to be FB 80. Have to fix this...
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

static int can_add_record_to_block(FM_BPAMHandle* bh, size_t rec_len)
{
  int line_length;
  if (bh->dcb->dcbexlst.dcbrecfm & dcbrecv) {
    const int hdr_size = sizeof(unsigned int);
    line_length = rec_len + hdr_size;
  } else if (bh->dcb->dcbexlst.dcbrecfm & dcbrecf) {
    line_length = bh->dcb->dcblrecl;
  }
  int rc = (line_length + bh->bytes_used <= bh->block_size);
  return rc;
}

/*
 * copy_record_to_block returns 'truncated' (non-zero if record truncated, otherwise zero)
 */
static int copy_record_to_block(FM_BPAMHandle* bh, size_t rec_len, const char* rec, DBG_Opts* opts)
{
  int truncated = 0;
  debug(opts, "Add Record of length: %d bytes. Block bytes used: %d\n", rec_len, bh->bytes_used);
 
  char* block_char = (char*) (bh->block);
  int rec_hdr_size;
  if (bh->dcb->dcbexlst.dcbrecfm & dcbrecv) {
    /*
     * Variable format
     */    
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
    if (rec_len > bh->dcb->dcblrecl) {
      rec_len = bh->dcb->dcblrecl;
    }
    next_rec[0] = rec_len;
    next_rec[1] = 0;
    bh->bytes_used += rec_hdr_size;

    debug(opts, "Disk Record length:%d bytes used:%d\n", next_rec[0], bh->bytes_used);
  } else {
    /*
     * Fixed format
     */
    rec_hdr_size = 0;
  }
  if (rec_len > bh->dcb->dcblrecl) {
    info(opts, "Long record encountered on line %d and truncated. Maximum %d expected but record is %d bytes\n", bh->line_num, bh->dcb->dcblrecl, rec_len);
    rec_len = bh->dcb->dcblrecl;
    truncated = 1;
  }

  memcpy(&block_char[bh->bytes_used], rec, rec_len);
  bh->bytes_used += rec_len; 
 
  if (bh->dcb->dcbexlst.dcbrecfm & dcbrecf) {
    /*
     *  If the record is FIXED, then pad the record out with blanks
     */
    int pad_length = bh->dcb->dcblrecl - rec_len;
    debug(opts, "Pad record %d by %d blanks\n", bh->line_num, pad_length);
    if (pad_length > 0) {
      memset(&block_char[bh->bytes_used], ' ', pad_length); /* msf - choose ASCII or EBCDIC space based on ccsid */
    }
    bh->bytes_used += pad_length;
  }
  return truncated;
}

static ssize_t write_record(FM_BPAMHandle* bh, size_t rec_len, const char* rec, DBG_Opts* opts)
{
  /*
   * Batch up records until there is a full block and write it out 
   */
  ssize_t rc;
  if (can_add_record_to_block(bh, rec_len)) {
    int truncated = copy_record_to_block(bh, rec_len, rec, opts);
    rc = 0;
  } else {
    rc = write_block(bh, opts);
  }
  return rc;
}

static ssize_t read_record(FM_BPAMHandle* bh, size_t max_rec_len, char* rec, size_t num_lines, DBG_Opts* opts)
{
  /*
   * See if we need to read another block
   */
  if ((num_lines == 0) || !next_record(bh, opts)) {
    ssize_t rc = read_block(bh, opts);
    if (rc) {
      fprintf(stderr, "read_block returned rc:%d\n", rc);
      return -1;
    }
    next_record(bh, opts);
  }

  ssize_t rec_len = bh->next_record_len;
  if (rec_len > max_rec_len) {
    fprintf(stderr, "record length is too large. max:%d received: %d.\n", max_rec_len, rec_len);
    return -1;
  }
  memcpy(rec, bh->next_record_start, rec_len);

  return rec_len;
}

static ssize_t read_member(FM_BPAMHandle* bh, const char* ds, const char* mem_name, char* buffer, size_t buffer_len, DBG_Opts* opts)
{
  int rc = find_member(bh, mem_name, opts);
  if (rc) {
    fprintf(stderr, "Unable to find %s(%s) for read. rc:%d\n", ds, mem_name, rc);
    return 8;
  }

  size_t max_bytes_to_read = buffer_len-1;
  char* cur = buffer;
  ssize_t bytes_read;
  size_t tot_bytes_read = 0;
  int num_lines = 0;
  while ((bytes_read = read_record(bh, max_bytes_to_read, cur, num_lines, opts)) >= 0) {
    cur += bytes_read;
    *cur = '\n'; /* msf - choose ASCII or EBCDIC newline based on ccsid */
    ++cur;
    max_bytes_to_read -= (bytes_read + 1);
    tot_bytes_read += bytes_read;
    ++num_lines;
  }
  fprintf(stdout, "Read %d lines (%d bytes) for member %s(%s)\n", num_lines, tot_bytes_read, ds, mem_name);      

  return tot_bytes_read;
}

static int write_member(FM_BPAMHandle* bh, const char* ds, const char* mem_name, const char* buffer, DBG_Opts* opts)
{
  const char* cur = buffer;
  const char* next;
  int num_lines = 0;

  bh->line_num = 0;
  while ((next = strchr(cur, '\n')) != NULL) { /* msf - choose ASCII or EBCDIC newline based on ccsid */
    size_t rec_len = next - cur;
    ssize_t bytes_written = write_record(bh, rec_len, cur, opts);
    if (bytes_written < 0) {
      fprintf(stderr, "Unable to write record %d for member %s(%s)\n", num_lines, ds, mem_name);
      return -1;    
    }
    cur = &next[1];
    ++num_lines;
    bh->line_num = num_lines;
  }
  ssize_t rc = write_block(bh, opts); /* msf - need to write the partial block - maybe this should be on 'close' ? */
  if (rc < 0) {
    fprintf(stderr, "Unable to write final block for member %s(%s)\n", ds, mem_name);
    return -1;
  }

  struct mstat mstat;
  char userid[USERID_LEN+1];
  char* alias_name = NULL;
  void* ttr = (void*) bh->memstart_ttr; /* msf - perhaps the TTR should not be part of the mstat? */

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

#define NUM_LINES_MEM (5)
#define NUM_LINES_NEWMEM (9)

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

#if 1
  opts.debug = 1;
#endif
  FM_BPAMHandle bh_write_mem;
  FM_BPAMHandle bh_read_mem;
  FM_BPAMHandle bh_write_newmem;
  FM_BPAMHandle bh_read_newmem;

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

  rc = open_pds_for_write(ds, &bh_write_mem, &opts);

  if (rc) {
    fprintf(stderr, "Unable to open %s for write. rc:%d\n", ds, rc);
    return 8;
  }

  if (write_member(&bh_write_mem, ds, mem, ascii_data, &opts)) {
    fprintf(stderr, "Unable to write initial member %s(%s). rc:%d\n", ds, mem, rc);
    return 8;    
  } else {
    fprintf(stdout, "Member %s(%s) had %d records written to it.\n", ds, mem, bh_write_mem.line_num);
  }

  rc = close_pds(&bh_write_mem, &opts);
  rc = open_pds_for_read(ds, &bh_read_mem, &opts);

  if (rc) {
    fprintf(stderr, "Unable to open %s for read. rc:%d\n", ds, rc);
    return 8;
  }

  /*
   * Read the member entry and validate it looks right (visual check of time stamp and number of lines in ascii_data)
   */
  struct mstat read_mstat;
  if (readmemdir_entry(&bh_read_mem, mem, &read_mstat, &opts)) {
    fprintf(stderr, "Unable to read directory entry for member %s(%s)\n", ds, mem);
    return 8;
  }
  fprintf(stdout, "Member %s last modified at %s\n", mem, ctime(&read_mstat.ext_changed));
  fprintf(stdout, "Member %s has ISPF statistics? %c \n", mem, read_mstat.ispf_stats ? 'Y' : 'N');
  if (read_mstat.ispf_stats) {
    fprintf(stdout, "Member %s has %d current lines. Expected %d lines.\n", mem, read_mstat.ispf_current_lines, NUM_LINES_MEM);
  }

  /*
   * Read the member back and compare it to the original buffer written out.
   * It should be the same once the blanks are excluded (if it's an FB file)
   */
  char* buffer;
  size_t first_file_len = NUM_LINES_MEM * (bh_read_mem.dcb->dcblrecl+1);
  size_t second_file_len = NUM_LINES_NEWMEM * (bh_read_mem.dcb->dcblrecl+1);
  size_t buffer_len = first_file_len + 1;
  ssize_t bytes_read;
  buffer = malloc(buffer_len);
  if (!buffer) {
    fprintf(stderr, "Unable to allocate buffer for read/write of member.\n");
    return 8;
  }
  debug(&opts, "Read into buffer of size %d to read %d lines of record length %d\n", first_file_len, NUM_LINES_MEM, bh_read_mem.dcb->dcblrecl);

  if ((bytes_read = read_member(&bh_read_mem, ds, mem, buffer, buffer_len, &opts)) < 0 ) {
    fprintf(stderr, "Unable to read back initial member %s(%s). rc:%d\n", ds, mem, rc);
    return 8;    
  }
  if (bytes_read != first_file_len || !memcmp(buffer, ascii_data, first_file_len)) {
    fprintf(stderr, "Expected to read %d bytes with value:\n%s but got %d bytes of value:\n%s", 
      first_file_len, ascii_data, bytes_read, buffer);
  }

  rc = close_pds(&bh_read_mem, &opts);
  rc = open_pds_for_write(ds, &bh_write_newmem, &opts);

  if (rc) {
    fprintf(stderr, "Unable to open %s for read. rc:%d\n", ds, rc);
    return 8;
  }  
  free(buffer);

  /*
   * Create a new member that has the '@' on the end and has the extra ascii data
   */
  buffer_len = second_file_len + 1;
  buffer = malloc(buffer_len);
  if (!buffer) {
    fprintf(stderr, "Unable to allocate buffer for read/write of new member.\n");
    return 8;
  }
  sprintf(buffer, "%s%s", ascii_data, ascii_extra_data);
  
  if (write_member(&bh_write_newmem, ds, newmem, buffer, &opts)) {
    fprintf(stderr, "Unable to write initial new member %s(%s). rc:%d\n", ds, newmem, rc);
    return 8;    
  }

  rc = close_pds(&bh_write_newmem, &opts);
  rc = open_pds_for_read(ds, &bh_read_newmem, &opts);

  if (rc) {
    fprintf(stderr, "Unable to open %s for read. rc:%d\n", ds, rc);
    return 8;
  }    
  /*
   * Read the new member entry and validate it looks right (visual check of time stamp and number of lines in ascii_data and ascii_extra_data)
   */
  if (readmemdir_entry(&bh_read_newmem, newmem, &read_mstat, &opts)) {
    fprintf(stderr, "Unable to read directory entry for member %s(%s)\n", ds, newmem);
    return 8;
  }
  fprintf(stdout, "Member %s last modified at %s\n", newmem, ctime(&read_mstat.ext_changed));
  fprintf(stdout, "Member %s has ISPF statistics? %c \n", newmem, read_mstat.ispf_stats ? 'Y' : 'N');
  if (read_mstat.ispf_stats) {
    fprintf(stdout, "Member %s has %d current lines\n", newmem, read_mstat.ispf_current_lines);
  }
  if ((bytes_read = read_member(&bh_read_newmem, ds, newmem, buffer, buffer_len, &opts)) < 0 ) {
    fprintf(stderr, "Unable to read back initial member %s(%s). rc:%d\n", ds, newmem, rc);
    return 8;    
  }
  if (bytes_read != buffer_len) {
    fprintf(stderr, "Expected to read %d bytes with value \n%s%s but got %d bytes with value \n%s", buffer_len, ascii_data, ascii_extra_data, bytes_read, buffer);
  }

  rc = close_pds(&bh_read_newmem, &opts);

  return 0;
}




