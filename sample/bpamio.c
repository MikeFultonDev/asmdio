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
#include "ztime.h"

static int bpam_open(FM_BPAMHandle* handle, int mode, const DBG_Opts* opts)
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
   * DCB set to PO, BPAM INPUT|OUTPUT and POINT
   */
  dcb->dcbeodad.dcbhiarc.dcbbftek.dcbbfaln = 0x84;
  dcb->dcboflgs = dcbofuex;

  switch (mode) {
    case OPEN_INPUT:
      dcb->dcbmacr.dcbmacr1 = dcbmrrd|dcbmrpt1;
      break;
    case OPEN_OUTPUT:
      dcb->dcbmacr.dcbmacr2 = dcbmrwrt|dcbmrpt2;
      break;
    default:
      fprintf(stderr, "bpam_open function only supports INPUT and OUTPUT. %d specified\n", mode);
      return 4;
  }

  opencb = MALLOC31(sizeof(struct opencb));
  if (!opencb) {
    fprintf(stderr, "Unable to obtain storage for OPEN cb\n");
    return 4;
  }
  *opencb = opencb_template;
  opencb->dcb24 = dcb;
  opencb->mode = mode;

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

static int bpam_open_read(FM_BPAMHandle* handle, const DBG_Opts* opts)
{
  return bpam_open(handle, OPEN_INPUT, opts);
}

static int bpam_open_write(FM_BPAMHandle* handle, const DBG_Opts* opts)
{
  return bpam_open(handle, OPEN_OUTPUT, opts);
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
 * Read block.
 */
int read_block(FM_BPAMHandle* bh, const DBG_Opts* opts)
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

  /* Read one block */
  int rc = READ(bh->decb);
  if (rc) {
    fprintf(stderr, "Unable to perform READ. rc:%d\n", rc);
    return rc;
  }
  rc = CHECK(bh->decb);
#if 0
  /* no RC from CHECK */
  if (rc) {
    fprintf(stderr, "Read to end of member. rc:%d\n", rc);
    return rc;
  }
#endif

#if 0
  fprintf(stdout, "Block read:%p (%d bytes)\n", bh->block, bh->dcb->dcbblksi);
  dumpstg(stdout, bh->block, bh->dcb->dcbblksi);
  fprintf(stdout, "\n");
#endif

  return 0;
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

  if (!bh->ttr_known) {
    bh->ttr = NOTE(bh->dcb);
    bh->ttr_known = 1;
  }

  bh->bytes_used = 0;
  return 0;
}

const struct desp desp_template = { { { "IGWDESP ", sizeof(struct desp), 1, 0 } } };
const struct decb decb_template = { 0, 0x8080 };
struct desp* PTR32 get_desp_all(const FM_BPAMHandle* bh, const DBG_Opts* opts)
{
  struct desp* PTR32 desp;
  struct desl* PTR32 desl;
  struct desl_name* PTR32 desl_name;
  struct desb* PTR32 desb;
  struct decb* PTR32 decb;
  struct smde* PTR32 smde;
  int rc;

  desp = MALLOC31(sizeof(struct desp));
  if (!desp) {
    fprintf(stderr, "Unable to obtain storage for DESERV\n");
    return NULL;
  }

  /*
   * DESERV GET_ALL BYPASS_LLA LIBTYPE DCB CONN_INTENT NONE EXT_ATTR NAME_LIST AREA
   */
  *desp = desp_template;
  desp->desp_func = desp_func_get_all;
  desp->desp_bypass_lla = 1;
  desp->desp_ext_attr = 1;
  desp->desp_libtype = desp_libtype_dcb;
  desp->desp_conn_intent = desp_conn_intent_none;

  /* setup DCB */
  desp->desp_dcb_ptr = bh->dcb;

  /* setup DESERV area */
  int desb_len = sizeof(struct desb);
  desb = MALLOC31(desb_len);
  if (!desb) {
    fprintf(stderr, "Unable to obtain storage for DESB area\n");
    return NULL;
  }

  desp->desp_area_ptr = desb;
  desp->desp_area2 = desb_len;
  desp->desp_areaptr_ptr = &desp->desp_area_ptr;

  /* call DESERV and get extended attributes */
  rc = DESERV(desp);
  if (rc) {
    fprintf(stderr, "Unable to PERFORM DESERV GET_ALL. rc:0x%x\n", rc);
    return NULL;
  }

  return desp;
}

struct desp* PTR32 init_desp(const FM_BPAMHandle* bh, const char* mem, const DBG_Opts* opts)
{

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

  tm_to_pdjd(&ids.create_century, ids.pd_create_julian, ltime);
  tm_to_pdjd(&ids.mod_century, ids.pd_mod_julian, ltime);
  ids.pd_mod_hours = d_to_pd(ltime->tm_hour, 0);
  ids.pd_mod_minutes = d_to_pd(ltime->tm_min, 0);
  ids.pd_mod_seconds = d_to_pd(ltime->tm_sec, 0);
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

