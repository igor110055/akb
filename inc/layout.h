#ifndef _UI_CONF_H_
#define _UI_CONF_H_

#define  LCD_W           1024
#define  LCD_H           600

#define HOME0            "/opt/ui/lbl_home0.png"
#define HOME1            "/opt/ui/lbl_home1.png"

#define row_h            40

#define bn_bar_h         42
#define bn_bar_y         (600-68)

#define LBL_UNKNOWN     "/opt/ui/zst_unknown.png"
#define LBL_DISARM      "/opt/ui/zst_disarm.png"
#define LBL_NRDY        "/opt/ui/zst_nrdy.png"
#define LBL_BYPASS      "/opt/ui/zst_bypass.png"
#define LBL_ARM         "/opt/ui/zst_arm.png"
#define LBL_ALARM       "/opt/ui/zst_alarm.png"
#define LBL_ALARMR      "/opt/ui/zst_alarmr.png"

/////////////////// keybar ///////////////////////
#define BG_KEYBAR        "/opt/ui/bg_keybar.png"
#define KEY00            "/opt/ui/key_00.png"
#define KEY10            "/opt/ui/key_10.png"
#define KEY20            "/opt/ui/key_20.png"
#define KEY30            "/opt/ui/key_30.png"
#define KEY40            "/opt/ui/key_40.png"
#define KEY50            "/opt/ui/key_50.png"
#define KEY60            "/opt/ui/key_60.png"
#define KEY70            "/opt/ui/key_70.png"
#define KEY80            "/opt/ui/key_80.png"
#define KEY90            "/opt/ui/key_90.png"
#define KEYP0            "/opt/ui/key_p0.png"
#define KEYOK0           "/opt/ui/key_ok0.png"
#define KEYCL0           "/opt/ui/key_cl0.png"

#define KEY01            "/opt/ui/key_01.png"
#define KEY11            "/opt/ui/key_11.png"
#define KEY21            "/opt/ui/key_21.png"
#define KEY31            "/opt/ui/key_31.png"
#define KEY41            "/opt/ui/key_41.png"
#define KEY51            "/opt/ui/key_51.png"
#define KEY61            "/opt/ui/key_61.png"
#define KEY71            "/opt/ui/key_71.png"
#define KEY81            "/opt/ui/key_81.png"
#define KEY91            "/opt/ui/key_91.png"
#define KEYP1            "/opt/ui/key_p1.png"
#define KEYOK1           "/opt/ui/key_ok1.png"
#define KEYCL1           "/opt/ui/key_cl1.png"

#define key_x1           0
#define key_x2           76
#define key_x3           (key_x2+71)
#define key_x4           (key_x3+71)
#define key_x5           (key_x4+71)
#define key_x6           (key_x5+71)
#define key_x7           (key_x6+71)
#define key_x8           (key_x7+71)
#define key_x9           (key_x8+71)
#define key_x0           (key_x9+71)
#define keyp_x           (key_x0+71)
#define keyok_x          (keyp_x+71)
#define keycl_x          (keyok_x+71)

#define keybar_w          932
#define keybar_h          58

///////////////////////////////////////////
#define BG_CHGZNR         "/opt/ui/bg_chgznr.png"
#define BG_DELDIAG        "/opt/ui/bg_deldiag.png"
#define BN_DELOK0         "/opt/ui/bn_delok0.png"
#define BN_DELOK1         "/opt/ui/bn_delok1.png"
#define BN_DELCL0         "/opt/ui/bn_delcl0.png"
#define BN_DELCL1         "/opt/ui/bn_delcl1.png"

#define deldiag_w         331
#define deldiag_h         146

#define deldiag_bn_w      116
#define deldiag_bn_h      38

#define deldiag_bn_yoff   80
#define deldiag_bn_xoff0  (deldiag_w-deldiag_bn_w*2)
#define deldiag_bn_xoff1  (deldiag_bn_xoff0+deldiag_bn_w)

///////////////////////////////////////////
#define  BN_RET1500       "/opt/ui/bn_ret1500.png"
#define  BN_RET1501       "/opt/ui/bn_ret1501.png"
#define  BN_GOK0          "/opt/ui/bn_genok0.png"
#define  BN_GOK1          "/opt/ui/bn_genok1.png"

#define  BN_RET184        "/opt/ui/bn_ret184.png"
#define  BN_PR182         "/opt/ui/bn_pr182.png"
#define  BN_PR156         "/opt/ui/bn_pr156.png"
#define  BN_DEL182        "/opt/ui/bn_del182.png"
#define  BN_DEL156        "/opt/ui/bn_del156.png"
#define  BN_PU182         "/opt/ui/bn_pu182.png"
#define  BN_PD182         "/opt/ui/bn_pd182.png"

#define  BN_PU180         "/opt/ui/bn_pu180.png"
#define  BN_PD180         "/opt/ui/bn_pd180.png"

#define  BN_RET160        "/opt/ui/bn_ret160.png"
#define  BN_PR109         "/opt/ui/bn_pr109.png"
#define  BN_DEL109        "/opt/ui/bn_del109.png"
#define  BN_PU142         "/opt/ui/bn_pu142.png"
#define  BN_PD142         "/opt/ui/bn_pd142.png"

#define  BN_RET156        "/opt/ui/bn_ret156.png"

#define  BN_RET182        "/opt/ui/bn_ret182.png"
#define  BN_RET136        "/opt/ui/bn_ret136.png"
#define  BN_RET182G       "/opt/ui/bn_ret182g.png"
#define  BN_RET160G       "/opt/ui/bn_ret160g.png"
#define  BN_RET136G       "/opt/ui/bn_ret136g.png"
#define  BN_RET1180       "/opt/ui/bn_ret1180.png"
#define  BN_RET118G       "/opt/ui/bn_ret118g.png"

#define  BN_PU132         "/opt/ui/bn_pu132.png"
#define  BN_PD131         "/opt/ui/bn_pd131.png"

#define  BN_DEL156G       "/opt/ui/bn_del156g.png"

#define  BN_QRY109        "/opt/ui/bn_qry109.png"
#define  BN_QRY156        "/opt/ui/bn_qry156.png"
#define  BN_CKV141        "/opt/ui/bn_ckv141.png"

#define  bbar_h           42
#define  bbar_y           (600-68)
#define  bbar_x           56

#define  ICO_DOTB         "/opt/ui/dot_14x14.png"
#define  ICO_DOTS         "/opt/ui/dot_7x8.png"

//////////////////////////////////////////////////
#define  BG_TIP           "/opt/ui/bg_tip.png"
#define  tip_w            332
#define  tip_h            92

