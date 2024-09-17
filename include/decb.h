#ifndef __DECB__
#define __DECB__ 1

#pragma pack(packed)
struct decb {
  unsigned int ecb;
  unsigned short type;
  unsigned short length;
  unsigned int dcb24_hoB: 8;
  unsigned int dcb24: 24;
  void* __ptr32 area;
  void* __ptr32 stat_addr;
  void* __ptr32 key_addr;
  void* __ptr32 block_addr;
  void* __ptr32 next_addr;
};
#pragma pack(pop)

#endif
