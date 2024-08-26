#ifndef __FINDCB__
  #define __FINDCB__ 1

  #pragma pack(packed)
  struct findcb {
    //unsigned short cb_len;
    //unsigned short filler;
    //unsigned int mname_len;
    char mname[8];
    //unsigned int gen_num;
  };

  #pragma pack(pop)

#endif
