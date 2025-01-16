#ifndef __FSIO_H__
  #define __FSIO_H__ 1

  #include "fm.h"
  #include "fmopts.h"

  FM_FileHandle* open_file_create(const char* filename, FM_FileHandle* fh, unsigned short ext_ccsid, const FM_Opts* opts);
  FM_FileHandle* open_file_read(const char* filename, FM_FileHandle* fh, const FM_Opts* opts);
  int close_file(FM_FileHandle* fh, const FM_Opts* opts);
  void calc_tag(FM_FileHandle* fh, unsigned short ccsid, const FM_Opts* opts);
#endif