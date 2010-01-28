# videodog

# general config

NAME = videodog
VERSION =0.31
CONFIG_FILE=/etc/vd.conf
# comment if you want to change it
SPECIAL= -DBENCH   # enables benchmark ( frame count / sec count )
JPEG = -DHASJPEG
JPEG_LIB=-ljpeg

#ASM_SWAP = -DBGR2RGB  # enables a faster swap routine (bgr24 -> rgb24)


# nothing to change below

# utils

CC = gcc
RM = rm -f
#ST = `which strip`
INSTALL = cp



INCLUDE = -I.
LIBS = -lm $(JPEG_LIB)

#LIBS = -lm
#CFLAGS = -O3 -Wall $(INCLUDE) $(JPEG) -DDEBUG $(SPECIAL) -DSTAMP 
CFLAGS = -O3 -Wall $(INCLUDE) $(JPEG) $(SPECIAL) $(ASMSWAP) -DSTAMP -DCONFIG_FILE=\"$(CONFIG_FILE)\" -DVERSION=\"$(VERSION)\" $(ASM_SWAP) # -DMOTION -g3  -DDEBUG 



prefix = /usr/local
bindir = $(prefix)/bin
docdir = $(prefix)/doc/$(NAME)-$(VERSION)

# files
OBJECTS = utils.o  \
	  loadset.o  \
	  effects.o \
	  video.o  \
	  stamp.o \
	  motion.o \
		  main.o

SOURCES =  functions.c  \
	   loadset.c  \
	   effects.c \
	   video.c     \
	   stamp.c \
	   motion.c \
		  main.c

DOC    = CHANGELOG \
	INSTALL \
	LICENSE \
	README \
	EFFECTS \
	TODO

# rules
.SUFFIXES: .c
.c.o:
		$(CC) $(CFLAGS) -c -o $@ $<

# build
all:	$(OBJECTS)
		$(CC) -o $(NAME) $(CFLAGS) $(OBJECTS) $(LIBS)
	#	$(ST) $(NAME)

install: $(NAME)
	mkdir -p $(docdir)
	mkdir -p $(bindir)
	$(INSTALL) $(NAME) $(bindir)
	$(INSTALL) $(DOC) $(docdir)

clean:
		$(RM) $(NAME) $(OBJECTS)
		$(RM) *~
