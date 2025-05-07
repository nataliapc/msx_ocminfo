.PHONY: all clean clean_h

ROOTDIR = ..
BINDIR = $(ROOTDIR)/bin
CONTRIBDIR = $(ROOTDIR)/contrib
INCDIR = $(ROOTDIR)/includes
UTILSDIR = $(ROOTDIR)/src/libs
OUTDIR = ./out
OUT_GUARD=@mkdir -p $(OUTDIR)

STRINGS_INI := strings.ini
STRINGS_BIN := strings.bin
STRINGS_IDX_H := strings_index.h
STRINGS_ZX0 := strings.bin.zx0
STRINGS_ZX0_C := utils_strings_zx0.c


all: $(OUTDIR)/$(STRINGS_ZX0_C)

$(OUTDIR)/$(STRINGS_BIN): $(STRINGS_INI)
	@$(OUT_GUARD)
	@$(BINDIR)/parse_strings.js $< $@
	@cp $(OUTDIR)/$(STRINGS_IDX_H) $(INCDIR)/
	@find ../src -name "*.c" -exec grep -l "$(STRINGS_IDX_H)" {} \; | xargs -r touch

$(OUTDIR)/$(STRINGS_ZX0): $(OUTDIR)/$(STRINGS_BIN)
	@$(MAKE) -C $(CONTRIBDIR) $(BINDIR)/zx0
	@$(BINDIR)/zx0 -f $< $@

$(OUTDIR)/$(STRINGS_ZX0_C): $(OUTDIR)/$(STRINGS_ZX0)
	@which xxd > /dev/null || { echo "Installing xxd..."; sudo apt-get update -qq && sudo apt-get install -qq -y xxd; }
	@xxd -i $< | sed '/_len = /d' | sed 's/unsigned char/const unsigned char/' > $@
	@cp $@ $(UTILSDIR)/

clean: clean_h
	@rm -rf $(OUTDIR)

clean_h:
	@rm -f $(INCDIR)/$(STRINGS_IDX_H)
	@rm -f $(UTILSDIR)/$(STRINGS_ZX0_C)
