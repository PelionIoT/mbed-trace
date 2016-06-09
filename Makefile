#
# Makefile for mbed Client C++ Library
#
# List of subdirectories to build
TEST_FOLDER := ./test/

# Define compiler toolchain with CC or PLATFORM variables
# Example (GCC toolchains, default $CC and $AR are used)
# make
#
# OR (Cross-compile GCC toolchain)
# make PLATFORM=arm-linux-gnueabi-
#
# OR (ArmCC/Keil)
# make CC=ArmCC AR=ArmAR
#
# OR (IAR-ARM)
# make CC=iccarm

LIB = libmbed-trace.a

# List of unit test directories for libraries
UNITTESTS := $(sort $(dir $(wildcard $(TEST_FOLDER)*/utest/*)))


include sources.mk
include include_dirs.mk

override CFLAGS += $(addprefix -I,$(INCLUDE_DIRS))
override CFLAGS += $(addprefix -D,$(FLAGS))
ifeq ($(DEBUG),1)
override CFLAGS += -DHAVE_DEBUG
endif

COVERAGEFILE := ./lcov/coverage.info

#
# Define compiler toolchain
#
include ../libService/toolchain_rules.mk

$(eval $(call generate_rules,$(LIB),$(SRCS)))

# Extend default clean rule
clean: clean-extra

$(CLEANDIRS):
	@make -C $(@:clean-%=%) clean

clean-extra: $(CLEANDIRS)
