#define _XOPEN_SOURCE
#define _ISOC99_SOURCE
#define _POSIX_SOURCE
#define _OPEN_SYS_FILE_EXT
#define _OPEN_SYS_EXT

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ps.h>

#include "asmdiocommon.h"

#include "util.h"
#include "dio.h"
#include "iosvcs.h"
#include "fm.h"
#include "fmopts.h"
#include "msg.h"
#include "bpamio.h"
#include "ispf.h"

int bpam_open_read(FM_BPAMHandle* handle, const DBG_Opts* opts)
{
  /* msf - tbd */
  return 4;
}

int bpam_open_write(FM_BPAMHandle* handle, const DBG_Opts* opts)
{
  struct ihadcb* PTR32 dcb;
  struct opencb* PTR32 opencb;
  struct decb* PTR32 decb;
  void* PTR32 block;

  const struct opencb opencb_template = { 1, 0, 0, 0, 0 };
  int rc;

  dcb = dcb_init(handle->ddname);
  if (!dcb) {
    fprintf(stderr, "Unable to obtain storage for OPEN dcb\n");
    return 4;
  }

  /*
   * DCB set to PO, BPAM WRITE and POINT
   */
  dcb->dcbeodad.dcbhiarc.dcbbftek.dcbbfaln = 0x84;
  dcb->dcboflgs = dcbofuex;
  dcb->dcbmacr.dcbmacr2 = dcbmrwrt|dcbmrpt2;

  opencb = MALLOC31(sizeof(struct opencb));
  if (!opencb) {
    fprintf(stderr, "Unable to obtain storage for OPEN cb\n");
    return 4;
  }
  *opencb = opencb_template;
  opencb->dcb24 = dcb;
  opencb->mode = OPEN_OUTPUT;

  rc = OPEN(opencb);
  if (rc) {
    fprintf(stderr, "Unable to perform OPEN. rc:%d\n", rc);
    return rc;
  }

  if (!dcb->dcbdsgpo) {
    fprintf(stderr, "Dataset is not a PDSE.\n");
    return 4;
  }

  decb = MALLOC24(sizeof(struct decb));
  if (!decb) {
    fprintf(stderr, "Unable to obtain storage for WRITE decb\n");
    return 4;
  }
  block = MALLOC24(dcb->dcbblksi);
  if (!block) {
    fprintf(stderr, "Unable to obtain storage for WRITE block\n");
    return 4;
  }

  handle->dcb = dcb;
  handle->opencb = opencb;
  handle->decb = decb;
  handle->block = block;
  handle->block_size = dcb->dcbblksi;
  handle->bytes_used = 0;

  return 0;
}

static void validate_block(FM_BPAMHandle* bh, const DBG_Opts* opts)
{
  if (!opts->debug) {
    return;
  }

  char* block_char = (char*) (bh->block);
  unsigned short* block_hw = (unsigned short*) (bh->block);
  unsigned short block_size = block_hw[0];

  debug(opts, "Validate Block: Block Size: %d\n", block_size);
  char* next_rec_start = &(((char*)block_hw)[4]);
  while ((next_rec_start - block_char) < block_size) {
    unsigned short rec_length = *((unsigned short*) next_rec_start); 
    debug(opts, "Record %d Length: %d\n", bh->line_num, rec_length);
    if (rec_length < 4) {
      fprintf(stderr, "Unexpected record length. Validation Failed.\n");
      exit(4);
    }
    next_rec_start = &next_rec_start[rec_length];
    bh->line_num++;
  }
  if (next_rec_start - block_char != block_size) {
    fprintf(stderr, "Total record length did not match block size: (%d,%d)\n", next_rec_start - block_char, block_size);
    exit(4);
  }
}

/*
 * Write out a block. Returns 0 if successful, non-zero otherwise
 */
