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

#pragma pack(full)
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
  unsigned short curr_num_lines;
  unsigned short init_num_lines;
  unsigned short mod_num_lines;
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

void* ispf_info(const char* dataset, FM_BPAMHandle* dd, const DBG_Opts* opts)
{
  struct mem_node* np = pds_mem(dataset, dd, opts);
  while (np) {
    if (np->userdata_len == 31 || np->userdata_len == 41) {
      /* ISPF USER DATA */
      /* https://tech.mikefulton.ca/ISPFStatsLayout */
      struct ispf_stats is;
      int rc = ispf_stats(np, &is);
      if (!rc) {
        char crttime_buff[4+1+2+1+2+1];                /* YYYY/MM/DD          */
        char modtime_buff[4+1+2+1+2+1+2+1+2+1+2+1];    /* YYYY/MM/DD HH:MM:SS */
        strftime(crttime_buff, sizeof(crttime_buff), "%Y/%m/%d", &is.create_time);
        strftime(modtime_buff, sizeof(modtime_buff), "%Y/%m/%d %H:%M:%S", &is.mod_time);
        printf(" %s %x%x%x %2.2d.%2.2d %s %s %10d %10d %10d %s\n", 
         np->name, np->ttr[0], np->ttr[1], np->ttr[2],
         is.ver_num, is.mod_num, crttime_buff, modtime_buff, is.curr_num_lines, is.mod_num_lines, is.init_num_lines, is.userid);
      } else {
        printf(" %s (invalid ispf stats) %d\n", np->name, np->userdata_len);
      }
    } else {
      printf(" %s %x%x%x %s\n", np->name, np->ttr[0], np->ttr[1], np->ttr[2], np->is_alias ? "(alias)" : "");
    }
    np = np->next;
  }
  return 0;
}

MEMDIR* openmemdir(const char* dataset, const DBG_Opts* opts)
{
  FM_BPAMHandle dd;
  if (open_pds_for_read(dataset, &dd, opts)) {
    return NULL;
  }
  struct mem_node* np = ispf_info(dataset, &dd, opts);
  struct desp* PTR32 desp = get_desp_all(&dd, opts);
  return NULL;
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
