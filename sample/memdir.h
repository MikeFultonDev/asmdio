#ifndef __MEM_DIR__
  #define __MEM_DIR__ 1

  #include <time.h>
  #include "dbgopts.h"

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
    struct tm ispf_created;                     /* time member created  */
    struct tm ispf_changed;                     /* time member modified */
    char*  ispf_id;                             /* ID of user that last modified the member (NULL terminated) */

    void*  _reserved;                           /* NULL (may change in the future) */
  };

  MEMDIR* openmemdir(const char* dataset, const DBG_Opts* opts);
  struct mstat* readmemdir(MEMDIR* memdir, const DBG_Opts* opts);
  int closememdir(MEMDIR* memdir, const DBG_Opts* opts);

#endif