#define  LBL_PWDERR       "/opt/ui/lbl_pwderr.png"
#define  LBL_AARMOK       "/opt/ui/lbl_aarmok.png"
#define  LBL_ADAMOK       "/opt/ui/lbl_adamok.png"
#define  LBL_BYPSOK       "/opt/ui/lbl_bypsok.png"
#define  LBL_PRTERR       "/opt/ui/lbl_prterr.png"
#define  LBL_ZINVLD       "/opt/ui/lbl_zinvld.png"
#define  LBL_PINVLD       "/opt/ui/lbl_pinvld.png"
#define  LBL_TINVLD       "/opt/ui/lbl_tinvld.png"
#define  LBL_NOMAP        "/opt/ui/lbl_nomap.png"
#define  LBL_NOZONE       "/opt/ui/lbl_nozone.png"
#define  LBL_UNDEFZ       "/opt/ui/lbl_undefz.png"
#define  LBL_REDEFZ       "/opt/ui/lbl_redefz.png"
#define  LBL_ARMED        "/opt/ui/lbl_armed.png"
#define  LBL_DISARMED     "/opt/ui/lbl_disarmed.png"

#define  LBL_NSYSP0       "/opt/ui/alt_nsysp.png"
#define  LBL_NSYSP1       "/opt/ui/arm_nsysp.png"

//////////////////////////////////////////////////

#define  gret_w           150

//////////////////////////////////////////////////
#define  BN_SV150         "/opt/ui/bn_sv150.png"
#define  BN_CL150         "/opt/ui/bn_cl150.png"

#define  bn_sys_w         150
#define  bn_sys_h         42

#define  bn_sys_y        (599-65)

#define  bn_sv_x         (1023-355)
#define  bn_cancel_x     (bn_sv_x + 150)
#define  bn_gret_x       (bn_sv_x - gret_w)


//////////////////// DONE ////////////////////////
#define  ICO_CROSS         "/opt/ui/cross.png"
#define  cross_w           25
#define  cross_h           25
#define  horizon_valv      50
#define  vertical_valv     50

#define  BN_LOGIN0         "/opt/ui/bn_login0.png"
#define  BN_LOGIN1         "/opt/ui/bn_login1.png"

#define  LBL_USR           "/opt/ui/lbl_usr.png"

#define  bn_login_x        660
#define  bn_login_y        354

//#define  login_usr_w       107
#define  login_usr_w       160
#define  login_usr_h       34
#define  login_usr_x       547
#define  login_usr_y       (310-login_usr_h)

#define  login_ent_w       248
#define  login_ent_h       33
#define  login_ent_x       298
#define  login_ent_y0      276
#define  login_ent_y1      353

//////////////////// DONE /////////////////////
#define  BG_MAIN           "/opt/ui/bg_start.jpg"

#define  LED_0        "/opt/ui/led_0.png"
#define  LED_1        "/opt/ui/led_1.png"
#define  LED_2        "/opt/ui/led_2.png"
#define  LED_3        "/opt/ui/led_3.png"
#define  LED_4        "/opt/ui/led_4.png"
#define  LED_5        "/opt/ui/led_5.png"
#define  LED_6        "/opt/ui/led_6.png"
#define  LED_7        "/opt/ui/led_7.png"
#define  LED_8        "/opt/ui/led_8.png"
#define  LED_9        "/opt/ui/led_9.png"
#define  LED_C        "/opt/ui/led_c.png"

#define  colon_x      ((770-28)/2)
#define  colon_y      ((192-68)/2+194)

#define  led_w         58
#define  led_h         92
#define  led_y        ((192-led_h)/2+194)
#define  led_x0       (colon_x-116)
#define  led_x1       (colon_x+28)


//////////////// sidebar //////////////////////
#define  SBAR_ARM          "/opt/ui/sidebar_arm1.png"
#define  SBAR_BP           "/opt/ui/sidebar_byp1.png"
#define  SBAR_MAP          "/opt/ui/sidebar_map1.png"

#define  sidebar_w         210
#define  sidebar_h         57

#define  sbar_x           (1023-231)
#define  sbar_y0           156
#define  sbar_y1          (sbar_y0+92)
#define  sbar_y2          (sbar_y1+92)
////////////////////////////////////////////////

//////////////  bottom menubar  ////////////////
#define  menubar_h         57
#define  menubar_y         (599-menubar_h-1)
#define  menubar_w0        171
#define  menubar_w1        171
#define  menubar_w2        171
#define  menubar_w3        171
#define  menubar_w4        171
#define  menubar_w5        169

#define  menubar_x0        0
#define  menubar_x1        (menubar_x0+menubar_w0)
#define  menubar_x2        (menubar_x1+menubar_w1)
#define  menubar_x3        (menubar_x2+menubar_w2)
#define  menubar_x4        (menubar_x3+menubar_w3)
#define  menubar_x5        (menubar_x4+menubar_w4)

#define  BN_ELOG         "/opt/ui/bn_elog1.png"
#define  BN_ALOG         "/opt/ui/bn_alog1.png"
#define  BN_SYSC         "/opt/ui/bn_sysc1.png"
#define  BN_ZINF         "/opt/ui/bn_zinf1.png"
#define  BN_SYSF         "/opt/ui/bn_sysf1.png"
#define  BN_SYSP         "/opt/ui/bn_sysp1.png"


////////////////////////////////////////////////////////
#define  BG_AMPWD         "/opt/ui/bg_ampwd.jpg"

#define  BG_FARMD         "/opt/ui/farm_dialog.png"

//#define  dialog_bn_y      81
#define  dialog_bn_y      89

#define  arm_kb_x         100
#define  arm_kb_y         244

#define  BN_ALLARM0       "/opt/ui/bn_aarm0.png"
#define  BN_ALLARM1       "/opt/ui/bn_aarm1.png"

#define  BN_ALLDISARM0    "/opt/ui/bn_adisarm0.png"
#define  BN_ALLDISARM1    "/opt/ui/bn_adisarm1.png"

#define  BN_PARTARM0      "/opt/ui/bn_parm0.png"
#define  BN_PARTARM1      "/opt/ui/bn_parm1.png"

#define  BN_PDISARM0      "/opt/ui/bn_pdisarm0.png"
#define  BN_PDISARM1      "/opt/ui/bn_pdisarm1.png"

#define  BN_ZARM0         "/opt/ui/bn_zarm0.png"
#define  BN_ZARM1         "/opt/ui/bn_zarm1.png"

#define  BN_ZDISARM0      "/opt/ui/bn_zdisarm0.png"
#define  BN_ZDISARM1      "/opt/ui/bn_zdisarm1.png"

#define  BN_TMRDN0        "/opt/ui/bn_tmrdn0.png"
#define  BN_TMRDN1        "/opt/ui/bn_tmrdn1.png"

#define  BN_TMREN0        "/opt/ui/bn_tmren0.png"
#define  BN_TMREN1        "/opt/ui/bn_tmren1.png"

#define  am_bn_w0        130
#define  am_bn_w1        128
#define  am_bn_h         46

#define  am_bn_x0          636
#define  am_bn_x1          (am_bn_x0+131)
#define  am_bn_y0          160
#define  am_bn_y1          (am_bn_y0+88)
#define  am_bn_y2          (am_bn_y1+86)
#define  am_bn_y3          (am_bn_y2+87)

#define  BN_DEL            "/opt/ui/bn_bsp1.png"
#define  bn_del_x          358
#define  bn_del_y          164
#define  bn_del_w          57
#define  bn_del_h          38

#define  pwd_ent_w         194
#define  pwd_ent_h         46

#define  pwd_ent_x         120
#define  pwd_ent_y         160

