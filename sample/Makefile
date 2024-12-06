csource = readupdate.c f2m.c msg.c filemap.c memdir.c ztime.c bpamio.c fmopts.c m2f.c mlsx.c mlsxopts.c mchtag.c
objects = $(csource:.c=.o) $(asource:.s=.o)$

all: f2m m2f mlsx mchtag readupdate

clean:
	rm -f f2m m2f mlsx mchtag readupdate *.o *.lst *.dbg

readupdate: readupdate.o msg.o bpamio.o ztime.o memdir.o ../src/*.o
	$(LD) $(LDFLAGS) -o $@ $< msg.o bpamio.o ztime.o memdir.o ../src/*.o >$@.lst

f2m: f2m.o msg.o fmopts.o filemap.o bpamio.o ztime.o ../src/*.o
	$(LD) $(LDFLAGS) -o $@ $< msg.o fmopts.o filemap.o bpamio.o ztime.o ../src/*.o >$@.lst

m2f: m2f.o
	$(LD) $(LDFLAGS) -o $@ $< >$@.lst

mlsx: mlsx.o mlsxopts.o bpamio.o memdir.o ztime.o
	$(LD) $(LDFLAGS) -o $@ $< msg.o mlsxopts.o memdir.o bpamio.o ztime.o ../src/*.o >$@.lst

mchtag: mchtag.o
	$(LD) $(LDFLAGS) -o $@ $< >$@.lst

%.d: %.c
	@set -e; rm -f $@; \
	clang -MM $(CPPFLAGS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

include $(csource:.c=.d)

readupdate.o: readupdate.c

f2m.o: f2m.c

msg.o: msg.c

filemap.o: filemap.c

memdir.o: memdir.c

ztime.o: ztime.c

bpamio.o: bpamio.c

fmopts.o: fmopts.c

m2f.o: m2f.c

mlsx.o: mlsx.c

mlsxopts.o: mlsxopts.c

mchtag.o: mchtag.c

