#include <sys/types.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>

#include "mxc_sdk.h"
#include "ulib.h"
#include "buffer.h"


extern void *memrchr(const void *s, int c, size_t n);

void
buffer_reset(buffer_t *buf)
{
	memset(buf->bufbase, 0, buf->totallen);
	buf->rptr = buf->bufbase;
	buf->misalign = 0;
	buf->off = 0;
}

void
buffer_init(buffer_t *buf, int sz)
{
	if (sz > 0)
	{
		buf->bufbase = calloc(sz, 1);
		buf->totallen = sz;
		buffer_reset(buf);
	}
}

void
buffer_init_fix(buffer_t *buf, int sz)
{
	buffer_init(buf, sz);
	buf->fix = 1;
}

buffer_t*
buffer_new(int sz)
{
	buffer_t *b;

	b = calloc(1, sizeof(buffer_t));
	if (b != NULL) {
		buffer_init(b, sz);
	}
	return b;
}

buffer_t*
buffer_new_fixed(int sz)
{
	buffer_t *b;

	b = calloc(1, sizeof(buffer_t));
	if (b != NULL) {
		buffer_init_fix(b, sz);
	}
	return b;
}

void
buffer_base_free(buffer_t *buf)
{
	if (buf->bufbase != NULL&&buf->totallen > 0) {
		free(buf->bufbase);
		buf->bufbase = NULL;
		buf->totallen = 0;
	}
}

void
buffer_free(buffer_t *buf)
{
	if (buf) {
		buffer_base_free(buf);
		free(buf);
	}
}

static void
buffer_align(buffer_t *buf)
{
	if (buf->misalign != 0) {
		memmove(buf->bufbase, buf->rptr, buf->off);
		buf->rptr = buf->bufbase;
		buf->misalign = 0;
	}
}

static int
buffer_expand(buffer_t *buf, size_t datlen)
{
	size_t need = buf->misalign + buf->off + datlen;

	if (buf->totallen >= need)
		return (0);

	if (buf->misalign >= datlen) {
		DBG("--- the misalignment fulfills our data needs ---\n");
		buffer_align(buf);
	} else if (buf->fix == 0) {
		void *newbuf;
		size_t length = buf->totallen;

		if (length < 256)
			length = 256;
		while (length < need)
			length <<= 1;

		if (buf->bufbase != buf->rptr)
			buffer_align(buf);
		if ((newbuf = realloc(buf->rptr, length)) == NULL)
		{
			DBG("--- realloc failed ---\n");
			return (-1);
		}
		DBG("--- realloc to %d ---\n", length);
		buf->bufbase = buf->rptr = newbuf;
		buf->totallen = length;

		return 0;
	} else {
		return (-1);
	}

	return (0);
}

/*
 * Reads data from a file descriptor into a buffer.
 */
int
buffer_read(buffer_t *buf, int fd)
{
	char *p;
	int   n;

	if (ioctl(fd, FIONREAD, &n) < 0) {
	    DBG("--- ioctl:FIONREAD error ---\n");
	    return -1;
	}

	if (n > 0) {
		if (buffer_expand(buf, n) == -1) {
			return (-1);
		}

		p = buf->rptr + buf->off;
		n = read(fd, p, n);
		if (n > 0)
		{
			buf->off += n;
		}
	}
	return n;
}

int
buffer_readn(buffer_t *buf, int fd, int n)
{
	char *p;

	if (buffer_expand(buf, n) == -1) {
		return (-1);
	}

	p = buf->rptr + buf->off;
	n = read(fd, p, n);
	if (n > 0)
	{
		buf->off += n;
	}

	return n;
}

/* DONE */
/* called when read data from buffer */
void
buffer_drain(buffer_t *buf, size_t len)
{
	if (len >= buf->off)
	{
		buf->off = 0;
		buf->rptr = buf->bufbase;
		buf->misalign = 0;
	}
	else
	{
		buf->rptr += len;
		buf->misalign += len;
		buf->off -= len;
	}
}

void
buffer_drain_ptr(buffer_t *buf, char *ptr)
{
	int len;

	len = ptr - buf->rptr;
	if (len >= buf->off)
	{
		buf->off = 0;
		buf->rptr = buf->bufbase;
		buf->misalign = 0;
	}
	else
	{
		buf->rptr = ptr;
		buf->misalign += len;
		buf->off -= len;
	}
}

void*
buffer_find_chr(buffer_t *buf, char c)
{
	return memchr(buf->rptr, (int)c, buf->off);
}

void*
buffer_find_rchr(buffer_t *buf, char c)
{
	return memrchr(buf->rptr, (int)c, buf->off);
}

void*
buffer_find(buffer_t *buf, const char *what, size_t len)
{
	char *search = buf->rptr, *end = search + buf->off;
	char *p;

	while (search < end && (p = memchr(search, *what, end - search)) != NULL) {
		if (p + len > end)
			break;
		if (memcmp(p, what, len) == 0)
			return (p);
		search = p + 1;
	}

	return (NULL);
}

int
buffer_write(buffer_t *rptr, int fd)
{
	int n;

	n = write(fd, rptr->rptr, rptr->off);
	if (n == -1)
		return (-1);
	if (n == 0)
		return (0);
	buffer_drain(rptr, n);

	return (n);
}

int
buffer_add(buffer_t *buf, const void *data, size_t datlen)
{
	size_t need = buf->misalign + buf->off + datlen;

	if (buf->totallen < need) {
		if (buffer_expand(buf, datlen) == -1)
			return (-1);
	}

	memcpy(buf->rptr + buf->off, data, datlen);
	buf->off += datlen;

	return (0);
}

int
buffer_add_printf(buffer_t *buf, const char *fmt, ...)
{
	char *ptr;
	int   left;
	int   n;
	va_list ap;

	buffer_align(buf);
	ptr = buf->rptr + buf->off;
	left = buf->totallen - buf->off - buf->misalign;
	va_start(ap, fmt);
	n = vsnprintf(ptr, left, fmt, ap);
	va_end(ap);

	if (n >= left||n == -1) {
		if (buffer_expand(buf, 4096) == -1)
			return (-1);

		ptr = buf->rptr + buf->off;
		left = buf->totallen - buf->off - buf->misalign;
		va_start(ap, fmt);
		n = vsnprintf(ptr, left, fmt, ap);
		va_end(ap);
	}

	return n;
}




