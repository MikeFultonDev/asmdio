all: basicread

clean:
	rm -f basicread *.o *.lst 

INCDIR=../include

basicread: basicread.o ../src/*.o
	$(LD) $(LDFLAGS) -o $@ $< ../src/*.o

basicread.o: basicread.c $(INCDIR)/dio.h
	$(CC) $(CFLAGS) $(CPPFLAGS) -I$(INCDIR) -c $<
