#include "dio.h"
#include "s99.h"
#include <stdio.h>
#include <stdlib.h>

const struct s99_rbx s99rbxtemplate = {"S99RBX",S99RBXVR,{0,1,0,0,0,0,0},0,0,0};

#define PASSLIB "SYS1.MACLIB"

#pragma noinline(ddalloc)
static int ddalloc(struct s99_common_text_unit* dsn, struct s99_common_text_unit* dd, struct s99_common_text_unit* disp)
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
  } else {
    fprintf(stdout, "SVC99 Succeeded. Dump of storage\n");
    s99_fmt_dmp(stdout, parms);
  }

	s99_free(parms);
	return 0;
}

#pragma noinline(ddfree)
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
		fprintf(stderr, "Unable to initialize SVC99 (DYNFFREE) control blocks\n");
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

static int pass(void) 
{
	struct s99_common_text_unit dsn = { DALDSNAM, 1, sizeof(PASSLIB)-1, PASSLIB };
	struct s99_common_text_unit dd = { DALDDNAM, 1, 6, "DDPASS" };
	struct s99_common_text_unit stats = { DALSTATS, 1, 1, {0x8} };
  int rc;
  FILE* fp;
  char buffer[80];

	printf("Allocate DD:DDPASS to %s.\n", PASSLIB);
	rc = ddalloc(&dsn, &dd, &stats);

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
