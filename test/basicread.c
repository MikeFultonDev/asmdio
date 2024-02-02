#include <stdio.h>
#include <stdlib.h>
#include "ihadcb.h"
#include "s99.h"
#include "dio.h"

const struct s99_rbx s99rbxtemplate = {"S99RBX",S99RBXVR,{0,1,0,0,0,0,0},0,0,0};

static int pdsdd_alloc(struct s99_common_text_unit* dsn, struct s99_common_text_unit* dd, struct s99_common_text_unit* disp)
{
  struct s99rb* __ptr32 parms;
  enum s99_verb verb = S99VRBAL;
  struct s99_flag1 s99flag1 = {0};
  struct s99_flag2 s99flag2 = {0};
  size_t num_text_units = 3;
  int rc;
  struct s99_rbx s99rbx = s99rbxtemplate;

  parms = s99_init(verb, s99flag1, s99flag2, &s99rbx, num_text_units, dsn, dd, disp );
  if (!parms) {
    fprintf(stderr, "Unable to initialize SVC99 (DYNALLOC) control blocks\n");
    return 16;
  }
  rc = S99(parms);
  if (rc) {
    fprintf(stderr, "SVC99 failed with rc:%d\n", rc);
    s99_fmt_dmp(stderr, parms);
    s99_prt_msg(stderr, parms, rc);
    return(rc);
  }

  s99_free(parms);
  return 0;
}

static int ddfree(struct s99_common_text_unit* dd)
{
  struct s99rb* __ptr32 parms;
  enum s99_verb verb = S99VRBUN;
  struct s99_flag1 s99flag1 = {0};
  struct s99_flag2 s99flag2 = {0};
  size_t num_text_units = 1;
  int rc;
  struct s99_rbx s99rbx = s99rbxtemplate;

  parms = s99_init(verb, s99flag1, s99flag2, &s99rbx, num_text_units, dd );
  if (!parms) {
    fprintf(stderr, "Unable to initialize SVC99 (DYNFREE) control blocks\n");
    return 16;
  }
  rc = S99(parms);
  if (rc) {
    s99_fmt_dmp(stderr, parms);
    s99_prt_msg(stderr, parms, rc);
    return rc;
  }

  s99_free(parms);
  return 0;
}

#define DS_MACLIB "SYS1.MACLIB"
#define DD_MACLIB "DDMAC"

/*
 * Basic Read of a PDS Member:
 * - Allocate DDName to PDS
 * - Establish DCB for DDName
 * - Perform OPEN on PDS
 * - Perform FIND on member
 * - Read records of member
 * - Close DCB
 * - Free DDName 
 */
const struct opencb opencb_template = { 1, 0, 0 };
int main(int argc, char* argv[]) {
  struct opencb* __ptr32 opencb;
  struct ihadcb* __ptr32 dcb24;
  int rc;

  struct s99_common_text_unit dsn = { DALDSNAM, 1, sizeof(DS_MACLIB)-1, DS_MACLIB };
  struct s99_common_text_unit dd = { DALDDNAM, 1, sizeof(DD_MACLIB)-1, DD_MACLIB };
  struct s99_common_text_unit stats = { DALSTATS, 1, 1, {0x8} };

  rc = pdsdd_alloc(&dsn, &dd, &stats);
  if (rc) {
    return 4;
  }

  opencb = MALLOC31(sizeof(struct opencb));
  if (!opencb) {
    fprintf(stderr, "Unable to obtain storage for OPEN cb\n");
    return 4;
  }
  dcb24 = MALLOC24(sizeof(struct ihadcb));
  if (!dcb24) {
    fprintf(stderr, "Unable to obtain storage for OPEN dcb\n");
    return 4;
  }

  *opencb = opencb_template;
  SET_24BIT_PTR(opencb->dcb24, dcb24);
  
  rc = OPEN(opencb);
  if (rc) {
    fprintf(stderr, "Unable to perform OPEN. rc:%d\n", rc);
    return rc;
  }

  CLOSE(opencb);
  if (rc) {
    fprintf(stderr, "Unable to perform CLOSE. rc:%d\n", rc);
    return rc;
  }

  rc = FREE24(dcb24, sizeof(struct ihadcb));
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
