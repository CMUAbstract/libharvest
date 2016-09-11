LIB = libharvest

OBJECTS = \
	charge.o \

# Override because the app/maker makefile sets it hard with -e
override SRC_ROOT := ../../src

override CFLAGS += \
	-I $(SRC_ROOT) \
	-I $(SRC_ROOT)/include/$(LIB) \

override DEPS += \
	libmsp \

include ../Makefile.config # one up from toolchain subdirs
