/*
 * Perform a read, then update of a PDS member.
 * Use services from bpamio.h and memdir.h
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

int main(int argc, char* argv[])
{
  if (argc != 3) {
    printf("Usage: %s <dataset> <member>\n", argv[0]);
    printf("  Read a member, get the directory info, then write out a new member <member>@ with just one block, updating ISPF statistics and setting the CCSID (ASCII)\n");
    return 4;
  }

  char* ds = argv[1];
  char* rmem = argv[2];
  char wmem[8+1];
  size_t rmem_len = strlen(rmem);

  DBG_Opts opts = { 0 };

#if 0
  opts.debug = 1;
#endif
  FM_BPAMHandle bh;
  int rc;

  if (strlen(ds) > 44 || rmem_len > 7) {
    fprintf(stderr, "dataset or member name too long\n");
    return 8;
  }

  uppercase(ds);
  uppercase(rmem);
  memcpy(wmem, rmem, rmem_len);
  wmem[rmem_len] = '@';
  wmem[rmem_len+1] = '\0';

  /*
   * Open the PDS/PDSE for read, find the member, get the directory information, read the member, close the dataset.
   */
  rc = open_pds_for_read(ds, &bh, &opts);

  if (rc) {
    fprintf(stderr, "Unable to open %s for read. rc:%d\n", ds, rc);
    return 8;
  }

  struct mstat read_mstat;
  if (readmemdir_entry(&bh, rmem, &read_mstat, &opts)) {
    fprintf(stderr, "Unable to read directory entry for member %s(%s)\n", ds, rmem);
    return 8;
  }
  printf("Member %s last modified at %s\n", rmem, ctime(&read_mstat.ext_changed));

  rc = find_member(&bh, rmem, &opts);
  if (rc) {
    fprintf(stderr, "Unable to find %s(%s) for read. rc:%d\n", ds, rmem, rc);
    return 8;
  }

  int blocks_read = 0;
  rc = read_block(&bh, &opts);
  if (rc) {
    fprintf(stderr, "Unable to read block from %s(%s). rc:%d\n", ds, rmem, rc);
  }
  ++blocks_read;

  rc = close_pds(&bh, &opts);

  /*
   * Open the PDS/PDSE for update, write the member, update the directory information, close the dataset.
   */

  rc = open_pds_for_write(ds, &bh, &opts);

  if (rc) {
    fprintf(stderr, "Unable to open %s for write. rc:%d\n", ds, rc);
    return 8;
  }

  int blocks_written = 0;
  rc = write_block(&bh, &opts);
  if (rc) {
    fprintf(stderr, "Unable to write block to %s(%s). rc:%d\n", ds, rmem, rc);
  }
  ++blocks_written;

  struct mstat write_mstat = read_mstat;
  write_mstat.name = wmem;

  if (ispf_enq_dataset_member(ds, wmem)) {
    fprintf(stderr, "Unable to obtain ENQ for PDS member %s(%s). Member not written\n", ds, wmem);
    return 8;
  }
  if (writememdir_entry(&bh, &write_mstat, &opts)) {
    fprintf(stderr, "Unable to write directory entry for member %s(%s)\n", ds, wmem);
    return 8;
  }
  if (ispf_deq_dataset_member(ds, wmem)) {
    fprintf(stderr, "Unable to obtain ENQ for PDS member %s(%s). Member not written\n", ds, wmem);
    return 8;
  }

  rc = close_pds(&bh, &opts);

  printf("New member %s written out\n", wmem);
  printf("Read %d blocks and wrote %d blocks\n", blocks_read, blocks_written);
  return 0;
}




