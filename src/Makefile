all: call31.o call31a.o dio.o s99.o dioa.o ihadcb.o

clean:
	rm -f *.o *.lst

IINCDIR=../intinclude
INCDIR=../include
MACDIR=../maclib

call31.o: call31.c $(IINCDIR)/wrappers.h
	$(CC) $(CFLAGS) $(CPPFLAGS) -I$(IINCDIR) -I$(INCDIR) -c $<

call31a.o: call31a.s
	$(AS) $(ASFLAGS) -I$(MACDIR) $< >$*.lst

dio.o: dio.c $(IINCDIR)/wrappers.h $(INCDIR)/*.h
	$(CC) $(CFLAGS) $(CPPFLAGS) -I$(INCDIR) -I$(IINCDIR) -c $<

s99.o: s99.c $(IINCDIR)/wrappers.h $(INCDIR)/dio.h $(INCDIR)/s99.h
	$(CC) $(CFLAGS) $(CPPFLAGS) -I$(INCDIR) -I$(IINCDIR) -c $<

ihadcb.o: ihadcb.c $(IINCDIR)/wrappers.h $(INCDIR)/dio.h $(INCDIR)/ihadcb.h
	$(CC) $(CFLAGS) $(CPPFLAGS) -I$(INCDIR) -I$(IINCDIR) -c $<

dioa.o: dioa.s
	$(AS) $(ASFLAGS) -I$(MACDIR) $< >$*.lst
