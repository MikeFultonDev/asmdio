#ifndef __FILEMAP_H__
  #define __FILEMAP_H__ 1

  #include <glob.h>
  #include <string.h>
  
  #include "asmdiocommon.h"
  #include "fmopts.h"
  #include "fm.h"

  const char* map_file_to_member(const char* file, char* member, const FM_Opts* opts);
  const char* map_ext_to_dataset(const char* dataset_pattern, const char* ext, char* dataset, const FM_Opts* opts);
  int check_for_duplicate_members(glob_t* globset, const FM_Table* table, const FM_Opts* opts);
  glob_t* expand_file_patterns(char* argv[], int first, int last, const char* dir, glob_t* globset, const FM_Opts* opts);
  FM_Table* create_table(glob_t* globset, const FM_Opts* opts);
  FM_Table* fill_table(glob_t* globset, FM_Table* table);

#endif
