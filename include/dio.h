#ifndef __DIO_H__
  #define __DIO_H__ 1

  #include "opencb.h"
  #include "s99.h"
  #include "ihadcb.h"
  #include "decb.h"
  #include "stow.h"
  #include "closecb.h"

  #define SET_24BIT_PTR(ref,val) (ref) = ((int)(val))

  #define DD_SYSTEM "????????"
  #define DS_MAX (44)

  int OPEN(struct opencb* __ptr32 opencb);
  int WRITE(struct decb* __ptr32 decb);
  int CHECK(struct decb* __ptr32 decb);
  unsigned int NOTE(struct ihadcb* __ptr32 dcb);
  int STOW(union stowlist* __ptr32 list, struct ihadcb* __ptr32 dcb, enum stowtype type);
  int CLOSE(struct closecb* __ptr32 closecb);

  void* __ptr32 MALLOC24(size_t len);
  int FREE24(void* __ptr32 addr, size_t len);

  int S99(struct s99rb* __ptr32 s99rbp);
  int S99MSG(struct s99_em* __ptr32 s99em);

  void dumpstg(FILE* stream, void* p, size_t len);

  void* __ptr32 MALLOC31(size_t len);
  void FREE31(void* __ptr32 addr);

#endif
