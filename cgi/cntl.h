#ifndef CNTL_H_
#define CNTL_H_


#define  LCONF    "/opt/conf/lc.conf"
#define  BCONF    "/opt/conf/bgsnd.conf"
#define  LKCNF    "/opt/conf/lnk.conf"
#define  SCONF    "/opt/conf/site.conf"
#define  DEVCNF   "/opt/conf/dev.conf"

#define LOC_MAX   16


/////////////////////////////////////

extern char  g_loc[LOC_MAX][16];
extern int   g_locCnt;
extern int   g_bsCnt;
extern unsigned char g_bsAddr[16];
extern unsigned char g_bsLoc[16];


extern void   read_site(int i);
extern char* trim(char *str);
extern void  set_reboot_flag();
extern void  clear_reboot_flag();
extern int   get_reboot_flag();
extern void  do_finish();

extern void  handle_security();
//////////////////////////////////////
extern void  handle_cdev(void);
extern void  show_cdev_page(void);

//////////////////////////////////////
//extern void  load_bsinfo();
extern void  handle_breq(void);
extern void  show_bgsnd_page(void);

extern void  show_scene_page(void);
extern void  handle_scene();

extern void  show_lnk_page(void);
extern void  handle_slnk();

#endif


