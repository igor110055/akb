#ifndef __HASHTBL_H__
#define __HASHTBL_H__

extern void   hashtable_init(void);
extern void   hashtable_set_type(int t);
extern void   hashtable_set(int key, int value);
extern int    hashtable_get(int key);
extern void   hashtable_clear(void);
extern void   hashtable_destroy(void);

#endif


