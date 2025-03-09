#define _XOPEN_SOURCE
#define _ISOC99_SOURCE
#define _POSIX_SOURCE
#define _OPEN_SYS_FILE_EXT 1
#define _XOPEN_SOURCE_EXTENDED 1

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include "mem.h"
#include "memdir.h"
#include "ztime.h"
#include "bpamio.h"
#include "ispf.h"

/*
 * msf - need to implement check of ranges of values
 */
static int valid_ispf_disk_stats(unsigned char userdata_len, const struct ispf_disk_stats* ids)
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
  int rc = valid_ispf_disk_stats(np->userdata_len, id);

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

static void print_members(struct mstat* mstat_arr, size_t members)
{
  printf("\n%d members to print\n", members);
  printf("MEM-ID ISPF EXT ALIAS MEM-NAME    ALIAS   EXT-ID    CCSID   EXT-CHANGED         VER MOD  CUR-LINES INIT-LINES  MOD-LINES CREATE-TIME           CHANGE-TIME  ISPF-ID\n");
  for (int i=0; i<members; ++i) {
    struct mstat* mstat = &mstat_arr[i];
    char crttime_buff[4+1+2+1+2+1];                /* YYYY/MM/DD          */
    char modtime_buff[4+1+2+1+2+1+2+1+2+1+2+1];    /* YYYY/MM/DD HH:MM:SS */
    char exttime_buff[4+1+2+1+2+1+2+1+2+1+2+1];    /* YYYY/MM/DD HH:MM:SS */

    unsigned int memid = mstat->mem_id;
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
      struct tm* ispf_created_time = localtime(&mstat->ispf_created);
      struct tm* ispf_changed_time = localtime(&mstat->ispf_changed);
      strftime(crttime_buff, sizeof(crttime_buff), "%Y/%m/%d", ispf_created_time);
      strftime(modtime_buff, sizeof(modtime_buff), "%Y/%m/%d %H:%M:%S", ispf_changed_time);
    } else {
      memcpy(crttime_buff, "none", 5);
      memcpy(modtime_buff, "none", 5);
    }
    char* ispf_id = (mstat->ispf_id) ? mstat->ispf_id : "NULL";

    printf("%6.6x %4s %3s %5s %8s %8s %8s %8x %21s %3d %3d %10d %10d %10d %11s %21s %8s\n", 
      memid, ispf, ext, alias, name, alias_name, ext_id, ccsid, exttime_buff, 
      ver_num, mod_num, cur, init, mod, 
      crttime_buff, modtime_buff, ispf_id);
  }
}

static struct mstat* memnode_to_mstat(const struct mem_node* np, struct mstat* mstat, const DBG_Opts* opts)
{
  char* alias_name;
  char* name;

  unsigned int mem_id = (*(unsigned int*) np->ttr) >> 8;
  mstat->mem_id = mem_id;
  if ((np)->is_alias) {
    mstat->is_alias = 1;
    alias_name = malloc(8+1);
    if (!alias_name) {
      return NULL;
    }
    memcpy(alias_name, np->name, 8);
    alias_name[8] = '\0';
    mstat->alias_name = alias_name;
  } else {
    mstat->is_alias = 0;
    name = malloc(8+1);
    if (!name) {
      return NULL;
    }
    memcpy(name, np->name, 8);
    name[8] = '\0';
    mstat->name = name;
  }

  if (np->userdata_len == 31 || np->userdata_len == 41) {
    /* ISPF USER DATA */
    /* https://tech.mikefulton.ca/ISPFStatsLayout */
    struct ispf_stats is;
    int rc = ispf_stats(np, &is);

    if (!rc) {
      mstat->ispf_stats = 1;
      mstat->ispf_created = mktime(&is.create_time);
      mstat->ispf_changed = mktime(&is.mod_time);

      char* ispf_id = malloc(8+1);
      if (!ispf_id) {
        return NULL;
      }
      memcpy(ispf_id, is.userid, 8);
      ispf_id[8] = '\0';
      mstat->ispf_id = ispf_id;

      mstat->ispf_version = is.ver_num;
      mstat->ispf_modification = is.mod_num;
      mstat->ispf_current_lines = is.curr_num_lines;
      mstat->ispf_initial_lines = is.init_num_lines;
      mstat->ispf_modified_lines = is.mod_num_lines;
    } else {
      mstat->ispf_stats = 0;
    }
  } else {
    mstat->ispf_stats = 0;
  }
  return mstat;
}

