#ifndef __DIO_H__
  #define __DIO_H__ 1

  #pragma pack(full)
  struct opencb {
    int last_entry:1;
    int disp:3;
    int mode:4;
    int dcb24:24;
  };
  #pragma pack(pop)

  #include "s99.h"

  #define SET_24BIT_PTR(ref,val) (ref) = ((int)(val))

  int OPEN(struct opencb* __ptr32 opencb);
  int CLOSE(struct opencb* __ptr32 opencb);
  void* __ptr32 MALLOC24(int len);
  int FREE24(void* __ptr32 addr, int len);

  int SVC99(struct s99rb* __ptr32 s99rbp);

#endif
