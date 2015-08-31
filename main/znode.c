#include <pthread.h>
#include "mxc_sdk.h"
#include "ulib.h"
#include "config.h"
#include "znode.h"

static pthread_mutex_t    ntx = PTHREAD_MUTEX_INITIALIZER;
#define zlink_lock()     pthread_mutex_lock(&ntx)
#define zlink_unlock()   pthread_mutex_unlock(&ntx)

struct _zhead;

// double linked list.
typedef struct _znode {
	struct list_hdr  list;
	struct _zhead   *head;
	int    nr;
} znode_t;

typedef struct _zhead {
	struct list_hdr  list;
	struct _znode   *curr;
} zhead_t;

static struct list_hdr  free_node = {&free_node, &free_node};
static zhead_t  nhead;      //not ready zones list.
static zhead_t  ahead;      //alarm/restored list.

static znode_t  *nrptr[MAXZ] = {NULL};

void
zhead_init(void)
{
	U_INIT_LIST_HEAD(&nhead.list);
	nhead.curr = NULL;
	U_INIT_LIST_HEAD(&ahead.list);
	ahead.curr = NULL;
}

/* DONE: called by zopt thread. */
static znode_t *
znode_new(int z)
{
	znode_t *zl = NULL;
	struct list_hdr  *list;

	if (!u_list_empty(&free_node)) {
		list = free_node.next;
		u_list_del(list);
		zl = (znode_t *)list;
		zl->nr = z;
	} else {
		zl = calloc(1, sizeof(znode_t));
		if (zl) {
			zl->nr = z;
			U_INIT_LIST_HEAD(&zl->list);
		}
	}
	return zl;
}

/* DONE: called by zopt thread. */
static inline void
znode_free(znode_t *zl)
{
	memset(zl, 0, sizeof(znode_t));
	zl->head = NULL;
	U_INIT_LIST_HEAD(&zl->list);
	u_list_append(&zl->list, &free_node);
}

static void
znode_put(int z, zhead_t *head)
{
	znode_t *zl = nrptr[z&0xffff];

	DBG("--- z = %d, a = %d ---\n", z&0xffff, z>>16);
	zlink_lock();
	if (zl) {
		zl->nr = z;
		DBG("--- znode exists ---\n");
		if (zl->head != head) {
			u_list_del(&zl->list);
			u_list_add(&zl->list, &head->list);
			zl->head = head;
		}
	} else {
		zl = znode_new(z);
		nrptr[z&0xffff] = zl;
		u_list_add(&zl->list, &head->list);
		zl->head = head;
	}
	head->curr = zl;
	zlink_unlock();
}

/* called by zopt thread. */
static void
znode_del(int z, zhead_t *head)
{
	znode_t *zl = nrptr[z&0xffff];
	struct list_hdr *list = NULL;

	if (!zl||zl->head != head) return;
	nrptr[z&0xffff] = NULL;
	zlink_lock();
	if (head->curr == zl) {
		list = head->curr->list.next;
	}
	u_list_del(&zl->list);
	znode_free(zl);

	if (u_list_empty(&head->list)) {
		head->curr = NULL;
	} else if (list != NULL) {
		if (list == &head->list) {
			list = head->list.next;
		}
		head->curr = (znode_t*)list;
	}
	zlink_unlock();
}

void
znode_remove(int z)
{
	znode_t *zl = nrptr[z&0xffff];
	struct list_hdr *list = NULL;
	zhead_t  *head;

	if (!zl) return;
	nrptr[z&0xffff] = NULL;
	head = zl->head;
	zlink_lock();
	if (head->curr == zl) {
		list = head->curr->list.next;
	}

	u_list_del(&zl->list);
	znode_free(zl);

	if (u_list_empty(&head->list)) {
		head->curr = NULL;
	} else if (list != NULL) {
		if (list == &head->list) {
			list = head->list.next;
		}
		head->curr = (znode_t*)list;
	}
	zlink_unlock();
}

/* called by zopt thread. */
static void
znode_delp(int p, zhead_t *head)
{
	struct list_hdr *list;
	znode_t  *zl;
	int  z;

	zlink_lock();
	list = head->list.next;
	while (list != &head->list) {
		zl = (znode_t *)list;
		list = list->next;
		z = zl->nr & 0xffff;
		if (p == pnrofz(z)) {
			DBG("--- delete the arm zone-%d in pzone-%d ---\n", z, p+1);
			nrptr[z] = NULL;
			u_list_del(&zl->list);
			znode_free(zl);
		}
	}
	if (u_list_empty(&head->list)) {
		head->curr = NULL;
	} else {
		head->curr = (znode_t *)head->list.next;
	}
	zlink_unlock();
}

static void
zlink_clear(zhead_t *head)
{
	struct list_hdr *list;
	znode_t   *zl;

	zlink_lock();
	while (!u_list_empty(&head->list)) {
		list = head->list.next;
		zl = (znode_t *)list;
		nrptr[zl->nr&0xffff] = NULL;
		u_list_del(list);
		znode_free(zl);
	}
	head->curr = NULL;
	zlink_unlock();
}

/* called by UI thread. */
static int
znode_get(zhead_t *head, int *last)
{
	int  z = -1;
	struct list_hdr *list;

	zlink_lock();
	if (head->curr != NULL) {
		z = head->curr->nr;
		DBG("--- z = %d, a = %d ---\n", z&0xffff, z>>16);
		list = head->curr->list.next;
		if (list == &head->list) {
			if (last)
				*last = 1;
			list = head->list.next;
			DBG("--- end of the list ---\n");
		}
		head->curr = (znode_t*)list;
	}
	zlink_unlock();
	return z;
}


/* DONE: called by zopt thread. */
/* z: [0, 99*MAXHOST) */
void
nready_put(int z)
{
	znode_put(z, &nhead);
}

/* called by zopt thread. */
/* z: [0, 99*MAXHOST) */
void
nready_del(int z)
{
	znode_del(z, &nhead);
}

/* called by zopt thread. */
void
nready_clear(void)
{
	zlink_clear(&nhead);
}

/* called by UI thread. */
int
nready_get(int *last)
{
	return znode_get(&nhead, last);
}

int 
nready_yet(void)
{
	return (!u_list_empty(&nhead.list));
}

//////////////////////////////////////////////////////////
/* DONE: called by zopt thread. */
/* a: 0-alarm restore, 1-alarm */
void
alarmst_put(int z, int a)
{
	DBG("--- z = %d, a = %d ---\n", z, a);
	znode_put(z|(a<<16), &ahead);
}

/* called by zopt thread. */
void
alarmst_del(int z)
{
	znode_del(z, &ahead);
}

/* called by zopt thread. */
void
alarmst_clear(void)
{
	zlink_clear(&ahead);
}

/* called by UI thread. */
int
alarmst_get(int *last)
{
	return znode_get(&ahead, last);
}

void
alarmst_clearpz(int p)
{
	znode_delp(p, &ahead);
}