#define  pwd_off_x         15
#define  pwd_off_y         ((pwd_ent_h - 16)/2)

////////////////////////////////////////////////////////
#define  KB_NUM0      "/opt/ui/kb_01.png"
#define  KB_NUM1      "/opt/ui/kb_11.png"
#define  KB_NUM2      "/opt/ui/kb_21.png"
#define  KB_NUM3      "/opt/ui/kb_31.png"
#define  KB_NUM4      "/opt/ui/kb_41.png"
#define  KB_NUM5      "/opt/ui/kb_51.png"
#define  KB_NUM6      "/opt/ui/kb_61.png"
#define  KB_NUM7      "/opt/ui/kb_71.png"
#define  KB_NUM8      "/opt/ui/kb_81.png"
#define  KB_NUM9      "/opt/ui/kb_91.png"
#define  KB_NUM10     "/opt/ui/kb_x1.png"
#define  KB_NUM11     "/opt/ui/kb_sharp1.png"

#define  KP_NUM0      "/opt/ui/kp_01.png"
#define  KP_NUM1      "/opt/ui/kp_11.png"
#define  KP_NUM2      "/opt/ui/kp_21.png"
#define  KP_NUM3      "/opt/ui/kp_31.png"
#define  KP_NUM4      "/opt/ui/kp_41.png"
#define  KP_NUM5      "/opt/ui/kp_51.png"
#define  KP_NUM6      "/opt/ui/kp_61.png"
#define  KP_NUM7      "/opt/ui/kp_71.png"
#define  KP_NUM8      "/opt/ui/kp_81.png"
#define  KP_NUM9      "/opt/ui/kp_91.png"
#define  KP_NUM10     "/opt/ui/kp_x1.png"
#define  KP_NUM11     "/opt/ui/kp_sharp1.png"

////////////////////////////////////////////////////////
#define  BG_BPENT     "/opt/ui/bg_bpent.jpg"

#define  BN_DEL1      "/opt/ui/bn_del1.png"
#define  bp_kb_x      102
#define  bp_kb_y      244

#define  BN_ZBPASS    "/opt/ui/bn_zbpass.png"
#define  BN_ZBPASSR   "/opt/ui/bn_zbpassr.png"
#define  BN_PBPASS    "/opt/ui/bn_pbpass.png"
#define  BN_PBPASSR   "/opt/ui/bn_pbpassr.png"

#define  bp_bn_w0     130
#define  bp_bn_w1     156
#define  bp_bn_h      46

#define  bp_bn_x0     621
#define  bp_bn_x1     (bp_bn_x0 + bp_bn_w0)

#define  bp_bn_y0     161
#define  bp_bn_y1     (bp_bn_y0 + 83)

////////////////////////////////////////////////////////

///////////////////////////////////////////////////////
#define  LBL_NETUNCONN   "/opt/ui/st_uncon.png"
#define  LBL_NETCONN     "/opt/ui/st_con.png"

#define  SLV1_0          "/opt/ui/slv1_0.png"
#define  SLV1_1          "/opt/ui/slv1_1.png"

#define  SLV2_0          "/opt/ui/slv2_0.png"
#define  SLV2_1          "/opt/ui/slv2_1.png"

#define  SLV3_0          "/opt/ui/slv3_0.png"
#define  SLV3_1          "/opt/ui/slv3_1.png"

#define  SLV4_0          "/opt/ui/slv4_0.png"
#define  SLV4_1          "/opt/ui/slv4_1.png"

#define  SLV5_0          "/opt/ui/slv5_0.png"
#define  SLV5_1          "/opt/ui/slv5_1.png"

#define  SLV6_0          "/opt/ui/slv6_0.png"
#define  SLV6_1          "/opt/ui/slv6_1.png"

#define  SLV7_0          "/opt/ui/slv7_0.png"
#define  SLV7_1          "/opt/ui/slv7_1.png"

#define  SLV8_0          "/opt/ui/slv8_0.png"
#define  SLV8_1          "/opt/ui/slv8_1.png"

#define  SLV9_0          "/opt/ui/slv9_0.png"
#define  SLV9_1          "/opt/ui/slv9_1.png"

#define  st_net_x        (1024-110)
#define  st_net_y        ((39-25)/2)

#define  clock_x         ((39-16)/2)    
#define  clock_y         clock_x

#if 0
#define  slv_x           (1024-276)
#define  slv_y           ((39-26)/2)
#define  slv_itvl        39
#else
#define  slv_itvl        34
#define  slv_x           (st_net_x-8-9*slv_itvl)
#define  slv_y           ((39-26)/2)
#endif

////////////////////////////////////////////////////////
#define  sys_h         50
#define  sys_y         69

#define  sys_w0        174
#define  sys_w1        206
#define  sys_w2        166
#define  sys_w3        144
#define  sys_w4        124

#define  sys_x0        97
#define  sys_x1        275
#define  sys_x2        (sys_x1+210)
#define  sys_x3        (sys_x2+170)
#define  sys_x4        (sys_x3+148)

#define  SYS_CBOX          "/opt/ui/sys_cbox1.png"
#define  sys_checkbox_w    36
#define  sys_checkbox_h    35
#define  sys_cbox_y        388
#define  sys_cbox_x0       163
#define  sys_cbox_x1       324
#define  sys_cbox_x2       520
#define  sys_cbox_x3       (1023-303)

#define  BG_SYSNET         "/opt/ui/bg_sysnet.jpg"
#define  sysnet_kbar_y     246
#define  ipent_offsetx     16
#define  ipent_offsety     ((30-16)/2)
#define  ipent_w           170
#define  ipent_h           31

#define  ipent_y           205
#define  ipent_x0          200
#define  ipent_x1          493
#define  ipent_x2          767

#define  ipent1_w          188

#define  ipent1_x0         303
#define  ipent1_x1         702
//#define  ipent1_y0         314
#define  ipent1_y0         313
//#define  ipent1_y1         422
#define  ipent1_y1         406


#define  BG_SYSUART        "/opt/ui/bg_sysuart.jpg"

#define  sysu_kbar_y       150
#define  uent_w0           68
#define  uent_w1           96
#define  uent_h            31

#define  uent_y0           220
#define  uent_y1           305
#define  uent_x0           299
#define  uent_x1           684
#define  uent_x2           488
#define  uent_x3           659

#define  BG_SYSGEN         "/opt/ui/bg_sysgen.jpg"
#define  ICO_ATIP00        "/opt/ui/ico_atip00.png"
#define  ICO_ATIP01        "/opt/ui/ico_atip01.png"
#define  ICO_ATIP10        "/opt/ui/ico_atip10.png"
#define  ICO_ATIP11        "/opt/ui/ico_atip11.png"
#define  ICO_ATIP20        "/opt/ui/ico_atip20.png"
#define  ICO_ATIP21        "/opt/ui/ico_atip21.png"
#define  ICO_ATIP30        "/opt/ui/ico_atip30.png"
#define  ICO_ATIP31        "/opt/ui/ico_atip31.png"

#define  TIP_TSCAL         "/opt/ui/tip_tscal.png"

