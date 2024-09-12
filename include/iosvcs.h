#ifndef __IO_SERVICES__
  #define __IO_SERVICES__ 1

  #include "s99.h"

  /*
   * Error codes from dsd_alloc
   */
   #define IOSVC_ERR_NOERROR                  0
   #define IOSVC_ERR_SVC99INIT_ALLOC_FAILURE  4
   #define IOSVC_ERR_SVC99_ALLOC_FAILURE      8

  int dsdd_alloc(struct s99_common_text_unit* dsn, struct s99_common_text_unit* dd, struct s99_common_text_unit* disp);
  int ddfree(struct s99_common_text_unit* dd);
  int init_dsnam_text_unit(const char* dsname, struct s99_common_text_unit* dsn);
#endif
