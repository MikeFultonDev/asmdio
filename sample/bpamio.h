#ifndef __BPAMIO_H__
  #define __BPAMIO_H__ 1

  #include "fm.h"
  #include "fmopts.h"
  int write_block(FM_BPAMHandle* bh, const FM_Opts* opts);
  int write_member_dir_entry(const FM_BPAMHandle* bh, const FM_FileHandle* fh, const char* ds, const char* member, const FM_Opts* opts);
  int open_pds_for_write(const char* dataset, FM_BPAMHandle* bh, const FM_Opts* opts);
  int close_pds(const char* dataset, const FM_BPAMHandle* bh, const FM_Opts* opts);
#endif