#define  ICO_ETIP00        "/opt/ui/ico_etip00.png"
#define  ICO_ETIP01        "/opt/ui/ico_etip01.png"
#define  ICO_ETIP10        "/opt/ui/ico_etip10.png"
#define  ICO_ETIP11        "/opt/ui/ico_etip11.png"
#define  ICO_ETIP20        "/opt/ui/ico_etip20.png"
#define  ICO_ETIP21        "/opt/ui/ico_etip21.png"

#define  BG_ATIP           "/opt/ui/bg_atip.png"
#define  BG_ETIP           "/opt/ui/bg_etip.png"

#define  sysg_tip_w         72
#define  sysg_tip_h         34
#define  sysg_tip_y         186
#define  sysg_atip_x        416
#define  sysg_etip_x        627

#define  sysg_kbar_y        120
#define  gent_h             31
#define  aent_w             68
#define  yent_w             78
#define  tent_w             52

#define  gent_y0            188
#define  gent_y1            321
#define  gent_y2            387

#define  aent_x             207
#define  vent_x             862
#define  nent_x             268

#define  tent_x0            207
#define  tent_x1            324
#define  tent_x2            413
#define  tent_x3            655
#define  tent_x4            752
#define  tent_x5            845

#define  sysgen_cbox_y      319
#define  sysgen_cbox_x0     376
#define  sysgen_cbox_x1     539
#define  sysgen_cbox_x2     713

#define  sysmap_w          278
#define  sysmap_h          48
#define  sysmap_x          80
#define  sysmap_y          446

//////////////////////////////////////////////
#define  BG_SYSTMR           "/opt/ui/bg_systmr.jpg"

#define  systmr_kbar_y       (164-keybar_h)
#define  pzent_w     56
#define  pzent_h     31
#define  pzent_x     158
#define  pzent_y0    172
#define  pzent_y1    228
#define  pzent_y2    285
#define  pzent_y3    341
#define  pzent_y4    398
#define  pzent_y5    454

#define  ptent_w       68
#define  ptent_h       31
#define  ptent_x0      (1024-324)
#define  ptent_x1      (1024-179)
#define  ptent_y0      217
#define  ptent_y1      330

#define  systmr_cbox_x0      368
#define  systmr_cbox_x1      (systmr_cbox_x0+188)
#define  systmr_cbox_y0      169
#define  systmr_cbox_y1      (systmr_cbox_y0+56)
#define  systmr_cbox_y2      (systmr_cbox_y1+57)
#define  systmr_cbox_y3      (systmr_cbox_y2+56)
#define  systmr_cbox_y4      (systmr_cbox_y3+57)
#define  systmr_cbox_y5      (systmr_cbox_y4+56)

#define  BG_SYSALR           "/opt/ui/bg_sysalr.jpg"
#define  BN_SND              "/opt/ui/bn_snd.png"
#define  sysalr_kbar_y       360
#define  azent_w             56
#define  azent_h             31
#define  azent_x             401
#define  azent_y             310

#define  sysalr_cbox_y0      217
#define  sysalr_cbox_y1      308

#define  sysalr_cbox_x0      514
#define  sysalr_cbox_x1      627
#define  sysalr_cbox_x2      534
#define  sysalr_cbox_x3      (sysalr_cbox_x2+155)

////////////////////////////////////////////////////////

#define  BG_ZINFO       "/opt/ui/bg_zinfo.jpg"
#define ziid_col_x       56
#define ziid_col_y       107
#define ziid_col_w       100
#define ZILB_X           158
#define ZILB_Y           ziid_col_y
//#define ZILB_W           (1024-(ZILB_X+56))
#define ZILB_W           810

#define zinf_col_x1      ((110-36)/2)
#define zinf_col_x2      ((330-270)/2+112)
#define zinf_col_x3      ((110-20)/2+444)
#define zinf_col_x4      ((254-160)/2+444+112)

#define ARROW_UP        "/opt/ui/arrow_up.png"
#define ARROW_DN        "/opt/ui/arrow_dn.png"
//#define zarrow_x        232
#define zarrow_x        236
//#define parrow_x        676
#define parrow_x        683
#define arrow_y         77

#define  BG_EINFO       "/opt/ui/bg_einfo.jpg"

#define  BG_EQRY        "/opt/ui/bg_eqry.png"
#define  qry_ent_yw     58
#define  qry_ent_tw     46
#define  qry_ent_h      50

#define  qry_tip_y      ((tip_h-16)/2)

#define  qry_ent_x0     (172+(1024-678)/2)
#define  qry_ent_x1     (258+(1024-678)/2)
#define  qry_ent_x2     (332+(1024-678)/2)
#define  qry_ent_x3     (404+(1024-678)/2)
#define  qry_ent_x4     (476+(1024-678)/2)
#define  qry_ent_x5     (548+(1024-678)/2)

#define  qry_ent_y0     (70+(600-370)/2)
#define  qry_ent_y1     (154+(600-370)/2)

#define  eqry_bn_y      ((600-370)/2+322)
#define  eqry_cl_x      ((1024-678)/2+189)
#define  eqry_ok_x      512
#define  eqry_kbar_y    ((600-370)/2+235)

#define  bn_scroll_w    182
#define  bn_ret_w       184

#define  bn_ret_x       421
#define  bn_up_x        (bn_ret_x + bn_ret_w)
#define  bn_dn_x        (bn_up_x + bn_scroll_w)

#define  eid_col_x      140
#define  eid_col_y      107
#define  eid_col_w      (282 - eid_col_x)

//#define  ELB_X          284
#define  ELB_X          eid_col_x
#define  ELB_Y          eid_col_y
//#define  ELB_W          684
#define  ELB_W         (684+eid_col_w+2)


/*
#define  elb_col_x0     8
#define  elb_col_x1     ((296-190)/2+388)
*/

#define  elb_col_x0     (eid_col_w-60)/2
#define  elb_col_x1     ((eid_col_w+2)+8)
#define  elb_col_x2     ((296-190)/2+388+eid_col_w+2)

#define  BN_SEL0        "/opt/ui/bn_csel0.png"
#define  BN_SEL1        "/opt/ui/bn_csel1.png"

#define  einf_sel_x     (((eid_col_x-56)-27)/2+56)
#define  einf_sel_y     ((row_h-28)/2+eid_col_y)

#define  BG_EVT         "/opt/ui/bg_evt.png"

//////////////////////////////////////////////////////////
#define  BG_AINFO       "/opt/ui/bg_ainf.jpg"
#define  BG_AQRY        "/opt/ui/bg_aqry.png"

#define  ALB_ROWS       10

#define  ALB_X          56
#define  ALB_Y          107
#define  ALB_W          (1024-(56+56))

#define  alb_col_x0     ((114-48)/2)
#define  alb_col_x1     ((64-32)/2+116)
#define  alb_col_x2     ((310-240)/2+66+116)
#define  alb_col_x3     ((64-16)/2+378+116)
#define  alb_col_x4     ((110-96)/2+444+116)
#define  alb_col_x5     ((190-152)/2+556+116)

