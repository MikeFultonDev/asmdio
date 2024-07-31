#ifndef __STOW__
  #define __STOW__ 1

  #pragma pack(packed)

  struct stowlist_add {
    char mem_name[8];
    unsigned short tt;
    unsigned char r;
    unsigned char c;
    unsigned char user_data[62];
  };

  struct stowlist_generic_includes_dcb {
    unsigned short list_len;
    unsigned char type;
    unsigned short reserved;
    int dcb24: 24;
    /* more stuff */
  };

  struct stowlist_iff {
    unsigned short list_len;
    unsigned char type;
    unsigned char reserved;
    int dcbHOB: 8; 
    int dcb24: 24;
    char timestamp[8];
    void* __ptr32 direntry;
    char user_descriptor[16];
    unsigned short ccsid;
  };

  union stowlist {
    struct stowlist_generic_includes_dcb;
    struct stowlist_iff iff;
    struct stowlist_add add;
  };

  enum stowtype {
    STOW_A=1,
    STOW_C=2,
    STOW_D=3,
    STOW_I=4,
    STOW_R=5,
    STOW_DISC=0x80,
    STOW_IFF=0x40,
    STOW_RG=0x20,
    STOW_DG=0x10,
    STOW_RECOVERG=0x08
  }; 
  #pragma pack(pop)

#endif