static struct mstat* memnodes_to_mstats(const struct mem_node* np, const DBG_Opts* opts, size_t* members)
{
  /*
   * Allocate array of mstat entries for all names coming from the PDS directory.
   * Zero out all the fields on allocation.
   */
  size_t entries = 0;
  const struct mem_node* cur_np = np;
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
    if (!memnode_to_mstat(cur_np, &mstat[entry], opts)) {
      return NULL;
    }
    cur_np = cur_np->next;
    entry++;
  }
  return mstat;
}

static struct mstat* desp_copy_name_and_alias(struct mstat* mstat, const struct smde* PTR32 smde)
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
    mstat->is_alias = 1;
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
      memcpy(mem_name, pmem, plen);
      mem_name[plen] = '\0';
    }
  } else {
    /*
     * Name is not an alias
     */
    mstat->is_alias = 0;
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


static struct mstat* smde_to_mstat(const struct smde* PTR32 smde, struct mstat* mstat, const DBG_Opts* opts)
{
  /*
   * Copy name, alias, and extended attributes.
   */
  const unsigned char* mlt = smde->smde_mltk.smde_mlt;
  unsigned int mem_id = (*(unsigned int*) mlt) >> 8;
  mstat->mem_id = mem_id;

  if (!desp_copy_name_and_alias(mstat, smde)) {
    return NULL;
  }
  if (smde->smde_ext_attr_off != 0) {
    struct smde_ext_attr* PTR32 ext_attr = (struct smde_ext_attr*) (((char*) smde) + smde->smde_ext_attr_off);
    unsigned long long tod = *((long long *) ext_attr->smde_change_timestamp);
    time_t ltime = tod_to_time(tod);

    mstat->has_ext = 1;
    mstat->ext_ccsid = *((unsigned short*)(ext_attr->smde_ccsid));

    char* ext_id = malloc(8+1);
    if (!ext_id) {
      return NULL;
    }
    memcpy(ext_id, ext_attr->smde_userid_last_change, 8);
    ext_id[8] = '\0';
    mstat->ext_id = ext_id;

    mstat->ext_changed = ltime;
  }
  return mstat;
}

static struct mstat* desp_to_mstats(const struct desp* PTR32 desp, const DBG_Opts* opts, size_t* tot_members)
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
     * Walk through linked list of SMDE's and copy over SMDE info to mstat.
     */
    struct smde* PTR32 smde = (struct smde* PTR32) (cur_desb->desb_data);
    for (i=0; i<sub_members; ++i) {
      if (!smde_to_mstat(smde, &mstat[entry], opts)) {
        return NULL;
      }
      smde = (struct smde* PTR32) (((char*) smde) + smde->smde_len);
      ++entry;
    }
    cur_desb = cur_desb->desb_next;
  }
  return mstat;
}

/*
 * compare by: memid, primary member, aliases, e.g.
 * when the memid (TTR/MLT) is the same, look to the 'alias' and sort non-alias first, then look to alias name and sort by alias
 * name.
 */
static int cmp_mem_primary_alias(const void* lhs, const void* rhs)
{
  const struct mstat* l_mstat = (const struct mstat*) lhs;
  const struct mstat* r_mstat = (const struct mstat*) rhs;

  if (l_mstat->mem_id != r_mstat->mem_id) {
    return l_mstat->mem_id - r_mstat->mem_id;
  } else {
    if (l_mstat->is_alias != r_mstat->is_alias) {
      return l_mstat->is_alias - r_mstat->is_alias;
    } else {
      if (r_mstat->alias_name == NULL) {
        if (l_mstat->alias_name == NULL) {
          return 0;
        } else {
          return 1;
        }
      } else {
        /* r_mstat->alias_name != NULL */
        if (l_mstat->alias_name == NULL) {
          return -1;
        } else {
          return strcmp(r_mstat->alias_name, l_mstat->alias_name);
        }
      }
    }
  }
}

