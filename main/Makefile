SOURCES = $(wildcard *.c)
HEADERS = $(wildcard *.h)

OBJS = $(patsubst %.c, %.o, $(SOURCES))

all:
	for i in $(SOURCES); do \
		$(CC) -c $$i $(CFLAGS) ; \
	done
	mv *.o ../obj

clean:
	rm -f *.o *.bak
