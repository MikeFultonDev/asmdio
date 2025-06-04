SRCDIR=src
DEPDIR=src/deps
BLDDIR=obj
ARDIR=lib
archive=bpamiosvcs.a

csource := $(shell find $(SRCDIR) -type f -name *.c)
cobjs   := $(patsubst $(SRCDIR)/%,$(BLDDIR)/%,$(csource:.c=.o))
objects := $(cobjs)
cdeps   := $(patsubst $(SRCDIR)/%,$(DEPDIR)/%,$(csource:.c=.d))
pgms    := $(BINDIR)/f2m $(BINDIR)/m2f $(BINDIR)/mlsx $(BINDIR)/mchtag $(BINDIR)/readupdate $(BINDIR)/readwriterec

all: $(ARDIR)/$(archive)

$(ARDIR)/$(archive): $(objects)
	mkdir -p $(ARDIR)
	ar -r $@ $^

clean:
	rm -rf $(BLDDIR) $(ARDIR) $(DEPDIR)

$(DEPDIR)/%.d: $(SRCDIR)/%.c
	mkdir -p $(DEPDIR) ; \
	rm -f $@; \
	clang -MM -MP $(SERVICES_CPPFLAGS) $< > $@;

include $(cdeps)

$(BLDDIR)/%.o: $(SRCDIR)/%.c
	mkdir -p $(BLDDIR) ;
	$(CC) $(SERVICES_CPPFLAGS) $(CFLAGS) -c -o $@ $<

.PHONY: all clean