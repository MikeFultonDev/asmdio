#ifndef __MEM_DIR__
  #define __MEM_DIR__ 1

  #include <time.h>
  #include "dbgopts.h"
  #include "bpamio.h"

  /*
   * MEMDIR: opaque structure (like FILE or DIR)
   * used to iterate over the members in a dataset
   */
  typedef struct {
    unsigned int version;
  } MEMDIR;

  /*
   * struct mstat: provide information for a given
   * member of a PDS or PDSE.
   */

  #define USERID_LEN (8)

   struct mstat {
    int    ispf_stats:1;                        /* true if ISPF statistics saved for this member */
    int    is_alias:1;                          /* true if this is an alias to a member          */
    int    has_ext:1;                           /* true if this member has extended attributes   */

    unsigned int mem_id;                        /* ID that is a unique value for all members in a dataset          */
    const char* name;                           /* name of underlying member (NULL terminated)                     */
    const char* alias_name;                     /* pointer to alias name (NULL if not an alias. Alias can be long) */

    /*
     * ext_* fields are valid only if has_ext is true
     */
    char*  ext_id;                              /* ID of user that last modified the member (NULL terminated)      */
    unsigned short  ext_ccsid;                  /* CCSID (0 indicates no CCSID specified. 0xFF indicates binary    */
    time_t ext_changed;                         /* time member modified */

    /*
     * ispf_* fields are valid only if ispf_stats is true
     */
    short  ispf_version;                        /* 0 to 99    */
    short  ispf_modification;                   /* 0 to 99    */
    int    ispf_current_lines;                  /* 0 to 2**31 */
    int    ispf_initial_lines;                  /* 0 to 2**31 */
    int    ispf_modified_lines;                 /* 0 to 2**31 */
    time_t ispf_created;                        /* time member created  */
    time_t ispf_changed;                        /* time member modified */
    char*  ispf_id;                             /* ID of user that last modified the member (NULL terminated) */

    void*  _reserved;                           /* NULL (may change in the future) */
  };

  /*
   * openmemdir: given a dataset, and how to sort (defaults to name, but 'time' can be specified), and
   *             whether to sort in reverse or not, return a sorted list of member name entries, 
   *             or NULL if an error occurred.
   */
  MEMDIR* openmemdir(const char* dataset, int sort_time, int sort_reverse, const DBG_Opts* opts);

  /*
   * Given a memdir, return the next member in the sorted list
   */
  struct mstat* readmemdir(MEMDIR* memdir, const DBG_Opts* opts);

  /*
   * Given a memdir, close any open files and free any storage associated with the memdir.
   */
  int closememdir(MEMDIR* memdir, const DBG_Opts* opts);

  /*
   * Given an open BPAM Handle for a dataset, and a corresponding mstat member directory entry,
   * write the entry to the dataset (via STOW).
   */
  int writememdir_entry(FM_BPAMHandle* bh, const struct mstat* mstat, const DBG_Opts* opts);

  /*
   *  Given an open BPAM Handle for a dataset, and the name of a member, read the mstat directory information
   *  from the dataset and populate the mstat passed in.
   */
  int readmemdir_entry(FM_BPAMHandle* bh, const char* memname, struct mstat* mstat, const DBG_Opts* opts);

  /*
   * Given a userid, alias name, name, TTR, and number of lines in a member, populate an mstat object and
   * return it.
   */
  struct mstat* create_mstat(struct mstat* mstat, char* userid, const char* alias_name, const char* name, const void* ttr, int num_lines, int ccsid, const DBG_Opts* opts);

  /*
   * print_member - print out a member and optionally print a header for the member
   */
  void print_member(struct mstat* mstat, int print_header);
#endif
