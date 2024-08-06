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
const struct desp desp_template = { { "IGWDESP ", sizeof(struct desp), 0, 1, 0 } };

int main(int argc, char* argv[]) {
  struct opencb* __ptr32 opencb;
  struct closecb* __ptr32 closecb;
  struct ihadcb* __ptr32 dcb;
  struct desp* __ptr32 desp;
  int rc;
  char* mem;
  size_t memlen;

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

  mem = argv[2]);
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
   * DESERV GET BYPASS_LLA LIBTYPE DCB CONN_INTENT HOLD
   */
  *desp = desp_template;
  desp->desp_func = desp_func_get;
  desp->desp_bypass_lla = 1;
  desp->desp_ext_attr = 1; 
  desp->desp_libtype = desp_libtype_dcb;
  desp->desp_gettype = desp_gettype_name_list;
  desp->desp_conn_intent = desp_conn_intent_hold;

  /* set up DESL list of 1 entry for member to GET */

  desp->desl = desl;
  desp->desp_de_list2 = 1;
/*
0                                   2232 *
                                    2233 * Copy the DESP template into DESP_AREA
                                    2234 * Set up the MEM_NAME information into MEM_NAME_LIST_DESL
                                    2235 *
 0000A8                             2236 MEM_DESERV  DS 0H
 0000A8 D267 A203 C0B4 00203 0028C  2237          MVC DESP_AREA(DESERV_LEN),CONST_DESP
 0000AE 4160 0006	     00006  2238          LA  R6,DESERV_NAME_LEN
 0000B2 4060 A27E	     0027E  2239          STH R6,MEM_NAME_LEN
 0000B6 D205 A280 C11C 00280 002F4  2240          MVC MEM_NAME_VAL(DESERV_NAME_LEN),DESERV_NAME
 0000BC 4170 A26E	     0026E  2241          LA  R7,MEM_NAME_LIST_DESL
                  R:7  00000        2242          USING DESL,R7
 0000C0 D70F 7000 7000 00000 00000  2243          XC  DESL_ENTRY,DESL_ENTRY
 0000C6 4160 A27E	     0027E  2244          LA  R6,MEM_NAME
 0000CA 5060 700C	     0000C  2245          ST  R6,DESL_NAME_PTR
                                    2246          DROP R7
0                                   2248          DESERV FUNC=GET,CONN_INTENT=HOLD,DCB=LIB_DCB,			+
                                                        NAME_LIST=(MEM_NAME_LIST_DESL,DESERV_NAME_COUNT),	+
                                                        AREA=(DESERV_AREA,DESERV_AREA_LEN),			+
                                                        MF=(E,DESP_AREA)
 0000CE 4110 A203	     00203  2253+         LA    1,DESP_AREA                                              01-DESER
 0000D2 D757 1010 1010 00010 00010  2254+         XC    DESP_FUNC-DESP(DESP_LEN_LIST,1),DESP_FUNC-DESP(1)	 01-DESER
 0000D8 9201 1010      00010        2255+         MVI   DESP_FUNC-DESP(1),DESP_FUNC_GET                		 01-DESER
 0000DC 9601 1018      00018        2256+         OI    DESP_FLAGS-DESP(1),B'00000001'                      @L2C 01-DESER
 0000E0 9202 101C      0001C	    2257+         MVI   DESP_LIBTYPE-DESP(1),DESP_LIBTYPE_DCB                    01-DESER
 0000E4 9201 101D      0001D        2258+         MVI   DESP_GETTYPE-DESP(1),DESP_GETTYPE_NAME_LIST		 01-DESER
 0000E8 9201 1022      00022        2259+         MVI   DESP_CONN_INTENT-DESP(1),DESP_CONN_INTENT_HOLD		 01-DESER

  *
  * got to here
  *
 0000EC 41F0 8000            00000  2260+         LA    15,LIB_DCB                              		 01-DESER
 0000F0 50F0 1024            00024  2261+         ST    15,DESP_DCB_PTR-DESP(0,1)                                01-DESER
 0000F4 41F0 A0BC            000BC  2262+         LA    15,DESERV_AREA		                                 01-DESER
 0000F8 50F0 1034            00034  2263+         ST    15,DESP_AREA_PTR-DESP(0,1)                     		 01-DESER
 0000FC 41F0 0147            00147  2264+         LA    15,DESERV_AREA_LEN	                                 01-DESER
 000100 50F0 1038            00038  2265+         ST    15,DESP_AREA2-DESP(0,1)                                  01-DESER
 000104 41F0 A26E            0026E  2266+         LA    15,MEM_NAME_LIST_DESL                                    01-DESER
 000108 50F0 1050            00050  2267+         ST    15,DESP_NAME_LIST_PTR-DESP(0,1)                          01-DESER
 00010C 41F0 0001            00001  2268+         LA    15,DESERV_NAME_COUNT                                     01-DESER
 000110 50F0 1054            00054  2269+         ST    15,DESP_NAME_LIST2-DESP(0,1)                             01-DESER
 */

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