static int cmp_date(const struct mstat* l_mstat, const struct mstat* r_mstat)
{
  time_t l_chgtime;
  time_t r_chgtime;

  if (l_mstat->has_ext) {
    l_chgtime = l_mstat->ext_changed;
  } else if (l_mstat->ispf_stats) {
    l_chgtime = l_mstat->ispf_changed;
  } else {
    l_chgtime = 0;
  }
  if (r_mstat->has_ext) {
    r_chgtime = r_mstat->ext_changed;
  } else if (r_mstat->ispf_stats) {
    r_chgtime = r_mstat->ispf_changed;
  } else {
    r_chgtime = 0;
  }
  if (l_chgtime - r_chgtime > 0) {
    return 1;
  } else if (l_chgtime - r_chgtime < 0) {
    return -1;
  }

  return 0;
}

static int cmp_mem_reverse_time(const void* lhs, const void* rhs)
{
  const struct mstat* l_mstat = (const struct mstat*) lhs;
  const struct mstat* r_mstat = (const struct mstat*) rhs;

  return cmp_date(l_mstat, r_mstat);
}

static int cmp_mem_time(const void* lhs, const void* rhs)
{
  const struct mstat* l_mstat = (const struct mstat*) lhs;
  const struct mstat* r_mstat = (const struct mstat*) rhs;

  return cmp_date(l_mstat, r_mstat) * -1;
}

static int cmp_mem_reverse_name(const void* lhs, const void* rhs)
{
  const struct mstat* l_mstat = (const struct mstat*) lhs;
  const struct mstat* r_mstat = (const struct mstat*) rhs;

  return strcmp(r_mstat->name, l_mstat->name);
}

static int cmp_mem_name(const void* lhs, const void* rhs)
{
  const struct mstat* l_mstat = (const struct mstat*) lhs;
  const struct mstat* r_mstat = (const struct mstat*) rhs;

  return strcmp(l_mstat->name, r_mstat->name);
}

struct MEMDIR_Internal {
  unsigned int version;

  size_t entries;
  size_t cur;
  struct mstat head[0];
};

static void free_mstat(struct mstat* mstat, size_t entries)
{
  for (int i=0; i<entries;++i) {
    if (mstat[i].name) {
      free((char*)mstat[i].name);
    }
    if (mstat[i].alias_name) {
      free((char*)mstat[i].alias_name);
    }
    if (mstat[i].ext_id) {
      free((char*)mstat[i].ext_id);
    }
    if (mstat[i].ispf_id) {
      free((char*)mstat[i].ispf_id);
    }
  }
}

