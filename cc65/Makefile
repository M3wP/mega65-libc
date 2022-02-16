CC65=	cc65
CL65=	cl65

COPTS=	-t c64 -Os -Icc65/include -I./include

MEGA65LIBCCFILES=	\
		src/conio.c\
		src/debug.c\
		src/fat32.c\
		src/fcio.c\
		src/hal.c\
		src/memory.c\
		src/mouse.c\
		src/random.c\
		src/sdcard.c\
		src/targets.c\
		src/tests.c\
		src/time.c

MEGA65LIBASMFILES=	\
		src/dirent.s\
		src/dos.s\
		src/fileio.s\
		src/_sdcard.s

MEGA65LIBHEADERFILES=	\
		include/conio.h\
		include/debug.h\
		include/dirent.h\
		include/dos.h\
		include/fcio.h\
		include/fileio.h\
		include/hal.h\
		include/memory.h\
		include/mouse.h\
		include/random.h\
		include/sdcard.h\
		include/targets.h\
		include/tests.h\
		include/time.h

.PHONY:		clean

lib/libmega65.a:	$(MEGA65LIBCCFILES:.c=.o) $(MEGA65LIBASMFILES:.s=.o) 
		$(warning ======== Making: $@)
		ar65 r $@ $(MEGA65LIBCCFILES:.c=.o) $(MEGA65LIBASMFILES:.s=.o) 

$(MEGA65LIBCCFILES:.c=.o):	$(MEGA65LIBCCFILES:.c=.s)

%.o:	%.s
		$(warning ======== Making: $@)
		ca65 -o $@ $<

$(MEGA65LIBCCFILES:.c=.s):	$(MEGA65LIBCCFILES)

%.s:	%.c
		$(warning ======== Making: $@)
		$(CC65) $(COPTS) -o $@ $<

$(MEGA65LIBASMFILES:.s=.o):	$(MEGA65LIBASMFILES)

$(MEGA65LIBCCFILES):	$(MEGA65LIBHEADERFILES)

$(MEGA65LIBASMFILES):	

ifeq ($(OS),Windows_NT)     # is Windows_NT on XP, 2000, 7, Vista, 10...
else
clean:		
		rm	$(MEGA65LIBASMFILES:.s=.o) $(MEGA65LIBCCFILES:.c=.o) $(MEGA65LIBCCFILES:.c=.s)

endif
