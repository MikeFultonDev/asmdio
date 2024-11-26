all: f2m m2f mlsx mchtag readupdate

clean:
	rm -f f2m m2f mlsx mchtag readupdate *.o *.lst *.dbg

INCDIR=../include

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

readupdate.o: readupdate.c $(INCDIR)/iosvcs.h $(INCDIR)/*.h
	$(CC) $(CFLAGS) $(CPPFLAGS) -I$(INCDIR) -c $<

f2m.o: f2m.c fm.h msg.h fmopts.h filemap.h bpamio.h dbgopts.h $(INCDIR)/*.h
	$(CC) $(CFLAGS) $(CPPFLAGS) -I$(INCDIR) -c $<

msg.o: msg.c msg.h fm.h fmopts.h dbgopts.h $(INCDIR)/*.h
	$(CC) $(CFLAGS) $(CPPFLAGS) -I$(INCDIR) -c $<

filemap.o: filemap.c msg.h fm.h fmopts.h $(INCDIR)/*.h
	$(CC) $(CFLAGS) $(CPPFLAGS) -I$(INCDIR) -c $<

memdir.o: memdir.c ztime.h dbgopts.h $(INCDIR)/*.h
	$(CC) $(CFLAGS) $(CPPFLAGS) -I$(INCDIR) -c $<

ztime.o: ztime.c ztime.h dbgopts.h
	$(CC) $(CFLAGS) $(CPPFLAGS) -I$(INCDIR) -c $<

bpamio.o: bpamio.c msg.h fm.h fmopts.h dbgopts.h bpamio.h $(INCDIR)/*.h
	$(CC) $(CFLAGS) $(CPPFLAGS) -I$(INCDIR) -c $<

fmopts.o: fmopts.c fm.h fmopts.h dbgopts.h $(INCDIR)/*.h
	$(CC) $(CFLAGS) $(CPPFLAGS) -I$(INCDIR) -c $<

m2f.o: m2f.c $(INCDIR)/*.h
	$(CC) $(CFLAGS) $(CPPFLAGS) -I$(INCDIR) -c $<

mlsx.o: mlsx.c memdir.h $(INCDIR)/*.h
	$(CC) $(CFLAGS) $(CPPFLAGS) -I$(INCDIR) -c $<

mlsxopts.o: mlsxopts.c mlsxopts.h dbgopts.h $(INCDIR)/*.h
	$(CC) $(CFLAGS) $(CPPFLAGS) -I$(INCDIR) -c $<

mchtag.o: mchtag.c $(INCDIR)/*.h
	$(CC) $(CFLAGS) $(CPPFLAGS) -I$(INCDIR) -c $<
