#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include "ihadcb.h"
#include "dio.h"

const struct ihadcb dcb_template = { 0 };
struct ihadcb* __ptr32 dcb_init(const char* ddname)
{
  struct ihadcb* __ptr32 dcb;
  if (sizeof(struct ihadcb) != 0x58) {
    fprintf(stderr, "DCB should be 0x58 bytes in size but it is 0x%x bytes\n", sizeof(struct ihadcb));
    return NULL;
  }

  dcb = MALLOC24(sizeof(struct ihadcb));

  if (!dcb) {
    fprintf(stderr, "Unable to obtain storage for OPEN dcb\n");
    return dcb;
  }

  *dcb = dcb_template;

  if (ddname) {
    size_t ddname_len = strlen(ddname);
    if (ddname_len > 8 || ddname_len == 0) {
      fprintf(stderr, "Invalid ddname passed in of length: %u. Length must be from 1 to 8 characters\n", ddname_len);
      return NULL;
    }
    memset(&dcb->dcbddnam, ' ', sizeof(dcb->dcbddnam));
    memcpy(dcb->dcbddnam, ddname, ddname_len);
  }
  dcb->dcbbufcb.dcbbufca = DCB_ADDR24_UNSET; 
  dcb->dcbiobad.dcbicqe.dcbodeb.dcbodeba = DCB_ADDR24_UNSET; 
  dcb->dcbeodad.dcbeoda = DCB_ADDR24_UNSET; 
  dcb->dcbeobr.dcbeobra = DCB_ADDR24_UNSET; 
  dcb->dcbgerr.dcbperr.dcbcheck.dcbchcka = DCB_ADDR24_UNSET;
  dcb->dcbsynad.dcbsyna = DCB_ADDR24_UNSET; 
  dcb->dcbcicba = DCB_ADDR24_UNSET; 
  dcb->dcbeobw = DCB_ADDR_UNSET; 
  dcb->dcbpoint = DCB_ADDR_UNSET; 

  return dcb;
}

void dcb_free(struct ihadcb* __ptr32 dcb)
{
  FREE24(dcb, sizeof(struct ihadcb));
}

void dcb_fmt_dmp(FILE* stream, struct ihadcb* __ptr32 dcb)
{
  dumpstg(stream, dcb, sizeof(struct ihadcb)); 
}

