TOOL_PREFIX = arm-linux-
CC      = $(TOOL_PREFIX)gcc
AR 	= $(TOOL_PREFIX)ar
RANLIB 	= $(TOOL_PREFIX)ranlib
STRIP   = $(TOOL_PREFIX)strip

INCDIR =  $(PWD)/inc -I$(PWD)/../sdk/inc -I$(PWD)/../lib/include -I$(PWD)/../utk/inc -I$(PWD)/../ulib/inc

CFLAGS = -fno-strict-aliasing -O3 -I$(INCDIR) -DLIBEV 

ifeq ($(findstring debug,$(MAKECMDGOALS)), debug)
CFLAGS += -g -DDEBUG
endif

CFLAGS += -Wall

LDFLAGS = -L../lib/lib -lutk -lsdk -lulibev -lcrypt -ljpeg -lpng12 -lasound -ldl -lz -lpthread -lm

dir_y  = ./main ./ui

OBJDIR = ./obj
OBJS   = $(OBJDIR)/*.o

TARGET = akbd

export CC
export STRIP
export CFLAGS
export LDFLAGS

all:
	[ -d $(OBJDIR) ] || mkdir -p $(OBJDIR)
	for i in $(dir_y) ; do \
		[ ! -d $$i ] || make -C $$i MAKECMDGOALS=$(MAKECMDGOALS) || exit $$? ; \
	done
	$(CC) -o $(TARGET) $(OBJS) $(LDFLAGS) -Wall
	$(STRIP) $(TARGET)
	chmod 755 $(TARGET)
	$(CC) -o daemon_akb daemon.c $(LDFLAGS) -Wall
#	$(STRIP) daemon_akb
	chmod 755 daemon_akb
	cp -a $(TARGET) /mnt/hgfs/dos/update_sd/
	cp -a daemon_akb /mnt/hgfs/dos/update_sd/
	mv $(TARGET) /home/m2416/rootfsa/opt/
	mv daemon_akb /home/m2416/rootfsa/opt/

test:
	make -C testx

debug:
	make MAKECMDGOALS=debug

clean:
	for i in $(dir_y) ; do \
		make -C $$i clean ; \
	done
	rm -f ./obj/*.o
	rm -f ./inc/*.bak
	rm -f $(TARGET)
	rm -f *.bak
