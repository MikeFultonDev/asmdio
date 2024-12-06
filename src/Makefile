csource = call31.c util.c dio.c mem.c s99.c iosvcs.c ihadcb.c
asource = dioa.s mema.s call31a.s
objects = $(csource:.c=.o) $(asource:.s=.o)

all: $(objects)

clean:
	rm -f *.o *.lst *.dbg

MACDIR=../maclib

%.d: %.c
	@set -e; rm -f $@; \
	clang -MM $(CPPFLAGS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

include $(csource:.c=.d)

call31.o: call31.c

util.o: util.c

dio.o: dio.c

mem.o: mem.c

s99.o: s99.c

iosvcs.o: iosvcs.c

ihadcb.o: ihadcb.c

dioa.o: dioa.s
	$(AS) $(ASFLAGS) -I$(MACDIR) $< >$*.lst

mema.o: mema.s
	$(AS) $(ASFLAGS) -I$(MACDIR) $< >$*.lst

call31a.o: call31a.s
	$(AS) $(ASFLAGS) -I$(MACDIR) $< >$*.lst