#define  bn_abar_w0     160
#define  bn_abar_w1     109
#define  bn_abar_w2     109
#define  bn_abar_w3     109
#define  bn_abar_w4     141
#define  bn_abar_w5     142
#define  bn_abar_w6     142

#define  bn_abar_x0      56
#define  bn_abar_x1     (bn_abar_x0 + bn_abar_w0)
#define  bn_abar_x2     (bn_abar_x1 + bn_abar_w1)
#define  bn_abar_x3     (bn_abar_x2 + bn_abar_w2)
#define  bn_abar_x4     (bn_abar_x3 + bn_abar_w3)
#define  bn_abar_x5     (bn_abar_x4 + bn_abar_w4)
#define  bn_abar_x6     (bn_abar_x5 + bn_abar_w5)

/////////////////////////////////////////////////////////
#define  BG_PZUI        "/opt/ui/bg_pzui.jpg"

//#define  BG_PTARM       "/opt/ui/bg_ptarm.jpg"

#define  BN_PARM0        "/opt/ui/bn_ptarm0.png"
#define  BN_PARM1        "/opt/ui/bn_ptarm1.png"
#define  BN_PZDISARM0     "/opt/ui/bn_ptdisarm0.png"
#define  BN_PZDISARM1     "/opt/ui/bn_ptdisarm1.png"

#define  bn_ptbar_y      (599-68)

#define  bn_ptbar_w0     182
#define  bn_ptbar_w1     186
#define  bn_ptbar_w2     185
#define  bn_ptbar_w3     180
#define  bn_ptbar_w4     180

#define  bn_ptbar_x0      55
#define  bn_ptbar_x1     (bn_ptbar_x0 + bn_ptbar_w0)
#define  bn_ptbar_x2     (bn_ptbar_x1 + bn_ptbar_w1)
#define  bn_ptbar_x3     (bn_ptbar_x2 + bn_ptbar_w2)
#define  bn_ptbar_x4     (bn_ptbar_x3 + bn_ptbar_w3)

#define  pid_col_x         56
#define  pid_col_y         107
#define  pid_col_w         74

#define  PZLB_X             132
#define  PZLB_Y             pid_col_y
#define  PZLB_W             656
#define  PZLB_H             400

/* offset from listbox */
#define  pzlb_col_x1        ((128-20)/2)
#define  pzlb_col_x2        ((526-300)/2+130)

#define  pst_col_x       790

#define  PST_ALARM0      "/opt/ui/pst_alarm0.png"
#define  PST_ALARM1      "/opt/ui/pst_alarm1.png"
#define  PST_ARM0        "/opt/ui/pst_arm0.png"
#define  PST_ARM1        "/opt/ui/pst_arm1.png"
#define  PST_DISARM0     "/opt/ui/pst_disarm0.png"
#define  PST_DISARM1     "/opt/ui/pst_disarm1.png"
#define  PST_ALARMR0     "/opt/ui/pst_alrmr0.png"
#define  PST_ALARMR1     "/opt/ui/pst_alrmr1.png"
#define  PST_BYPASS0     "/opt/ui/pst_bypass0.png"
#define  PST_BYPASS1     "/opt/ui/pst_bypass1.png"
#define  PST_PTARM0      "/opt/ui/pst_ptarm0.png"
#define  PST_PTARM1      "/opt/ui/pst_ptarm1.png"
#define  PST_NRDY0       "/opt/ui/pst_nrdy0.png"
#define  PST_NRDY1       "/opt/ui/pst_nrdy1.png"
#define  PST_PTBYPASS0   "/opt/ui/pst_ptbypass0.png"
#define  PST_PTBYPASS1   "/opt/ui/pst_ptbypass1.png"
#define  PST_TMRARM0     "/opt/ui/pst_tmrarm0.png"
#define  PST_TMRARM1     "/opt/ui/pst_tmrarm1.png"
#define  PST_TMRDISARM0  "/opt/ui/pst_tmrdisarm0.png"
#define  PST_TMRDISARM1  "/opt/ui/pst_tmrdisarm1.png"
#define  PST_UNKNOWN0    "/opt/ui/pst_unknown0.png"
#define  PST_UNKNOWN1    "/opt/ui/pst_unknown1.png"
#define  PST_FRCARM0     "/opt/ui/pst_frcarm0.png"
#define  PST_FRCARM1     "/opt/ui/pst_frcarm1.png"

///////////////////////////////////////////////////////
#define  BG_PZCK        "/opt/ui/bg_pzck.jpg"


///////////////////////////////////////////////////////
#define  BG_ZONEUI      "/opt/ui/bg_zoneui.jpg"
#define  BG_ZARM        "/opt/ui/bg_zarm.jpg"

#define  BN_ZZARM0        "/opt/ui/bn_zzarm0.png"
#define  BN_ZZARM1        "/opt/ui/bn_zzarm1.png"
#define  BN_ZZDISARM0     "/opt/ui/bn_zzdisarm0.png"
#define  BN_ZZDISARM1     "/opt/ui/bn_zzdisarm1.png"

#define  BN_ZNRINP      "/opt/ui/bn_znrinp1.png"

#define  ICO_ZSEL0      "/opt/ui/ico_zunsel.png"
#define  ICO_ZSEL1      "/opt/ui/ico_zsel.png"

#define  bn_zbar_w0     136
#define  bn_zbar_w1     291
#define  bn_zbar_w2     111
#define  bn_zbar_w3     111
#define  bn_zbar_w4     132
#define  bn_zbar_w5     131

#define  bn_zbar_x0      56
#define  bn_zbar_x1     (bn_zbar_x0 + bn_zbar_w0)
#define  bn_zbar_x2     (bn_zbar_x1 + bn_zbar_w1)
#define  bn_zbar_x3     (bn_zbar_x2 + bn_zbar_w2)
#define  bn_zbar_x4     (bn_zbar_x3 + bn_zbar_w3)
#define  bn_zbar_x5     (bn_zbar_x4 + bn_zbar_w4)

#define  zid_col_x         56
#define  zid_col_y         107
//#define  zid_col_w         100
#define  zid_col_w         84

#define  zst_col_w          178
#define  zst_col_x         (782+(186-zst_col_w)/2)

//#define  zarm_lb_x          158
#define  zarm_lb_x          142
#define  zarm_lb_y          zid_col_y
//#define  zarm_lb_w          622
#define  zarm_lb_w          690

/* offset from listbox */
#define  zarm_lb_col_x1        ((330-300)/2)
#define  zarm_lb_col_x2        ((110-40)/2+332)
#define  zarm_lb_col_x3        ((178-160)/2+444)

/////////////////////////////////////////////////////////
#define  BG_TMR         "/opt/ui/bg_tmr.jpg"
#define  BN_TMRCK       "/opt/ui/bn_tmrck1.png"

#define  ICO_TRIA       "/opt/ui/ico_tria.png"
#define  ICO_UNSEL      "/opt/ui/ico_ivld.png"
#define  ICO_SEL        "/opt/ui/ico_vld.png"

#define  bn_tbar_h       42
#define  bn_tbar_y       (LCD_H-68)

