#include "dio.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[]) {
  struct opencb opencb = { 1, 0, 0 };
  int opencb_len = sizeof(struct opencb);
  void* __ptr32 stg;
  int rc;

  stg = MALLOC24(opencb_len);
  if (stg == 0) {
    fprintf(stderr, "Unable to obtain %zu bytes of storage\n", opencb_len);
    return 4;
  }

  printf("Allocated %d bytes at %p\n", opencb_len, stg);
  SET_24BIT_PTR(opencb.dcb24, stg);
  
  rc = OPEN(&opencb);
  if (rc) {
    fprintf(stderr, "Unable to perform OPEN. rc:%d\n", rc);
    return rc;
  }

  CLOSE(&opencb);
  if (rc) {
    fprintf(stderr, "Unable to perform CLOSE. rc:%d\n", rc);
    return rc;
  }

  printf("Free %d bytes at %p\n", opencb_len, stg);
  rc = FREE24(stg, opencb_len);
  if (rc) {
    fprintf(stderr, "Unable to free storage. rc:%d\n", rc);
    return rc;
  }

  return 0;
}
