#
# This makefile system follows the structuring conventions
# recommended by Peter Miller in his excellent paper:
#
#	Recursive Make Considered Harmful
#	http://aegis.sourceforge.net/auug97.pdf
#
OBJDIR := obj

ifdef LAB
SETTINGLAB := true
else
-include conf/lab.mk
endif

-include conf/env.mk

ifndef SOL
SOL := 0
endif
ifndef LABADJUST
LABADJUST := 0
endif

ifndef LABSETUP
LABSETUP := ./
endif


TOP = .

# Cross-compiler fos toolchain
#
# This Makefile will automatically use the cross-compiler toolchain
# installed as 'i386-fos-elf-*', if one exists.  If the host tools ('gcc',
# 'objdump', and so forth) compile for a 32-bit x86 ELF target, that will
# be detected as well.  If you have the right compiler toolchain installed
# using a different name, set GCCPREFIX explicitly in conf/env.mk

# try to infer the correct GCCPREFIX
ifndef GCCPREFIX
GCCPREFIX := $(shell if i386-fos-elf-objdump -i 2>&1 | grep '^elf32-i386$$' >/dev/null 2>&1; \
	then echo 'i386-fos-elf-'; \
	elif objdump -i 2>&1 | grep 'elf32-i386' >/dev/null 2>&1; \
	then echo ''; \
	else echo "***" 1>&2; \
	echo "*** Error: Couldn't find an i386-*-elf version of GCC/binutils." 1>&2; \
	echo "*** Is the directory with i386-fos-elf-gcc in your PATH?" 1>&2; \
	echo "*** If your i386-*-elf toolchain is installed with a command" 1>&2; \
	echo "*** prefix other than 'i386-fos-elf-', set your GCCPREFIX" 1>&2; \
	echo "*** environment variable to that prefix and run 'make' again." 1>&2; \
	echo "*** To turn off this error, run 'gmake GCCPREFIX= ...'." 1>&2; \
	echo "***" 1>&2; exit 1; fi)
endif

CC	:= $(GCCPREFIX)gcc -pipe
GCC_LIB := $(shell $(CC) -print-libgcc-file-name)
AS	:= $(GCCPREFIX)as
AR	:= $(GCCPREFIX)ar
LD	:= $(GCCPREFIX)ld
OBJCOPY	:= $(GCCPREFIX)objcopy
OBJDUMP	:= $(GCCPREFIX)objdump
NM	:= $(GCCPREFIX)nm

# Native commands
NCC	:= gcc $(CC_VER) -pipe
TAR	:= gtar
PERL	:= perl

# Compiler flags
# -fno-builtin is required to avoid refs to undefined functions in the kernel.
# Obselete: Only optimize to -O1 to discourage inlining, which complicates backtraces.
# Current: no optimization with -O0
CFLAGS	:= $(CFLAGS) $(DEFS) $(LABDEFS) -O0 -fno-builtin -I$(TOP) -MD -Wall -Wno-format -Wno-unused -Werror -gstabs

# Linker flags for FOS user programs
ULDFLAGS := -T user/user.ld

# Lists that the */Makefrag makefile fragments will add to
OBJDIRS :=

# Make sure that 'all' is the first target
all:

# Eliminate default suffix rules
.SUFFIXES:

# Delete target files if there is an error (or make is interrupted)
.DELETE_ON_ERROR:

# make it so that no intermediate .o files are ever deleted
.PRECIOUS: %.o $(OBJDIR)/boot/%.o $(OBJDIR)/kern/%.o \
	$(OBJDIR)/lib/%.o $(OBJDIR)/fs/%.o $(OBJDIR)/user/%.o

KERN_CFLAGS := $(CFLAGS) -DFOS_KERNEL -gstabs
USER_CFLAGS := $(CFLAGS) -DFOS_USER -gstabs




# Include Makefrags for subdirectories
include boot/Makefrag
include kern/Makefrag
include lib/Makefrag
include user/Makefrag


IMAGES = $(OBJDIR)/kern/bochs.img

bochs: $(IMAGES)
	bochs 'display_library: nogui'

# For deleting the build
clean:
	rm -rf $(OBJDIR)

realclean: clean
	rm -rf lab$(LAB).tar.gz bochs.out bochs.log

distclean: realclean
	rm -rf conf/gcc.mk

grade: $(LABSETUP)grade.sh
	$(V)$(MAKE) clean >/dev/null 2>/dev/null
	$(MAKE) all
	sh $(LABSETUP)grade.sh

handin: tarball
	@echo Please visit http://pdos.csail.mit.edu/cgi-bin/828handin
	@echo and upload lab$(LAB)-handin.tar.gz.  Thanks!

tarball: realclean
	tar cf - `find . -type f | grep -v '^\.*$$' | grep -v '/CVS/' | grep -v '/\.svn/' | grep -v 'lab[0-9].*\.tar\.gz'` | gzip > lab$(LAB)-handin.tar.gz

# For test runs

#run-%:
#	$(V)rm -f $(OBJDIR)/kern/init.o $(IMAGES)
#	$(V)$(MAKE) "DEFS=-DTEST=_binary_obj_user_$*_start -DTESTSIZE=_binary_obj_user_$*_size" $(IMAGES)
#	bochs -q 'display_library: nogui'

#xrun-%:
#	$(V)rm -f $(OBJDIR)/kern/init.o $(IMAGES)
#	$(V)$(MAKE) "DEFS=-DTEST=_binary_obj_user_$*_start -DTESTSIZE=_binary_obj_user_$*_size" $(IMAGES)
#	bochs -q

# This magic automatically generates makefile dependencies
# for header files included from C source files we compile,
# and keeps those dependencies up-to-date every time we recompile.
# See 'mergedep.pl' for more information.
$(OBJDIR)/.deps: $(foreach dir, $(OBJDIRS), $(wildcard $(OBJDIR)/$(dir)/*.d))
	@mkdir -p $(@D)
	@$(PERL) mergedep.pl $@ $^

-include $(OBJDIR)/.deps

always:
	@:

.PHONY: all always \
	handin tarball clean realclean clean-labsetup distclean grade labsetup
