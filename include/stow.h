#ifndef __STOW__
  #define __STOW__ 1

  #pragma pack(packed)

  struct stow_list_disc {
    unsigned short list_len;
    unsigned char type;
    unsigned short reserved;
    int dcb24: 24;
    /* more stuff */
  };

  struct stow_list_iff {
    unsigned short list_len;
    unsigned char type;
    unsigned short reserved;
    int dcb24: 24;
    char timestamp[8];
    void* __ptr32 direntry;
    char user_descriptor[16];
    unsigned short ccsid;
  };

  #pragma pack(pop)

#endif


