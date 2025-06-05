#include <stdio.h>
#include <stdlib.h>
#include "mem.h"

/*
 * basicmem tests malloc/free to ensure the addresses coming back are good
 */

static int pass(void) 
{

  for (int i=0; i<1000; ++i) {
    char* below_bar = MALLOC31(250);
    char* below_line = MALLOC24(200);

    if (!below_line || !below_bar) {
      fprintf(stderr, "Unable to allocate storage on %d invocation\n", i);
      return 4;
    }

    unsigned int below_bar_int = (unsigned int) below_bar;
    if (below_bar_int > 0x7FFFFFFF) {
      fprintf(stderr, "Below bar storage too high:%p\n", below_bar);
      return 4;
    }
    unsigned int below_line_int = (unsigned int) below_line;
    if (below_line_int > (1 << 24)) {
      fprintf(stderr, "Below line storage too high:%p\n", below_line);
      return 4;
    }

    FREE31(below_bar);
    FREE24(below_line, 200);
  }
  return 0;
}

int main(int argc, char* argv[]) 
{
  return pass();
}
