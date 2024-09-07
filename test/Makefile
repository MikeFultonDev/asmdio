all: basiccreate basicread basicalloc

clean:
	rm -f basicreate basicread basicalloc *.o *.lst

INCDIR=../include

basicread: basicread.o ../src/*.o
	$(LD) $(LDFLAGS) -o $@ $< ../src/*.o

basiccreate: basiccreate.o ../src/*.o
	$(LD) $(LDFLAGS) -o $@ $< ../src/*.o

basicalloc: basicalloc.o ../src/*.o
	$(LD) $(LDFLAGS) -o $@ $< ../src/*.o

basicread.o: basicread.c $(INCDIR)/ioservices.h $(INCDIR)/*.h
	$(CC) $(CFLAGS) $(CPPFLAGS) -I$(INCDIR) -c $<

basiccreate.o: basiccreate.c $(INCDIR)/ioservices.h $(INCDIR)/*.h
	$(CC) $(CFLAGS) $(CPPFLAGS) -I$(INCDIR) -c $<

basicalloc.o: basicalloc.c $(INCDIR)/ioservices.h $(INCDIR)/*.h
	$(CC) $(CFLAGS) $(CPPFLAGS) -I$(INCDIR) -c $<

