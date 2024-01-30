all: call31.o call31a.o dio.o dioa.o

clean:
	rm -f *.o *.lst 

IINCDIR=../intinclude
INCDIR=../include
MACDIR=../maclib

call31.o: call31.c $(IINCDIR)/wrappers.h
	$(CC) $(CFLAGS) $(CPPFLAGS) -I$(IINCDIR) -c $<

call31a.o: call31a.s $(IINCDIR)/wrappers.h
	$(AS) $(ASFLAGS) -I$(MACDIR) $< >$*.lst

dio.o: dio.c $(IINCDIR)/wrappers.h $(INCDIR)/dio.h
	$(CC) $(CFLAGS) $(CPPFLAGS) -I$(INCDIR) -I$(IINCDIR) -c $<

dioa.o: dioa.s $(IINCDIR)/wrappers.h $(INCDIR)/dio.h
	$(AS) $(ASFLAGS) -I$(MACDIR) $< >$*.lst
