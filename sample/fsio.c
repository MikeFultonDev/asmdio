#define _OPEN_SYS_FILE_EXT 1
#define POSIX_SOURCE
#include <sys/stat.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

#include "fm.h"
#include "fmopts.h"
#include "memdir.h"
#include "msg.h"

/*
 * This code will calculate the newline character based
 * on looking at the file tag of the file being copied and,
 * if the file tag isn't specified, it will read the 
 * active data buffer for 'clues'.
 */
void calc_tag(FM_FileHandle* fh, unsigned short ccsid, const FM_Opts* opts)
{
  if (ccsid == ISO8859_CCSID) {
    fh->newline_char = ISO8859_NL;
    fh->space_char = ISO8859_SPACE;
  } else if (fh->tag.ft_ccsid == IBM1047_CCSID) {
    fh->newline_char = IBM1047_NL;
    fh->space_char = IBM1047_SPACE;
  } else {
    /* msf: this needs to be fleshed out */
    fh->newline_char = IBM1047_NL;  /* default to EBCDIC right now */
    fh->space_char = IBM1047_SPACE; /* default to EBCDIC right now */
  }
}

/*
 * Open the file and initialize the file handle for reading a file
 */
FM_FileHandle* open_file_read(const char* filename, FM_FileHandle* fh, const FM_Opts* opts)
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

  fh->file_buffer = NULL; /* file_buffer used for 'write' only */

  info(&opts->dbg, "Code page of input file:%d\n", fh->tag.ft_ccsid);
  return fh;
}
/*
 * Open the file and initialize the file handle 
 */

FM_FileHandle* open_file_create(const char* filename, FM_FileHandle* fh, unsigned short ext_ccsid, const FM_Opts* opts)
{
  int fd = open(filename, O_CREAT|O_EXCL|O_WRONLY, S_IRUSR|S_IWUSR);
  if (fd < 0) {
    perror("create");
    fprintf(stderr, "Unable to open filename %s for create.\n", filename);
    return NULL;
  }

  memset(fh, 0, sizeof(FM_FileHandle));

  fh->fd = fd;
  
#if 0
  attrib_t attributes = { 0 };
  int attr_len = sizeof(attributes);

  attributes.att_filetagchg = 1;
  attributes.att_filetag.ft_ccsid = ext_ccsid;
  attributes.att_filetag.ft_txtflag = 1;

  if (__fchattr(fd, &attributes, attr_len) < 0) {
    perror("chattr");
    fprintf(stderr, "Unable to set attributes for filename %s.\n", filename);
    close(fd);    
    return NULL;
  }
#endif

  /*
   * Turn auto-convert off
   */
  struct f_cnvrt req = {SETCVTOFF, 0, 0};
  if (fcntl(fd, F_CONTROL_CVT, &req) < 0) {
    perror("fcntl");
    fprintf(stderr, "Unable to fcntl filename %s.\n", filename);
    close(fd);    
    return NULL;
  }

  struct stat stat_info;
  if (fstat(fd, &stat_info) < 0) {
    perror("fstat");
    fprintf(stderr, "Unable to fstat filename %s.\n", filename);
    close(fd);
    return NULL;
  }
  fh->tag = stat_info.st_tag;

  calc_tag(fh, ext_ccsid, opts);

  fh->active.data = NULL;   /* active.data only used for file read   */
  fh->inactive.data = NULL; /* inactive.data only used for file read */

  info(&opts->dbg, "Code page of output file:%d\n", fh->tag.ft_ccsid);

  fh->file_buffer = malloc(MAX_TEXT_FILE_SIZE);
  if (!fh->file_buffer) {
    fprintf(stderr, "Unable to allocate file buffer");
    close(fd);
    return NULL;
  }
  fh->file_buffer_max = MAX_TEXT_FILE_SIZE;
  fh->file_buffer_cur = 0;
  return fh;
}

/*
 * Write to a buffer. Returns number of bytes written to buffer
 */
int buffer_write(FM_FileHandle* fh, const char* start, size_t len)
{
  if (fh->file_buffer_cur + len > fh->file_buffer_max) {
    fprintf(stderr, "Unable to write to buffer - buffer full. Data dropped!\n");
    return 0;
  }
  memcpy(&fh->file_buffer[fh->file_buffer_cur], start, len);
  fh->file_buffer_cur += len;

  return len;
}

/*
 * Write the buffer out to the file. Returns number of bytes written to file
 */
int write_file(FM_FileHandle* fh, const FM_Opts* opts)
{
  return write(fh->fd, fh->file_buffer, fh->file_buffer_cur);
}

/*
 * Close the file. Returns zero on success, non-zero otherwise
 */
int close_file(FM_FileHandle* fh, const FM_Opts* opts)
{
  if (fh->file_buffer) {
    free(fh->file_buffer);
  }
  return close(fh->fd);
}

