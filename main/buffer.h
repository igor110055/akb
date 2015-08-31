#ifndef BUFFER_H_
#define BUFFER_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _buffer {
	char  *rptr;
	char  *bufbase;

	size_t misalign;
	size_t totallen;
	size_t off;
	int    fix;
} buffer_t;

#define BUFFER_LENGTH(x)   ((x).off)
#define BUFFER_DATA(x)	   ((x).rptr)
#define BUFFER_SPACE(x)    ((x).totallen - (x).off)

#define BUFPTR_LENGTH(x)   ((x)->off)
#define BUFPTR_DATA(x)	   ((x)->rptr)
#define BUFPTR_SPACE(x)    ((x)->totallen - (x)->off)


buffer_t* buffer_new(int sz);
buffer_t* buffer_new_fixed(int sz);

void   buffer_free(buffer_t *buf);
void   buffer_reset     (buffer_t *buf);
void   buffer_init      (buffer_t *buf, int sz);
void   buffer_init_fix  (buffer_t *buf, int sz);
void   buffer_base_free (buffer_t *rptr);
int    buffer_read      (buffer_t *buf, int fd);
int    buffer_readn   (buffer_t *buf, int fd, int n);
int    buffer_write    (buffer_t *rptr, int fd);
int    buffer_add(buffer_t *buf, const void *data, size_t datlen);
void   buffer_drain     (buffer_t *buf, size_t len);
void   buffer_drain_ptr (buffer_t *buf, char *ptr);
void  *buffer_find_chr  (buffer_t *buf, char c);
void  *buffer_find_rchr (buffer_t *buf, char c);
void  *buffer_find      (buffer_t *buf, const char *what, size_t len);
int    buffer_add_printf(buffer_t *buf, const char *fmt, ...);

#ifdef __cplusplus
}
#endif

#endif

