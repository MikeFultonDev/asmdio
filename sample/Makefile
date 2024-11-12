all: f2m m2f mlsx mchtag

clean:
	rm -f f2m m2f mlsx mchtag *.o *.lst *.dbg

INCDIR=../include

f2m: f2m.o msg.o fmopts.o filemap.o bpamio.o ../src/*.o
	$(LD) $(LDFLAGS) -o $@ $< msg.o fmopts.o filemap.o bpamio.o ../src/*.o >$@.lst

m2f: m2f.o
	$(LD) $(LDFLAGS) -o $@ $< >$@.lst

mlsx: mlsx.o mlsxopts.o ../src/memdir.o
	$(LD) $(LDFLAGS) -o $@ $< mlsxopts.o ../src/memdir.o >$@.lst

mchtag: mchtag.o
	$(LD) $(LDFLAGS) -o $@ $< >$@.lst

f2m.o: f2m.c fm.h msg.h fmopts.h filemap.h bpamio.h $(INCDIR)/*.h
	$(CC) $(CFLAGS) $(CPPFLAGS) -I$(INCDIR) -c $<

msg.o: msg.c msg.h fm.h fmopts.h $(INCDIR)/*.h
	$(CC) $(CFLAGS) $(CPPFLAGS) -I$(INCDIR) -c $<

filemap.o: filemap.c msg.h fm.h fmopts.h $(INCDIR)/*.h
	$(CC) $(CFLAGS) $(CPPFLAGS) -I$(INCDIR) -c $<

bpamio.o: bpamio.c msg.h fm.h fmopts.h bpamio.h $(INCDIR)/*.h
	$(CC) $(CFLAGS) $(CPPFLAGS) -I$(INCDIR) -c $<

fmopts.o: fmopts.c fm.h fmopts.h dbgopts.h $(INCDIR)/*.h
	$(CC) $(CFLAGS) $(CPPFLAGS) -I$(INCDIR) -c $<

m2f.o: m2f.c $(INCDIR)/*.h
	$(CC) $(CFLAGS) $(CPPFLAGS) -I$(INCDIR) -c $<

mlsx.o: mlsx.c $(INCDIR)/*.h
	$(CC) $(CFLAGS) $(CPPFLAGS) -I$(INCDIR) -c $<

mlsxopts.o: mlsxopts.c mlsxopts.h dbgopts.h $(INCDIR)/*.h
	$(CC) $(CFLAGS) $(CPPFLAGS) -I$(INCDIR) -c $<

mchtag.o: mchtag.c $(INCDIR)/*.h
	$(CC) $(CFLAGS) $(CPPFLAGS) -I$(INCDIR) -c $<