int write_block(FM_BPAMHandle* bh, const DBG_Opts* opts)
{
  const struct decb decb_template = { 0, 0x8020 };
  *(bh->decb) = decb_template;
  SET_24BIT_PTR(bh->decb->dcb24, bh->dcb);
  bh->decb->area = bh->block;

  debug(opts, "FB:%c VB:%c bytes_used:%d block_size:%d\n", 
    (bh->dcb->dcbexlst.dcbrecfm & dcbrecf) ? 'Y' : 'N', 
    (bh->dcb->dcbexlst.dcbrecfm & dcbrecv) ? 'Y' : 'N', 
    bh->bytes_used, 
    bh->block_size
  );
  if (bh->dcb->dcbexlst.dcbrecfm & dcbrecv) {
    /*
     * Specify the block size for the variable length records 
     */
    unsigned short* halfword = (unsigned short*) (bh->block);
    halfword[0] = bh->bytes_used;  /* size of block */
    halfword[1] = 0;
    halfword[3] = 0;
    bh->dcb->dcbblksi = bh->block_size;

    validate_block(bh, opts);
    debug(opts, "(Block Write) First Record length:%d bytes used:%d\n", halfword[2], halfword[0]);

  } else if (bh->dcb->dcbexlst.dcbrecfm & dcbrecf) { 
    bh->dcb->dcbblksi = bh->bytes_used;
  } else {
    fprintf(stderr, "Not sure how to write a block that is not recv or recf\n");
    return 4;
  }

  int rc = WRITE(bh->decb);
  if (rc) {
    fprintf(stderr, "Unable to perform WRITE. rc:%d\n", rc);
    return rc;
  }

  rc = CHECK(bh->decb);
  if (rc) {
    fprintf(stderr, "Unable to perform CHECK. rc:%d\n", rc);
    return rc;
  }

  if (!bh->ttr_known) {
    bh->ttr = NOTE(bh->dcb);
    bh->ttr_known = 1;
  }

  bh->bytes_used = 0;
  return 0;
}

struct desp* PTR32 init_desp(const FM_BPAMHandle* bh, const char* mem, const DBG_Opts* opts)
{
  const struct desp desp_template = { { { "IGWDESP ", sizeof(struct desp), 1, 0 } } };
  const struct decb decb_template = { 0, 0x8080 };

  struct desp* PTR32 desp;
  struct desl* PTR32 desl;
  struct desl_name* PTR32 desl_name;
  struct desb* PTR32 desb;
  struct decb* PTR32 decb;
  int rc;
  size_t memlen;

  memlen = strlen(mem);

  desp = MALLOC31(sizeof(struct desp));
  if (!desp) {
    fprintf(stderr, "Unable to obtain storage for DESERV\n");
    return NULL;
  }
  desl = MALLOC31(sizeof(struct desl));
  if (!desl) {
    fprintf(stderr, "Unable to obtain storage for DESERV DESL\n");
    return NULL;
  }
  desl_name = MALLOC31(sizeof(struct desl_name));
  if (!desl_name) {
    fprintf(stderr, "Unable to obtain storage for DESERV DESL NAME\n");
    return NULL;
  }
  desl_name->desl_name_len = memlen;
  memcpy(desl_name->desl_name, mem, memlen);

  desl->desl_name_ptr = desl_name;

  /*
   * DESERV GET BYPASS_LLA LIBTYPE DCB CONN_INTENT HOLD EXT_ATTR NAME_LIST AREA
   */
  *desp = desp_template;
  desp->desp_func = desp_func_get;
  desp->desp_bypass_lla = 1;
  desp->desp_ext_attr = 1;
  desp->desp_libtype = desp_libtype_dcb;
  desp->desp_gettype = desp_gettype_name_list;
  desp->desp_conn_intent = desp_conn_intent_hold;

  /* setup DCB */
  desp->desp_dcb_ptr = bh->dcb;

