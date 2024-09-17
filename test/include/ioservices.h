#ifndef __IO_SERVICES__
  #define __IO_SERVICES__ 1

  #include "s99.h"

  int pdsdd_alloc(struct s99_common_text_unit* dsn, struct s99_common_text_unit* dd, struct s99_common_text_unit* disp);
  int ddfree(struct s99_common_text_unit* dd);
  int init_dsnam_text_unit(const char* dsname, struct s99_common_text_unit* dsn);
#endif
