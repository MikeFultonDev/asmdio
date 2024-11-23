#ifndef __FM_H__
  #define __FM_H__ 1

  #include <glob.h>
  #include <sys/stat.h>
  #include <stddef.h>
  #include "asmdiocommon.h"
  #include "dio.h"

  typedef struct {
    int cur_value;
    char* values[0];
  } FM_FileTable;

  typedef struct {
    const char* key;
    FM_FileTable* table;
    int count;
  } FM_Entry;

  typedef struct {
    size_t max_size;
    size_t size;
    FM_Entry* entry;
  } FM_Table;

  typedef struct {
    char ddname[DD_MAX+1];
    struct ihadcb* PTR32 dcb;
    struct opencb* PTR32 opencb;
    struct decb* PTR32 decb;
    void* PTR32 block;
    size_t block_size;
    size_t bytes_used;
    unsigned int ttr;
    int ttr_known:1;
    size_t line_num;
  } FM_BPAMHandle;

  typedef struct {
    size_t record_offset;
    size_t record_length;
    size_t data_length;
    char* data;
  } FM_FileBuffer;

  #define REC_LEN REC_LEN_MAX

  typedef struct {
    struct file_tag tag;
    FM_FileBuffer active;
    FM_FileBuffer inactive;
    int fd;
    char newline_char;
    char space_char;

    char data_a[REC_LEN];
    char data_b[REC_LEN];
    size_t line_num;
  } FM_FileHandle;

  typedef struct {
    char member[MEM_MAX+1];
    char* filename;
  } FM_MemFilePair;

#endif
