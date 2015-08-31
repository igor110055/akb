#ifndef PINYIN_H_
#define PINYIN_H_

typedef struct _hzcandidate {
	char   text[4];
} hzcandidate_t;

extern void  py_init(void);
extern int   py_ime(char *instr, hzcandidate_t *arr, int maxsz);

#endif

