#include <stdio.h>
#include <stdlib.h>
#include "ihadcb.h"
#include "s99.h"
#include "dio.h"
#include "ioservices.h"

#define MYDD "MYDD"

/*
 * Basic Read of a PDS Member:
 * - Allocate DDName to PDS
 * - Establish DCB for DDName
 * - Perform OPEN on PDS
 * - Perform FIND on member (to be written)
 * - Read records of member (to be written)
 * - Close DCB
 * - Free DDName 
 */
const struct opencb opencb_template = { 1, 0, 0 };
const struct closecb closecb_template = { 1, 0, 0 };

int main(int argc, char* argv[]) {
  struct opencb* __ptr32 opencb;
  struct closecb* __ptr32 closecb;
  struct ihadcb* __ptr32 dcb;
  int rc;

  struct s99_common_text_unit dsn = { DALDSNAM, 1, 0, 0 };
  struct s99_common_text_unit dd = { DALDDNAM, 1, sizeof(MYDD)-1, MYDD };
  struct s99_common_text_unit stats = { DALSTATS, 1, 1, {0x8} };


  if (argc != 3) {
    fprintf(stderr, "Syntax: %s <dataset> <member>\n", argv[0]);
    return 4;
  }

  rc = init_dsnam_text_unit(argv[1], &dsn);
  if (rc) {
    return rc;
  }
  rc = pdsdd_alloc(&dsn, &dd, &stats);
  if (rc) {
    return rc;
  }

  opencb = MALLOC31(sizeof(struct opencb));
  if (!opencb) {
    fprintf(stderr, "Unable to obtain storage for OPEN cb\n");
    return 4;
  }
  dcb = dcb_init(MYDD);
  if (!dcb) {
    fprintf(stderr, "Unable to obtain storage for OPEN dcb\n");
    return 4;
  }

  /*
   * DCB set to PO, BPAM READ and POINT
   */
  dcb->dcbdsgpo = 1; 
  dcb->dcboflgs = dcbofuex;
  dcb->dcbmacr.dcbmacr1 = dcbmrrd|dcbmrpt1;

  *opencb = opencb_template;
  opencb->dcb24 = dcb;
  
  rc = OPEN(opencb);
  if (rc) {
    fprintf(stderr, "Unable to perform OPEN. rc:%d\n", rc);
    return rc;
  }

  closecb = MALLOC31(sizeof(struct closecb));
  if (!closecb) {
    fprintf(stderr, "Unable to obtain storage for CLOSE cb\n");
    return 4;
  }
  *closecb = closecb_template;
  closecb->dcb24 = closecb;
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