  /* setup DESERV area */
  int desb_len = sizeof(struct desb) + SMDE_NAME_MAXLEN;
  desb = MALLOC31(desb_len);
  if (!desb) {
    fprintf(stderr, "Unable to obtain storage for DESB area\n");
    return NULL;
  }
  desp->desp_area_ptr = desb;
  desp->desp_area2 = desb_len;

  /* setup NAMELIST */
  /* set up DESL list of 1 entry for member to GET */

  desp->desp_name_list_ptr = desl;
  desp->desp_name_list2 = 1;

  return desp;
}

void free_desp(struct desp* PTR32 desp, const FM_Opts* opts)
{
  free(desp->desp_name_list_ptr->desl_name_ptr);
  free(desp->desp_name_list_ptr);
  free(desp->desp_area_ptr);
  free(desp);
}

int read_member_dir_entry(struct desp* PTR32 desp, const DBG_Opts* opts)
{
  /* call DESERV and get extended attributes */
  int rc = DESERV(desp);
  if (rc) {
    fprintf(stderr, "Unable to PERFORM DESERV. rc:0x%x\n", rc);
    return 4;
  }

  struct smde* PTR32 smde = (struct smde* PTR32) (desp->desp_area_ptr->desb_data);
  debug(opts, "Extended attributes for %.*s\n", desp->desp_name_list_ptr->desl_name_ptr->desl_name_len, desp->desp_name_list_ptr->desl_name_ptr->desl_name);
  if (smde->smde_ext_attr_off == 0) {
    debug(opts, "(no extended attributes) SMDE Address:%p SMDE Eye-catcher %8.8s\n", smde, smde->smde_id);
  } else {
    struct smde_ext_attr* PTR32 ext_attr = (struct smde_ext_attr*) (((char*) smde) + smde->smde_ext_attr_off);
    debug(opts, "CCSID: 0x%x%x last change userid: %8.8s change timestamp: 0x%llx\n",
      ext_attr->smde_ccsid[0], ext_attr->smde_ccsid[1], ext_attr->smde_userid_last_change, *((long long*) ext_attr->smde_change_timestamp));
  }

  return 0;
}

static void d2pd(char* pd, int val, int set_positive_sign)
{
  if (set_positive_sign) {
    *pd = 0xF;
    *pd |= (val << 4);
  } else {
    *pd = (val % 10);
    *pd |= (val / 10) << 4;
  }
}
    
static void set_ispf_date(unsigned char* century, char* pdjd, struct tm* ltime)
{
  int YEAR,MONTH,DAY,I,J,K;
  int JD;

  YEAR = ltime->tm_year + 1900;
  MONTH = ltime->tm_mon + 1;
  DAY = ltime->tm_mday;

  /*
   * From: https://aa.usno.navy.mil/faq/JD_formula
   */
  I  = YEAR;
  J  = MONTH;
  K  = DAY;
  JD = K-32075+1461*(I+4800+(J-14)/12)/4+367*(J-2-(J-14)/12*12)/12-3*((I+4900+(J-14)/12)/100)/4;

  /*
   * adjust to days from 2000
   */
  *century = 1;   /* msf - add code for dates before 2000 */
  JD -= 2436310;  /* 01/01/2000 Julian                    */

  int thousands = JD/1000;
  int tens = (JD % 1000) / 10;
  int ones = (JD % 10);
  d2pd(&pdjd[0], thousands, 0);
  d2pd(&pdjd[1], tens, 0);
  d2pd(&pdjd[2], ones, 1);
}