#define  bn_tret_w       182
#define  bn_tmrck_w      231
#define  bn_tret_x       (1023-469)
#define  bn_tmrck_x      (bn_tret_x + bn_tret_w)

//////////////////////////////////////////////////////////
#define  BG_TMRCK        "/opt/ui/bg_tmrck.jpg"

#define  PTARM_SEL       "/opt/ui/ptarm_sel.png"
#define  PTARM_UNSEL     "/opt/ui/ptarm_unsel.png"

#define  bn_tcret_w      136
#define  bn_tcret_x      56

#define  tmr_lbl_h       16
#define  tmr_lbl_w       100
#define  tmr_lbl_y       ((600-60) + (26 - tmr_lbl_h)/2)

#define  tmr_lbl_x0       (323 + (222 - tmr_lbl_w)/2)
#define  tmr_lbl_x1       (711 + (222 - tmr_lbl_w)/2)

#define  BG_BYPASS       "/opt/ui/bg_bypass.jpg"

#define  BN_BPASS0        "/opt/ui/bn_bypass0.png"
#define  BN_BPASS1        "/opt/ui/bn_bypass1.png"
#define  BN_BPASSR0       "/opt/ui/bn_bypassr0.png"
#define  BN_BPASSR1       "/opt/ui/bn_bypassr1.png"

#define  bn_bpret_w      182
#define  bn_bypass_w     180

#define  bn_bpret_x      426
#define  bn_bypass_x     (bn_bpret_x+bn_bpret_w)
#define  bn_bypassr_x    (bn_bypass_x+bn_bypass_w)

//////////////////////////////////////////////////////////
#define  BG_ZBYPASS      "/opt/ui/bg_zbypass.jpg"

#define  BN_ZBYPASS0     "/opt/ui/bn_zbypass0.png"
#define  BN_ZBYPASS1     "/opt/ui/bn_zbypass1.png"
#define  BN_ZBYPASSR0    "/opt/ui/bn_zbypassr0.png"
#define  BN_ZBYPASSR1    "/opt/ui/bn_zbypassr1.png"
#define  BN_ZBPPUP       "/opt/ui/bn_zbppu1.png"
#define  BN_ZBPPDN       "/opt/ui/bn_zbppd1.png"

#define  zbar_h           42
#define  zbar_y           (600-26-zbar_h)

#define  zbar_w0          136
#define  zbar_w1          291
#define  zbar_w2          89
#define  zbar_w3          133
#define  zbar_w4          132
#define  zbar_w5          131

#define  zbar_x0          56
#define  zbar_x1         (zbar_x0+zbar_w0)
#define  zbar_x2         (zbar_x1+zbar_w1)
#define  zbar_x3         (zbar_x2+zbar_w2)
#define  zbar_x4         (zbar_x3+zbar_w3)
#define  zbar_x5         (zbar_x4+zbar_w4)

////////////////////////////////////////////////////////////
#define  BG_SYST         "/opt/ui/bg_syst.jpg"
#define  BG_PZST         "/opt/ui/bg_pzst.jpg"

#define  pzst_bar_h        42
#define  pzst_bar_y       (LCD_H-68)

#define  pzst_bar_w0       184
#define  pzst_bar_w1       182
#define  pzst_bar_w2       182

#define  pzst_bar_x0       (1024-604)
#define  pzst_bar_x1       (pzst_bar_x0 + pzst_bar_w0)
#define  pzst_bar_x2       (pzst_bar_x1 + pzst_bar_w1)

///////////////////////////////////////////////////////////////////

#define  BG_AINFCK       "/opt/ui/bg_ainfck.jpg"
#define  BN_AIHDL        "/opt/ui/bn_aihdl1.png"
#define  BN_AIHDLA       "/opt/ui/bn_aihdla1.png"
#define  BN_MAPCK        "/opt/ui/bn_mapck1.png"
#define  BN_AICKPU0      "/opt/ui/bn_aickpu0.png"
#define  BN_AICKPU1      "/opt/ui/bn_aickpu1.png"
#define  BN_AICKPD0      "/opt/ui/bn_aickpd0.png"
#define  BN_AICKPD1      "/opt/ui/bn_aickpd1.png"

#define  ainfck_bar_h    42
#define  ainfck_bar_y   (LCD_H-66)

#define  ainfck_bar_w0   160
#define  ainfck_bar_w1   156
#define  ainfck_bar_w2   156
#define  ainfck_bar_w3   156
#define  ainfck_bar_w4   142
#define  ainfck_bar_w5   142

#define  ainfck_bar_x0   56
#define  ainfck_bar_x1   (ainfck_bar_x0+ainfck_bar_w0)
#define  ainfck_bar_x2   (ainfck_bar_x1+ainfck_bar_w1)
#define  ainfck_bar_x3   (ainfck_bar_x2+ainfck_bar_w2)
#define  ainfck_bar_x4   (ainfck_bar_x3+ainfck_bar_w3)
#define  ainfck_bar_x5   (ainfck_bar_x4+ainfck_bar_w4)

#define  zaid_col_x      56
#define  zaid_col_y      267
#define  zaid_col_w      114

//#define  ZALB_X          172
#define  ZALB_X          57
#define  ZALB_Y          zaid_col_y
//#define  ZALB_W          796
#define  ZALB_W          912


/* offset from listbox */
/*
#define  zalb_col_x0        ((64-30)/2)
#define  zalb_col_x1        ((310-300)/2+66)
#define  zalb_col_x2        ((64-20)/2+378)
#define  zalb_col_x3        ((110-100)/2+444)
#define  zalb_col_x4        ((240-190)/2+556)
*/

#define  zalb_col_x0        ((114-60)/2)
#define  zalb_col_x1        ((64-32)/2+116)
#define  zalb_col_x2        ((310-300)/2+66+116)
#define  zalb_col_x3        ((64-20)/2+378+116)
#define  zalb_col_x4        ((110-100)/2+444+116)
#define  zalb_col_x5        ((240-190)/2+556+116)

/////////////////////// DONE ///////////////////////
#define  DIALOG_MSK     "/opt/ui/bg_mask.png"
#define  CDIALOG_BG     "/opt/ui/bg_imxpwd.png"

#define  YES_BN0        "/opt/ui/imxbn_ok0.png"
#define  YES_BN1        "/opt/ui/imxbn_ok1.png"
#define  NO_BN0         "/opt/ui/imxbn_cancel0.png"
#define  NO_BN1         "/opt/ui/imxbn_cancel1.png"

#define  dialog_w       432
#define  dialog_h       220

#define  imx_bn_y       ((LCD_H-dialog_h)/2+145)
#define  imx_ok_x       (223+(LCD_W-dialog_w)/2)
#define  imx_cl_x       (imx_ok_x+95)

/////////////////////////////////////////////////////

#define  pwd_kb_x      (10 + (LCD_W-dialog_w)/2)
#define  pwd_kb_y      (10 + (LCD_H-dialog_h)/2)

#define  imxpwd_ent_w   173
#define  imxpwd_ent_h   30

#define  imxpwd_ent_x   (231 + (LCD_W-dialog_w)/2)
#define  imxpwd_ent_y   (72 + (LCD_H-dialog_h)/2)

