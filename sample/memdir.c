#define _XOPEN_SOURCE
#define _ISOC99_SOURCE
#define _POSIX_SOURCE
#define _OPEN_SYS_FILE_EXT

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "memdir.h"
#include "ztime.h"
#include "bpamio.h"

#pragma pack(1)
struct ispf_disk_stats {
  char encoded_userdata_length;

  unsigned char ver_num;
  unsigned char mod_num;
  int sclm:1;
  int reserve_a:1;
  int extended:1;
  int reserve_b:5;
  unsigned char pd_mod_seconds;

  unsigned char create_century;
  char pd_create_julian[3];

  unsigned char mod_century;
  char pd_mod_julian[3];

  unsigned char pd_mod_hours;
  unsigned char pd_mod_minutes;
  unsigned short curr_num_lines;

  unsigned short init_num_lines;
  unsigned short mod_num_lines;

  char userid[8];

  /* following is available only in extended format */
  unsigned int full_curr_num_lines;
  unsigned int full_init_num_lines;
  unsigned int full_mod_num_lines;
};
#pragma pack(pop)

struct ispf_stats {
  struct tm create_time;
  struct tm mod_time;
  unsigned int curr_num_lines;
  unsigned int init_num_lines;
  unsigned int mod_num_lines;
  unsigned char userid[8+1];
  unsigned char ver_num;
  unsigned char mod_num;
  unsigned char sclm;
};

/*
 * msf - need to implement check of ranges of values
 */
static int valid_ispf_disk_stats(const struct ispf_disk_stats* ids)
{
  return 0; 
}

const struct tm zerotime = { 0 };
static void set_create_time(struct ispf_stats* is, struct ispf_disk_stats* id)
{
  is->create_time = zerotime;
  pdjd_to_tm(id->pd_create_julian, id->create_century, &is->create_time);
}

static void set_mod_time(struct ispf_stats* is, struct ispf_disk_stats* id)
{
  is->mod_time = zerotime;
  pdjd_to_tm(id->pd_mod_julian, id->create_century, &is->mod_time);
  is->mod_time.tm_hour = pd_to_d(id->pd_mod_hours);
  is->mod_time.tm_min = pd_to_d(id->pd_mod_minutes);
  is->mod_time.tm_sec = pd_to_d(id->pd_mod_seconds);
}

static int ispf_stats(const struct mem_node* np, struct ispf_stats* is)
{
  struct ispf_disk_stats* id = (struct ispf_disk_stats*) (np->userdata);
  int rc = valid_ispf_disk_stats(id);

  if (rc) {
    return rc;
  }
  set_create_time(is, id);
  set_mod_time(is, id);
  memcpy(is->userid, id->userid, 8);
  is->userid[8] = '\0';

  is->ver_num = id->ver_num;
  is->mod_num = id->mod_num;
  is->sclm = id->sclm;

  if (id->extended) {
    is->curr_num_lines = id->full_curr_num_lines;
    is->init_num_lines = id->full_init_num_lines;
    is->mod_num_lines = id->full_mod_num_lines;
  } else {
    is->curr_num_lines = id->curr_num_lines;
    is->init_num_lines = id->init_num_lines;
    is->mod_num_lines = id->mod_num_lines;
  }
    
  return 0;
}

static struct mstat* memnode_to_mstat(struct mem_node* np, const DBG_Opts* opts, size_t* members)
{

  /*
   * Allocate array of mstat entries for all names coming from the PDS directory.
   * Zero out all the fields on allocation.
   */
  size_t entries = 0;
  struct mem_node* cur_np = np;
  while (cur_np) {
    cur_np = cur_np->next;
    ++entries;
  }
  struct mstat* mstat = calloc(entries, sizeof(struct mstat));
  if (!mstat) {
    return NULL;
  }
  *members = entries;

