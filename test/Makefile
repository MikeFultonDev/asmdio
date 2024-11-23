all: basiccreate basicread basicrddir basicalloc basicenq

clean:
	rm -f basicreate basicread basicalloc basicrddir *.o *.lst *.dbg

INCDIR=../include

basicrddir: basicrddir.o ../src/*.o
	$(LD) $(LDFLAGS) -o $@ $< ../src/*.o >$@.lst

basicread: basicread.o ../src/*.o
	$(LD) $(LDFLAGS) -o $@ $< ../src/*.o >$@.lst

basiccreate: basiccreate.o ../src/*.o
	$(LD) $(LDFLAGS) -o $@ $< ../src/*.o >$@.lst

basicenq: basicenq.o ../src/*.o
	$(LD) $(LDFLAGS) -o $@ $< ../src/*.o

basicread.o: basicread.c $(TESTINCDIR)/ioservices.h $(INCDIR)/*.h
	$(CC) $(CFLAGS) $(CPPFLAGS) -I$(INCDIR) -I$(TESTINCDIR) -c $<

basicalloc: basicalloc.o ../src/*.o
	$(LD) $(LDFLAGS) -o $@ $< ../src/*.o >$@.lst

basicrddir.o: basicrddir.c $(INCDIR)/iosvcs.h $(INCDIR)/*.h
	$(CC) $(CFLAGS) $(CPPFLAGS) -I$(INCDIR) -c $<

basicread.o: basicread.c $(INCDIR)/iosvcs.h $(INCDIR)/*.h
	$(CC) $(CFLAGS) $(CPPFLAGS) -I$(INCDIR) -c $<

basicenq.o: basicenq.c $(INCDIR)/*.h
	$(CC) $(CFLAGS) $(CPPFLAGS) -I$(INCDIR) -I$(TESTINCDIR) -c $<

ioservices.o: ioservices.c $(TESTINCDIR)/ioservices.h $(INCDIR)/dio.h $(INCDIR)/s99.h
	$(CC) $(CFLAGS) $(CPPFLAGS) -I$(INCDIR) -I$(TESTINCDIR) -c $<

basiccreate.o: basiccreate.c $(INCDIR)/iosvcs.h $(INCDIR)/*.h
	$(CC) $(CFLAGS) $(CPPFLAGS) -I$(INCDIR) -c $<

basicalloc.o: basicalloc.c $(INCDIR)/iosvcs.h $(INCDIR)/*.h
	$(CC) $(CFLAGS) $(CPPFLAGS) -I$(INCDIR) -c $<