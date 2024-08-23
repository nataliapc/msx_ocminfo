.PHONY: clean test release res resview imxview dsk rom

SDCC_VER := 4.2.0
DOCKER_IMG = nataliapc/sdcc:$(SDCC_VER)
DOCKER_RUN = docker run -i --rm -u $(shell id -u):$(shell id -g) -v .:/src -w /src $(DOCKER_IMG)

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

ROOTDIR = .
BINDIR = $(ROOTDIR)/bin
SRCDIR = $(ROOTDIR)/src
SRCLIB = $(SRCDIR)/libs
LIBDIR = $(ROOTDIR)/libs
INCDIR = $(ROOTDIR)/includes
OBJDIR = $(ROOTDIR)/obj
DSKDIR = $(ROOTDIR)/dsk
EXTERNALS = $(ROOTDIR)/externals
DIR_GUARD=@mkdir -p $(OBJDIR)
LIB_GUARD=@mkdir -p $(LIBDIR)

AS = $(DOCKER_RUN) sdasz80
AR = $(DOCKER_RUN) sdar
CC = $(DOCKER_RUN) sdcc
HEX2BIN = hex2bin
MAKE = make -s --no-print-directory
JAVA = java
DSKTOOL = $(BINDIR)/dsktool
OPEMNSX = openmsx

EMUEXT = -ext debugdevice -ext gfx9000
EMUEXT1 = $(EMUEXT) -ext fmpac -ext Mitsubishi_ML-30DC_ML-30FD
EMUEXT2 = $(EMUEXT) -ext fmpac
EMUEXT2P = $(EMUEXT)
# -ext msxdos2
EMUSCRIPTS = -script ./emulation/boot.tcl


DEFINES := -DMSXDOS1 -DMSX1 -DIMGX_DISABLE_V9958 -DCOMP_DISABLE_ZX7 -DCOMP_DISABLE_RLE
# -DCOMP_DISABLE_ZX0
#DEBUG := -DDEBUG
FULLOPT :=  --max-allocs-per-node 2000000
LDFLAGS = -rc
OPFLAGS = --std-sdcc2x --less-pedantic --opt-code-size -pragma-define:CRT_ENABLE_STDIO=0
WRFLAGS = --disable-warning 196 --disable-warning 84
CCFLAGS = --code-loc 0x0108 --data-loc 0 -mz80 --no-std-crt0 --out-fmt-ihx $(OPFLAGS) $(WRFLAGS) $(DEFINES) $(DEBUG)


LIBS = conio.lib utils.lib
REL_LIBS = $(addprefix $(OBJDIR)/, \
				crt0msx_msxdos.rel \
				heap.rel \
				ocm_ioports.rel \
			) \
			$(addprefix $(LIBDIR)/, $(LIBS))

PROGRAM = ocminfo.com
DSKNAME = ocminfo.dsk

all: res $(OBJDIR)/$(PROGRAM) release


$(LIBDIR)/conio.lib:
	$(MAKE) -C $(EXTERNALS)/sdcc_msxconio all SDCC_VER=$(SDCC_VER) DEFINES=-DXXXXX
	@cp $(EXTERNALS)/sdcc_msxconio/lib/conio.lib $@
	@cp $(EXTERNALS)/sdcc_msxconio/include/conio.h $(INCDIR)
#	@sdar -d $@ dos_cputs.c.rel

$(LIBDIR)/utils.lib: $(patsubst $(SRCLIB)/%, $(OBJDIR)/%.rel, $(wildcard $(SRCLIB)/utils_*))
	@echo "$(COL_WHITE)######## Compiling $@$(COL_RESET)"
	@$(LIB_GUARD)
	@$(AR) $(LDFLAGS) $@ $^ ;

$(OBJDIR)/%.rel: $(SRCDIR)/%.s
	@echo "$(COL_BLUE)#### ASM $@$(COL_RESET)"
	@$(DIR_GUARD)
	@$(AS) -go $@ $^ ;

$(OBJDIR)/%.rel: $(SRCDIR)/%.c
	@echo "$(COL_BLUE)#### CC $@$(COL_RESET)"
	@$(DIR_GUARD)
	@$(CC) $(CCFLAGS) $(FULLOPT) -I$(INCDIR) -c -o $@ $^ ;