  /*
   * Walk through the nodes and populate corresponding entries with the information
   * available.
   * Even though some names are fixed length, allocate them out of the heap so that
   * it is easier to just free all pointers (and also reduce code modification if
   * longer names ever supported) since the mstat structure is external.
   *
   * Entries from the PDS directory will have EITHER a name OR an alias name.
   */
  cur_np = np;
  int entry = 0;
  while (cur_np) {
    char* alias_name;
    char* name;

    memcpy(mstat[entry].mem_id, cur_np->ttr, 3);
    if ((cur_np)->is_alias) {
      mstat[entry].is_alias = 1;
      alias_name = malloc(8+1);
      if (!alias_name) {
        return NULL;
      }
      memcpy(alias_name, cur_np->name, 8);
      alias_name[8] = '\0';
      mstat[entry].alias_name = alias_name;
    } else {
      mstat[entry].is_alias = 0;
      name = malloc(8+1);
      if (!name) {
        return NULL;
      }
      memcpy(name, cur_np->name, 8);
      name[8] = '\0';
      mstat[entry].name = name;
    }
    mstat[entry].has_ext = 0;

    if (cur_np->userdata_len == 31 || cur_np->userdata_len == 41) {
      /* ISPF USER DATA */
      /* https://tech.mikefulton.ca/ISPFStatsLayout */
      struct ispf_stats is;
      int rc = ispf_stats(cur_np, &is);

      if (!rc) {
        mstat[entry].ispf_stats = 1;
        mstat[entry].ispf_created = is.create_time;
        mstat[entry].ispf_changed = is.mod_time;

        char* ispf_id = malloc(8+1);
        if (!ispf_id) {
          return NULL;
        }
        memcpy(ispf_id, is.userid, 8);
        ispf_id[8] = '\0';
        mstat[entry].ispf_id = ispf_id;

        mstat[entry].ispf_version = is.ver_num;
        mstat[entry].ispf_modification = is.mod_num;
        mstat[entry].ispf_current_lines = is.curr_num_lines;
        mstat[entry].ispf_initial_lines = is.init_num_lines;
        mstat[entry].ispf_modified_lines = is.mod_num_lines;
      }
    }
    cur_np = cur_np->next;
    entry++;
  }
  return mstat;
}

static struct mstat* desp_copy_name_and_alias(struct mstat* mstat, struct smde* PTR32 smde)
{

  char* mem_name = NULL;
  char* alias_name = NULL;

  struct smde_name* PTR32 varname = (struct smde_name*) (((char*) smde) + smde->smde_name_off);
  char* PTR32 name = varname->smde_name_val;
  int len = varname->smde_name_len;

  /*
   * If the SMDE entry is from a PDS, there will be no name offset if this entry is for 
   * an alias. 
   * However, if the SMDE entry is from a PDSE, there _will_ be a name offset if the entry
   * is for an alias.
   */
  if (smde->smde_flag_alias) {
    /*
     * Name is an alias
     */
    alias_name = malloc(len+1);
    if (!alias_name) {
      return NULL;
    }
    memcpy(alias_name, name, len);
    alias_name[len] = '\0';
    if (smde->smde_pname_off != 0) {
      /*
       * Pointer to underlying member name is available
       */
      struct smde_pname* PTR32 pname = (struct smde_pname*) (((char*) smde) + smde->smde_pname_off);
      char* PTR32 pmem = pname->smde_pname_val;
      int plen = pname->smde_pname_len;

      mem_name = malloc(plen+1);
      if (!name) {
        return NULL;
      }
      memcpy(mem_name, pname, plen);
      mem_name[plen] = '\0';
    }
  } else {
    /*
     * Name is not an alias
     */
    mem_name = malloc(len+1);
    if (!mem_name) {
      return NULL;
    }
    memcpy(mem_name, name, len);
    mem_name[len] = '\0';
  }
  mstat->name = mem_name;
  mstat->alias_name = alias_name;

  return mstat;
}

static void print_members(struct mstat* mstat_arr, size_t members)
{
  printf("\n%d members to print\n", members);
  printf("ISPF EXT ALIAS MEM-NAME    ALIAS   EXT-ID    CCSID   EXT-CHANGED         VER MOD  CUR-LINES INIT-LINES  MOD-LINES CREATE-TIME           CHANGE-TIME  ISPF-ID\n");
  for (int i=0; i<members; ++i) {
    struct mstat* mstat = &mstat_arr[i];
    char crttime_buff[4+1+2+1+2+1];                /* YYYY/MM/DD          */
    char modtime_buff[4+1+2+1+2+1+2+1+2+1+2+1];    /* YYYY/MM/DD HH:MM:SS */
    char exttime_buff[4+1+2+1+2+1+2+1+2+1+2+1];    /* YYYY/MM/DD HH:MM:SS */

    char* ispf  = mstat->ispf_stats ? "Y" : "N";
    char* ext = mstat->has_ext ? "Y" : "N";
    char* alias = mstat->is_alias ? "Y" : "N";
    const char* name = (mstat->name) ? mstat->name : "NULL";
    const char* alias_name = (mstat->alias_name) ? mstat->alias_name : "NULL";;

    char* ext_id = (mstat->ext_id) ? mstat->ext_id : "NULL";
    unsigned short ccsid = mstat->ext_ccsid;
    if (mstat->has_ext) {
      struct tm* exttime = localtime(&mstat->ext_changed);
      strftime(exttime_buff, sizeof(exttime_buff), "%Y/%m/%d %H:%M:%S", exttime);
    } else {
      memcpy(exttime_buff, "none", 5);
    }
    short ver_num = mstat->ispf_version;
    short mod_num = mstat->ispf_modification;
    int cur = mstat->ispf_current_lines;
    int init = mstat->ispf_initial_lines;
    int mod = mstat->ispf_modified_lines;
    if (mstat->ispf_stats) {
      strftime(crttime_buff, sizeof(crttime_buff), "%Y/%m/%d", &mstat->ispf_created);
      strftime(modtime_buff, sizeof(modtime_buff), "%Y/%m/%d %H:%M:%S", &mstat->ispf_changed);
    } else {
      memcpy(crttime_buff, "none", 5);
      memcpy(modtime_buff, "none", 5);
    }
    char* ispf_id = (mstat->ispf_id) ? mstat->ispf_id : "NULL";

    printf("%4s %3s %5s %8s %8s %8s %8x %21s %3d %3d %10d %10d %10d %11s %21s %8s\n", 
      ispf, ext, alias, name, alias_name, ext_id, ccsid, exttime_buff, 
      ver_num, mod_num, cur, init, mod, 
      crttime_buff, modtime_buff, ispf_id);
  }
}

