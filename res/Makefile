.PHONY: all clean clean_h

BINDIR = ../bin
INCDIR = ../includes
UTILSDIR = ../src/libs
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
	@zx0 -f $< $@

$(OUTDIR)/$(STRINGS_ZX0_C): $(OUTDIR)/$(STRINGS_ZX0)
	@xxd -i $< | sed '/_len = /d' | sed 's/unsigned char/const unsigned char/' > $@
	@cp $@ $(UTILSDIR)/

clean: clean_h
	@rm -rf $(OUTDIR)

clean_h:
	@rm -f $(INCDIR)/$(STRINGS_IDX_H)
	@rm -f $(UTILSDIR)/$(STRINGS_ZX0_C)
