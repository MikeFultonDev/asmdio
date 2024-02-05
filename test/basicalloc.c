#include <stdio.h>
#include <stdlib.h>
#include "dio.h"
#include "s99.h"
#include "ioservices.h"

const struct s99_rbx s99rbxtemplate = {"S99RBX",S99RBXVR,{0,1,0,0,0,0,0},0,0,0};

#define PASSLIB "SYS1.MACLIB"

static int pass(void) 
{
	struct s99_common_text_unit dsn = { DALDSNAM, 1, sizeof(PASSLIB)-1, PASSLIB };
	struct s99_common_text_unit dd = { DALDDNAM, 1, 6, "DDPASS" };
	struct s99_common_text_unit stats = { DALSTATS, 1, 1, {0x8} };
  int rc;
  FILE* fp;
  char buffer[80];

	printf("Allocate DD:DDPASS to %s.\n", PASSLIB);
	rc = pdsdd_alloc(&dsn, &dd, &stats);

  if (rc) {
    return rc;
  }

  fp = fopen("//DD:DDPASS(DYNALLOC)", "rb,type=record");
  if (!fp) {
    perror("fopen");
    return 4;
  }

  rc = fread(buffer, sizeof(buffer), 1, fp);
  if (rc != 1) {
    perror("fread");
    return 8;
  }
  printf("First record of SYS1.MACLIB(DYNALLOC): <%80.80s>\n", buffer);

  rc = fclose(fp);
  if (rc) {
    perror("fclose");
    return 12;
  }

	printf("Free DD:DDPASS.\n");
	rc = ddfree(&dd);

  return 0;
}

int main(int argc, char* argv[]) 
{
  return pass();
}
