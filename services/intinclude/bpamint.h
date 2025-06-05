  struct FM_BPAMHandle_InternalStruct {
    char ddname[DD_MAX+1];
    struct ihadcb* PTR32 dcb;
    struct opencb* PTR32 opencb;
    struct decb* PTR32 decb;
    void* PTR32 block;
    char* PTR32 next_record_start;
    size_t next_record_len;
    size_t block_size;
    size_t bytes_used;
    unsigned int memstart_ttr;
    unsigned int pdsstart_ttr;
    int memstart_ttr_known:1;
    int pdsstart_ttr_known:1;
    size_t line_num;
  };