#define  imxpwd_off_x    12
#define  imxpwd_off_y   ((imxpwd_ent_h-16)/2)

#define  IMXKB_NUM1      "/opt/ui/imxkb_11.png"
#define  IMXKB_NUM2      "/opt/ui/imxkb_21.png"
#define  IMXKB_NUM3      "/opt/ui/imxkb_31.png"
#define  IMXKB_NUM4      "/opt/ui/imxkb_41.png"
#define  IMXKB_NUM5      "/opt/ui/imxkb_51.png"
#define  IMXKB_NUM6      "/opt/ui/imxkb_61.png"
#define  IMXKB_NUM7      "/opt/ui/imxkb_71.png"
#define  IMXKB_NUM8      "/opt/ui/imxkb_81.png"
#define  IMXKB_NUM9      "/opt/ui/imxkb_91.png"
#define  IMXKB_STAR      "/opt/ui/imxkb_x1.png"
#define  IMXKB_NUM0      "/opt/ui/imxkb_01.png"
#define  IMXKB_CLR       "/opt/ui/imxkb_clr1.png"
#define  IMXKB_DOT       "/opt/ui/imxkb_dot.png"

#define  w_1        64
#define  w_2        66
#define  w_3        64

#define  h_1        49
#define  h_2        50
#define  h_3        50

//////////////////////////////////////////////////
//               系统编程界面                   //
//////////////////////////////////////////////////
#define  BG_SYSP          "/opt/ui/bg_sysp.jpg"
#define  BN_ZPRG          "/opt/ui/bn_zprg.png"
#define  BN_CHRS          "/opt/ui/bn_chrs.png"

/*
#define  sysp_bar_w       182
#define  sysp_bar_h       42
#define  sysp_bar_y       (LCD_H-66)

#define  sysp_bar_x0      445
#define  sysp_bar_x1      (sysp_bar_x0+160)
#define  sysp_bar_x2      (sysp_bar_x1+182)
*/

#define  sysp_bar_y       (LCD_H-66)
#define  sysp_bar_h       42

#define  sysp_bar_w0      160
#define  sysp_bar_w1      142
#define  sysp_bar_w2      160
#define  sysp_bar_w3      182
#define  sysp_bar_w4      182

#define  sysp_bar_x0      144
#define  sysp_bar_x1      (sysp_bar_x0 + sysp_bar_w0)
#define  sysp_bar_x2      (sysp_bar_x1 + sysp_bar_w1)
#define  sysp_bar_x3      (sysp_bar_x2 + sysp_bar_w2)
#define  sysp_bar_x4      (sysp_bar_x3 + sysp_bar_w3)


//////////////////////////////////////////////////
#define  BG_USRM          "/opt/ui/bg_usrm.jpg"
#define  BN_UADD          "/opt/ui/bn_uadd1.png"
#define  BN_USAVE         "/opt/ui/bn_usv1.png"

#define  ULB_W            702
#define  ULB_H            (row_h*8)
#define  ULB_X            249
#define  ULB_Y            173

#define  ulb_col_x0       60
#define  ulb_col_x1       (287+60)

#define  uid_col_x        73
#define  uid_col_y        ULB_Y
#define  uid_col_w        174

#define  um_bar_h          42
#define  um_bar_y          (LCD_H-66)

#define  um_bar_w0         160
#define  um_bar_w1         156
#define  um_bar_w2         156
#define  um_bar_w3         142

#define  um_bar_x0         354
#define  um_bar_x1         (um_bar_x0+um_bar_w0)
#define  um_bar_x2         (um_bar_x1+um_bar_w1)
#define  um_bar_x3         (um_bar_x2+um_bar_w2)

#define  tmap_w      144
#define  tmap_h      50
#define  tmap_y      69
#define  tmap_x0     97
#define  tmap_x1     245

//////////////////////////////////////////////////
#define  BG_PWDCFG         "/opt/ui/bg_pwdcfg.jpg"

#define  BN_PWDSV          "/opt/ui/bn_pwdsv1.png"
#define  BN_PWDCL          "/opt/ui/bn_pwdcl1.png"
#define  BN_SEL            "/opt/ui/bn_sel1.png"

#define  ucfg_ent_w        102
#define  ucfg_ent_h        30
#if 0
#define  ucfg_ent_y        172
#else
#define  ucfg_ent_y        147
#endif
#define  ucfg_pwd_x        455
#define  ucfg_usr_x       (1024-261)

#define  uid_lbl_x         ((ucfg_ent_w-10)/2 + 168)
#define  uid_lbl_y         ((ucfg_ent_h-16)/2 + ucfg_ent_y)
#define  uid_lbl_w         10
#define  uid_lbl_h         16

#define  ucfg_kbar_y       234

#define  pwdmap_w          144
#define  pwdmap_h          49
#define  pwdmap_x          245
#define  pwdmap_y          69

#define  pwdcfg_bar_y      (LCD_H-67)
#define  pwdcfg_bar_h      42

#define  pwdcfg_bar_w0     160
#define  pwdcfg_bar_w1     160
#define  pwdcfg_bar_w2     142

#define  pwdcfg_bar_x0     (666-160)
#define  pwdcfg_bar_x1     666
#define  pwdcfg_bar_x2     (pwdcfg_bar_x1 + pwdcfg_bar_w1)

#define  psel_w            32
#define  psel_h            32

#if 0
#define  psel_x0           227
#define  psel_x1           445
#define  psel_x2           664
#define  psel_x3           883
#define  psel_x4           227
#define  psel_x5           445

#define  psel_y0           321
#define  psel_y1           (psel_y0+79)

#else
#define  psel_x0           200
#define  psel_x1           438
#define  psel_x2           652
#define  psel_x3           876
#define  psel_x4           438
#define  psel_x5           652

#define  psel_y0           253
#define  psel_y1           315

#endif

///////////////////////////////////////////////////////////////////
#define  BG_GEN            "/opt/ui/bg_gen.jpg"

#define  BN_TOMAP          "/opt/ui/bn_tomap.png"
#define  BN_GENSV          "/opt/ui/bn_gensv1.png"
#define  BN_GENCL          "/opt/ui/bn_gencl1.png"

#define  gcfg_kbar_y       398
#define  gcfg_ent_w        99
#define  gcfg_ent_h        30
#define  gcfg_ent_y        167
#define  gcfg_ent_x0       245
#define  gcfg_ent_x1       540
#define  gcfg_ent_x2       832

#define  genmap_w          144
#define  genmap_h          49
#define  genmap_x          97
#define  genmap_y          69

#define  tomap_w           174
#define  tomap_h           48
#define  tomap_x           77
#define  tomap_y           (599-200)

#define  gen_bar_w0        160
#define  gen_bar_w1        144
#define  gen_bar_w2        144

#define  gen_bar_h         42
#define  gen_bar_y         (LCD_H-66)
#define  gen_bar_x0        520
#define  gen_bar_x1        (gen_bar_x0+gen_bar_w0)
#define  gen_bar_x2        (gen_bar_x1+gen_bar_w1)

