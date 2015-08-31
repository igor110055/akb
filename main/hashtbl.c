#include "mxc_sdk.h"
#include "config.h"
#include "hashtbl.h"

typedef struct _hnode {
	struct hl_node  list;
	int  key;
	int  value;
} hnode_t;

typedef struct _hash_op {
	void (*set)(int key, int value);
	int  (*get)(int key);
	void (*clr)(void);
	void (*destroy)(void);
} hash_op_t;

static char     hidarr[256] = {0};
struct hl_head  htab[MAXHOST];
struct hl_head  free_head;

static void  hidarr_set(int key, int value);
static int   hidarr_get(int key);
static void  hidarr_clr(void);
static void  hidarr_destroy(void);

static void  htable_set(int key, int value);
static int   htable_get(int key);
static void  htable_clr(void);
static void  htable_destroy(void);


static hash_op_t  hashops[] = {
	[0] = {
		.set = hidarr_set,
		.get = hidarr_get,
		.clr = hidarr_clr,
		.destroy = hidarr_destroy,
	},
	[1] = {
		.set = htable_set,
		.get = htable_get,
		.clr = htable_clr,
		.destroy = htable_destroy,
	},
};

static hash_op_t  *curr_hop = &hashops[0];

static void
hidarr_set(int key, int value)
{
	hidarr[key] = value;
}

static int
hidarr_get(int key)
{
	return hidarr[key];
}

static void
hidarr_clr(void)
{
	CLEARA(hidarr);
}

static void
hidarr_destroy(void)
{
}


////////////////////////////////////////////////////////////
hnode_t*
hnode_new(void)
{
	hnode_t  *hn = NULL;
	struct hl_node  *n;
	
	if (u_hlist_empty(&free_head)) {
		hn = calloc(1, sizeof(hnode_t));
		U_INIT_HLIST_NODE(&hn->list);
	} else {
		n = free_head.first;
		u_hlist_del(n);
		hn = (hnode_t*)n;
	}
	return hn;
}

static void
hnode_free(hnode_t *n)
{
	n->key = 0;
	n->value = 0;
	u_hlist_add_head(&n->list, &free_head);
}

static void
htable_set(int key, int value)
{
	hnode_t  *hn = NULL;
	struct hl_node  *n = NULL;
	struct hl_head  *h = &htab[key&31];

	DBG("--- insert hash: bucket = %d ---\n", key&31);
	if (!u_hlist_empty(h)) {
		u_hlist_for_each_entry(hn, n, h, list) {
			if (hn->key == key) {
				hn->value = value;
				return;
			}
		}
	}

	hn = hnode_new();
	hn->key = key;
	hn->value = value;
#ifdef DEBUG
	if (h->first != NULL) {
		DBG("--- hash collusion: bucket = %d ---\n", key&31);
	}
#endif
	u_hlist_add_head(&hn->list, h);
}

static int
htable_get(int key)
{
	struct hl_head  *h = &htab[key&31];
	struct hl_node  *n;
	hnode_t  *hn;

	if (!u_hlist_empty(h)) {
		u_hlist_for_each_entry(hn, n, h, list) {
			if (hn->key == key) {
				return hn->value;
			}
		}
	}

	return 0;
}

static void
htable_clr(void)
{
	int  i;
	struct hl_head  *h;
	struct hl_node  *n;

	for (i = 0; i < MAXHOST; ++i) {
		h = &htab[i];
		while (!u_hlist_empty(h)) {
			n = h->first;
			u_hlist_del(n);
			hnode_free((hnode_t *)n);
		}
	}
}

static void
htable_destroy(void)
{
	struct hl_head  *h = &free_head;
	struct hl_node  *n;

	while (!u_hlist_empty(h)) {
		n = h->first;
		u_hlist_del(n);
		free(n);
	}
}


/* called only once when boot. */
void
hashtable_init(void)
{
	int  i;

	CLEARA(hidarr);
	U_INIT_HLIST_HEAD(&free_head);
	for (i = 0; i < MAXHOST; ++i) {
		U_INIT_HLIST_HEAD(&htab[i]);
	}
}

void
hashtable_set_type(int t)
{
	DBG("--- hash type = %d ---\n", !!t);
	curr_hop = &hashops[!!t];
}

void
hashtable_set(int key, int value)
{
	DBG("--- hash set: key = %d, value = %d ---\n", key, value);
	curr_hop->set(key, value);
}

int
hashtable_get(int key)
{
	int  id;
	id = curr_hop->get(key);
	DBG("--- hostid = %d ---\n", id);
	return id;
}

void
hashtable_clear(void)
{
	curr_hop->clr();
}

void
hashtable_destroy(void)
{
	curr_hop->destroy();
}


