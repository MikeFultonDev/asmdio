#include <stdio.h>
#include <stdlib.h>
#include "ihadcb.h"
#include "s99.h"
#include "dio.h"
#include "decb.h"
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
const struct opencb opencb_template = { 1 };
const struct findcb findcb_template = { "        " };
const struct desp desp_template = { { { "IGWDESP ", sizeof(struct desp), 1, 0 } } };
const struct decb decb_template = { 0, 0x8080 };
const struct closecb closecb_template = { 1 };

int main(int argc, char* argv[]) {
  struct opencb* __ptr32 opencb;
  struct findcb* __ptr32 findcb;
  struct closecb* __ptr32 closecb;
  struct ihadcb* __ptr32 dcb;
  struct desp* __ptr32 desp;
  struct desl* __ptr32 desl;
  struct desl_name* __ptr32 desl_name;
  struct desb* __ptr32 desb;
  struct smde* __ptr32 smde;
  struct decb* __ptr32 decb;
  void* __ptr32 block;
  int rc;
  char* ds;
  char* mem;
  size_t memlen;

  struct s99_common_text_unit dsn = { DALDSNAM, 1, 0, 0 };
  struct s99_common_text_unit dd = { DALDDNAM, 1, sizeof(MYDD)-1, MYDD };
  struct s99_common_text_unit stats = { DALSTATS, 1, 1, {0x8} };

  if (argc != 3) {
    fprintf(stderr, "Syntax: %s <dataset> <member>\n", argv[0]);
    return 4;
  }

  ds = argv[1];
  rc = init_dsnam_text_unit(ds, &dsn);
  if (rc) {
    return rc;
  }
  rc = pdsdd_alloc(&dsn, &dd, &stats);
  if (rc) {
    return rc;
  }

  mem = argv[2];
  memlen = strlen(mem);

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

  desp = MALLOC31(sizeof(struct desp));
  if (!desp) {
    fprintf(stderr, "Unable to obtain storage for DESERV\n");
    return 4;
  }
  desl = MALLOC31(sizeof(struct desl));
  if (!desl) {
    fprintf(stderr, "Unable to obtain storage for DESERV DESL\n");
    return 4;
  }
  desl_name = MALLOC31(sizeof(struct desl_name));
  if (!desl_name) {
    fprintf(stderr, "Unable to obtain storage for DESERV DESL NAME\n");
    return 4;
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
  desp->desp_dcb_ptr = dcb;

  /* setup DESERV area */
  int desb_len = sizeof(struct desb) + SMDE_NAME_MAXLEN;
  desb = MALLOC31(desb_len);
  if (!desb) {
    fprintf(stderr, "Unable to obtain storage for DESB area\n");
    return 4;
  }
  desp->desp_area_ptr = desb;
  desp->desp_area2 = desb_len;

  /* setup NAMELIST */

  /* set up DESL list of 1 entry for member to GET */

  desp->desp_name_list_ptr = desl;
  desp->desp_name_list2 = 1;

  fprintf(stdout, "DESP:%p\n", desp);
  dumpstg(stdout, desp, sizeof(struct desp));
  fprintf(stdout, "\n");

  /* call DESERV and get extended attributes */
  rc = DESERV(desp);
  if (rc) {
    fprintf(stderr, "Unable to PERFORM DESERV. rc:0x%x\n", rc);
    return 4;
  }

  smde = (struct smde* __ptr32) (desp->desp_area_ptr->desb_data);
  if (smde->smde_ext_attr_off == 0) {
    fprintf(stdout, "No extended attributes for %s(%s)\n", ds, mem);
    fprintf(stdout, "SMDE Address:%p SMDE Eye-catcher %8.8s\n", smde, smde->smde_id);
  } else {
    struct smde_ext_attr* __ptr32 ext_attr = (struct smde_ext_attr*) (((char*) smde) + smde->smde_ext_attr_off);
    fprintf(stdout, "CCSID: 0x%x%x last change userid: %8.8s change timestamp: 0x%llx\n", 
      ext_attr->smde_ccsid[0], ext_attr->smde_ccsid[1], ext_attr->smde_userid_last_change, ext_attr->smde_change_timestamp);
  }

  /* Call FIND to find the start of the member */
  findcb = MALLOC31(sizeof(struct findcb));
  if (!findcb) {
    fprintf(stderr, "Unable to obtain storage for FIND macro\n");
    return 4;
  }
  *findcb = findcb_template;
  //findcb->mname_len = memlen;
  memcpy(findcb->mname, mem, memlen);

  rc = FIND(findcb, dcb);
  if (rc) {
    fprintf(stderr, "Unable to perform FIND. rc:%d\n", rc);
    return rc;
  }

  /* Establish DECB for call to READ */
  decb = MALLOC24(sizeof(struct decb));
  if (!decb) {
    fprintf(stderr, "Unable to obtain storage for READ decb\n");
    return 4;
  }
  block = MALLOC24(dcb->dcbblksi);
  if (!block) {
    fprintf(stderr, "Unable to obtain storage for READ block\n");
    return 4;
  }

  *decb = decb_template;
  SET_24BIT_PTR(decb->dcb24, dcb);
  decb->area = block;

  /* Read one block */
  rc = READ(decb);
  if (rc) {
    fprintf(stderr, "Unable to perform READ. rc:%d\n", rc);
    return rc;
  }

  rc = CHECK(decb);
  if (rc) {
    fprintf(stderr, "Unable to perform CHECK. rc:%d\n", rc);
    return rc;
  }

  fprintf(stdout, "Block read:%p (%d bytes)\n", block, dcb->dcbblksi);
  dumpstg(stdout, block, dcb->dcbblksi);
  fprintf(stdout, "\n");
 /*
   00016C                             2284 MEM_READ  DS  0H
 00016C D213 A058 C0A0 00058 002B8  2285          MVC   READ_DECB(READLEN),DECBMODW
                                    2286          READ READ_DECB,SF,LIB_DCB,READ_BUFFER,'S',MF=E
 000172 4110 A058            00058  2290+         LA    1,READ_DECB                       LOAD DECB ADDRESS      02-IHBRD
 000176 9280 1005      00005        2291+         MVI   5(1),X'80'               SET TYPE FIELD                  02-IHBRD
 00017A 41E0 8000            00000  2292+         LA    14,LIB_DCB                        LOAD DCB ADDRESS       02-IHBRD
 00017E 50E1 0008            00008  2293+         ST    14,8(1,0)                         STORE DCB ADDRESS      02-IHBRD
 000182 41E0 A06C            0006C  2294+         LA    14,READ_BUFFER      LOAD ADDR OF 64-BIT PTR         @L2C 02-IHBRD
 000186 50E1 000C            0000C  2295+         ST    14,12(1,0)          STORE ADDR OF 64-BIT PTR             02-IHBRD
 00018A 9280 1004      00004        2296+         MVI   4(1),X'80'                        SET TYPE FIELD         02-IHBRD
 00018E 58F0 1008            00008  2297+         L     15,8(,1)                     LOAD DCB ADDR          @01M 02-IHBRD
 000192 BFF7 F031            00031  2298+         ICM   15,B'0111',49(15)            LOAD RDWR ROUTINE ADDR @01M 02-IHBRD
 000196 05EF                        2299+         BALR  14,15                        LINK TO RDWR ROUTINE   @L1C 02-IHBRD
0000198                             2301 MEM_CHECK  DS 0H
                                    2302          CHECK READ_DECB           WAIT UNTIL COMPLETE
 000198 4110 A058            00058  2306+         LA    1,READ_DECB                       LOAD PARAMETER REG 1   02-IHBIN
 00019C 58E0 1008            00008  2307+         L     14,8(0,1)           PICK UP DCB ADDRESS                  01-CHECK
 0001A0 1BFF                        2308+         SR    15,15                                               @01A 01-CHECK
00001A6 0DEF                        2310+         BASR  14,15               CALL THE CHECK ROUTINE          @L2C 01-CHECK
.
.
                                    2457 READ    READ  DECBMODW,SF,0,0,'S',MF=L
 0002B8                             2461+READ     DS    0F                                                       02-IHBRD
 0002B8 00000000                    2462+DECBMODW DC    F'0'                              EVENT CONTROL BLOCK    02-IHBRD
 0002BC 80                          2463+         DC    X'80'                             TYPE FIELD             02-IHBRD
 0002BD 80                          2464+         DC    X'80'                             TYPE FIELD             02-IHBRD
 0002BE 0000                        2465+         DC    AL2(0)                            LENGTH                 02-IHBRD
 0002C0 00000000                    2466+         DC    A(0)                              DCB ADDRESS            02-IHBRD
 0002C4 00000000                    2467+         DC    A(0)                ADDRESS OF 64-BIT PTR           @L2A 02-IHBRD
 0002C8 00000000                    2468+         DC    A(0)                              RECORD POINTER WORD    02-IHBRD
                       00014        2469 READLEN EQU   *-DECBMODW
 */


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