static MEMDIR* merge_mstat(struct mstat* mn_mstat, size_t mn_members, struct mstat* de_mstat, size_t de_members, int sort_time, int sort_reverse, const DBG_Opts* opts)
{
  if (mn_members != de_members) {
    fprintf(stderr, 
      "Internal error: Directory has %d members and alias but Directory Entry Services reports %d members aliases. These should be the same.\n", 
      mn_members, de_members);
    return NULL;
  }
  if (mn_members == 0) {
    return NULL;
  }

  /*
   * Sort the two arrays so they can be walked together
   */
  qsort(de_mstat, de_members, sizeof(struct mstat), cmp_mem_primary_alias);
  qsort(mn_mstat, mn_members, sizeof(struct mstat), cmp_mem_primary_alias);

  size_t entries = de_members;
  struct MEMDIR_Internal* mdi = malloc(sizeof(struct MEMDIR_Internal) + (entries*sizeof(struct mstat)));
  if (!mdi) {
    return NULL;
  }
  mdi->entries = entries;
  mdi->cur = 0;
  struct mstat* merge_mstat = mdi->head;
 
  /*
   * Copy the mn member info over first and then copy the extended
   * information and the alias if it is missing
   */
  for (int i=0; i<entries; ++i) {
    merge_mstat[i] = mn_mstat[i];

    /*
     * Make sure all names are properly dup'ed so that the original
     * mstat structures can be completely freed
     */
    merge_mstat[i].name = strdup(mn_mstat[i].name);

    if (merge_mstat[i].is_alias) {
      if (merge_mstat[i].alias_name == NULL) {
        merge_mstat[i].alias_name = strdup(de_mstat[i].alias_name);
      } else {
        merge_mstat[i].alias_name = strdup(mn_mstat[i].alias_name);
      }
    }
    if (merge_mstat[i].ispf_stats && merge_mstat[i].ispf_id != NULL) {
      merge_mstat[i].ispf_id = strdup(mn_mstat[i].ispf_id);
    }

    if (de_mstat[i].has_ext) {
      merge_mstat[i].has_ext = 1;
      if (de_mstat[i].ext_id != NULL) {
        merge_mstat[i].ext_id = strdup(de_mstat[i].ext_id);
      }
      merge_mstat[i].ext_ccsid = de_mstat[i].ext_ccsid;
      merge_mstat[i].ext_changed = de_mstat[i].ext_changed;
    }
  }

  /*
   * Now walk the merged list and copy in any missing names
   * for aliases without the original name
   */
  for (int i=0; i<entries; ++i) {
    if (!merge_mstat[i].is_alias) {
      int next=i+1;
      while (next < entries) {
        if (!merge_mstat[next].is_alias) break;
        if (merge_mstat[i].mem_id == merge_mstat[next].mem_id && merge_mstat[next].name == NULL) {
          merge_mstat[next].name = strdup(merge_mstat[i].name);
        }
        ++next;
      }
    }
  }

  free_mstat(de_mstat, entries);
  free(de_mstat);
  free_mstat(mn_mstat, entries);
  free(mn_mstat);

  /*
   * Sort the array of mstat information
   */
  if (sort_time) {
    if (sort_reverse) {
      qsort(merge_mstat, entries, sizeof(struct mstat), cmp_mem_reverse_time);
    } else {
      qsort(merge_mstat, entries, sizeof(struct mstat), cmp_mem_time);
    }
  } else {
    if (sort_reverse) {
      qsort(merge_mstat, entries, sizeof(struct mstat), cmp_mem_reverse_name);
    } else {
      qsort(merge_mstat, entries, sizeof(struct mstat), cmp_mem_name);
    }
  }

  if (opts->debug) {
    print_members(merge_mstat, entries);
  }
    
  return (MEMDIR*) mdi;
}

MEMDIR* openmemdir(const char* dataset, int sort_time, int sort_reverse, const DBG_Opts* opts)
{
  FM_BPAMHandle bh;
  size_t de_members;
  size_t mn_members;
  if (open_pds_for_read(dataset, &bh, opts)) {
    return NULL;
  }
  struct mem_node* np = pds_mem(&bh, opts);
  struct desp* PTR32 desp = get_desp_all(&bh, opts);
  if (np == NULL || desp == NULL) {
    return NULL;
  }
  struct mstat* de_mstat = desp_to_mstats(desp, opts, &de_members);
  struct mstat* mn_mstat = memnodes_to_mstats(np, opts, &mn_members);

  if (opts->debug) {
    printf("Members before merge:\n");
    print_members(mn_mstat, mn_members);
    print_members(de_mstat, de_members);
  }

  MEMDIR* merge = merge_mstat(mn_mstat, mn_members, de_mstat, de_members, sort_time, sort_reverse, opts);

  close_pds(&bh, opts);

  return merge;
}

struct mstat* readmemdir(MEMDIR* memdir, const DBG_Opts* opts)
{
  struct MEMDIR_Internal* mdi = (struct MEMDIR_Internal*) memdir;
  if (mdi->cur == mdi->entries) {
    return NULL;
  } else {
    return &mdi->head[mdi->cur++];
  }
}

int closememdir(MEMDIR* memdir, const DBG_Opts* opts)
{
  struct MEMDIR_Internal* mdi = (struct MEMDIR_Internal*) memdir;
  free_mstat(mdi->head, mdi->entries);
  free(mdi);

  return 0;
}

