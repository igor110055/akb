all:
	$(CC) -o fbtest fb_test.c $(CFLAGS) $(LDFLAGS) -Wall
	$(CC) -o itest itest.c $(CFLAGS) $(LDFLAGS) -Wall
	$(CC) -o otest otest.c $(CFLAGS) $(LDFLAGS) -Wall
	$(CC) -o rs485 rs485.c $(CFLAGS) $(LDFLAGS) -Wall
	$(STRIP) fbtest
	$(STRIP) itest
	$(STRIP) otest
	$(STRIP) rs485
	mv fbtest /mnt/hgfs/dos/update_sd/
	mv itest /mnt/hgfs/dos/update_sd/
	mv otest /mnt/hgfs/dos/update_sd/
	mv rs485 /mnt/hgfs/dos/update_sd/

clean:
	rm -f *.o *.bak
