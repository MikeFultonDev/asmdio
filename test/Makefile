all: basiccreate basicread basicrddir basicalloc basicenq basicmem

clean:
	rm -f basicreate basicread basicalloc basicrddir basicenq basicmem *.o *.lst *.dbg

INCDIR=../include

basicrddir: basicrddir.o ../src/*.o
	$(LD) $(LDFLAGS) -o $@ $< ../src/*.o >$@.lst

basicread: basicread.o ../src/*.o
	$(LD) $(LDFLAGS) -o $@ $< ../src/*.o >$@.lst

basiccreate: basiccreate.o ../src/*.o
	$(LD) $(LDFLAGS) -o $@ $< ../src/*.o >$@.lst

basicenq: basicenq.o ../src/*.o
	$(LD) $(LDFLAGS) -o $@ $< ../src/*.o >$@.lst

basicmem: basicmem.o ../src/*.o
	$(LD) $(LDFLAGS) -o $@ $< ../src/*.o >$@.lst

basicalloc: basicalloc.o ../src/*.o
	$(LD) $(LDFLAGS) -o $@ $< ../src/*.o >$@.lst

basicrddir.o: basicrddir.c $(INCDIR)/iosvcs.h $(INCDIR)/*.h
	$(CC) $(CFLAGS) $(CPPFLAGS) -I$(INCDIR) -c $<

basicread.o: basicread.c $(INCDIR)/iosvcs.h $(INCDIR)/*.h
	$(CC) $(CFLAGS) $(CPPFLAGS) -I$(INCDIR) -c $<

basicenq.o: basicenq.c $(INCDIR)/*.h
	$(CC) $(CFLAGS) $(CPPFLAGS) -I$(INCDIR) -I$(TESTINCDIR) -c $<

basicmem.o: basicmem.c $(INCDIR)/*.h
	$(CC) $(CFLAGS) $(CPPFLAGS) -I$(INCDIR) -I$(TESTINCDIR) -c $<

basiccreate.o: basiccreate.c $(INCDIR)/iosvcs.h $(INCDIR)/*.h
	$(CC) $(CFLAGS) $(CPPFLAGS) -I$(INCDIR) -c $<

basicalloc.o: basicalloc.c $(INCDIR)/iosvcs.h $(INCDIR)/*.h
	$(CC) $(CFLAGS) $(CPPFLAGS) -I$(INCDIR) -c $<
