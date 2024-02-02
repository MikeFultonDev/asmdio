all: basicread basicalloc

clean:
	rm -f basicread basicalloc *.o *.lst 

INCDIR=../include

basicread: basicread.o ../src/*.o
	$(LD) $(LDFLAGS) -o $@ $< ../src/*.o

basicalloc: basicalloc.o ../src/*.o
	$(LD) $(LDFLAGS) -o $@ $< ../src/*.o

basicread.o: basicread.c $(INCDIR)/dio.h $(INCDIR)/s99.h $(INCDIR)/ihadcb.h
	$(CC) $(CFLAGS) $(CPPFLAGS) -I$(INCDIR) -c $<

basicalloc.o: basicalloc.c $(INCDIR)/dio.h $(INCDIR)/s99.h
	$(CC) $(CFLAGS) $(CPPFLAGS) -I$(INCDIR) -c $<
