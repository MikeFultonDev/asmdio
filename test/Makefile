all: basiccreate basicread basicalloc

clean:
	rm -f basicreate basicread basicalloc *.o *.lst

INCDIR=../include
TESTINCDIR=include

basicread: basicread.o ioservices.o ../src/*.o
	$(LD) $(LDFLAGS) -o $@ $< ioservices.o ../src/*.o

basiccreate: basiccreate.o ioservices.o ../src/*.o
	$(LD) $(LDFLAGS) -o $@ $< ioservices.o ../src/*.o

basicalloc: basicalloc.o ioservices.o ../src/*.o
	$(LD) $(LDFLAGS) -o $@ $< ioservices.o ../src/*.o

basicread.o: basicread.c $(TESTINCDIR)/ioservices.h $(INCDIR)/*.h
	$(CC) $(CFLAGS) $(CPPFLAGS) -I$(INCDIR) -I$(TESTINCDIR) -c $<

basiccreate.o: basiccreate.c $(TESTINCDIR)/ioservices.h $(INCDIR)/*.h
	$(CC) $(CFLAGS) $(CPPFLAGS) -I$(INCDIR) -I$(TESTINCDIR) -c $<

basicalloc.o: basicalloc.c $(TESTINCDIR)/ioservices.h $(INCDIR)/*.h
	$(CC) $(CFLAGS) $(CPPFLAGS) -I$(INCDIR) -I$(TESTINCDIR) -c $<

ioservices.o: ioservices.c $(TESTINCDIR)/ioservices.h $(INCDIR)/dio.h $(INCDIR)/s99.h
	$(CC) $(CFLAGS) $(CPPFLAGS) -I$(INCDIR) -I$(TESTINCDIR) -c $<

