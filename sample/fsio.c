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

  fh->active.data = fh->data_a;
  fh->inactive.data = fh->data_b;

  info(&opts->dbg, "Code page of output file:%d\n", fh->tag.ft_ccsid);
  return fh;
}

/*
 * Close the file. Returns zero on success, non-zero otherwise
 */
int close_file(FM_FileHandle* fh, const FM_Opts* opts)
{
  return close(fh->fd);
}

