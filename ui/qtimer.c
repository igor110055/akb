#include "ulib.h"
#include "gui.h"
#include "config.h"
#include "qtimer.h"

static int    qflag = 0;
static ev_timer   qw;
static int    pfd[2];

static void
timer_cb(struct ev_loop *loop, ev_timer *w, int revents)
{
//	ev_timer_stop(loop, w);
	show_main_win();
}

static void
pipe_cb(struct ev_loop *loop, ev_io *w, int revents)
{
	char  d = 0;

	if (revents & EV_READ) {
		if (read(w->fd, &d, sizeof(d)) == sizeof(d)) {
			switch (d) {
			case 'e':
				DBG("--- received the del timer cmd ---\n");
				if (qflag == 1) {
					ev_timer_stop(loop, &qw);
					qflag = 0;
				}
				break;
			case 'b':
				DBG("--- received the add timer cmd ---\n");
				if (qflag == 0) {
					ev_timer_init(&qw, timer_cb, QTMEOUT, 0.);
					ev_timer_start(loop, &qw);
					qflag = 1;
				}
				break;
			case 'r':
				DBG("--- received the restart timer cmd ---\n");
				if (qflag == 1) {
					ev_timer_stop(loop, &qw);
					ev_timer_init(&qw, timer_cb, QTMEOUT, 0.);
					ev_timer_start(loop, &qw);
				}
				break;
			default:
				break;
			}
		}
	}
}

static void*
qtimer_thread(void* arg)
{
	struct ev_loop *l = NULL;
	static ev_io    pw;

	pthread_detach(pthread_self());
	l = ev_loop_new(EVFLAG_AUTO);
	if (l == NULL) pthread_exit(NULL);

	ev_io_init(&pw, pipe_cb, pfd[0], EV_READ);
	ev_io_start(l, &pw);

	ev_run(l, 0);      

	ev_loop_destroy(l);

	pthread_exit(NULL);
}

static int
start_qtimer(void *arg)
{
	char  cmd = 'b';
	ignore_result(write(pfd[1], &cmd, sizeof(cmd)));
	return 0;
}

static int
stop_qtimer(void *arg)
{
	char  cmd = 'e';
	ignore_result(write(pfd[1], &cmd, sizeof(cmd)));
	return 0;
}

static int
restart_qtimer(void *arg)
{
	char  cmd = 'r';
	ignore_result(write(pfd[1], &cmd, sizeof(cmd)));
	return 0;
}

void 
init_qtimer(void)
{
	pthread_t  tid;

	init_pipe(pfd);
	pthread_create(&tid, NULL, qtimer_thread, NULL);
}

void
register_qtimer(void *w)
{
	utk_widget_glb_evt_handler((UtkWindow*)w, restart_qtimer, (void*)0);
	utk_widget_show_after((UtkWindow*)w, start_qtimer, (void*)0);
	utk_widget_hide_before((UtkWindow*)w, stop_qtimer, (void*)0);
}