#define  gsel_x0           328
#define  gsel_x1           603
#define  gsel_x2           878
#define  gsel_x3           278
#define  gsel_x4           471
#define  gsel_x5           664
#define  gsel_x6           878

#define  gsel_y0           243
#define  gsel_y1           (gsel_y0+88)

///////////////////////////////////////////////////////////////////
#define  BG_ZMGR           "/opt/ui/bg_zmgr.jpg"

#define  BN_ZADD           "/opt/ui/bn_zadd1.png"
#define  BN_ZMOD           "/opt/ui/bn_zmod1.png"
#define  BN_ZDEL           "/opt/ui/bn_zdel1.png"
#define  BN_ZPU            "/opt/ui/bn_zpu1.png"
#define  BN_ZPD            "/opt/ui/bn_zpd1.png"

#define  id_col_x          56
#define  id_col_y          107
//#define  id_col_w          130
#define  id_col_w          100

//#define  ZLB_X             188
#define  ZLB_X             158
#define  ZLB_Y             107
//#define  ZLB_W             780
#define  ZLB_W             810
#define  ZLB_H             400

/* offset from listbox */
#if 0
#define  zlb_col_x1        ((136-32)/2)
#define  zlb_col_x2        ((400-300)/2+270-132)
#define  zlb_col_x3        ((240-160)/2+672-132)
#else
#define  zlb_col_x1        ((136-32)/2)
#define  zlb_col_x2        ((400-300)/2+270-132)
#define  zlb_col_x3        ((240-160)/2+672-132)
#endif

#define  zmbar_w0          160
#define  zmbar_w1          156
#define  zmbar_w2          156
#define  zmbar_w3          156
#define  zmbar_w4          142
#define  zmbar_w5          142

#define  zmbar_h           42
#define  zmbar_y           (LCD_H-66)

#define  zmbar_x0          56
#define  zmbar_x1         (zmbar_x0+zmbar_w0)
#define  zmbar_x2         (zmbar_x1+zmbar_w1)
#define  zmbar_x3         (zmbar_x2+zmbar_w2)
#define  zmbar_x4         (zmbar_x3+zmbar_w3)
#define  zmbar_x5         (zmbar_x4+zmbar_w4)

///////////////////////////////////////////////////////////////////
#define  BG_ZCFG         "/opt/ui/bg_zcfg.jpg"
#define  BG_ZT           "/opt/ui/bg_zt.png"

#define  DM_DZ0          "/opt/ui/dm_dz0.png"
#define  DM_DZ1          "/opt/ui/dm_dz1.png"
#define  DM_IZ0          "/opt/ui/dm_iz0.png"
#define  DM_IZ1          "/opt/ui/dm_iz1.png"
#define  DM_AZ0          "/opt/ui/dm_az0.png"
#define  DM_AZ1          "/opt/ui/dm_az1.png"

#define  id_lbl_w        44
#define  id_lbl_h        16
#define  id_lbl_x        (183+16)
#define  id_lbl_y        (106+7)

#define  BN_DM           "/opt/ui/bn_dm1.png"
#define  bn_dm_x         643
#define  bn_dm_y         104
#define  bn_dm_w         36
#define  bn_dm_h         35

#define  znr_w           76
#define  znr_h           30
#define  znr_x           183
#define  znr_y           172

#define  pnr_w           znr_w
#define  pnr_h           znr_h
#define  pnr_x           znr_x
#define  pnr_y           234

#define  dnm_w           111
#define  dnm_h           30
#define  dnm_x           389
#define  dnm_y           106

#define  znm_w           266
#define  znm_h           30
#define  znm_x           412
#define  znm_y           172

#define  pnm_w           znm_w
#define  pnm_h           znm_h
#define  pnm_x           znm_x
#define  pnm_y           234

#define  vip_w           191
#define  vip_h           30
#define  vip_x           486
#define  vip_y           298

#define  zcfg_kbar_y     408

#define  zcfg_zt_x       ((117-96)/2+526)
#define  zcfg_zt_y       ((46-34)/2+98)
#define  zcfg_zt_w        96
#define  zcfg_zt_h        34

#define  zcfg_bar_h      42
#define  zcfg_bar_y      (LCD_H-66)

#define  zcfg_bar_w0     160
#define  zcfg_bar_w1     160
#define  zcfg_bar_w2     142

#define  zcfg_bar_x0     507
#define  zcfg_bar_x1     (zcfg_bar_x0+zcfg_bar_w0)
#define  zcfg_bar_x2     (zcfg_bar_x1+zcfg_bar_w1)

#define  zsel_x0         217
#define  zsel_x1         387
#define  zsel_x2         531

#define  zsel_y0         297
#define  zsel_y1         346

#define  prot_x          182
#define  prot_y0         345
#define  prot_y1         391
#define  prot_y2         438

///////////////////////////////////////////////////////////////////
#define  BG_MAP            "/opt/ui/mapedit.png"

#define  BN_OZT0           "/opt/ui/bn_oz0.png"
#define  BN_OZT1           "/opt/ui/bn_oz1.png"
#define  BN_IZT0           "/opt/ui/bn_iz0.png"
#define  BN_IZT1           "/opt/ui/bn_iz1.png"
#define  BN_MCL0           "/opt/ui/bn_mcl0.png"
#define  BN_MCL1           "/opt/ui/bn_mcl1.png"
#define  BN_MOK0           "/opt/ui/bn_mok0.png"
#define  BN_MOK1           "/opt/ui/bn_mok1.png"
#define  BN_MZNR0          "/opt/ui/bn_mznr0.png"
#define  BN_MZNR1          "/opt/ui/bn_mznr1.png"
#define  BN_MMOD0          "/opt/ui/bn_mmod0.png"
#define  BN_MMOD1          "/opt/ui/bn_mmod1.png"
#define  BN_MDIS0          "/opt/ui/bn_mdis0.png"
#define  BN_MDIS1          "/opt/ui/bn_mdis1.png"
#define  BN_MDEL0          "/opt/ui/bn_mdel0.png"
#define  BN_MDEL1          "/opt/ui/bn_mdel1.png"


#define  mbar_w0          118
#define  mbar_w1          162
#define  mbar_w2          162
#define  mbar_w3          81
#define  mbar_w4          81
#define  mbar_w5          146
#define  mbar_w6          81
#define  mbar_w7          81

#define  mbar_h           42
#define  mbar_y           (599-70)

#define  mbar_x0          55
#define  mbar_x1         (mbar_x0+mbar_w0)
#define  mbar_x2         (mbar_x1+mbar_w1)
#define  mbar_x3         (mbar_x2+mbar_w2)
#define  mbar_x4         (mbar_x3+mbar_w3)
#define  mbar_x5         (mbar_x4+mbar_w4)
#define  mbar_x6         (mbar_x5+mbar_w5)
#define  mbar_x7         (mbar_x6+mbar_w6)

////////////////////////////////////////////////////
#define BG_SLV    "/opt/ui/bg_slv.jpg"
#define BG_HOST   "/opt/ui/bg_host.jpg"
#define XOFF0     ((98-48)/2)
#define XOFF1     ((184-180)/2)
#define YOFF      ((42-24)/2)

#define MAXI      15


#endif


