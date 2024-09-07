all: f2m m2f mchtag

clean:
	rm -f f2m m2f mchtag *.o *.lst

INCDIR=../include

f2m: f2m.o
	$(LD) $(LDFLAGS) -o $@ $<

m2f: m2f.o
	$(LD) $(LDFLAGS) -o $@ $<

mchtag: mchtag.o
	$(LD) $(LDFLAGS) -o $@ $<

f2m.o: f2m.c $(INCDIR)/*.h
	$(CC) $(CFLAGS) $(CPPFLAGS) -I$(INCDIR) -c $<

m2f.o: m2f.c $(INCDIR)/*.h
	$(CC) $(CFLAGS) $(CPPFLAGS) -I$(INCDIR) -c $<

mchtag.o: mchtag.c $(INCDIR)/*.h
	$(CC) $(CFLAGS) $(CPPFLAGS) -I$(INCDIR) -c $<