const struct stowlist_add stowlistadd_template = { "        ", 0, 0, 0, 0 };
static void add_mem_stats(struct stowlist_add* PTR32 sla, const char* memname, size_t memlen, unsigned int ttr)
{
  char userid[8+1] = "        "; 
  *sla = stowlistadd_template;
  memcpy(sla->mem_name, memname, memlen);
  STOW_SET_TTR((*sla), ttr);

  unsigned int userdata_len = sizeof(struct ispf_disk_stats)/2; /* number of halfwords of ISPF statistics */
  sla->c = userdata_len;

  struct ispf_disk_stats ids = { 0 };
  __getuserid(userid, sizeof(userid));

  ids.extended = 1;
  time_t t;
  struct tm * ltime;

  time ( &t );
  ltime = localtime ( &t );

  set_ispf_date(&ids.create_century, ids.pd_create_julian, ltime);
  set_ispf_date(&ids.mod_century, ids.pd_mod_julian, ltime);
  d2pd(&ids.pd_mod_hours, ltime->tm_hour, 0);
  d2pd(&ids.pd_mod_minutes, ltime->tm_min, 0);
  d2pd(&ids.pd_mod_seconds, ltime->tm_sec, 0);
  memcpy(&ids.userid, userid, sizeof(userid)-1);

  memcpy(sla->user_data, &ids, sizeof(struct ispf_disk_stats));
}

static int write_pds_member_dir_entry(struct ihadcb* PTR32 dcb, const char* ds, const char* member, struct stowlist_add* stowlistadd, const DBG_Opts* opts)
{
  union stowlist* stowlist;

  stowlist = MALLOC24(sizeof(struct stowlist_add));
  if (!stowlist) {
    fprintf(stderr, "Unable to obtain storage for STOW\n");
    return 4;
  }
  stowlist->add = *stowlistadd;

  /*
   * msf - this STOW is not working if the member already exists - it is failing
   * with an rc of 20.
   */
  int rc = STOW(stowlist, dcb, STOW_R);
  if (rc == STOW_REPLACE_MEMBER_DOES_NOT_EXIST || rc == STOW_CC_OK) {
    debug(opts, "Member %s(%s) successfully replaced\n", ds, member);
  } else {
    fprintf(stderr, "STOW REPLACE failed for PDS member %s(%s) with rc:%d\n", ds, member, rc);
  }

  return rc;
}

static int update_pdse_member_dir_entry(const FM_BPAMHandle* bh, const char* ds, const char* member, union stowlist* stowlist, const DBG_Opts* opts)
{
  int rc;
  struct desp* PTR32 desp = init_desp(bh, member, opts);
  if (!read_member_dir_entry(desp, opts)) {
    /*
     * Try again and perform an UPDATE
     */
    struct smde* PTR32 smde = (struct smde* PTR32) (desp->desp_area_ptr->desb_data);
    struct smde_ext_attr* PTR32 ext_attr = (struct smde_ext_attr*) (((char*) smde) + smde->smde_ext_attr_off);
    memcpy(stowlist->iff.timestamp, ext_attr->smde_change_timestamp, STOWLIST_IFF_TIMESTAMP_LEN);
    rc = STOW(stowlist, NULL, STOW_IFF);
    if (rc != STOW_CC_OK) {
      fprintf(stderr, "STOW failed for PDSE member update of %s(%s) with rc:%d\n", ds, member, rc);
    }
  }
  return rc;
}