$(OBJDIR)/%.c.rel: $(SRCLIB)/%.c
	@echo "$(COL_BLUE)#### CC $@$(COL_RESET)"
	@$(DIR_GUARD)
	@$(CC) $(CCFLAGS) $(FULLOPT) -I$(INCDIR) -c -o $@ $^ ;

$(OBJDIR)/%.s.rel: $(SRCLIB)/%.s
	@echo "$(COL_BLUE)#### ASM $@$(COL_RESET)"
	@$(DIR_GUARD)
	@$(AS) -go $@ $^ ;

$(OBJDIR)/$(PROGRAM): $(REL_LIBS) $(SRCDIR)/ocminfo.c
	@echo "$(COL_YELLOW)######## Compiling $@$(COL_RESET)"
	@$(DIR_GUARD)
	@$(CC) $(CCFLAGS) $(FULLOPT) -I$(INCDIR) -L$(LIBDIR) $(subst .com,.c,$^) -o $(subst .com,.ihx,$@) ;
	@$(HEX2BIN) -e com $(subst .com,.ihx,$@) ;

release:
	@echo "$(COL_WHITE)**** Copying .COM file to $(DSKDIR)$(COL_RESET)"
	@cp $(OBJDIR)/$(PROGRAM) $(DSKDIR)

$(DSKNAME): all
	@echo "$(COL_WHITE)**** $(DSKNAME) generating ****$(COL_RESET)"
	@rm -f $(DSKNAME)
	@$(DSKTOOL) c 360 $(DSKNAME) > /dev/null
	@cd dsk ; ../$(DSKTOOL) a ../$(DSKNAME) \
		MSXDOS.SYS COMMAND.COM AUTOEXEC.BAT \
		ocminfo.com > /dev/null

dsk: $(DSKNAME)

###################################################################################################

clean: cleanobj cleanlibs
	@rm -f $(OBJDIR)/$(PROGRAM) $(DSKDIR)/$(PROGRAM) \
	       $(DSKNAME)

cleanprogram:
	@echo "$(COL_ORANGE)##  Cleaning program files$(COL_RESET)"
	@rm -f $(REL_LIBS)

cleanobj:
	@echo "$(COL_ORANGE)##  Cleaning obj$(COL_RESET)"
	@rm -f $(DSKDIR)/$(PROGRAM)
	@rm -f *.com *.asm *.lst *.sym *.bin *.ihx *.lk *.map *.noi *.rel
	@rm -f $(OBJDIR)/*

cleanlibs:
	@echo "$(COL_ORANGE)##  Cleaning libs$(COL_RESET)"
	@rm -f $(addprefix $(LIBDIR)/, $(LIBS))
	@$(MAKE) -C $(EXTERNALS)/sdcc_msxconio clean


###################################################################################################

test: all
	@bash -c 'if pgrep -x "openmsx" > /dev/null \
	; then \
		echo "**** openmsx already running..." \
	; else \
#		$(OPEMNSX) -machine Philips_NMS_8245 $(EMUEXT2) -diska $(DSKDIR) $(EMUSCRIPTS) \
#		$(OPEMNSX) -machine Toshiba_HX-10 $(EMUEXT1) -diska $(DSKDIR) $(EMUSCRIPTS) \
#		$(OPEMNSX) -machine msx2plus $(EMUEXT2P) -diska $(DSKDIR) $(EMUSCRIPTS) \
#		$(OPEMNSX) -machine turbor $(EMUEXT) -diska $(DSKDIR) $(EMUSCRIPTS) \
#		$(OPEMNSX) -machine Sony_HB-F1XD $(EMUEXT2) -diska $(DSKDIR) $(EMUSCRIPTS) \
		$(OPEMNSX) -machine Panasonic_FS-A1WSX $(EMUEXT2) -diska $(DSKDIR) $(EMUSCRIPTS) \
	; fi'

testrom: rom
	$(OPEMNSX) -machine msx1_eu -ext gfx9000 -ext fmpac -cartb $(ROMNAME) $(EMUSCRIPTS)

resview:
	@$(JAVA) -jar $(BINDIR)/imgWizard.jar l $(DSKDIR)/sdkuland.res

imxview:
	@$(JAVA) -jar $(BINDIR)/imgWizard.jar l $(DSKDIR)/loading.imx
