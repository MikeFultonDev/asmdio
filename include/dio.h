#ifndef __DIO_H__
#define __DIO_H__ 1

#include "asmdiocommon.h"

#include "closecb.h"
#include "decb.h"
#include "deserv.h"
#include "findcb.h"
#include "ihadcb.h"
#include "opencb.h"
#include "s99.h"
#include "smde.h"
#include "stow.h"

#define SET_24BIT_PTR(ref,val) (ref) = ((int)(val))

#define DD_SYSTEM "????????"
#define DS_MAX (44)

#define MEM_MAX (8)
#define DD_MAX (8)
#define REC_LEN_MAX (32760)

int OPEN(struct opencb* PTR32 opencb);
int FIND(struct findcb* PTR32 findcb, struct ihadcb* PTR32 dcb);
int READ(struct decb* PTR32 decb);
int WRITE(struct decb* PTR32 decb);
int CHECK(struct decb* PTR32 decb);
unsigned int NOTE(struct ihadcb* PTR32 dcb);
unsigned int DESERV(struct desp* PTR32 desp);
int STOW(union stowlist* PTR32 list, struct ihadcb* PTR32 dcb, enum stowtype type);
int CLOSE(struct closecb* PTR32 closecb);
int SYEXENQ(char* __ptr32 qname, char* __ptr32 rname, unsigned int rname_len);
int SYEXDEQ(char* __ptr32 qname, char* __ptr32 rname, unsigned int rname_len);

#pragma map(OPEN, "DOPEN")
#pragma map(FIND, "DFIND")
#pragma map(READ, "DREAD")
#pragma map(WRITE, "DWRITE")
#pragma map(CHECK, "DCHECK")
#pragma map(NOTE, "DNOTE")
#pragma map(STOW, "DSTOW")
#pragma map(CLOSE, "DCLOSE")

void* PTR32 MALLOC24(size_t len);
int FREE24(void* PTR32 addr, size_t len);

int S99(struct s99rb* PTR32 s99rbp);
int S99MSG(struct s99_em* PTR32 s99em);

void dumpstg(FILE* stream, void* p, size_t len);

void* PTR32 MALLOC31(size_t len);
void FREE31(void* PTR32 addr);

#endif
