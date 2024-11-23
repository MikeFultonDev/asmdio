#include <stdio.h>
#include <stdlib.h>
#include "dio.h"

/*
 * BasicEnq gets the ENQ for dataset member SYS1.MACLIB(ADRDEX01) 
 */

static int pass(void) 
{

  char dsname[44+1] = "SYS1.MACLIB";
  char memname[8+1] = "ADRDEX01";
  char enqname[8+1] = "SPFEDIT ";
  char* __ptr32 qname;
  char* __ptr32 rname;
  unsigned int rname_len = sizeof(dsname)+sizeof(memname)-2 ; 

  qname = MALLOC31(sizeof(enqname));
  if (!qname) {
    fprintf(stderr, "Unable to obtain storage for ENQ\n");
    return 4;
  }
  rname = MALLOC31(sizeof(dsname)+sizeof(memname));
  if (!rname) {
    fprintf(stderr, "Unable to obtain storage for ENQ\n");
    return 4;
  }

  sprintf(qname, "%-8s", enqname);
  sprintf(rname, "%-44s%-8s", dsname, memname);
  printf("Call SYEXENQ <%s> <%s>\n", qname, rname);

  printf("parameters: %p %p %d\n", qname, rname, rname_len);
  int rc = SYEXENQ(qname, rname, rname_len);

  printf("Return code from SYEXENQ :%d\n", rc);
  return rc;
}

int main(int argc, char* argv[]) 
{
  return pass();
}
