all: basicread

INCDIR=../include

basicread: basicread.o
	$(LD) $(LDFLAGS) -o $@ $< ../src/*.o

basicread.o: basicread.c $(INCDIR)/dio.h
	$(CC) $(CFLAGS) $(CPPFLAGS) -I$(INCDIR) -c $<
