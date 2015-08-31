CFLAGS = -Wall
CC = arm-926ejs-linux-gcc
STRIP = arm-926ejs-linux-strip

all:
	$(CC) -c cgic.c
	$(CC) -c basic.c $(CFLAGS)
	$(CC) -c sec.c $(CFLAGS)
	$(CC) -c cdev.c $(CFLAGS)
	$(CC) -c bgsnd.c $(CFLAGS)
	$(CC) -c scene.c $(CFLAGS)
	$(CC) -c slnk.c $(CFLAGS)
	$(CC) -c set.c $(CFLAGS)
	$(CC) -o set.cgi cgic.o set.o scene.o cdev.o bgsnd.o basic.o sec.o slnk.o
	$(STRIP) set.cgi
	cp *.cgi /home/mx27/rootfs/opt/indoor
	mv *.cgi /mnt/hgfs/dos/update_sd/indoor
clean:
	rm -f $(EXEC) *.gdb *.elf *.o *.bak

