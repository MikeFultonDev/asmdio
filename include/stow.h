#ifndef __STOW__
  #define __STOW__ 1

  #pragma pack(packed)

  struct stowlist_disc {
    unsigned short list_len;
    unsigned char type;
    unsigned short reserved;
    int dcb24: 24;
    /* more stuff */
  };

  struct stowlist_iff {
    unsigned short list_len;
    unsigned char type;
    unsigned short reserved;
    int dcb24: 24;
    char timestamp[8];
    void* __ptr32 direntry;
    char user_descriptor[16];
    unsigned short ccsid;
  };

  union stowlist {
    struct stowlist_iff iff;
    struct stowlist_disc disc;
  };

  enum stowtype {
    STOW_A=1,
    STOW_C,
    STOW_D,
    STOW_I,
    STOW_R,
    STOW_DISC,
    STOW_IFF,
    STOW_RG,
    STOW_DG,
    STOW_RECOVERG
  }; 
  #pragma pack(pop)

#endif