  int rc = STOW(stowlist, dcb, STOW_R);
  if (rc == STOW_REPLACE_MEMBER_DOES_NOT_EXIST || rc == STOW_CC_OK) {
    debug(opts, "Member %s(%s) successfully replaced\n", ds, member);
    rc = 0;
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

/*
 * ADD_NAME: Add a new member name to the linked node. The new member is
 * added to the end so that the original ordering is maintained.
 */
static char *add_member_node(struct mem_node** node, const char *name, int is_alias, char* ttr, struct mem_node** last_ptr, const char* userdata, char userdata_len)
{

  struct mem_node* newnode;

  /*
   * malloc space for the new node
   */

  newnode = (struct mem_node*)malloc(sizeof(struct mem_node));
  if (newnode == NULL) {
    fprintf(stderr,"malloc failed for %d bytes\n",sizeof(struct mem_node));
    exit(-1);
  }

   /* copy the name into the node and NULL terminate it */

  memcpy(newnode->name,name,MEM_MAX);
  newnode->name[MEM_MAX] = '\0';
  newnode->next = NULL;

  memcpy(newnode->ttr, ttr, TTR_LEN);
  newnode->is_alias = is_alias ? 1 : 0;
  memcpy(newnode->userdata, userdata, userdata_len);
  newnode->userdata_len = userdata_len;

  /*
   * add the new node to the linked list
   */

  if (*last_ptr != NULL) {
    (*last_ptr)->next = newnode;
    *last_ptr = newnode;
  }
  else {
    *node = newnode;
    *last_ptr = newnode;
  }
  return(newnode->name);
}

/*
 * GEN_struct mem_node() processes the record passed. The main loop scans through the
 * record until it has read at least rec->count bytes, or a directory end
 * marker is detected.
 *
 * Each record has the form:
 *
 * +------------+------+------+------+------+----------------+
 * + # of bytes ¦Member¦Member¦......¦Member¦  Unused        +
 * + in record  ¦  1   ¦  2   ¦      ¦  n   ¦                +
 * +------------+------+------+------+------+----------------+
 *  ¦--count---¦¦-----------------rest-----------------------¦
 *  (Note that the number stored in count includes its own
 *   two bytes)
 *
 * And, each member has the form:
 *
 * +--------+-------+----+-----------------------------------+
 * + Member ¦TTR    ¦info¦                                   +
 * + Name   ¦       ¦byte¦  User Data TTRN's (halfwords)     +
 * + 8 bytes¦3 bytes¦    ¦                                   +
 * +--------+-------+----+-----------------------------------+
 */

/*
 * bit 0 of the info-byte is '1' if the member is an alias,
 * 0 otherwise. ALIAS_MASK is used to extract this information
 */

#define ALIAS_MASK ((unsigned int) 0x80)

/*
 * The number of user data half-words is in bits 3-7 of the info byte.
 * SKIP_MASK is used to extract this information.  Since this number is
 * in half-words, it needs to be double to obtain the number of bytes.
 */
#define SKIP_MASK ((unsigned int) 0x1F)

/*
 * 8 hex FF's mark the end of the directory
 */

char *endmark = "\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF";

/*
 * RECORD: each record of a pds will be read into one of these structures.
 *         The first 2 bytes is the record length, which is put into 'count',
 *         the remaining 254 bytes are put into rest. Each record is 256 bytes long.
 */

#define RECLEN  254

typedef struct {
  unsigned short int count;
  char rest[RECLEN];
} RECORD;

static int gen_node(struct mem_node** node, RECORD *rec, struct mem_node** last_ptr)
{

   char *ptr, *name;
   int skip, count = 2;
   unsigned int info_byte, alias, ttrn;
   char ttr[TTR_LEN];
   int list_end = 0;

   ptr = rec->rest;

   while(count < rec->count) {
     if (!memcmp(ptr,endmark,MEM_MAX)) {
       list_end = 1;
       break;
     }

     /* member name */
     name = ptr;
     ptr += MEM_MAX;

     /* ttr */
     memcpy(ttr,ptr,TTR_LEN);
     ptr += TTR_LEN;

     /* info_byte */
     info_byte = (unsigned int) (*ptr);
     alias = info_byte & ALIAS_MASK;
     skip = (info_byte & SKIP_MASK) * 2 + 1;
     add_member_node(node,name,alias,ttr,last_ptr,ptr,skip);
     ptr += skip;
     count += (TTR_LEN + MEM_MAX + skip);
   }
   return(list_end);
}

struct mem_node* pds_mem(const char* dataset, FM_BPAMHandle* bh, const DBG_Opts* opts)
{
  struct mem_node* node, *last_ptr;
  RECORD* rec;
  int list_end;

  node = NULL;
  last_ptr = NULL;
  int rc;
  int offset;

  /*
   * Read the PDS directory one block at a time until either the 
   * end of the directory or end-of-file is detected. 
   * Break the block into records and call up gen_node() with every 
   * record read, to add member names to the linked list.
  */

  while ((rc = read_block(bh, opts)) == 0) {
    for (offset = 0; offset < bh->dcb->dcbblksi; offset += sizeof(RECORD)) {
      rec = (RECORD*) &(((char*)bh->block)[offset]);
      list_end = gen_node(&node, rec, &last_ptr);
      if (list_end) {
        return node;
      }
    }
  }
  return NULL;
}

/*
 * FREE_MEM: This function should be used
 * as soon as you are finished using the linked list. It frees the storage
 * allocated by the linked list.
*/

void free_mem(struct mem_node* node)
{
  struct mem_node* next_node=node;

  while (next_node != NULL) {
     next_node = node->next;
     free(node);
     node = next_node;
  }
  return;
}
