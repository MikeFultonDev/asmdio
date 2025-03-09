#ifndef __FM_H__
  #define __FM_H__ 1

  #include <glob.h>
  #include <sys/stat.h>
  #include <stddef.h>
  #include "asmdiocommon.h"
  #include "dio.h"

  #define BINARY_CCSID 65535
  #define UNTAG_CCSID      0
  #define ISO8859_CCSID  819
  #define IBM1047_CCSID 1047

  #define ISO8859_NL    0x0A
  #define IBM1047_NL    0x15
  #define ISO8859_SPACE 0x20
  #define IBM1047_SPACE 0x40

  /*
   * msf: need to make this dynamic but for now, support write of text members up to 2**24 bytes (16MB)
   */
  #define MAX_TEXT_FILE_SIZE (1<<24) 
  
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
    char* PTR32 next_record_start;
    size_t next_record_len;
    size_t block_size;
    size_t bytes_used;
    unsigned int memstart_ttr;
    unsigned int pdsstart_ttr;
    int memstart_ttr_known:1;
    int pdsstart_ttr_known:1;
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
    char* file_buffer;
    size_t file_buffer_max;
    size_t file_buffer_cur;
    size_t line_num;
  } FM_FileHandle;

  typedef struct {
    char member[MEM_MAX+1];
    char* filename;
  } FM_MemFilePair;

#endif
