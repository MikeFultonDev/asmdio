#include <ctype.h>
#include <stdio.h>
#include <string.h>

#include "asmdiocommon.h"
#include "util.h"

/**
 * @brief Convert a string to all capital letters.
 *
 * @param string The string to be converted.
 * @return int 0 on success; -1 on error.
 */
int uppercase(char *string) {
  
  if (!string) {
    return -1;
  }

  const size_t len = strlen(string);

  for (int i = 0; i < len; i++) {
    string[i] = toupper(string[i]);
  }

  return 0;
}