/*
 * msf - may want to either push read/write member directory services to bpamio
 * or pull them all back and put them into memdir.
 * Right now, it's odd having a bit of both in both.
 */
int writememdir_entry(FM_BPAMHandle* bh, const struct mstat* mstat, const DBG_Opts* opts)
{
  return write_member_dir_entry(mstat, bh, opts);
}

int readmemdir_entry(FM_BPAMHandle* bh, const char* mem, struct mstat* mstat, const DBG_Opts* opts)
{
  /*
   * Find the SMDE for the member and then find the mem_node for the member.
   * Merge the contents together.
   * Free up the 31-bit storage allocated for desp
   */ 
  struct desp* PTR32 desp;
  struct smde* PTR32 smde;

  desp = find_desp(bh, mem, opts);
  if (!desp) {
    return 4;
  }

  smde = (struct smde* PTR32) (desp->desp_area_ptr->desb_data);

  struct mstat smde_mstat = { 0 };
  if (!smde_to_mstat(smde, &smde_mstat, opts)) {
    return 4;
  }

  struct mem_node match_node = { 0 };
  if (!find_mem(bh, mem, &match_node, opts)) {
    return 4;
  }
  struct mstat memnode_mstat = { 0 };
  if (!memnode_to_mstat(&match_node, &memnode_mstat, opts)) {
    return 4;
  }

  /*
   * Copy over all the ISPF stats from mnode_mstat
   * and if there are also extended attributes, copy
   * them. 
   * This leaves a few wrinkles with aliases, which
   * could be supported when necessary. (msf tbd)
   */ 
  *mstat = memnode_mstat;
  if (smde_mstat.has_ext) {
    mstat->has_ext = 1;
    mstat->ext_id = strdup(smde_mstat.ext_id);
    mstat->ext_ccsid = smde_mstat.ext_ccsid;
    mstat->ext_changed = smde_mstat.ext_changed;
  }

  free_desp(desp, opts);

  if (opts->debug) {
    print_members(mstat, 1);
  }
  return 0;
}

static char* PTR32 ispf_rname(const char* ds, const char* mem)
{
  unsigned int rname_len = strlen(ds) + strlen(mem);

  if (rname_len > 44+8) {
    fprintf(stderr, "Invalid dataset or member name passed to ENQ/DEQ %s(%s)\n", ds, mem);
    return NULL;
  }

  char* PTR32 rname;
  rname = MALLOC31(52+1);
  if (!rname) {
    fprintf(stderr, "Unable to obtain storage for ENQ/DEQ\n");
    return NULL;
  }
  sprintf(rname, "%-44s%-8s", ds, mem);

  return rname;
}

static char* PTR32 ispf_qname(const char* qn)
{
  unsigned int qname_len = strlen(qn);

  if (qname_len > 8) {
    fprintf(stderr, "Invalid queue name passed to ENQ/DEQ %s\n", qn);
    return NULL;
  }

  char* PTR32 qname;
  qname = MALLOC31(8+1);
  if (!qname) {
    fprintf(stderr, "Unable to obtain storage for ENQ/DEQ\n");
    return NULL;
  }
  sprintf(qname, "%-8s", qn);

  return qname;
}

int ispf_enq_dataset_member(const char* ds, const char* wmem) 
{
  char* PTR32 rname = ispf_rname(ds, wmem);
  char* PTR32 qname = ispf_qname("SPFEDIT");

  if (!rname || !qname) {
    return 4;
  }
  int rc = SYEXENQ(qname, rname, strlen(rname));
  free(rname);
  free(qname);
  return rc;
}

int ispf_deq_dataset_member(const char* ds, const char* wmem) 
{
  char* PTR32 rname = ispf_rname(ds, wmem);
  char* PTR32 qname = ispf_qname("SPFEDIT");

  if (!rname || !qname) {
    return 4;
  }
  int rc = SYEXDEQ(qname, rname, strlen(rname));
  free(rname);
  free(qname);
  return rc;
}
