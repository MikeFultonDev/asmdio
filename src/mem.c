#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mem.h"
#include "wrappers.h"

//#define DEBUG 1

#ifdef DEBUG
#define MEMPATTERN 0xFE
#else
#define MEMPATTERN 0x00
#endif

/**
 * @brief Allocate 24-bit memory.
 *
 * @param bytes Size of memory to allocate.
 * @return void* Pointer to 24-bit memory.
 */
void* PTR32 MALLOC24(unsigned int bytes)
{
  int ptr24;
  ptr24 = MALOC24A(bytes);
  void* PTR32 ptr = (void* PTR32) ptr24;
  if (ptr24 == 0) {
    fprintf(stderr, "Internal Error: Unable to allocate %d bytes below the bar\n", bytes);
  } else {
    memset(ptr, MEMPATTERN, bytes);
  }
  return ptr;
}

/**
 * @brief Free a pointer to 24-bit memory.
 *
 * @param addr Pointer to memory to free.
 * @param len Size of memory to free.
 * @return int Return code
 */
int FREE24(void* PTR32 addr, unsigned int len)
{
  return FREE24A(addr, len);
}

/**
 * @brief Allocate 31-bit memory
 *
 * @param bytes Size of memory to allocate.
 * @return void* Pointer to 31-bit memory.
 */
void* PTR32 MALLOC31(unsigned int bytes)
{
  void* PTR32 p = __malloc31(bytes);

  if (p == 0) {
    fprintf(stderr, "Internal Error: Unable to allocate %d bytes below the line\n", bytes);
  } else {
    memset(p, MEMPATTERN, bytes);
  }

  return p;
}

/**
 * @brief Free a pointer to 31-bit memory.
 *
 * @param addr Pointer to 31-bit memory.
 */
void FREE31(void* PTR32 addr)
{
  return free(addr);
}

/**
 * @brief
 *
 * @param stream
 * @param p
 * @param len
 */
void dumpstg(FILE* stream, void* p, size_t len)
{
  char* buff = p;
  size_t i;

  for (i=0; i<len; ++i) {
    if ((i != 0) && (i % 16 == 0)) {
      fprintf(stream, "\n");
    }
    if (i % 4 == 0) {
      fprintf(stream, " ");
    }
    fprintf(stream, "%2.2X", buff[i]);
  }
}
