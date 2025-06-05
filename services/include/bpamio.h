#ifndef __BPAMIO_H__
  #define __BPAMIO_H__ 1

  #include "dbgopts.h"

  /*
   * struct mem_node: a pointer to this structure is returned from the call to pds_mem().
   * It is a linked list of information about the member - each array contains a member
   * name and possibly user data. Each next pointer points * to the next member, except the last
   * next member which points to NULL.
  */

  #ifndef MEM_MAX
    #define MEM_MAX (8)
  #endif
  #ifndef DD_MAX
    #define DD_MAX (8)
  #endif
  #ifndef DS_MAX
    #define DS_MAX (44) 
  #endif 
  
  struct mstat;

  struct ihadcb;
  struct opencb;
  struct decb;

  #ifndef PTR32
    #ifdef VSCODE
      #define PTR32
    #else
      #define PTR32 __ptr32
    #endif
  #endif
  
  typedef struct FM_BPAMHandle_InternalStruct FM_BPAMHandle;

  struct desp* PTR32 get_desp_all(const FM_BPAMHandle* bh, const DBG_Opts* opts);
  struct desp* PTR32 find_desp(FM_BPAMHandle* bh, const char* memname, const DBG_Opts* opts);
  void free_desp(struct desp* PTR32, const DBG_Opts* opts);

  FM_BPAMHandle* open_pds_for_write(const char* dataset, const DBG_Opts* opts);
  FM_BPAMHandle* open_pds_for_read(const char* dataset, const DBG_Opts* opts);

  int find_member(FM_BPAMHandle* bh, const char* mem, const DBG_Opts* opts);
  int write_member_dir_entry(const struct mstat* mstat, FM_BPAMHandle* bh, const DBG_Opts* opts);

  ssize_t read_record(FM_BPAMHandle* bh, size_t max_rec_len, char* rec, const DBG_Opts* opts);
  ssize_t read_record_direct(FM_BPAMHandle* bh, char** rec, size_t* rec_len, const DBG_Opts* opts);

  ssize_t write_record(FM_BPAMHandle* bh, size_t rec_len, const char* rec, const DBG_Opts* opts);
  int flush(FM_BPAMHandle* bh, const DBG_Opts* opts);

  size_t record_length(FM_BPAMHandle* bh, const DBG_Opts* opts);

  int close_pds(FM_BPAMHandle* bh, const DBG_Opts* opts);

  int ispf_enq_dataset_member(const char* dataset, const char* wmem);
  int ispf_deq_dataset_member(const char* dataset, const char* wmem);
    
#endif