static struct mstat* desp_to_mstat(struct desp* PTR32 desp, const DBG_Opts* opts, size_t* tot_members)
{
  /*
   * Allocate array of mstat entries for all names coming from the Directory Entry Services
   * Zero out all the fields on allocation.
   */
  size_t entries = 0;

  struct desb* PTR32 cur_desb = desp->desp_area_ptr;
  while (cur_desb) {
    int sub_members = cur_desb->desb_count;
    entries += sub_members;
    cur_desb = cur_desb->desb_next;
  }

  struct mstat* mstat = calloc(entries, sizeof(struct mstat));
  if (!mstat) {
    return NULL;
  }
  *tot_members = entries;

  /*
   * Walk through the nodes and populate corresponding entries with the information
   * available.
   * Unlike the PDS directory entries, a DES directory entry will have an alias AND a name
   * or just a name (if there are no aliases for the name). 
   */
  cur_desb = desp->desp_area_ptr;
  int entry = 0;
  while (cur_desb) {
    int i;
    int sub_members = cur_desb->desb_count;

    /*
     * Walk through linked list of SMDE's
     */
    struct smde* PTR32 smde = (struct smde* PTR32) (cur_desb->desb_data);
    for (i=0; i<sub_members; ++i) {
      char* mlt = smde->smde_mltk.smde_mlt;
      memcpy(mstat[entry].mem_id, mlt, 3);

      if (!desp_copy_name_and_alias(&mstat[entry], smde)) {
        return NULL;
      }
      if (smde->smde_ext_attr_off != 0) {
        struct smde_ext_attr* PTR32 ext_attr = (struct smde_ext_attr*) (((char*) smde) + smde->smde_ext_attr_off);
        unsigned long long tod = *((long long *) ext_attr->smde_change_timestamp);
        time_t ltime = tod_to_time(tod);

        mstat[entry].has_ext = 1;
        mstat[entry].ext_ccsid = *((short*)(ext_attr->smde_ccsid));

        char* ext_id = malloc(8+1);
        if (!ext_id) {
          return NULL;
        }
        memcpy(ext_id, ext_attr->smde_userid_last_change, 8);
        ext_id[8] = '\0';
        mstat[entry].ext_id = ext_id;

        mstat[entry].ext_changed = ltime;
      }
      smde = (struct smde* PTR32) (((char*) smde) + smde->smde_len);
      ++entry;
    }
    cur_desb = cur_desb->desb_next;
  }
  return mstat;
}

static MEMDIR* merge_mstat(struct mstat* mn_mstat, size_t mn_members, struct mstat* de_mstat, size_t de_members, const DBG_Opts* opts)
{
  return 0;
}

struct MEMDIR_Internal {
  unsigned int version;

  size_t entries;
  size_t cur;
  struct mstat head[0];
};

MEMDIR* openmemdir(const char* dataset, const DBG_Opts* opts)
{
  FM_BPAMHandle dd;
  size_t de_members;
  size_t mn_members;
  if (open_pds_for_read(dataset, &dd, opts)) {
    return NULL;
  }
  struct mem_node* np = pds_mem(dataset, &dd, opts);
  struct desp* PTR32 desp = get_desp_all(&dd, opts);
  if (np == NULL || desp == NULL) {
    return NULL;
  }
  struct mstat* de_mstat = desp_to_mstat(desp, opts, &de_members);
  struct mstat* mn_mstat = memnode_to_mstat(np, opts, &mn_members);

  if (opts->debug) {
    print_members(de_mstat, de_members);
    print_members(mn_mstat, mn_members);
  }

  return merge_mstat(mn_mstat, mn_members, de_mstat, de_members, opts);
}

struct mement* readmemdir(MEMDIR* memdir, const DBG_Opts* opts)
{
  return NULL;
}

int closememdir(MEMDIR* memdir, const DBG_Opts* opts)
{
  return 0;
}

int mstat(struct mement* mement, struct mstat* mem, const DBG_Opts* opts)
{
  return 0;
}