int write_member_dir_entry(const FM_BPAMHandle* bh, const FM_FileHandle* fh, const char* ds, const char* member, const DBG_Opts* opts)
{
  const struct stowlist_iff stowlistiff_template = { sizeof(struct stowlist_iff), 0, 0, 0, 0, 0, 0, 0 };
  union stowlist* stowlist;
  struct stowlist_add* stowlistadd;
  size_t memlen = strlen(member);
  stowlist = MALLOC24(sizeof(struct stowlist_iff));
  stowlistadd = MALLOC24(sizeof(struct stowlist_add));
  int rc;

  if ((!stowlist) || (!stowlistadd)) {
    fprintf(stderr, "Unable to obtain storage for STOW\n");
    return 4;
  }

  add_mem_stats(stowlistadd, member, memlen, bh->ttr);

  stowlist->iff = stowlistiff_template;

  SET_24BIT_PTR(stowlist->iff.dcb24, bh->dcb);
  stowlist->iff.type = STOW_IFF;
  stowlist->iff.direntry = stowlistadd;
  stowlist->iff.ccsid = fh->tag.ft_ccsid;

  /*
   * Assume the is a PDSE and we can STOW with IFF.
   * Also assume the member does not exist yet.
   */
  rc = STOW(stowlist, NULL, STOW_IFF);
  switch (rc) {
    case STOW_IFF_CC_CREATE_OK:
      rc = 0;
      break;
    case STOW_IFF_CC_PDS_UPDATE_UNSUPPORTED:
      debug(opts, "Member %s(%s) is in a PDS and not a PDSE - do a STOW and not a STOW_IFF\n", ds, member);
      free(stowlist);
      rc = write_pds_member_dir_entry(bh->dcb, ds, member, stowlistadd, opts);
      break;
    case STOW_IFF_CC_MEMBER_EXISTS:
      debug(opts, "Member %s already exists - update it.\n", member);
      rc = update_pdse_member_dir_entry(bh, ds, member, stowlist, opts);
      break;
    default:
      fprintf(stderr, "STOW failed for member create of %s(%s) with rc:0x%x\n", ds, member, rc);
      break;
  }
  return rc;
}

static int alloc_pds(const char* dataset, FM_BPAMHandle* bh, const DBG_Opts* opts)
{
  struct s99_common_text_unit dsn = { DALDSNAM, 1, 0, 0 };
  struct s99_common_text_unit dd = { DALRTDDN, 1, sizeof(DD_SYSTEM)-1, DD_SYSTEM };
  struct s99_common_text_unit stats = { DALSTATS, 1, 1, { DALSTATS_SHR } };

  int rc = init_dsnam_text_unit(dataset, &dsn);
  if (rc) {
    return rc;
  }
  rc = dsdd_alloc(&dsn, &dd, &stats);
  if (rc) {
    return rc;
  }

  /*
   * Copy system generated DD name into passed in handle
   */
  memcpy(bh->ddname, dd.s99tupar, dd.s99tulng);
  bh->ddname[dd.s99tulng] = '\0';

  debug(opts, "Allocated DD:%s to %s\n", bh->ddname, dataset);

  return 0;
}

int open_pds_for_read(const char* dataset, FM_BPAMHandle* bh, const DBG_Opts* opts)
{
  int rc = alloc_pds(dataset, bh, opts);
  if (!rc) {
    rc = bpam_open_read(bh, opts);
  }
  return rc;
}

int open_pds_for_write(const char* dataset, FM_BPAMHandle* bh, const DBG_Opts* opts)
{
  int rc = alloc_pds(dataset, bh, opts);
  if (!rc) {
    rc = bpam_open_write(bh, opts);
  }
  return rc;
}

int close_pds(const char* dataset, const FM_BPAMHandle* bh, const DBG_Opts* opts)
{
  const struct closecb closecb_template = { 1, 0, 0 };
  struct closecb* PTR32 closecb;
  int rc;

  struct s99_common_text_unit dd = { DUNDDNAM, 1, 0, 0 };
  int ddname_len = strlen(bh->ddname);
  dd.s99tulng = ddname_len;
  memcpy(dd.s99tupar, bh->ddname, ddname_len);

  closecb = MALLOC31(sizeof(struct closecb));
  if (!closecb) {
    fprintf(stderr, "Unable to obtain storage for CLOSE cb\n");
    return 4;
  }
  *closecb = closecb_template;
  closecb->dcb24 = bh->dcb;

  rc = CLOSE(closecb);
  if (rc) {
    fprintf(stderr, "Unable to perform CLOSE. rc:%d\n", rc);
    return rc;
  }

  rc = ddfree(&dd);
  debug(opts, "Free DD:%s\n", bh->ddname);

  return rc;
}
