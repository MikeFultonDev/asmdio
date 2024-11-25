#ifndef __BPAMIO_H__
  #define __BPAMIO_H__ 1

  #include "asmdiocommon.h"
  #include "fm.h"
  #include "dbgopts.h"

  /*
   * struct mem_node: a pointer to this structure is returned from the call to pds_mem().
   * It is a linked list of information about the member - each array contains a member
   * name and possibly user data. Each next pointer points * to the next member, except the last
   * next member which points to NULL.
  */

  #define TTR_LEN (3)
  struct mem_node {
    struct mem_node *next;
    char name[MEM_MAX+1];
    char userdata_len;
    int is_alias:1;
    char ttr[TTR_LEN];
    char userdata[64];
  };

  struct mem_node* pds_mem(FM_BPAMHandle* bh, const DBG_Opts* opts);
  struct desp* PTR32 get_desp_all(const FM_BPAMHandle* bh, const DBG_Opts* opts);
  struct mem_node* find_mem(FM_BPAMHandle* bh, const char* memname, struct mem_node* match_node, const DBG_Opts* opts);

  int open_pds_for_write(const char* dataset, FM_BPAMHandle* bh, const DBG_Opts* opts);
  int open_pds_for_read(const char* dataset, FM_BPAMHandle* bh, const DBG_Opts* opts);

  int find_member(FM_BPAMHandle* bh, const char* mem, const DBG_Opts* opts);
  int write_member_dir_entry(const struct file_tag* tag, const char* member, FM_BPAMHandle* bh, const DBG_Opts* opts);

  int read_block(FM_BPAMHandle* bh, const DBG_Opts* opts);
  int write_block(FM_BPAMHandle* bh, const DBG_Opts* opts);

  int close_pds(FM_BPAMHandle* bh, const DBG_Opts* opts);
#endif
