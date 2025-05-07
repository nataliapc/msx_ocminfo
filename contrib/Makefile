.PHONY: clean

UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
	COL_RED = \e[1;31m
	COL_YELLOW = \e[1;33m
	COL_ORANGE = \e[1;38:5:208m
	COL_BLUE = \e[1;34m
	COL_GRAY = \e[1;30m
	COL_WHITE = \e[1;37m
	COL_RESET = \e[0m
endif

ROOTDIR = ..
BINDIR = $(ROOTDIR)/bin
CONTRIBDIR = .

MAKE = make -s --no-print-directory

HEX2BIN_TAR = $(CONTRIBDIR)/hex2bin.tar.bz2


all: $(BINDIR)/zx0 $(BINDIR)/hex2bin


$(BINDIR)/hex2bin:
	@echo "$(COL_WHITE)######## HEX2BIN$(COL_RESET)"
	@echo "Downloading hex2bin..."
	@wget -q https://sourceforge.net/projects/hex2bin/files/latest/download -O $(HEX2BIN_TAR)
	@mkdir -p $(CONTRIBDIR)/hex2bin
	@echo "Unpacking hex2bin..."
	@tar -xjf $(HEX2BIN_TAR) -C $(CONTRIBDIR)/hex2bin --strip-components=1
	@rm -f $(HEX2BIN_TAR)
	@patch --merge -i $(CONTRIBDIR)/hex2bin.diff $(CONTRIBDIR)/hex2bin/Makefile
	@echo "Compiling hex2bin..."
	@$(MAKE) -C $(CONTRIBDIR)/hex2bin hex2bin 2> /dev/null
	@echo "Copying hex2bin..."
	@cp $(CONTRIBDIR)/hex2bin/hex2bin $@


$(BINDIR)/zx0:
	@echo "$(COL_WHITE)######## ZX0$(COL_RESET)"
	@if cd $(CONTRIBDIR)/ZX0.git 2> /dev/null; then \
		git pull; \
	else \
		git clone https://github.com/einar-saukas/ZX0 $(CONTRIBDIR)/ZX0.git && \
		patch --merge -i $(CONTRIBDIR)/zx0.diff $(CONTRIBDIR)/ZX0.git/src/Makefile ; \
	fi
	@$(MAKE) -C $(CONTRIBDIR)/ZX0.git/src 2> /dev/null
	@cp $(CONTRIBDIR)/ZX0.git/src/zx0 $@


clean: cleanhex2bin cleanzx0

cleanhex2bin:
	@rm -rf $(CONTRIBDIR)/hex2bin $(BINDIR)/hex2bin

cleanzx0:
	@rm -rf $(CONTRIBDIR)/ZX0.git $(BINDIR)/zx0
