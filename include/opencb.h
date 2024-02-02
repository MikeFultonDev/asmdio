#ifndef __OPENCB__
  #define __OPENCB__ 1

  #pragma pack(full)
  struct opencb {
    int last_entry:1;
    int disp:3;
    int mode:4;
    int dcb24:24;
  };
  #pragma pack(pop)

#endif
