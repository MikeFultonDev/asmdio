#include <stdio.h>
#include <stdlib.h>
#include "ihadcb.h"
#include "s99.h"
#include "decb.h"
#include "dio.h"

/*
 * Basic Create of a PDS Member:
 * - Allocate DDName to PDS
 * - Establish DCB for DDName
 * - Perform OPEN on PDS
 * - Perform STOW to create PDS member
 * - Close DCB
 * - Free DDName 
 */

const struct opencb opencb_template = { 1, 0, 0, 0, 0 };
const struct stowlist_iff stowlistiff_template = { sizeof(struct stowlist_iff), 0, 0, 0, 0, 0, 0, 0 };
const struct stowlist_add stowlistadd_template = { "        ", 0, 0, 0, 0 };
const struct decb decb_template = { 0, 0x8020 };
const struct closecb closecb_template = { 1, 0, 0 };

#define MYDD "MYDD"

int main(int argc, char* argv[]) {
  struct opencb* __ptr32 opencb;
  struct closecb* __ptr32 closecb;
  struct ihadcb* __ptr32 dcb;
  struct decb* __ptr32 decb;
  int rc;
  unsigned int ttr;

  struct s99_common_text_unit dsn = { DALDSNAM, 1, 0, 0 };
  struct s99_common_text_unit dd = { DALDDNAM, 1, sizeof(MYDD)-1, MYDD };
  struct s99_common_text_unit stats = { DALSTATS, 1, 1, {0x8} }; /* OLD=1, SHR=8 */
  void* __ptr32 block;

  union stowlist* stowlist;
  struct stowlist_add* stowlistadd;
  size_t memlen;

  if (argc != 3) {
    fprintf(stderr, "Syntax: %s <dataset> <member>\n", argv[0]);
    return 4;
  }

  memlen = strlen(argv[2]);
  if (memlen == 0 || memlen > 8) {
    fprintf(stderr, "Member must be 1 to 8 characters long\n");
    return 4;
  }

  rc = init_dsnam_text_unit(argv[1], &dsn);
  if (rc) {
    return 4;
  }
  rc = pdsdd_alloc(&dsn, &dd, &stats);
  if (rc) {
    return 4;
  }

  dcb = dcb_init(MYDD);
  if (!dcb) {
    fprintf(stderr, "Unable to obtain storage for OPEN dcb\n");
    return 4;
  }

  /*
   * DCB set to PO, BPAM WRITE and POINT
   */
  dcb->dcbdsgpo = 1; 
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

  fprintf(stderr, "\nA(dcb):%p\n", dcb);
  dumpstg(stderr, dcb, sizeof(struct ihadcb));

  fprintf(stderr, "\nA(dcbe):%p\n", dcb->dcbdcbe);
  dumpstg(stderr, dcb->dcbdcbe, sizeof(struct dcbe));
  fprintf(stderr, "\n");

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
  memset(block, 'a', dcb->dcbblksi);

  *decb = decb_template;
  SET_24BIT_PTR(decb->dcb24, dcb);
  decb->area = block;

  rc = WRITE(decb);
  if (rc) {
    fprintf(stderr, "Unable to perform WRITE. rc:%d\n", rc);
    return rc;
  }

  rc = CHECK(decb);
  if (rc) {
    fprintf(stderr, "Unable to perform CHECK. rc:%d\n", rc);
    return rc;
  }

  ttr = NOTE(dcb);
  fprintf(stderr, "NOTE: ttr:0x%x\n", ttr);

#define STOW_IFF_ON 1
#ifdef STOW_IFF_ON
  stowlist = MALLOC24(sizeof(struct stowlist_iff));
  stowlistadd = MALLOC24(sizeof(struct stowlist_add));
  if ((!stowlist) || (!stowlistadd)) {
    fprintf(stderr, "Unable to obtain storage for STOW\n");
    return 4;
  }
  stowlist->iff = stowlistiff_template;
  *stowlistadd = stowlistadd_template;
  memcpy(stowlistadd->mem_name, argv[2], memlen);
  STOW_SET_TTR((*stowlistadd), ttr);

  SET_24BIT_PTR(stowlist->iff.dcb24, dcb);
  stowlist->iff.type = STOW_IFF;
  stowlist->iff.direntry = stowlistadd;
  stowlist->iff.ccsid = 819; 

  fprintf(stderr, "\nA(stowlist):%p A(stowlist_iff):%p type:%x dcb24:%p ccsid:%d\n", 
    stowlist, &(stowlist->iff), stowlist->iff.type, stowlist->iff.dcb24, stowlist->iff.ccsid);

  fprintf(stderr, "\nSTOWList before: %p\n", stowlist);
  dumpstg(stderr, stowlist, sizeof(struct stowlist_iff));
  fprintf(stderr, "\n");

  rc = STOW(stowlist, NULL, STOW_IFF);
  if (rc) {
    fprintf(stderr, "Unable to perform STOW. rc:%d\n", rc);
    fprintf(stderr, "stowlistadd:%p\n", stowlistadd);
    dumpstg(stderr, stowlistadd, sizeof(struct stowlist_add));
    fprintf(stderr, "\nSTOWList after: %p\n", stowlist);
    dumpstg(stderr, stowlist, sizeof(struct stowlist_iff));
    fprintf(stderr, "\n");
    return rc;
  }
#else
  stowlist = MALLOC24(sizeof(struct stowlist_add));
  if (!stowlist) {
    fprintf(stderr, "Unable to obtain storage for STOW\n");
    return 4;
  }
  stowlist->add = stowlistadd_template;
  memcpy(stowlist->add.mem_name, argv[2], memlen);
  STOW_SET_TTR((stowlist->add), ttr);

  fprintf(stderr, "\nSTOWList before: %p\n", stowlist);
  dumpstg(stderr, stowlist, sizeof(struct stowlist_add));
  fprintf(stderr, "\n");

  rc = STOW(stowlist, dcb, STOW_A);
  if (rc) {
    fprintf(stderr, "Unable to perform STOW. rc:%d\n", rc);
    fprintf(stderr, "stowlist:%p\n", stowlist);
    dumpstg(stderr, stowlist, sizeof(struct stowlist_add));
    fprintf(stderr, "\n");
    return rc;
  }
#endif
  closecb = MALLOC31(sizeof(struct closecb));
  if (!closecb) {
    fprintf(stderr, "Unable to obtain storage for CLOSE cb\n");
    return 4;
  }
  *closecb = closecb_template;
  closecb->dcb24 = dcb;

  rc = CLOSE(closecb);
  if (rc) {
    fprintf(stderr, "Unable to perform CLOSE. rc:%d\n", rc);
    return rc;
  }

  rc = FREE24(dcb, sizeof(struct ihadcb));
  if (rc) {
    fprintf(stderr, "Unable to free dcb storage. rc:%d\n", rc);
    return rc;
  }
  FREE31(opencb);

  rc = ddfree(&dd);
  if (rc) {
    return 4;
  }

  return 0;
}
