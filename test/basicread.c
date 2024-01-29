#include "dio.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[]) {
  struct opencb opencb = { 1, 0, 0 };
  int dcb_len = sizeof(struct dcb);
  int stg = MALLOC24(dcb_len);
  int rc;

  if (stg == 0) {
    fprintf(stderr, "Unable to obtain %zu bytes of storage\n", dcb_len);
    return 4;
  }
  SET_24BIT_PTR(opencb.dcb24, stg);
  
  rc = OPENA(&opencb);
  if (rc) {
    fprintf(stderr, "Unable to perform OPEN. rc:%d\n", rc);
    return rc;
  }

  CLOSEA(&opencb);
  if (rc) {
    fprintf(stderr, "Unable to perform CLOSE. rc:%d\n", rc);
    return rc;
  }

  rc = FREE24(stg, dcb_len);
  if (rc) {
    fprintf(stderr, "Unable to free storage. rc:%d\n", rc);
    return rc;
  }

  return 0;
}
