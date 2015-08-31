#include "mxc_sdk.h"
#include "ulib.h"
#include "pzone.h"
#include "zdelay.h"
#include "zopt.h"

typedef struct _ztimer {
	struct list_hdr  list;
	ev_timer  w;
	int       idx;
} timer_z;

// new add for exit delay and entrance delay
unsigned int  zodelay[ZBNUM] = {0};        //1bit represents a zone.0: cannot alarm, 1: can alarm. used for exit delay.
unsigned int  zidelay[ZBNUM] = {0};        //alert flag: 1bit represents a zone.0: no alert, 1: alert. used for enter delay.
unsigned int  zarflag[ZBNUM] = {0};        //alarm restore flag in entering delay mode.
unsigned int  zalert[ZBNUM] = {0};

static timer_z  *tmrz[MAXZ] = {NULL};
static struct list_hdr     freez = {&freez, &freez};
static struct list_hdr     zhead = {&zhead, &zhead};


static timer_z*
z_timer_new(int z)
{
	timer_z *tz;
	struct list_hdr *list;

	if (!u_list_empty(&freez)) {
		list = freez.next;
		u_list_del(list);
		tz = (timer_z *)list;
		tz->idx = z;
	} else {
		tz = calloc(1, sizeof(timer_z));
		if (tz) {
			tz->idx = z;
			U_INIT_LIST_HEAD(&tz->list);
		}
	}
	tmrz[z] = tz;
	if (tz) {
		u_list_append(&tz->list, &zhead);
	}
	return tz;
}

static void
z_timer_free(timer_z *tz)
{
	if (!u_list_empty(&tz->list)) {
		u_list_del(&tz->list);
	}
	tmrz[tz->idx] = NULL;
	memset(tz, 0, sizeof(timer_z));
	u_list_append(&tz->list, &freez);
}

/* called when quit the zopt_loop */
void
z_timer_cleanup(void)
{
	struct list_hdr *list;
	timer_z  *tz;

	list = zhead.next;
	while (list != &zhead) {
		tz = (timer_z*)list;
		list = list->next;
		z_timer_free(tz);
	}
}

static int
add_ztimer(int z, void *cb, int t)
{
	timer_z *tz;
	tz = z_timer_new(z);
	if (tz == NULL) return -1;
	ev_timer_init(&tz->w, cb, t + 0.0, 0.);
	ztimer_start(&tz->w);
	return 0;
}

static void
qtimer_cb(struct ev_loop *loop, ev_timer *watcher, int revents)
{
	timer_z *tz = U_CONTAINER_OF(watcher, timer_z, w);
	zodelay_clr(tz->idx);
	z_timer_free(tz);
}

static void
itimer_cb(struct ev_loop *loop, ev_timer *watcher, int revents)
{
	timer_z *tz = U_CONTAINER_OF(watcher, timer_z, w);
	(*st_in[ZST_ALARM])(tz->idx);
	netxfer_zopt(tz->idx, ZOP_ALM);
	zidelay_clr(tz->idx);
	if (zarflag_isset(tz->idx)) {
		(*st_out[ZST_ALARM])(tz->idx);
		(*st_in[ZST_ALARMR])(tz->idx);
		netxfer_zopt(tz->idx, ZOP_ALR);
		zarflag_clr(tz->idx);
	}
	z_timer_free(tz);
	DBG("--- exit enter delay mode and alarm!! ---\n");
}

/* called when arming zones before exit. */
void
start_zodelay(int z)
{
	DBG("--- entering exit delay: %d ---\n", odelay);
	zodelay_set(z);
	if (add_ztimer(z, qtimer_cb, odelay) < 0) {
		zodelay_clr(z);
	}
}

/* called when alert before alarm. */
void
start_zidelay(int z)
{
	if (!zidelay_isset(z)) {
		DBG("--- into enter delay mode: %d secs ---\n", idelay);
		add_ztimer(z, itimer_cb, idelay);
		zidelay_set(z);
	}
	zarflag_clr(z);
}

/* called when disarming zones. */
void
finish_zidelay(int z)
{
	timer_z  *tz = tmrz[z];

	if (zone_type(z) == 1&&idelay > 0) {
		if ((tz != NULL)&&zidelay_isset(z)&&(tz->idx == z)) {
			ztimer_stop(&tz->w);
			zarflag_clr(z);
			z_timer_free(tz);
			DBG("--- cancel enter delay ---\n");
		}
	}
	zidelay_clr(z);
}


