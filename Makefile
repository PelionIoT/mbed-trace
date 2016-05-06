SRCS := $(wildcard source/*.c)

LIB := libmbed-trace.a

include ../../source/library_rules.mk

.PHONY: export-headers
export-headers:
	cp -r --update ./mbed-trace ../../libService/
