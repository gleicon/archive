# videodog - extras

# general config


# nothing to change below

# utils

CC = gcc
RM = rm -f
ST = `which strip`
INSTALL = cp



INCLUDE = -I.
LIBS = -ljpeg -lm

XLIBFLAGS = -L/usr/X11R6/lib -lX11  -lXext -lm -ljpeg
GTKFLAGS = `gtk-config --libs gthread` `gtk-config --cflags gthread` `gdk-pixbuf-config --libs` `gdk-pixbuf-config --cflags`

SDLFLAGS = -lSDL -lm

prefix = /usr/local
bindir = $(prefix)/bin
docdir = $(prefix)/doc/$(NAME)-$(VERSION)


# rules
.SUFFIXES: .c
.c.o:
		$(CC) $(CFLAGS) -c -o $@ $<

xputjpeg:
	$(CC) -o xputjpeg xputjpeg.c $(XLIBFLAGS)

gtkpnm:
	$(CC) -o gtkpnm gtkpnm.c $(GTKFLAGS)
gtk_cam:
	$(CC) -o gtk_cam gtk_cam.c $(GTKFLAGS)
sdlcap:
	$(CC) -o sdlcap sdlcap-v4l.c $(SDLFLAGS)

clean:
		$(RM) xputjpeg
		$(RM) gtk_cam
		$(RM) gtkpnm
		$(RM) *~
