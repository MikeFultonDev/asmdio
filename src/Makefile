all: call31.o call31a.o dio.o mem.o s99.o dioa.o mema.o ihadcb.o util.o iosvcs.o

clean:
	rm -f *.o *.lst *.dbg

IINCDIR=../intinclude
INCDIR=../include
MACDIR=../maclib

call31.o: call31.c $(IINCDIR)/wrappers.h
	$(CC) $(CFLAGS) $(CPPFLAGS) -I$(IINCDIR) -I$(INCDIR) -c $<

util.o: util.c $(INCDIR)/util.h
	$(CC) $(CFLAGS) $(CPPFLAGS) -I$(IINCDIR) -I$(INCDIR) -c $<

call31a.o: call31a.s
	$(AS) $(ASFLAGS) -I$(MACDIR) $< >$*.lst

dio.o: dio.c $(IINCDIR)/wrappers.h $(INCDIR)/*.h
	$(CC) $(CFLAGS) $(CPPFLAGS) -I$(INCDIR) -I$(IINCDIR) -c $<

mem.o: mem.c $(IINCDIR)/wrappers.h $(INCDIR)/*.h
	$(CC) $(CFLAGS) $(CPPFLAGS) -I$(INCDIR) -I$(IINCDIR) -c $<

s99.o: s99.c $(IINCDIR)/wrappers.h $(INCDIR)/dio.h $(INCDIR)/s99.h
	$(CC) $(CFLAGS) $(CPPFLAGS) -I$(INCDIR) -I$(IINCDIR) -c $<

iosvcs.o: iosvcs.c $(INCDIR)/iosvcs.h $(INCDIR)/dio.h $(INCDIR)/s99.h
	$(CC) $(CFLAGS) $(CPPFLAGS) -I$(INCDIR) -c $<

memdir.o: memdir.c $(INCDIR)/memdir.h
	$(CC) $(CFLAGS) $(CPPFLAGS) -I$(INCDIR) -c $<

ztime.o: ztime.c $(INCDIR)/ztime.h
	$(CC) $(CFLAGS) $(CPPFLAGS) -I$(INCDIR) -c $<

ihadcb.o: ihadcb.c $(IINCDIR)/wrappers.h $(INCDIR)/dio.h $(INCDIR)/ihadcb.h
	$(CC) $(CFLAGS) $(CPPFLAGS) -I$(INCDIR) -I$(IINCDIR) -c $<

dioa.o: dioa.s
	$(AS) $(ASFLAGS) -I$(MACDIR) $< >$*.lst

mema.o: mema.s
	$(AS) $(ASFLAGS) -I$(MACDIR) $< >$*.lst
