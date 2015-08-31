#include "mxc_sdk.h"
#include "ulib.h"
#include "config.h"
#include "xfer.h"
#include "zopt.h"
#include "pzone.h"
#include "pztimer.h"

typedef int  (*FISHORT) (unsigned short);

typedef struct _pztnode {
	struct list_hdr  list0;
	struct list_hdr  list1;
	int    pnr;      // [1, MAXPZ]
	int    tflag;    //bit0: timeslice0 enable, bit1: timeslice1 enable.
} pztnode_t;

typedef struct _evtnode {
	struct _evtnode *next;
	unsigned int     time;
	int              opt;  //bit0: 0-disarm, 1-arm; bit1: 0-timeslice0, 1-timeslice1.
} evtnode_t;

static pztnode_t  tpznode[MAXTPZ];

static struct list_hdr   thead0 = {&thead0, &thead0};
static struct list_hdr   thead1 = {&thead1, &thead1};
static struct list_hdr  *thead[] = {&thead0, &thead1};

unsigned int  tmslice[4];

static evtnode_t  *evhead = NULL;
static evtnode_t  *evtail = NULL;
static evtnode_t  *evcurr = NULL;
static evtnode_t   evnode[4];
static int         ev_idx = 0;

#define  evnode_list_reset()  do {evhead = NULL; evtail = NULL; evcurr = NULL; ev_idx = 0;} while (0)
#define  evnode_list_empty()  (evhead == NULL)

unsigned int    tmrpzmap = 0;       //timer arm flag, set when arming and clr when disarming.
unsigned int    pztmrflg = 0;       //pzone timer disabled or enabled.need to be saved

