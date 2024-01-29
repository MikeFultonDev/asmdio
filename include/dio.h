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

  #pragma pack(full)
  struct dcb {
    int stuff;
  };
  #pragma pack(pop)

  #define SET_24BIT_PTR(ref,val) (ref) = (val)
#endif
