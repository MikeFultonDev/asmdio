SRCDIR=src
DEPDIR=src/deps
BLDDIR=obj
BINDIR=bin
corear=../core/lib/bpamiocore.a
svcsar=../services/lib/bpamiosvcs.a

csource := $(shell find $(SRCDIR) -type f -name *.c)
cobjs   := $(patsubst $(SRCDIR)/%,$(BLDDIR)/%,$(csource:.c=.o))
objects := $(cobjs)
cdeps   := $(patsubst $(SRCDIR)/%,$(DEPDIR)/%,$(csource:.c=.d))
pgms    := $(BINDIR)/f2m $(BINDIR)/m2f $(BINDIR)/mlsx $(BINDIR)/mchtag $(BINDIR)/readupdate $(BINDIR)/readwriterec
all: $(pgms)

clean:
	rm -rf $(BLDDIR) $(BINDIR) $(DEPDIR)

$(DEPDIR)/%.d: $(SRCDIR)/%.c
	mkdir -p $(DEPDIR) ; \
	rm -f $@; \
	clang -MM -MP $(SAMPLE_CPPFLAGS) $< > $@;

include $(cdeps)

$(BINDIR)/readwriterec: $(BLDDIR)/readwriterec.o $(svcsar) $(corear)
	mkdir -p $(BINDIR)
	$(LD) $(LDFLAGS) -o $@ $^ >$@.llst

$(BINDIR)/readupdate: $(BLDDIR)/readupdate.o $(svcsar) $(corear)
	mkdir -p $(BINDIR)
	$(LD) $(LDFLAGS) -o $@ $^ >$@.llst

$(BINDIR)/f2m: $(BLDDIR)/f2m.o $(BLDDIR)/fsio.o $(BLDDIR)/fmopts.o $(BLDDIR)/filemap.o $(svcsar) $(corear)
	mkdir -p $(BINDIR)
	$(LD) $(LDFLAGS) -o $@ $^ >$@.llst

$(BINDIR)/mlsx: $(BLDDIR)/mlsx.o $(BLDDIR)/mlsxopts.o $(svcsar) $(corear)
	mkdir -p $(BINDIR)
	$(LD) $(LDFLAGS) -o $@ $^ >$@.llst

$(BINDIR)/mchtag: $(BLDDIR)/mchtag.o
	mkdir -p $(BINDIR)
	$(LD) $(LDFLAGS) -o $@ $< >$@.lst

$(BINDIR)/m2f: $(BLDDIR)/m2f.o $(BLDDIR)/fsio.o $(BLDDIR)/fmopts.o $(BLDDIR)/filemap.o $(svcsar) $(corear)
	mkdir -p $(BINDIR)
	$(LD) $(LDFLAGS) -o $@ $^ >$@.llst

$(BLDDIR)/%.o: $(SRCDIR)/%.c
	mkdir -p $(BLDDIR) ;
	$(CC) $(SAMPLE_CPPFLAGS) $(CFLAGS) -c -o $@ $<

.PHONY: all clean