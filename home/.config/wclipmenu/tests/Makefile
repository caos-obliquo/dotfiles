CC      ?= cc
CFLAGS  ?= -std=c11 -Wall -Wextra -O2 -g
SRCDIR  := src
BINDIR  := bin
UTIL    := $(SRCDIR)/util.c
TESTS   := test_daemon test_text test_search test_reverse test_image test_picker
BINS    := $(addprefix $(BINDIR)/,$(TESTS))
all: $(BINS)
$(BINDIR)/%: $(SRCDIR)/%.c $(SRCDIR)/test.h $(SRCDIR)/util.h $(UTIL)
	@mkdir -p $(BINDIR)
	$(CC) $(CFLAGS) -o $@ $< $(UTIL)
test: all
	@set -e; for t in $(TESTS); do echo "=== $$t ==="; $(BINDIR)/$$t; done; echo "ALL TESTS PASSED"
clean:
	rm -rf $(BINDIR)
.PHONY: all test clean
