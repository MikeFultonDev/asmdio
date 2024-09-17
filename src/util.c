#include <ctype.h>
#include <stdio.h>
#include <string.h>

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

