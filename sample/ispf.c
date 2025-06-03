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
void set_create_time(struct ispf_stats* is, struct ispf_disk_stats* id)
{
  is->create_time = zerotime;
  pdjd_to_tm(id->pd_create_julian, id->create_century, &is->create_time);
}

void set_mod_time(struct ispf_stats* is, struct ispf_disk_stats* id)
{
  is->mod_time = zerotime;
  pdjd_to_tm(id->pd_mod_julian, id->create_century, &is->mod_time);
  is->mod_time.tm_hour = pd_to_d(id->pd_mod_hours);
  is->mod_time.tm_min = pd_to_d(id->pd_mod_minutes);
  is->mod_time.tm_sec = pd_to_d(id->pd_mod_seconds);
}

int ispf_disk_stats_to_ispf_stats(const char* userdata, int userdata_len, struct ispf_stats* is)
{
  struct ispf_disk_stats* id = (struct ispf_disk_stats*) (userdata);
  int rc = valid_ispf_disk_stats(userdata_len, id);

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