#if 0
{
#endif

/* DONE: arg: [0, MAXPZ)  */
static int
pzone_tmrdisarm(unsigned short arg)
{
	if (pztimer_enabled(arg)&&pstat[arg] >= PST_PTARM)
	{
		pzopt_zones(arg, disarmz);
		bparmflag_clr(arg);
		tmrpzmap &= ~(1 << arg);
		frcpzmap &= ~(1 << arg);
		update_pzone_state(arg);
		send_ainfo(PTDM + MAXZCMD, arg + 1);
//		update_zmapst(-1, (arg << 1));
		DBG("--- pzone-%d timer disarm opt ---\n", arg + 1);
		syst_save();
		netxfer_pzopt(arg, 0, 1);
	}
	return 0;
}

/* DONE: arg: [0, MAXPZ] */
static int
pzone_tmrarm(unsigned short arg)
{
	if (cannot_tmrarm(arg) != 0) return 1;
	pzopt_zones(arg, farmz);
	bparmflag_set(arg);
	tmrpzmap |= (1 << arg);
	frcpzmap &= ~(1 << arg);
	update_pzone_state(arg);
	send_ainfo(PTAM + MAXZCMD, arg + 1);
//	update_zmapst(-1, (arg << 1)|1);
	syst_save();
	netxfer_pzopt(arg, 1, 1);
	return 0;
}

static FISHORT pztmr_opt[] = {pzone_tmrdisarm, pzone_tmrarm};

static inline void
init_pznode(pztnode_t *node)
{
	memset(node, 0, sizeof(pztnode_t));
	U_INIT_LIST_HEAD(&node->list0);
	U_INIT_LIST_HEAD(&node->list1);
}

/* DONE */
void
init_tmrpz_node(void)
{
	int  i;

	for (i = 0; i < MAXTPZ; ++i) {
		init_pznode(&tpznode[i]);
	}
}

static evtnode_t*
evnode_get(void)
{
	evtnode_t *node;

	node = &evnode[ev_idx];
	memset(node, 0, sizeof(evtnode_t));
	ev_idx = (ev_idx+1)&3;

	return node;
}

static void
evnode_insert(evtnode_t *node)
{
	if (evtail == NULL) {
		evhead = node;
		evtail = node;
		node->next = evhead;
	} else {
		node->next = evtail->next;
		evtail->next = node;
		evtail = node;
	}
}

static void
evnode_set_current(unsigned int now)
{
	evtnode_t *node;
	unsigned int  tend = 0;
	unsigned int  tmp  = 0;

	if (evtail->time >= SECS_PER_DAY) {
		tend = evtail->time - SECS_PER_DAY;
	}

	if (now >= tend&&now < evhead->time) {
		evcurr = evhead;
		return;
	} else {
		tmp = evhead->time;
		node = evhead->next;
	}

	while (node != evhead)
	{
		if (now < node->time&&now >= tmp) {
			evcurr = node;
			return;
		}
		tmp = node->time;
		node = node->next;
	}

	if (tend > 0&&now < tend) {
		evcurr = evtail;
	} else {
		evcurr = evhead;
	}
}

static void
evnode_list_init(void)
{
	evtnode_t  *node;
	evnode_list_reset();
	if (tmslice[0] != tmslice[1]) {
		node = evnode_get();
		node->opt = 1;
		node->time = tmslice[0];
		evnode_insert(node);
		
		node = evnode_get();
		node->opt = 0;
		node->time = tmslice[1];
		evnode_insert(node);
	}

	if (tmslice[2] != tmslice[3]) {
		node = evnode_get();
		node->opt = 3;
		node->time = tmslice[2];
		evnode_insert(node);
		
		node = evnode_get();
		node->opt = 2;
		node->time = tmslice[3];
		evnode_insert(node);
	}
}

unsigned int
get_daytm(struct tm *ntm)
{
	struct tm  nowtm;
	get_tm(&nowtm, NULL);
	if (ntm) {
		memcpy(ntm, &nowtm, sizeof(struct tm));
	}
	return (unsigned long)(nowtm.tm_hour * 3600 + nowtm.tm_min * 60 + nowtm.tm_sec);
}

static inline unsigned int
get_exp(unsigned int now)
{
	unsigned int  end;
	unsigned int  exp;

	end = evcurr->time;
	
	if (now > end) {
		exp = end + SECS_PER_DAY - now;
	} else {
		exp = end - now;
	}
	return (exp > SECS_PER_DAY) ? exp - SECS_PER_DAY : exp;
}

typedef pztnode_t* (*PNL)(struct list_hdr *);

static pztnode_t*
get_ts0node(struct list_hdr *list)
{
	return U_CONTAINER_OF(list, pztnode_t, list0);
}

static pztnode_t*
get_ts1node(struct list_hdr *list)
{
	return U_CONTAINER_OF(list, pztnode_t, list1);
}

static PNL  get_pznode[] = {get_ts0node, get_ts1node};

/* i: timeslice, opt: arm or disarm. */
static void
pzone_tmropt(int i, int opt)
{
	pztnode_t	*node;
	struct list_hdr *list;

	DBG("--- timeslice-%d ---\n", i);
	if (!u_list_empty(thead[i])) {
		DBG("--- timeslice-%d not empty ---\n", i);
		list = thead[i]->next;
		while (list != thead[i]) {
			node = (*get_pznode[i])(list);
			DBG("---- pzone-%d: %s of time slice-%d: %s ---\n", node->pnr, (opt == 0)?"end":"start", i, (opt == 0)?"disarm":"arm");
			(*pztmr_opt[opt])(node->pnr-1);
			list = list->next;
		}
	}
}

static void
pztimer_opt(struct ev_loop *loop, ev_timer *w, int revents)
{
	unsigned int     tm;
	unsigned int     exp;
	int    i, opt;

	/* i: timeslice, opt: arm or disarm. */
	i = (evcurr->opt >> 1) & 1;
	opt = evcurr->opt & 1;
	tm = evcurr->time;

	pzone_tmropt(i, opt);

	evcurr = evcurr->next;
	if (evcurr->time < tm) {
		exp = evcurr->time + SECS_PER_DAY - tm;
	} else {
		exp = evcurr->time - tm;
	}
	DBG("--- timer trigger after %u secs ---\n", exp);
	pz_timer_add(exp, pztimer_opt);
}

static void
_start_pztimer(void)
{
	unsigned int  now;
	unsigned int  exp;

	now = get_daytm(NULL);
	evnode_set_current(now);
	exp = get_exp(now);
	if ((evcurr->opt & 1) == 0) {
		DBG("--- timer arm ---\n");
		pzone_tmropt((evcurr->opt>>1) & 1, 1);
	} else if ((evcurr->opt & 1) == 1) {
		DBG("--- timer disarm ---\n");
		pzone_tmropt((evcurr->opt>>1) & 1, 0); //modified on 2013.12.30
	}
	DBG("--- timer trigger after %u secs ---\n", exp);
	pz_timer_add(exp, pztimer_opt);
}

static int
pznode_list_init(pztm_t *pzt, unsigned short *flg)
{
	int    i;
	int    p;
	int    r = 0;
	pztnode_t *pznode;
	unsigned short  f = 0;

	for (i = 0; i < tpzcnt; ++i) {
		pznode = &tpznode[i];
		init_pznode(pznode);
		p = tpzarr[i];
		pznode->pnr = p;
		pznode->tflag = pzt->pt[i];
		if (pztimer_enabled(p-1)) {
			if (pzt->pt[i] & 1) {
				u_list_append(&pznode->list0, &thead0);
				++r;
			}

			if (pzt->pt[i] & 2) {
				u_list_append(&pznode->list1, &thead1);
				++r;
			}
			f |= (1 << (p - 1));
		}
	}

	if (flg) {
		*flg = f;
	}

	return r;
}

static int
start_pz_timer(void)
{
	unsigned short flg = 0;
	pztm_t  *pzt = get_pztm_conf();

	DBG("--- start pzone timer ---\n");
	if (g_tflag == 0||pzt->npzt == 0) return 1;
	evnode_list_init();
	if (evnode_list_empty()) return 1;
	if (pznode_list_init(pzt, &flg) > 0) {
		_start_pztimer();
	}
	pztmrflg = flg;
	return 0;
}

void
stop_pz_timer(void)
{
	int  r;
	int  i;

	r = pz_timer_remove();
	for (i = 0; i < tpzcnt; ++i) {
		pzone_tmrdisarm(tpznode[i].pnr-1);
	}

	if (r == 0) {
		if (!u_list_empty(&thead0)) {
			u_list_del(&thead0);
		}

		if (!u_list_empty(&thead1)) {
			u_list_del(&thead1);
		}

		tmrpzmap = 0;
		DBG("--- stop the pzone timer ---\n");
	}
}

int
pztimer_init(unsigned int var)
{
	unsigned short v = var & ARG_MASK;
	int  r = 0;

	if (v == 0) {
		stop_pz_timer();
		pztmrflg = 0;
	} else {
		r = start_pz_timer();
	}
	return r;
}

/* i: index/position of pzone in list, not the pzone number. */
int
enable_pztimer(unsigned int var)
{
	unsigned short i = var & ARG_MASK;
	pztnode_t *node = &tpznode[i];

	DBG("--- trying to enable the timer pzone-%d ---\n", node->pnr);
	if ((g_tflag == 0)||node->pnr == 0) return 1;
	if (pztimer_enabled(node->pnr-1)) return 0;
	DBG("--- enable the timer pzone-%d ---\n", node->pnr);
	if ((node->tflag&1) == 1) {
		u_list_append(&node->list0, &thead0);
	}

	if ((node->tflag&2) == 2) {
		u_list_append(&node->list1, &thead1);
	}
	set_timer_pzflg(node->pnr-1);
	netxfer_pztmr(i, 1);
	_start_pztimer();
	set_tpz_flag(i, 1);

	return 0;
}

/* i: index/position of pzone in list, not the pzone number. */
int
disable_pztimer(unsigned int var)
{
	unsigned short i = var & ARG_MASK;
	pztnode_t *node = &tpznode[i];

	if (node->pnr == 0) return 1;
	if (!pztimer_enabled(node->pnr-1)) return 0;
	DBG("--- disable the timer pzone-%d ---\n", node->pnr);
	if (!u_list_empty(&node->list0)&&(node->tflag&1) == 1) {
		u_list_del(&node->list0);
	}

	if (!u_list_empty(&node->list1)&&(node->tflag&2) == 2) {
		u_list_del(&node->list1);
	}

	netxfer_pztmr(i, 0);
	pzone_tmrdisarm(node->pnr-1);
	clr_timer_pzflg(node->pnr-1);
	set_tpz_flag(i, 0);
	if (u_list_empty(&thead0)&&u_list_empty(&thead1)) {
		DBG("--- no timer pzone and remove the timer ---\n");
		pz_timer_remove();
		pztmrflg = 0;
	}

	return 0;
}

void
tmer_opt(int op, int i)
{
	if (op)
		set_timer_pzflg(tpzarr[i]);
	else
		clr_timer_pzflg(tpzarr[i]);
	set_tpz_flag(i, !!op); //implemented in tmr.c
}


#if 0
}
#endif


