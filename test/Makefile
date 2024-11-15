all: basiccreate basicread basicrddir basicrdpds basicalloc

clean:
	rm -f basicreate basicread basicalloc basicrdpds basicrddir *.o *.lst *.dbg

INCDIR=../include

basicrdpds: basicrdpds.o 
	$(LD) $(LDFLAGS) -o $@ $< >$@.lst

basicrddir: basicrddir.o ../src/*.o
	$(LD) $(LDFLAGS) -o $@ $< ../src/*.o >$@.lst

basicread: basicread.o ../src/*.o
	$(LD) $(LDFLAGS) -o $@ $< ../src/*.o >$@.lst

basiccreate: basiccreate.o ../src/*.o
	$(LD) $(LDFLAGS) -o $@ $< ../src/*.o >$@.lst

basicalloc: basicalloc.o ../src/*.o
	$(LD) $(LDFLAGS) -o $@ $< ../src/*.o >$@.lst

basicrdpds.o: basicrdpds.c $(INCDIR)/basicrdpds.h
	$(CC) $(CFLAGS) $(CPPFLAGS) -I$(INCDIR) -c $<

basicrddir.o: basicrddir.c $(INCDIR)/iosvcs.h $(INCDIR)/*.h
	$(CC) $(CFLAGS) $(CPPFLAGS) -I$(INCDIR) -c $<

basicread.o: basicread.c $(INCDIR)/iosvcs.h $(INCDIR)/*.h
	$(CC) $(CFLAGS) $(CPPFLAGS) -I$(INCDIR) -c $<

basiccreate.o: basiccreate.c $(INCDIR)/iosvcs.h $(INCDIR)/*.h
	$(CC) $(CFLAGS) $(CPPFLAGS) -I$(INCDIR) -c $<

basicalloc.o: basicalloc.c $(INCDIR)/iosvcs.h $(INCDIR)/*.h
	$(CC) $(CFLAGS) $(CPPFLAGS) -I$(INCDIR) -c $<

