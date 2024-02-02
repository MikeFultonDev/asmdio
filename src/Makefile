all: call31.o call31a.o dio.o s99.o dioa.o

clean:
	rm -f *.o *.lst 

IINCDIR=../intinclude
INCDIR=../include
MACDIR=../maclib

call31.o: call31.c $(IINCDIR)/wrappers.h call31a.s
	$(CC) $(CFLAGS) $(CPPFLAGS) -I$(IINCDIR) -c $<

call31a.o: call31a.s
	$(AS) $(ASFLAGS) -I$(MACDIR) $< >$*.lst

dio.o: dio.c $(IINCDIR)/wrappers.h $(INCDIR)/dio.h $(INCDIR)/s99.h dioa.s
	$(CC) $(CFLAGS) $(CPPFLAGS) -I$(INCDIR) -I$(IINCDIR) -c $<

s99.o: s99.c $(IINCDIR)/wrappers.h $(INCDIR)/dio.h $(INCDIR)/s99.h dioa.s
	$(CC) $(CFLAGS) $(CPPFLAGS) -I$(INCDIR) -I$(IINCDIR) -c $<

dioa.o: dioa.s
	$(AS) $(ASFLAGS) -I$(MACDIR) $< >$*.lst
