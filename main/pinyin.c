#include <string.h>
#include "gui.h"
#include "pinyin.h"

typedef struct py_index
{
    char *py;
    char *py_mb;
	int   len;
} py_index_t;

//"Æ´ÒôÊäÈë·¨ºº×ÖÅÅÁĞ±í,Âë±í(mb)"
char PY_mb_a[]     ={"°¢°¡"};
char PY_mb_ai[]    ={"°¥°§°¦°£°¤°¨°©°«°ª°¬°®°¯°­"};
char PY_mb_an[]    ={"°²°±°°°³°¶°´°¸°·°µ"};
char PY_mb_ang[]   ={"°¹°º°»"};
char PY_mb_ao[]    ={"°¼°½°¾°¿°À°Á°Â°Ä°Ã"};
char PY_mb_ba[]    ={"°Ë°Í°È°Ç°É°Å°Ì°Æ°Ê°Î°Ï°Ñ°Ğ°Ó°Ö°Õ°Ô"};
char PY_mb_bai[]   ={"°×°Ù°Û°Ø°Ú°Ü°İ°Ş"};
char PY_mb_ban[]   ={"°â°à°ã°ä°ß°á°å°æ°ì°ë°é°ç°è°í°ê"};
char PY_mb_bang[]  ={"°î°ï°ğ°ó°ñ°ò°ö°ø°ô°ù°õ°÷"};
char PY_mb_bao[]   ={"°ü°ú°û°ı±¢±¦±¥±£±¤±¨±§±ª±«±©±¬°ş±¡ÆÙ"};
char PY_mb_bei[]   ={"±°±­±¯±®±±±´±·±¸±³±µ±¶±»±¹±º±²"};
char PY_mb_ben[]   ={"±¼±¾±½±¿º»"};
char PY_mb_beng[]  ={"±À±Á±Â±Ã±Å±Ä"};
char PY_mb_bi[]    ={"±Æ±Ç±È±Ë±Ê±É±Ò±Ø±Ï±Õ±Ó±Ñ±İ±Ğ±Ö±Ô±Í±×±Ì±Î±Ú±Ü±Û"};
char PY_mb_bian[]  ={"±ß±à±Ş±á±â±å±ã±ä±é±æ±ç±è"};
char PY_mb_biao[]  ={"±ë±ê±ì±í"};
char PY_mb_bie[]   ={"±ï±î±ğ±ñ"};
char PY_mb_bin[]   ={"±ö±ò±ó±õ±ô±÷"};
char PY_mb_bing[]  ={"±ù±ø±û±ü±ú±ş±ı²¢²¡"};
char PY_mb_bo[]    ={"²¦²¨²£²§²±²¤²¥²®²µ²¯²´²ª²¬²°²©²³²«²­²²²·"};
char PY_mb_bu[]    ={"²¹²¸²¶²»²¼²½²À²¿²º²¾"};
char PY_mb_ca[]    ={"²Á"};
char PY_mb_cai[]   ={"²Â²Å²Ä²Æ²Ã²É²Ê²Ç²È²Ë²Ì"};
char PY_mb_can[]   ={"²Î²Í²Ğ²Ï²Ñ²Ò²Ó"};
char PY_mb_cang[]  ={"²Ö²×²Ô²Õ²Ø"};
char PY_mb_cao[]   ={"²Ù²Ú²Ü²Û²İ"};
char PY_mb_ce[]    ={"²á²à²Ş²â²ß"};
char PY_mb_ceng[]  ={"²ã²äÔø"};
char PY_mb_cha[]   ={"²æ²å²é²ç²è²ë²ì²ê²í²ï²îÉ²"};
char PY_mb_chai[]  ={"²ğ²ñ²ò"};
char PY_mb_chan[]  ={"²ô²ó²÷²ö²ø²õ²ú²ù²û²ü"};
char PY_mb_chang[] ={"²ı²ş³¦³¢³¥³£³§³¡³¨³©³«³ª"};
char PY_mb_chao[]  ={"³­³®³¬³²³¯³°³±³³³´´Â"};
char PY_mb_che[]   ={"³µ³¶³¹³¸³·³º"};
char PY_mb_chen[]  ={"³»³¾³¼³À³Á³½³Â³¿³Ä³Ã"};
char PY_mb_cheng[] ={"³Æ³Å³É³Ê³Ğ³Ï³Ç³Ë³Í³Ì³Î³È³Ñ³Ò³Ó"};
char PY_mb_chi[]   ={"³Ô³Õ³Ú³Ø³Û³Ù³Ö³ß³Ş³İ³Ü³â³à³ã³á"};
char PY_mb_chong[] ={"³ä³å³æ³ç³è"};
char PY_mb_chou[]  ={"³é³ğ³ñ³ë³î³í³ï³ê³ì³ó³ò³ô"};
char PY_mb_chu[]   ={"³ö³õ³ı³ø³ü³ú³û³÷³ù´¡´¢³ş´¦´¤´¥´£Ğó"};
char PY_mb_chuai[] ={"´§"};
char PY_mb_chuan[] ={"´¨´©´«´¬´ª´­´®"};
char PY_mb_chuang[]={"´³´¯´°´²´´"};
char PY_mb_chui[]  ={"´µ´¶´¹´·´¸"};
char PY_mb_chun[]  ={"´º´»´¿´½´¾´¼´À"};
char PY_mb_chuo[]  ={"´Á"};
char PY_mb_ci[]    ={"´Ã´Ê´Ä´É´È´Ç´Å´Æ´Ë´Î´Ì´Í"};
char PY_mb_cong[]  ={"´Ñ´Ó´Ò´Ğ´Ï´Ô"};
char PY_mb_cou[]   ={"´Õ"};
char PY_mb_cu[]    ={"´Ö´Ù´×´Ø"};
char PY_mb_cuan[]  ={"´Ú´Ü´Û"};
char PY_mb_cui[]   ={"´Ş´ß´İ´à´ã´á´â´ä"};
char PY_mb_cun[]   ={"´å´æ´ç"};
char PY_mb_cuo[]   ={"´ê´è´é´ì´ë´í"};
char PY_mb_da[]    ={"´î´ï´ğ´ñ´ò´ó"};
char PY_mb_dai[]   ={"´ô´õ´ö´ú´ø´ıµ¡´ù´û´ü´ş´÷"};
char PY_mb_dan[]   ={"µ¤µ¥µ£µ¢µ¦µ¨µ§µ©µ«µ®µ¯µ¬µ­µ°µª"};
char PY_mb_dang[]  ={"µ±µ²µ³µ´µµ"};
char PY_mb_dao[]   ={"µ¶µ¼µºµ¹µ·µ»µ¸µ½µ¿µÁµÀµ¾"};
char PY_mb_de[]    ={"µÃµÂµÄ"};
char PY_mb_deng[]  ={"µÆµÇµÅµÈµËµÊµÉ"};
char PY_mb_di[]    ={"µÍµÌµÎµÒµÏµĞµÓµÑµÕµ×µÖµØµÜµÛµİµÚµŞµÙ"};
char PY_mb_dian[]  ={"µàµáµßµäµãµâµçµèµéµêµæµëµíµìµîµå"};
char PY_mb_diao[]  ={"µóµğµòµïµñµõµöµô"};
char PY_mb_die[]   ={"µùµøµüµıµşµúµû"};
char PY_mb_ding[]  ={"¶¡¶£¶¢¶¤¶¥¶¦¶©¶¨¶§"};
char PY_mb_diu[]   ={"¶ª"};
char PY_mb_dong[]  ={"¶«¶¬¶­¶®¶¯¶³¶±¶²¶°¶´"};
char PY_mb_dou[]   ={"¶¼¶µ¶·¶¶¶¸¶¹¶º¶»"};
char PY_mb_du[]    ={"¶½¶¾¶Á¶¿¶À¶Â¶Ä¶Ã¶Ê¶Å¶Ç¶È¶É¶Æ"};
char PY_mb_duan[]  ={"¶Ë¶Ì¶Î¶Ï¶Ğ¶Í"};
char PY_mb_dui[]   ={"¶Ñ¶Ó¶Ô¶Ò"};
char PY_mb_dun[]   ={"¶Ö¶Ø¶Õ¶×¶Ü¶Û¶Ù¶İ"};
char PY_mb_duo[]   ={"¶à¶ß¶á¶Ş¶ä¶â¶ã¶ç¶é¶æ¶è¶å"};
char PY_mb_e[]     ={"¶ï¶í¶ğ¶ë¶ì¶ê¶î¶ò¶ó¶ñ¶ö¶õ¶ô"};
char PY_mb_en[]    ={"¶÷"};
char PY_mb_er[]    ={"¶ù¶ø¶û¶ú¶ı¶ü¶ş·¡"};
char PY_mb_fa[]    ={"·¢·¦·¥·£·§·¤·¨·©"};
char PY_mb_fan[]   ={"·«·¬·­·ª·²·¯·°·³·®·±·´·µ·¸·º·¹·¶··"};
char PY_mb_fang[]  ={"·½·»·¼·À·Á·¿·¾·Â·Ã·Ä·Å"};
char PY_mb_fei[]   ={"·É·Ç·È·Æ·Ê·Ë·Ì·Í·Ï·Ğ·Î·Ñ"};
char PY_mb_fen[]   ={"·Ö·Ô·×·Ò·Õ·Ó·Ø·Ú·Ù·Û·İ·Ü·Ş·ß·à"};
char PY_mb_feng[]  ={"·á·ç·ã·â·è·å·é·æ·ä·ë·ê·ì·í·ï·î"};
char PY_mb_fo[]    ={"·ğ"};
char PY_mb_fou[]   ={"·ñ"};
char PY_mb_fu[]    ={"·ò·ô·õ·ó¸¥·ü·ö·÷·ş·ı·ú¸¡¸¢·û¸¤·ù¸£·ø¸§¸¦¸®¸«¸©¸ª¸¨¸­¸¯¸¸¸¼¸¶¸¾¸º¸½¸À¸·¸´¸°¸±¸µ¸»¸³¸¿¸¹¸²"};
char PY_mb_ga[]    ={"¸Â¸Á"};
char PY_mb_gai[]   ={"¸Ã¸Ä¸Æ¸Ç¸È¸Å"};
char PY_mb_gan[]   ={"¸É¸Ê¸Ë¸Î¸Ì¸Í¸Ñ¸Ï¸Ò¸Ğ¸Ó"};
char PY_mb_gang[]  ={"¸Ô¸Õ¸Ú¸Ù¸Ø¸×¸Ö¸Û¸Ü"};
char PY_mb_gao[]   ={"¸Ş¸á¸ß¸à¸İ¸â¸ã¸å¸ä¸æ"};
char PY_mb_ge[]    ={"¸ê¸í¸ç¸ì¸ë¸î¸é¸è¸ó¸ï¸ñ¸ğ¸ô¸ö¸÷¸õ¿©"};
char PY_mb_gei[]   ={"¸ø"};
char PY_mb_gen[]   ={"¸ù¸ú"};
char PY_mb_geng[]  ={"¸ü¸ı¸û¸ş¹¡¹¢¹£"};
char PY_mb_gong[]  ={"¹¤¹­¹«¹¦¹¥¹©¹¬¹§¹ª¹¨¹®¹¯¹°¹²¹±"};
char PY_mb_gou[]   ={"¹´¹µ¹³¹·¹¶¹¹¹º¹¸¹»"};
char PY_mb_gu[]    ={"¹À¹¾¹Ã¹Â¹Á¹½¹¼¹¿¹Å¹È¹É¹Ç¹Æ¹Ä¹Ì¹Ê¹Ë¹Í"};
char PY_mb_gua[]   ={"¹Ï¹Î¹Ğ¹Ñ¹Ò¹Ó"};
char PY_mb_guai[]  ={"¹Ô¹Õ¹Ö"};
char PY_mb_guan[]  ={"¹Ø¹Û¹Ù¹Ú¹×¹İ¹Ü¹á¹ß¹à¹Ş"};
char PY_mb_guang[] ={"¹â¹ã¹ä"};
char PY_mb_gui[]   ={"¹é¹ç¹ê¹æ¹ë¹è¹å¹ì¹î¹ï¹í¹ô¹ñ¹ó¹ğ¹ò"};
char PY_mb_gun[]   ={"¹õ¹ö¹÷"};
char PY_mb_guo[]   ={"¹ù¹ø¹ú¹û¹ü¹ı"};
char PY_mb_ha[]    ={"¸ò¹ş"};
char PY_mb_hai[]   ={"º¢º¡º£º¥º§º¦º¤"};
char PY_mb_han[]   ={"º¨º©º¬ºªº¯º­º®º«º±º°ººº¹ºµº·º´º¸º¶º³º²"};
char PY_mb_hang[]  ={"º¼º½ĞĞ"};
char PY_mb_hao[]   ={"ºÁºÀº¿º¾ºÃºÂºÅºÆºÄ"};
char PY_mb_he[]    ={"ºÇºÈºÌºÏºÎºÍºÓºÒºËºÉºÔºĞºÊºØºÖºÕº×"};
char PY_mb_hei[]   ={"ºÚºÙ"};
char PY_mb_hen[]   ={"ºÛºÜºİºŞ"};
char PY_mb_heng[]  ={"ºàºßºãºáºâ"};
char PY_mb_hong[]  ={"ºäºåºæºëºìºêºéºçºè"};
char PY_mb_hou[]   ={"ºîºíºïºğºóºñºò"};
char PY_mb_hu[]    ={"ºõºôºö»¡ºüºúºøºşºùº÷ºıºû»¢»£»¥»§»¤»¦"};
char PY_mb_hua[]   ={"»¨»ª»©»¬»«»¯»®»­»°"};
char PY_mb_huai[]  ={"»³»²»´»±»µ"};
char PY_mb_huan[]  ={"»¶»¹»·»¸»º»Ã»Â»½»»»Á»¼»À»¾»¿"};
char PY_mb_huang[] ={"»Ä»Å»Ê»Ë»Æ»Ì»Í»È»Ç»É»Ğ»Î»Ñ»Ï"};
char PY_mb_hui[]   ={"»Ò»Ö»Ó»Ô»Õ»Ø»×»Ú»Ü»ã»á»ä»æ»å»â»ß»Ş»à»İ»Ù»Û"};
char PY_mb_hun[]   ={"»è»ç»é»ë»ê»ì"};
char PY_mb_huo[]   ={"»í»î»ğ»ï»ò»õ»ñ»ö»ó»ô"};
char PY_mb_ji[]    ={"¼¥»÷¼¢»ø»ú¼¡¼¦¼£¼§»ı»ù¼¨¼©»û»ş»ü¼¤¼°¼ª¼³¼¶¼´¼«¼±¼²¼¬¼¯¼µ¼­¼®¼¸¼º¼·¼¹¼Æ¼Ç¼¿¼Í¼Ë¼É¼¼¼Ê¼Á¼¾¼È¼Ã¼Ì¼Å¼Ä¼Â¼À¼»¼½½å"};
char PY_mb_jia[]   ={"¼Ó¼Ğ¼Ñ¼Ï¼Ò¼Î¼Ô¼Õ¼×¼Ö¼Ø¼Û¼İ¼Ü¼Ù¼Ş¼ÚĞ®"};
char PY_mb_jian[]  ={"¼é¼â¼á¼ß¼ä¼ç¼è¼æ¼à¼ã¼ê¼å¼ğ¼ó¼í¼ë¼ñ¼õ¼ô¼ì¼ï¼ò¼î¼û¼ş½¨½¤½£¼ö¼ú½¡½§½¢½¥½¦¼ù¼ø¼ü¼ı"};
char PY_mb_jiang[] ={"½­½ª½«½¬½©½®½²½±½°½¯½³½µ½´"};
char PY_mb_jiao[]  ={"½»½¼½¿½½½¾½º½·½¹½¶½¸½Ç½Æ½Ê½È½Ã½Å½Â½Á½Ë½É½Ğ½Î½Ï½Ì½Ñ½Í¾õ½À"};
char PY_mb_jie[]   ={"½×½Ô½Ó½Õ½Ò½Ö½Ú½Ù½Ü½à½á½İ½Ş½Ø½ß½ã½â½é½ä½æ½ì½ç½ê½ë½è"};
char PY_mb_jin[]   ={"½í½ñ½ï½ğ½ò½î½ó½ö½ô½÷½õ¾¡¾¢½ü½ø½ú½ş½ı½û½ù"};
char PY_mb_jing[]  ={"¾©¾­¾¥¾£¾ª¾§¾¦¾¬¾¤¾«¾¨¾®¾±¾°¾¯¾»¾¶¾·¾º¾¹¾´¾¸¾³¾²¾µ"};
char PY_mb_jiong[] ={"¾¼¾½"};
char PY_mb_jiu[]   ={"¾À¾¿¾¾¾Å¾Ã¾Ä¾Á¾Â¾Æ¾É¾Ê¾Ì¾Î¾Ç¾È¾Í¾Ë"};
char PY_mb_ju[]    ={"¾Ó¾Ğ¾Ñ¾Ô¾Ò¾Ï¾Ö½Û¾Õ¾×¾Ú¾Ù¾Ø¾ä¾Ş¾Ü¾ß¾æ¾ã¾ç¾å¾İ¾à¾â¾Û¾á"};
char PY_mb_juan[]  ={"¾ê¾è¾é¾í¾ë¾î¾ì"};
char PY_mb_jue[]   ={"¾ï¾ö¾÷¾ñ¾ø¾ó¾ò¾ô¾ğ"};
char PY_mb_jun[]   ={"¾ü¾ı¾ù¾û¾ú¿¡¿¤¾ş¿£¿¥¿¢"};
char PY_mb_ka[]    ={"¿§¿¦¿¨"};
char PY_mb_kai[]   ={"¿ª¿«¿­¿®¿¬"};
char PY_mb_kan[]   ={"¼÷¿¯¿±¿°¿²¿³¿´"};
char PY_mb_kang[]  ={"¿µ¿¶¿·¿¸¿º¿¹¿»"};
char PY_mb_kao[]   ={"¿¼¿½¿¾¿¿"};
char PY_mb_ke[]    ={"¿À¿Á¿Â¿Æ¿Ã¿Å¿Ä¿Ç¿È¿É¿Ê¿Ë¿Ì¿Í¿Î"};
char PY_mb_ken[]   ={"¿Ï¿Ñ¿Ò¿Ğ"};
char PY_mb_keng[]  ={"¿Ô¿Ó"};
char PY_mb_kong[]  ={"¿Õ¿×¿Ö¿Ø"};
char PY_mb_kou[]   ={"¿Ù¿Ú¿Û¿Ü"};
char PY_mb_ku[]    ={"¿İ¿Ş¿ß¿à¿â¿ã¿á"};
char PY_mb_kua[]   ={"¿ä¿å¿æ¿è¿ç"};
char PY_mb_kuai[]  ={"¿é¿ì¿ë¿ê"};
char PY_mb_kuan[]  ={"¿í¿î"};
char PY_mb_kuang[] ={"¿ï¿ğ¿ñ¿ö¿õ¿ó¿ò¿ô"};
char PY_mb_kui[]   ={"¿÷¿ù¿ø¿ú¿ü¿û¿ı¿şÀ¢À£À¡"};
char PY_mb_kun[]   ={"À¤À¥À¦À§"};
char PY_mb_kuo[]   ={"À©À¨À«Àª"};
char PY_mb_la[]    ={"À¬À­À²À®À°À¯À±"};
char PY_mb_lai[]   ={"À´À³Àµ"};
char PY_mb_lan[]   ={"À¼À¹À¸À·À»À¶À¾À½ÀºÀÀÀ¿ÀÂÀÁÀÃÀÄ"};
char PY_mb_lang[]  ={"ÀÉÀÇÀÈÀÅÀÆÀÊÀË"};
char PY_mb_lao[]   ={"ÀÌÀÍÀÎÀÏÀĞÀÑÀÔÀÓÀÒ"};
char PY_mb_le[]    ={"ÀÖÀÕÁË"};
char PY_mb_lei[]   ={"À×ÀØÀİÀÚÀÙÀÜÀßÀáÀàÀÛÀŞ"};
char PY_mb_leng[]  ={"ÀâÀãÀä"};
char PY_mb_li[]    ={"ÀåÀæÀêÀëÀòÀçÀìÁ§ÀèÀéÀñÀîÀïÁ¨ÀíÀğÁ¦ÀúÀ÷Á¢ÀôÀöÀûÀøÁ¤ÀıÁ¥ÀşÀóÀõÀùÁ£ÀüÁ¡"};
char PY_mb_lian[]  ={"Á¬Á±Á¯Á°Á«ÁªÁ®Á­Á²Á³Á·Á¶ÁµÁ´"};
char PY_mb_liang[] ={"Á©Á¼Á¹ÁºÁ¸Á»Á½ÁÁÁÂÁ¾ÁÀÁ¿"};
char PY_mb_liao[]  ={"ÁÊÁÉÁÆÁÄÁÅÁÈÁÎÁÃÁÇÁÍÁÏÁÌ"};
char PY_mb_lie[]   ={"ÁĞÁÓÁÒÁÔÁÑ"};
char PY_mb_lin[]   ={"ÁÚÁÖÁÙÁÜÁÕÁØÁ×ÁÛÁİÁßÁŞÁà"};
char PY_mb_ling[]  ={"ÁæÁéÁëÁáÁèÁåÁêÁçÁâÁãÁäÁìÁîÁí"};
char PY_mb_liu[]   ={"ÁïÁõÁ÷ÁôÁğÁòÁóÁñÁöÁøÁù"};
char PY_mb_long[]  ={"ÁúÁüÁıÁûÂ¡ÁşÂ¤Â¢Â£"};
char PY_mb_lou[]   ={"Â¦Â¥Â§Â¨ÂªÂ©"};
char PY_mb_lu[]    ={"Â¶Â¬Â®Â«Â¯Â­Â±Â²Â°Â³Â½Â¼Â¸Â¹Â»ÂµÂ·Â¾ÂºÂ´"};
char PY_mb_luan[]  ={"ÂÏÂÍÂÎÂĞÂÑÂÒ"};
char PY_mb_lue[]   ={"ÂÓÂÔ"};
char PY_mb_lun[]   ={"ÂÕÂØÂ×ÂÙÂÚÂÖÂÛ"};
char PY_mb_luo[]   ={"ÂŞÂÜÂßÂàÂáÂâÂİÂãÂåÂçÂæÂä"};
char PY_mb_lv[]    ={"ÂËÂ¿ÂÀÂÂÂÃÂÁÂÅÂÆÂÄÂÉÂÇÂÊÂÌÂÈ"};
char PY_mb_ma[]    ={"ÂèÂéÂíÂêÂëÂìÂîÂğÂï"};
char PY_mb_mai[]   ={"ÂñÂòÂõÂóÂôÂö"};
char PY_mb_man[]   ={"ÂùÂøÂ÷ÂúÂüÃ¡ÂıÂşÂû"};
char PY_mb_mang[]  ={"Ã¦Ã¢Ã¤Ã£Ã§Ã¥"};
char PY_mb_mao[]   ={"Ã¨Ã«Ã¬Ã©ÃªÃ®Ã­Ã¯Ã°Ã³Ã±Ã²"};
char PY_mb_me[]    ={"Ã´"};
char PY_mb_mei[]   ={"Ã»Ã¶ÃµÃ¼Ã·Ã½ÃºÃ¸Ã¹Ã¿ÃÀÃ¾ÃÃÃÁÃÄÃÂ"};
char PY_mb_men[]   ={"ÃÅÃÆÃÇ"};
char PY_mb_meng[]  ={"ÃÈÃËÃÊÃÍÃÉÃÌÃÏÃÎ"};
char PY_mb_mi[]    ={"ÃÖÃÔÃÕÃÑÃÓÃÒÃ×ÃĞÃÚÃÙÃØÃÜÃİÃÛ"};
char PY_mb_mian[]  ={"ÃßÃàÃŞÃâÃãÃäÃáÃåÃæ"};
char PY_mb_miao[]  ={"ÃçÃèÃéÃëÃìÃêÃîÃí"};
char PY_mb_mie[]   ={"ÃğÃï"};
char PY_mb_min[]   ={"ÃñÃóÃòÃöÃõÃô"};
char PY_mb_ming[]  ={"ÃûÃ÷ÃùÃúÃøÃü"};
char PY_mb_miu[]   ={"Ãı"};
char PY_mb_mo[]    ={"ºÑÃşÄ¡Ä£Ä¤Ä¦Ä¥Ä¢Ä§Ä¨Ä©Ä­Ä°ÄªÄ¯Ä®Ä«Ä¬"};
char PY_mb_mou[]   ={"Ä²Ä±Ä³"};
char PY_mb_mu[]    ={"Ä¸Ä¶ÄµÄ·Ä´Ä¾Ä¿ÄÁÄ¼Ä¹Ä»ÄÀÄ½ÄºÄÂ"};
char PY_mb_na[]    ={"ÄÃÄÄÄÇÄÉÄÈÄÆÄÅ"};
char PY_mb_nai[]   ={"ÄËÄÌÄÊÄÎÄÍ"};
char PY_mb_nan[]   ={"ÄĞÄÏÄÑ"};
char PY_mb_nang[]  ={"ÄÒ"};
char PY_mb_nao[]   ={"ÄÓÄÕÄÔÄÖÄ×"};
char PY_mb_ne[]    ={"ÄØ"};
char PY_mb_nei[]   ={"ÄÚÄÙ"};
char PY_mb_nen[]   ={"ÄÛ"};
char PY_mb_neng[]  ={"ÄÜ"};
char PY_mb_ni[]    ={"ÄİÄáÄàÄßÄŞÄãÄâÄæÄäÄçÄå"};
char PY_mb_nian[]  ={"ÄéÄêÄíÄìÄëÄîÄè"};
char PY_mb_niang[] ={"ÄïÄğ"};
char PY_mb_niao[]  ={"ÄñÄò"};
char PY_mb_nie[]   ={"ÄóÄùÄôÄöÄ÷ÄøÄõ"};
char PY_mb_nin[]   ={"Äú"};
char PY_mb_ning[]  ={"ÄşÅ¡ÄüÄûÄıÅ¢"};
char PY_mb_niu[]   ={"Å£Å¤Å¦Å¥"};
char PY_mb_nong[]  ={"Å©Å¨Å§Åª"};
char PY_mb_nu[]    ={"Å«Å¬Å­"};
char PY_mb_nuan[]  ={"Å¯"};
char PY_mb_nue[]   ={"Å±Å°"};
char PY_mb_nuo[]   ={"Å²ÅµÅ³Å´"};
char PY_mb_nv[]    ={"Å®"};
char PY_mb_o[]     ={"Å¶"};
char PY_mb_ou[]    ={"Å·Å¹Å¸Å»Å¼ÅºÅ½"};
char PY_mb_pa[]    ={"Å¿Å¾ÅÀ°ÒÅÃÅÁÅÂ"};
char PY_mb_pai[]   ={"ÅÄÅÇÅÅÅÆÅÉÅÈ"};
char PY_mb_pan[]   ={"ÅËÅÊÅÌÅÍÅĞÅÑÅÎÅÏ"};
char PY_mb_pang[]  ={"ÅÒÅÓÅÔÅÕÅÖ"};
char PY_mb_pao[]   ={"Å×ÅÙÅØÅÚÅÛÅÜÅİ"};
char PY_mb_pei[]   ={"ÅŞÅßÅãÅàÅâÅáÅæÅåÅä"};
char PY_mb_pen[]   ={"ÅçÅè"};
char PY_mb_peng[]  ={"ÅêÅéÅëÅóÅíÅïÅğÅîÅôÅìÅñÅòÅõÅö"};
char PY_mb_pi[]    ={"±ÙÅúÅ÷ÅûÅøÅüÅùÆ¤ÅşÆ£Æ¡ÅıÆ¢Æ¥Æ¦Æ¨Æ§Æ©"};
char PY_mb_pian[]  ={"Æ¬Æ«ÆªÆ­"};
char PY_mb_piao[]  ={"Æ¯Æ®Æ°Æ±"};
char PY_mb_pie[]   ={"Æ²Æ³"};
char PY_mb_pin[]   ={"Æ´Æ¶ÆµÆ·Æ¸"};
char PY_mb_ping[]  ={"Æ¹Æ½ÆÀÆ¾ÆºÆ»ÆÁÆ¿Æ¼"};
char PY_mb_po[]    ={"ÆÂÆÃÆÄÆÅÆÈÆÆÆÉÆÇ"};
char PY_mb_pou[]   ={"ÆÊ"};
char PY_mb_pu[]    ={"¸¬ÆÍÆËÆÌÆÎÆĞÆÏÆÑÆÓÆÔÆÒÆÖÆÕÆ×ÆØ"};
char PY_mb_qi[]    ={"ÆßÆãÆŞÆâÆàÆÜÆİÆÚÆÛÆáÆîÆëÆäÆæÆçÆíÆêÆéÆèÆïÆåÆìÆòÆóÆñÆôÆğÆøÆıÆùÆúÆûÆüÆõÆöÆ÷"};
char PY_mb_qia[]   ={"ÆşÇ¡Ç¢"};
char PY_mb_qian[]  ={"Ç§ÇªÇ¤Ç¨Ç¥Ç£Ç¦Ç«Ç©Ç°Ç®Ç¯Ç¬Ç±Ç­Ç³Ç²Ç´Ç·ÇµÇ¶Ç¸"};
char PY_mb_qiang[] ={"ÇºÇ¼Ç¹Ç»Ç¿Ç½Ç¾ÇÀ"};
char PY_mb_qiao[]  ={"ÇÄÇÃÇÂÇÁÇÇÇÈÇÅÇÆÇÉÇÎÇÍÇÏÇÌÇËÇÊ"};
char PY_mb_qie[]   ={"ÇĞÇÑÇÒÇÓÇÔ"};
char PY_mb_qin[]   ={"Ç×ÇÖÇÕÇÛÇØÇÙÇİÇÚÇÜÇŞÇß"};
char PY_mb_qing[]  ={"ÇàÇâÇáÇãÇäÇåÇéÇçÇèÇæÇêÇëÇì"};
char PY_mb_qiong[] ={"ÇîÇí"};
char PY_mb_qiu[]   ={"ÇğÇñÇïÇôÇóÇöÇõÇò"};
char PY_mb_qu[]    ={"ÇøÇúÇıÇüÇùÇûÇ÷ÇşÈ¡È¢È£È¥È¤"};
char PY_mb_quan[]  ={"È¦È«È¨ÈªÈ­È¬È©È§È®È°È¯"};
char PY_mb_que[]   ={"È²È±È³È´È¸È·ÈµÈ¶"};
char PY_mb_qun[]   ={"È¹Èº"};
char PY_mb_ran[]   ={"È»È¼È½È¾"};
char PY_mb_rang[]  ={"È¿ÈÂÈÀÈÁÈÃ"};
char PY_mb_rao[]   ={"ÈÄÈÅÈÆ"};
char PY_mb_re[]    ={"ÈÇÈÈ"};
char PY_mb_ren[]   ={"ÈËÈÊÈÉÈÌÈĞÈÏÈÎÈÒÈÑÈÍ"};
char PY_mb_reng[]  ={"ÈÓÈÔ"};
char PY_mb_ri[]    ={"ÈÕ"};
char PY_mb_rong[]  ={"ÈÖÈŞÈ×ÈÙÈİÈÜÈØÈÛÈÚÈß"};
char PY_mb_rou[]   ={"ÈáÈàÈâ"};
char PY_mb_ru[]    ={"ÈçÈãÈåÈæÈäÈêÈéÈèÈëÈì"};
char PY_mb_ruan[]  ={"ÈîÈí"};
char PY_mb_rui[]   ={"ÈïÈñÈğ"};
char PY_mb_run[]   ={"ÈòÈó"};
char PY_mb_ruo[]   ={"ÈôÈõ"};
char PY_mb_sa[]    ={"ÈöÈ÷Èø"};
char PY_mb_sai[]   ={"ÈûÈùÈúÈü"};
char PY_mb_san[]   ={"ÈıÈşÉ¡É¢"};
char PY_mb_sang[]  ={"É£É¤É¥"};
char PY_mb_sao[]   ={"É¦É§É¨É©"};
char PY_mb_se[]    ={"É«É¬Éª"};
char PY_mb_sen[]   ={"É­"};
char PY_mb_seng[]  ={"É®"};
char PY_mb_sha[]   ={"É±É³É´É°É¯ÉµÉ¶É·ÏÃ"};
char PY_mb_shai[]  ={"É¸É¹"};
char PY_mb_shan[]  ={"É½É¾É¼ÉÀÉºÉ¿ÉÁÉÂÉÇÉ»ÉÈÉÆÉÉÉÃÉÅÉÄÕ¤"};
char PY_mb_shang[] ={"ÉËÉÌÉÊÉÑÉÎÉÍÉÏÉĞ"};
char PY_mb_shao[]  ={"ÉÓÉÒÉÕÉÔÉ×ÉÖÉØÉÙÉÛÉÜÉÚ"};
char PY_mb_she[]   ={"ÉİÉŞÉàÉßÉáÉèÉçÉäÉæÉâÉåÉã"};
char PY_mb_shen[]  ={"ÉêÉìÉíÉëÉğÉïÉéÉîÉñÉòÉóÉôÉöÉõÉøÉ÷Ê²"};
char PY_mb_sheng[] ={"ÉıÉúÉùÉüÊ¤ÉûÉşÊ¡Ê¥Ê¢Ê£"};
char PY_mb_shi[]   ={"³×Ê¬Ê§Ê¦Ê­Ê«Ê©Ê¨ÊªÊ®Ê¯Ê±Ê¶ÊµÊ°Ê´Ê³Ê·Ê¸Ê¹Ê¼Ê»ÊºÊ¿ÊÏÊÀÊËÊĞÊ¾Ê½ÊÂÊÌÊÆÊÓÊÔÊÎÊÒÊÑÊÃÊÇÊÁÊÊÊÅÊÍÊÈÊÄÊÉËÆ"};
char PY_mb_shou[]  ={"ÊÕÊÖÊØÊ×ÊÙÊÜÊŞÊÛÊÚÊİ"};
char PY_mb_shu[]   ={"ÊéÊãÊåÊàÊâÊáÊçÊèÊæÊäÊßÊëÊêÊìÊîÊòÊğÊóÊñÊíÊïÊõÊùÊøÊöÊ÷ÊúË¡ÊüÊıÊûÊşÊô"};
char PY_mb_shua[]  ={"Ë¢Ë£"};
char PY_mb_shuai[] ={"Ë¥Ë¤Ë¦Ë§"};
char PY_mb_shuan[] ={"Ë©Ë¨"};
char PY_mb_shuang[]={"Ë«ËªË¬"};
char PY_mb_shui[]  ={"Ë­Ë®Ë°Ë¯"};
char PY_mb_shun[]  ={"Ë±Ë³Ë´Ë²"};
char PY_mb_shuo[]  ={"ËµË¸Ë·Ë¶"};
char PY_mb_si[]    ={"Ë¿Ë¾Ë½Ë¼Ë¹Ë»ËºËÀËÈËÄËÂËÅËÇËÃËÁ"};
char PY_mb_song[]  ={"ËÉËËËÊËÏËÎËĞËÍËÌ"};
char PY_mb_sou[]   ={"ËÔËÑËÒËÓ"};
char PY_mb_su[]    ={"ËÕËÖË×ËßËàËØËÙËÚËÜËİËÛ"};
char PY_mb_suan[]  ={"ËáËâËã"};
char PY_mb_sui[]   ={"ËäËçËåËæËèËêËîËìËéËíËë"};
char PY_mb_sun[]   ={"ËïËğËñ"};
char PY_mb_suo[]   ={"ËôËóËòËõËùË÷ËöËø"};
char PY_mb_ta[]    ={"ËıËûËüËúËşÌ¡Ì¢Ì¤Ì£"};
char PY_mb_tai[]   ={"Ì¥Ì¨Ì§Ì¦Ì«Ì­Ì¬Ì©Ìª"};
char PY_mb_tan[]   ={"Ì®Ì°Ì¯Ì²Ì±Ì³Ì¸ÌµÌ·Ì¶Ì´Ì¹Ì»ÌºÌ¾Ì¿Ì½Ì¼"};
char PY_mb_tang[]  ={"ÌÀÌÆÌÃÌÄÌÁÌÂÌÅÌÇÌÈÌÊÌÉÌÌÌË"};
char PY_mb_tao[]   ={"ÌÎÌĞÌÍÌÏÌÓÌÒÌÕÌÔÌÑÌÖÌ×"};
char PY_mb_te[]    ={"ÌØ"};
char PY_mb_teng[]  ={"ÌÛÌÚÌÜÌÙ"};
char PY_mb_ti[]    ={"ÌŞÌİÌàÌßÌäÌáÌâÌãÌåÌëÌêÌéÌèÌæÌç"};
char PY_mb_tian[]  ={"ÌìÌíÌïÌñÌğÌîÌóÌò"};
char PY_mb_tiao[]  ={"µ÷ÌôÌõÌöÌ÷Ìø"};
char PY_mb_tie[]   ={"ÌùÌúÌû"};
char PY_mb_ting[]  ={"ÌüÍ¡ÌıÌşÍ¢Í¤Í¥Í£Í¦Í§"};
char PY_mb_tong[]  ={"Í¨Í¬Í®Í©Í­Í¯ÍªÍ«Í³Í±Í°Í²Í´"};
char PY_mb_tou[]   ={"ÍµÍ·Í¶Í¸"};
char PY_mb_tu[]    ={"Í¹ÍºÍ»Í¼Í½Í¿Í¾ÍÀÍÁÍÂÍÃ"};
char PY_mb_tuan[]  ={"ÍÄÍÅ"};
char PY_mb_tui[]   ={"ÍÆÍÇÍÈÍËÍÉÍÊ"};
char PY_mb_tun[]   ={"¶ÚÍÌÍÍÍÎ"};
char PY_mb_tuo[]   ={"ÍĞÍÏÍÑÍÔÍÓÍÕÍÒÍ×ÍÖÍØÍÙ"};
char PY_mb_wa[]    ={"ÍÛÍŞÍÚÍİÍÜÍßÍà"};
char PY_mb_wai[]   ={"ÍáÍâ"};
char PY_mb_wan[]   ={"ÍäÍåÍãÍèÍêÍæÍçÍéÍğÍìÍíÍñÍïÍîÍëÍòÍó"};
char PY_mb_wang[]  ={"ÍôÍöÍõÍøÍùÍ÷ÍıÍüÍúÍû"};
char PY_mb_wei[]   ={"Î£ÍşÎ¢Î¡ÎªÎ¤Î§Î¥Î¦Î¨Î©Î¬Î«Î°Î±Î²Î³Î­Î¯Î®ÎÀÎ´Î»Î¶Î·Î¸Î¾Î½Î¹Î¼ÎµÎ¿Îº"};
char PY_mb_wen[]   ={"ÎÂÎÁÎÄÎÆÎÅÎÃÎÇÎÉÎÈÎÊ"};
char PY_mb_weng[]  ={"ÎÌÎËÎÍ"};
char PY_mb_wo[]    ={"ÎÎÎĞÎÑÎÏÎÒÎÖÎÔÎÕÎÓ"};
char PY_mb_wu[]    ={"ÎÚÎÛÎØÎ×ÎİÎÜÎÙÎŞÎãÎâÎáÎßÎàÎåÎçÎéÎëÎäÎêÎæÎèÎğÎñÎìÎïÎóÎòÎîÎí"};
char PY_mb_xi[]    ={"Ï¦Ï«Î÷ÎüÏ£ÎôÎöÎùÏ¢ÎşÏ¤Ï§Ï©ÎøÎúÏ¬Ï¡ÏªÎıÏ¨ÎõÎûÏ¥Ï°Ï¯Ï®Ï±Ï­Ï´Ï²Ï·ÏµÏ¸Ï¶"};
char PY_mb_xia[]   ={"ÏºÏ¹Ï»ÏÀÏ¿ÏÁÏ¾Ï½Ï¼ÏÂÏÅÏÄè¦åÚßÈ÷ïáòÏÃÅ{"};
char PY_mb_xian[]  ={"Ï³ÏÉÏÈÏËÏÆÏÇÏÊÏĞÏÒÏÍÙşÏÌÏÑÏÏÏÎÏÓÏÔÏÕÏØÏÖÏßÏŞÏÜÏİÏÚÏÛÏ×ÏÙæµÜÈò¹á­åßŞºğïõÑë¯Ï³"};
char PY_mb_xiang[] ={"ÏçÏàÏãÏáÏæÏäÏåÏâÏêÏéÏèÏíÏìÏëÏòÏïÏîÏóÏñÏğâÃæø÷Ïöß½µ"};
char PY_mb_xiao[]  ={"ÏüÏûÏôÏõÏúÏöÏùåĞèÉæçÏıĞ¡ÏşĞ¢Ğ¤ÏøĞ§Ğ£Ğ¦Ğ¥óãäìÏøç¯Ï÷"};
char PY_mb_xie[]   ={"½âĞ©Ğ¨ĞªĞ«Ğ­Ğ°Ğ²Ğ±Ğ³Ğ¯Ğ¬Ğ´Ğ¹ĞºĞ¶Ğ¼ĞµĞ»Ğ¸Ğ·ÑªĞ²ÙÉé¿ÛÆß¢ò¡Ùôåâ"};
char PY_mb_xin[]   ={"ĞÄĞÃĞ¾ĞÁĞÀĞ¿ĞÂĞ½ĞÅĞÆöÎê¿Ü°ì§İ·"};
char PY_mb_xing[]  ={"ĞËĞÇĞÊĞÉĞÈĞÌĞÏĞÎĞÍĞÑĞÓĞÕĞÒĞÔĞĞĞÈÊ¡ß©"};
char PY_mb_xiong[] ={"Ğ×ĞÖĞÙĞÚĞØĞÛĞÜ"};
char PY_mb_xiu[]   ={"ËŞĞİĞŞĞßĞàĞãĞåĞäĞâĞáßİá¶äåõ÷âÊ"};
char PY_mb_xu[]    ={"ĞçĞëĞéĞêĞèĞæĞìĞíĞñĞòĞğĞôĞ÷ĞøĞïĞöĞõĞîÓõñãĞæìãèòíìŞ£"};
char PY_mb_xuan[]  ={"ĞùĞûĞúĞşĞüĞıÑ¡Ñ¢Ñ¤Ñ£ìÅİæè¯äÖäöãù"};
char PY_mb_xue[]   ={"Ï÷Ñ¥Ñ¦Ñ¨Ñ§Ñ©ÑªÚÊ"};
char PY_mb_xun[]   ={"Ñ«Ñ¬Ñ°Ñ²Ñ®Ñ±Ñ¯Ñ­ÑµÑ¶Ñ´Ñ¸Ñ·Ñ³Ş¹Ü÷Ñ¤ä±"};
char PY_mb_ya[]    ={"Ñ¾Ñ¹Ñ½ÑºÑ»Ñ¼ÑÀÑ¿ÑÁÑÂÑÄÑÃÑÆÑÅÑÇÑÈíıÔş"};
char PY_mb_yan[]   ={"ÑÊÑÌÑÍÑÉÑËÑÓÑÏÑÔÑÒÑØÑ×ÑĞÑÎÑÖÑÑÑÕÑÙÑÜÑÚÑÛÑİÑáÑåÑâÑäÑçÑŞÑéÑèÑßÑæÑãÑàåûêÌóÛäÎëÙæÌéÜçü÷ÊÙÈäÙ"};
char PY_mb_yang[]  ={"ÑëÑêÑíÑìÑïÑòÑôÑîÑğÑñÑóÑöÑøÑõÑ÷ÑùÑúì¾áàãóí¦÷±ìÈ"};
char PY_mb_yao[]   ={"½ÄÑıÑüÑûÒ¢Ò¦Ò¤Ò¥Ò¡Ò£ÑşÒ§Ò¨Ò©ÒªÒ«Ô¿ëÈçÛßºèÃğÎ"};
char PY_mb_ye[]    ={"Ò¬Ò­Ò¯Ò®Ò²Ò±Ò°ÒµÒ¶Ò·Ò³Ò¹Ò´ÒºÒ¸ìÇÚşÚËØÌ"};
char PY_mb_yi[]    ={"Ò»ÒÁÒÂÒ½ÒÀÒ¿Ò¼Ò¾ß×âùß½âÂÒÇÒÄÒÊÒËÒÌÒÈÒÆÒÅÒÃÒÉÒÍÒÒÒÑÒÔÒÓÒÏÒĞÒÎÒåÒÚÒäÒÕÒéÒàÒÙÒìÒÛÒÖÒëÒØÒ×ÒïÒèÒßÒæÒêÒîÒİÒâÒçÒŞÒáÒãÒíÒÜŞÈŞÄäôàæÜ²ñ´éóß®ôèêİØıìÚì½"};
char PY_mb_yin[]   ={"ÒòÒõÒöÒğÒñÒôÒóÒ÷ÒúÒùÒøÒüÒıÒûÒşÓ¡Ûóö¸ñ«Ø·ë³â¹ö¯à³"};
char PY_mb_ying[]  ={"Ó¦Ó¢Ó¤Ó§Ó£Ó¥Ó­Ó¯Ó«Ó¨Ó©ÓªÓ¬Ó®Ó±Ó°Ó³Ó²çøİºå­İÓäŞâßéºó¿ğĞàÓÛ«äëè¬"};
char PY_mb_yo[]    ={"Ó´à¡"};
char PY_mb_yong[]  ={"Ó¶ÓµÓ¸Ó¹ÓºÓ·ÓÀÓ½Ó¾ÓÂÓ¿ÓÁÓ¼Ó»ÓÃÙ¸ğ®Ü­ã¼"};
char PY_mb_you[]   ={"ÓÅÓÇÓÄÓÆÓÈÓÉÓÌÓÊÓÍÓËÓÎÓÑÓĞÓÏÓÖÓÒÓ×ÓÓÓÕÓÔ÷øØüèÖğàå¶÷îéàöÏ"};
char PY_mb_yu[]    ={"â×ÓØÓÙÓåÓÚÓèÓàÓÛÓãÓáÓéÓæÓçÓäÓâÓŞÓÜÓİÓßÓëÓîÓìÓğÓêÓíÓïÓñÔ¦ÓóÓıÓôÓüÓøÔ¡Ô¤ÓòÓûÓ÷Ô¢ÓùÔ£ÓöÓúÓşÔ¥ªì¶Ø¹ìÏí²êÅİÇÚÄêìëéæ¥"};
char PY_mb_yuan[]  ={"Ô©Ô§Ô¨ÔªÔ±Ô°Ô«Ô­Ô²Ô¬Ô®ÔµÔ´Ô³Ô¯Ô¶Ô·Ô¹ÔºÔ¸æÂö½"};
char PY_mb_yue[]   ={"Ô»Ô¼ÔÂÔÀÔÃÔÄÔ¾ÔÁÔ½«h"};
char PY_mb_yun[]   ={"ÔÆÔÈÔËÔÊÔÉÔĞÔÎÔÇÔÅÔÍÔÏÔÌÜ¿ìÙéæã³Û©"};
char PY_mb_za[]    ={"ÔÑÔÓÔÒÕ¦"};
char PY_mb_zai[]   ={"ÔÖÔÕÔÔÔ×ÔØÔÙÔÚ×ĞáÌ"};
char PY_mb_zan[]   ={"ÔÛÔÜÔİÔŞô¢è¶öÉ"};
char PY_mb_zang[]  ={"ÔßÔàÔáê°²ØŞÊ"};
char PY_mb_zao[]   ={"ÔâÔãÔäÔçÔæÔéÔèÔåÔîÔíÔìÔëÔïÔê"};
char PY_mb_ze[]    ={"ÔòÔñÔóÔğØÆßõ"};
char PY_mb_zei[]   ={"Ôô"};
char PY_mb_zen[]   ={"ÔõÚÚ"};
char PY_mb_zeng[]  ={"ÔöÔ÷ÔùÔøï­"};
char PY_mb_zha[]   ={"²éÔûÔüÔúÔıÔşÕ¢Õ¡Õ£Õ§Õ©Õ¨Õ¥×õÕ¤ß¸"};
char PY_mb_zhai[]  ={"Õ«ÕªÕ¬µÔÕ­Õ®Õ¯"};
char PY_mb_zhan[]  ={"Õ´Õ±Õ³Õ²Õ°Õ¶Õ¹ÕµÕ¸Õ·Õ¼Õ½Õ»Õ¾ÕÀÕ¿Õº²ü"};
char PY_mb_zhang[] ={"³¤ÕÅÕÂÕÃÕÄÕÁÕÇÕÆÕÉÕÌÕÊÕÈÕÍÕËÕÏÕÎ"};
char PY_mb_zhao[]  ={"ÕĞÕÑÕÒÕÓÕÙÕ×ÕÔÕÕÕÖÕØ×¦×ÅîÈÚ¯èş³¯"};
char PY_mb_zhe[]   ={"ÕÚÕÛÕÜÕİÕŞÕßÕàÕâÕãÕá×Å†´éüô÷ñŞğÑµ—"};
char PY_mb_zhen[]  ={"ÕêÕëÕìÕäÕæÕèÕåÕçÕéÕïÕíÕîÕóÕñÕòÕğÖ¡ëŞìõÛÚğ²êâóğçÇÕè"};
char PY_mb_zheng[] ={"Ö£ÕùÕ÷ÕúÕõÕøÕöÕôÕüÕûÕıÖ¤ÕşÖ¢á¿óİÚº"};
char PY_mb_zhi[]   ={"Ö®Ö§Ö­Ö¥Ö¨Ö¦ÖªÖ¯Ö«Ö¬Ö©Ö´Ö¶Ö±ÖµÖ°Ö²Ö³Ö¹Ö»Ö¼Ö·Ö½Ö¸ÖºÖÁÖ¾ÖÆÖÄÖÎÖËÖÊÖÅÖ¿ÖÈÖÂÖÀÖÌÖÏÖÇÖÍÖÉÖÃè×ÜÆğëìóèÙ"};
char PY_mb_zhong[] ={"ÖĞÖÒÖÕÖÑÖÓÖÔÖ×ÖÖÖÙÖÚÖØÚ£õà"};
char PY_mb_zhou[]  ={"ÖİÖÛÖßÖÜÖŞÖàÖáÖâÖãÖäÖæÖçÖåÖèëĞæûôíæ¨"};
char PY_mb_zhu[]   ={"óÃÖì×£ÖïÖêÖéÖîÖíÖëÖñÖòÖğÖ÷ÖôÖóÖöÖõ×¡Öú×¢Öü×¤ÖùÖøÖûÖşÖıÜïä¾ØùèÌ"};
char PY_mb_zhua[]  ={"×¥"};
char PY_mb_zhuai[] ={"×§ÛJ"};
char PY_mb_zhuan[] ={"×¨×©×ª×«×­×¬´«ò§âÍ"};
char PY_mb_zhuang[]={"×±×¯×®×°×³×´´±×²"};
char PY_mb_zhui[]  ={"×·×µ×¶×¹×º×¸æíã·çÄ"};
char PY_mb_zhun[]  ={"×»×¼ëÆ"};
char PY_mb_zhuo[]  ={"×¿×¾×½×À×Æ×Â×Ç×Ã×Ä×Áåªïíí½ßªäÃŠƒä·×Å"};
char PY_mb_zi[]    ={"×Î×È×É×Ë×Ê×Í×Ì×Ñ×Ó×Ï×Ò×Ö×Ô×Õ×ĞÖ¨è÷æ¢í§ßÚíö†êïÅç»æÜ÷Úê¢"};
char PY_mb_zong[]  ={"×Ü×Ú×Û×Ø×Ù×××İôÕÙÌ"};
char PY_mb_zou[]   ={"×Ş×ß×à×áÚÁ"};
char PY_mb_zu[]    ={"×â×ã×ä×å×ç×è×é×æÙŞ"};
char PY_mb_zuan[]  ={"×¬×ë×êß¬õò"};
char PY_mb_zui[]   ={"×ì×î×ï×í¾×"};
char PY_mb_zun[]   ={"×ğ×ñé×÷®ß¤À–"};
char PY_mb_zuo[]   ={"×ö×ò×ó×ô×÷×ø×ùßòìñ"};
char PY_mb_space[] = {""};

/*"Æ´ÒôÊäÈë·¨²éÑ¯Âë±í,¶ş¼¶×ÖÄ¸Ë÷Òı±í(index)"*/
struct py_index PY_index_a[] = {{"",PY_mb_a},
                                    {"i",PY_mb_ai},
                                    {"n",PY_mb_an},
                                    {"ng",PY_mb_ang},
                                    {"o",PY_mb_ao}};

struct py_index PY_index_b[] = {{"a",PY_mb_ba},
                                    {"ai",PY_mb_bai},
                                    {"an",PY_mb_ban},
                                    {"ang",PY_mb_bang},
                                    {"ao",PY_mb_bao},
                                    {"ei",PY_mb_bei},
                                    {"en",PY_mb_ben},
                                    {"eng",PY_mb_beng},
                                    {"i",PY_mb_bi},
                                    {"ian",PY_mb_bian},
                                    {"iao",PY_mb_biao},
                                    {"ie",PY_mb_bie},
                                    {"in",PY_mb_bin},
                                    {"ing",PY_mb_bing},
                                    {"o",PY_mb_bo},
                                    {"u",PY_mb_bu}};

struct py_index PY_index_c[] = {{"a",PY_mb_ca},
                                    {"ai",PY_mb_cai},
                                    {"an",PY_mb_can},
                                    {"ang",PY_mb_cang},
                                    {"ao",PY_mb_cao},
                                    {"e",PY_mb_ce},
                                    {"eng",PY_mb_ceng},
                                    {"ha",PY_mb_cha},
                                    {"hai",PY_mb_chai},
                                    {"han",PY_mb_chan},
                                    {"hang",PY_mb_chang},
                                    {"hao",PY_mb_chao},
                                    {"he",PY_mb_che},
                                    {"hen",PY_mb_chen},
                                    {"heng",PY_mb_cheng},
                                    {"hi",PY_mb_chi},
                                    {"hong",PY_mb_chong},
                                    {"hou",PY_mb_chou},
                                    {"hu",PY_mb_chu},
                                    {"huai",PY_mb_chuai},
                                    {"huan",PY_mb_chuan},
                                    {"huang",PY_mb_chuang},
                                    {"hui",PY_mb_chui},
                                    {"hun",PY_mb_chun},
                                    {"huo",PY_mb_chuo},
                                    {"i",PY_mb_ci},
                                    {"ong",PY_mb_cong},
                                    {"ou",PY_mb_cou},
                                    {"u",PY_mb_cu},
                                    {"uan",PY_mb_cuan},
                                    {"ui",PY_mb_cui},
                                    {"un",PY_mb_cun},
                                    {"uo",PY_mb_cuo}};


struct py_index PY_index_d[] = {{"a",PY_mb_da},
                                    {"ai",PY_mb_dai},
                                    {"an",PY_mb_dan},
                                    {"ang",PY_mb_dang},
                                    {"ao",PY_mb_dao},
                                    {"e",PY_mb_de},
                                    {"eng",PY_mb_deng},
                                    {"i",PY_mb_di},
                                    {"ian",PY_mb_dian},
                                    {"iao",PY_mb_diao},
                                    {"ie",PY_mb_die},
                                    {"ing",PY_mb_ding},
                                    {"iu",PY_mb_diu},
                                    {"ong",PY_mb_dong},
                                    {"ou",PY_mb_dou},
                                    {"u",PY_mb_du},
                                    {"uan",PY_mb_duan},
                                    {"ui",PY_mb_dui},
                                    {"un",PY_mb_dun},
                                    {"uo",PY_mb_duo}};

struct py_index PY_index_e[]={{"",PY_mb_e},
                                    {"n",PY_mb_en},
                                    {"r",PY_mb_er}};

struct py_index PY_index_f[] = {{"a",PY_mb_fa},
                                    {"an",PY_mb_fan},
                                    {"ang",PY_mb_fang},
                                    {"ei",PY_mb_fei},
                                    {"en",PY_mb_fen},
                                    {"eng",PY_mb_feng},
                                    {"o",PY_mb_fo},
                                    {"ou",PY_mb_fou},
                                    {"u",PY_mb_fu}};

struct py_index PY_index_g[] = {{"a",PY_mb_ga},
                                    {"ai",PY_mb_gai},
                                    {"an",PY_mb_gan},
                                    {"ang",PY_mb_gang},
                                    {"ao",PY_mb_gao},
                                    {"e",PY_mb_ge},
                                    {"ei",PY_mb_gei},
                                    {"en",PY_mb_gan},
                                    {"eng",PY_mb_geng},
                                    {"ong",PY_mb_gong},
                                    {"ou",PY_mb_gou},
                                    {"u",PY_mb_gu},
                                    {"ua",PY_mb_gua},
                                    {"uai",PY_mb_guai},
                                    {"uan",PY_mb_guan},
                                    {"uang",PY_mb_guang},
                                    {"ui",PY_mb_gui},
                                    {"un",PY_mb_gun},
                                    {"uo",PY_mb_guo}};


struct py_index PY_index_h[] = {{"a",PY_mb_ha},
                                    {"ai",PY_mb_hai},
                                    {"an",PY_mb_han},
                                    {"ang",PY_mb_hang},
                                    {"ao",PY_mb_hao},
                                    {"e",PY_mb_he},
                                    {"ei",PY_mb_hei},
                                    {"en",PY_mb_hen},
                                    {"eng",PY_mb_heng},
                                    {"ong",PY_mb_hong},
                                    {"ou",PY_mb_hou},
                                    {"u",PY_mb_hu},
                                    {"ua",PY_mb_hua},
                                    {"uai",PY_mb_huai},
                                    {"uan",PY_mb_huan},
                                    {"uang ",PY_mb_huang},
                                    {"ui",PY_mb_hui},
                                    {"un",PY_mb_hun},
                                    {"uo",PY_mb_huo}};

struct py_index PY_index_i[] = {{"",PY_mb_space}};

struct py_index PY_index_j[] = {{"i",PY_mb_ji},
                                    {"ia",PY_mb_jia},
                                    {"ian",PY_mb_jian},
                                    {"iang",PY_mb_jiang},
                                    {"iao",PY_mb_jiao},
                                    {"ie",PY_mb_jie},
                                    {"in",PY_mb_jin},
                                    {"ing",PY_mb_jing},
                                    {"iong",PY_mb_jiong},
                                    {"iu",PY_mb_jiu},
                                    {"u",PY_mb_ju},
                                    {"uan",PY_mb_juan},
                                    {"ue",PY_mb_jue},
                                    {"un",PY_mb_jun}};

struct py_index PY_index_k[] = {{"a",PY_mb_ka},
                                    {"ai",PY_mb_kai},
                                    {"an",PY_mb_kan},
                                    {"ang",PY_mb_kang},
                                    {"ao",PY_mb_kao},
                                    {"e",PY_mb_ke},
                                    {"en",PY_mb_ken},
                                    {"eng",PY_mb_keng},
                                    {"ong",PY_mb_kong},
                                    {"ou",PY_mb_kou},
                                    {"u",PY_mb_ku},
                                    {"ua",PY_mb_kua},
                                    {"uai",PY_mb_kuai},
                                    {"uan",PY_mb_kuan},
                                    {"uang",PY_mb_kuang},
                                    {"ui",PY_mb_kui},
                                    {"un",PY_mb_kun},
                                    {"uo",PY_mb_kuo}};

struct py_index PY_index_l[] = {{"a",PY_mb_la},
                                    {"ai",PY_mb_lai},
                                    {"an",PY_mb_lan},
                                    {"ang",PY_mb_lang},
                                    {"ao",PY_mb_lao},
                                    {"e",PY_mb_le},
                                    {"ei",PY_mb_lei},
                                    {"eng",PY_mb_leng},
                                    {"i",PY_mb_li},
                                    {"ian",PY_mb_lian},
                                    {"iang",PY_mb_liang},
                                    {"iao",PY_mb_liao},
                                    {"ie",PY_mb_lie},
                                    {"in",PY_mb_lin},
                                    {"ing",PY_mb_ling},
                                    {"iu",PY_mb_liu},
                                    {"ong",PY_mb_long},
                                    {"ou",PY_mb_lou},
                                    {"u",PY_mb_lu},
                                    {"uan",PY_mb_luan},
                                    {"ue",PY_mb_lue},
                                    {"un",PY_mb_lun},
                                    {"uo",PY_mb_luo},
                                    {"v",PY_mb_lv}};

struct py_index PY_index_m[] = {{"a",PY_mb_ma},
                                    {"ai",PY_mb_mai},
                                    {"an",PY_mb_man},
                                    {"ang",PY_mb_mang},
                                    {"ao",PY_mb_mao},
                                    {"e",PY_mb_me},
                                    {"ei",PY_mb_mei},
                                    {"en",PY_mb_men},
                                    {"eng",PY_mb_meng},
                                    {"i",PY_mb_mi},
                                    {"ian",PY_mb_mian},
                                    {"iao",PY_mb_miao},
                                    {"ie",PY_mb_mie},
                                    {"in",PY_mb_min},
                                    {"ing",PY_mb_ming},
                                    {"iu",PY_mb_miu},
                                    {"o",PY_mb_mo},
                                    {"ou",PY_mb_mou},
                                    {"u",PY_mb_mu}};

struct py_index PY_index_n[] = {{"a",PY_mb_na},
                                    {"ai",PY_mb_nai},
                                    {"an",PY_mb_nan},
                                    {"ang",PY_mb_nang},
                                    {"ao",PY_mb_nao},
                                    {"e",PY_mb_ne},
                                    {"ei",PY_mb_nei},
                                    {"en",PY_mb_nen},
                                    {"eng",PY_mb_neng},
                                    {"i",PY_mb_ni},
                                    {"ian",PY_mb_nian},
                                    {"iang",PY_mb_niang},
                                    {"iao",PY_mb_niao},
                                    {"ie",PY_mb_nie},
                                    {"in",PY_mb_nin},
                                    {"ing",PY_mb_ning},
                                    {"iu",PY_mb_niu},
                                    {"ong",PY_mb_nong},
                                    {"u",PY_mb_nu},
                                    {"uan",PY_mb_nuan},
                                    {"ue",PY_mb_nue},
                                    {"uo",PY_mb_nuo},
                                    {"v",PY_mb_nv}};

struct py_index PY_index_o[] = {{"",PY_mb_o},
                                    {"u",PY_mb_ou}};

struct py_index PY_index_p[] = {{"a",PY_mb_pa},
                                    {"ai",PY_mb_pai},
                                    {"an",PY_mb_pan},
                                    {"ang",PY_mb_pang},
                                    {"ao",PY_mb_pao},
                                    {"ei",PY_mb_pei},
                                    {"en",PY_mb_pen},
                                    {"eng",PY_mb_peng},
                                    {"i",PY_mb_pi},
                                    {"ian",PY_mb_pian},
                                    {"iao",PY_mb_piao},
                                    {"ie",PY_mb_pie},
                                    {"in",PY_mb_pin},
                                    {"ing",PY_mb_ping},
                                    {"o",PY_mb_po},
                                    {"ou",PY_mb_pou},
                                    {"u",PY_mb_pu}};

struct py_index  PY_index_q[] = {{"i",PY_mb_qi},
                                    {"ia",PY_mb_qia},
                                    {"ian",PY_mb_qian},
                                    {"iang",PY_mb_qiang},
                                    {"iao",PY_mb_qiao},
                                    {"ie",PY_mb_qie},
                                    {"in",PY_mb_qin},
                                    {"ing",PY_mb_qing},
                                    {"iong",PY_mb_qiong},
                                    {"iu",PY_mb_qiu},
                                    {"u",PY_mb_qu},
                                    {"uan",PY_mb_quan},
                                    {"ue",PY_mb_que},
                                    {"un",PY_mb_qun}};

struct py_index PY_index_r[] = {{"an",PY_mb_ran},
                                    {"ang",PY_mb_rang},
                                    {"ao",PY_mb_rao},
                                    {"e",PY_mb_re},
                                    {"en",PY_mb_ren},
                                    {"eng",PY_mb_reng},
                                    {"i",PY_mb_ri},
                                    {"ong",PY_mb_rong},
                                    {"ou",PY_mb_rou},
                                    {"u",PY_mb_ru},
                                    {"uan",PY_mb_ruan},
                                    {"ui",PY_mb_rui},
                                    {"un",PY_mb_run},
                                    {"uo",PY_mb_ruo}};

struct py_index PY_index_s[] = {{"a",PY_mb_sa},
                                    {"ai",PY_mb_sai},
                                    {"an",PY_mb_san},
                                    {"ang",PY_mb_sang},
                                    {"ao",PY_mb_sao},
                                    {"e",PY_mb_se},
                                    {"en",PY_mb_sen},
                                    {"eng",PY_mb_seng},
                                    {"ha",PY_mb_sha},
                                    {"hai",PY_mb_shai},
                                    {"han",PY_mb_shan},
                                    {"hang ",PY_mb_shang},
                                    {"hao",PY_mb_shao},
                                    {"he",PY_mb_she},
                                    {"hen",PY_mb_shen},
                                    {"heng",PY_mb_sheng},
                                    {"hi",PY_mb_shi},
                                    {"hou",PY_mb_shou},
                                    {"hu",PY_mb_shu},
                                    {"hua",PY_mb_shua},
                                    {"huai",PY_mb_shuai},
                                    {"huan",PY_mb_shuan},
                                    {"huang",PY_mb_shuang},
                                    {"hui",PY_mb_shui},
                                    {"hun",PY_mb_shun},
                                    {"huo",PY_mb_shuo},
                                    {"i",PY_mb_si},
                                    {"ong",PY_mb_song},
                                    {"ou",PY_mb_sou},
                                    {"u",PY_mb_su},
                                    {"uan",PY_mb_suan},
                                    {"ui",PY_mb_sui},
                                    {"un",PY_mb_sun},
                                    {"uo",PY_mb_suo}};

struct py_index  PY_index_t[] = {{"a",PY_mb_ta},
                                    {"ai",PY_mb_tai},
                                    {"an",PY_mb_tan},
                                    {"ang",PY_mb_tang},
                                    {"ao",PY_mb_tao},
                                    {"e",PY_mb_te},
                                    {"eng",PY_mb_teng},
                                    {"i",PY_mb_ti},
                                    {"ian",PY_mb_tian},
                                    {"iao",PY_mb_tiao},
                                    {"ie",PY_mb_tie},
                                    {"ing",PY_mb_ting},
                                    {"ong",PY_mb_tong},
                                    {"ou",PY_mb_tou},
                                    {"u",PY_mb_tu},
                                    {"uan",PY_mb_tuan},
                                    {"ui",PY_mb_tui},
                                    {"un",PY_mb_tun},
                                    {"uo",PY_mb_tuo}};


struct py_index  PY_index_u[] = {{"",PY_mb_space}};
struct py_index  PY_index_v[] = {{"",PY_mb_space}};
struct py_index  PY_index_w[] = {{"a",PY_mb_wa},
                                    {"ai",PY_mb_wai},
                                    {"an",PY_mb_wan},
                                    {"ang",PY_mb_wang},
                                    {"ei",PY_mb_wei},
                                    {"en",PY_mb_wen},
                                    {"eng",PY_mb_weng},
                                    {"o",PY_mb_wo},
                                    {"u",PY_mb_wu}};

struct py_index  PY_index_x[] = {{"i",PY_mb_xi},
                                    {"ia",PY_mb_xia},
                                    {"ian",PY_mb_xian},
                                    {"iang",PY_mb_xiang},
                                    {"iao",PY_mb_xiao},
                                    {"ie",PY_mb_xie},
                                    {"in",PY_mb_xin},
                                    {"ing",PY_mb_xing},
                                    {"iong",PY_mb_xiong},
                                    {"iu",PY_mb_xiu},
                                    {"u",PY_mb_xu},
                                    {"uan",PY_mb_xuan},
                                    {"ue",PY_mb_xue},
                                    {"un",PY_mb_xun}};

struct py_index  PY_index_y[] = {{"a",PY_mb_ya},
                                    {"an",PY_mb_yan},
                                    {"ang",PY_mb_yang},
                                    {"ao",PY_mb_yao},
                                    {"e",PY_mb_ye},
                                    {"i",PY_mb_yi},
                                    {"in",PY_mb_yin},
                                    {"ing",PY_mb_ying},
                                    {"o",PY_mb_yo},
                                    {"ong",PY_mb_yong},
                                    {"ou",PY_mb_you},
                                    {"u",PY_mb_yu},
                                    {"uan",PY_mb_yuan},
                                    {"ue",PY_mb_yue},
                                    {"un",PY_mb_yun}};

struct py_index  PY_index_z[] = {{"a",PY_mb_za},
                                    {"ai",PY_mb_zai},
                                    {"an",PY_mb_zan},
                                    {"ang",PY_mb_zang},
                                    {"ao",PY_mb_zao},
                                    {"e",PY_mb_ze},
                                    {"ei",PY_mb_zei},
                                    {"en",PY_mb_zen},
                                    {"eng",PY_mb_zeng},
                                    {"ha",PY_mb_zha},
                                    {"hai",PY_mb_zhai},
                                    {"han",PY_mb_zhan},
                                    {"hang",PY_mb_zhang},
                                    {"hao",PY_mb_zhao},
                                    {"he",PY_mb_zhe},
                                    {"hen",PY_mb_zhen},
                                    {"heng",PY_mb_zheng},
                                    {"hi",PY_mb_zhi},
                                    {"hong",PY_mb_zhong},
                                    {"hou",PY_mb_zhou},
                                    {"hu",PY_mb_zhu},
                                    {"hua",PY_mb_zhua},
                                    {"huai",PY_mb_zhuai},
                                    {"huan",PY_mb_zhuan},
                                    {"huang",PY_mb_zhuang},
                                    {"hui",PY_mb_zhui},
                                    {"hun",PY_mb_zhun},
                                    {"huo",PY_mb_zhuo},
                                    {"i",PY_mb_zi},
                                    {"ong",PY_mb_zong},
                                    {"ou",PY_mb_zou},
                                    {"u",PY_mb_zu},
                                    {"uan",PY_mb_zuan},
                                    {"ui",PY_mb_zui},
                                    {"un",PY_mb_zun},
                                    {"uo",PY_mb_zuo}};

static int  pyi_len[] = {5, 16, 33, 20, 3, 9, 19, 19, 0, 14, 18, 24, 19, 23, 2, 17, 14, 14, 34, 19, 0, 0, 9, 14, 15, 36};

/*¶¨ÒåÊ××ÖÄ¸Ë÷Òı±í*/
struct py_index *pyi_root[] = {PY_index_a,
                                            PY_index_b,
                                            PY_index_c,
                                            PY_index_d,
                                            PY_index_e,
                                            PY_index_f,
                                            PY_index_g,
                                            PY_index_h,
                                            PY_index_i,
                                            PY_index_j,
                                            PY_index_k,
                                            PY_index_l,
                                            PY_index_m,
                                            PY_index_n,
                                            PY_index_o,
                                            PY_index_p,
                                            PY_index_q,
                                            PY_index_r,
                                            PY_index_s,
                                            PY_index_t,
                                            PY_index_u,
                                            PY_index_v,
                                            PY_index_w,
                                            PY_index_x,
                                            PY_index_y,
                                            PY_index_z};

int
py_ime(char *instr, hzcandidate_t *arr, int maxsz)
{
	struct py_index *pi;
	int    i = 0;
	int    j;
	int    k;
	int    x;
	char  *p;

	if (*instr == '\0'||*instr == 'i'||*instr=='u'||*instr=='v')
		return 0;
	
	x = instr[0]-'a';
	instr++;
	pi = pyi_root[x];
	for (k = 0; k < pyi_len[x]; ++k) {
		if (strstr(pi->py, instr) == pi->py) {
			p = pi->py_mb;
			for (j = 0; j < pi->len; j+=2) {
				memcpy(arr[i].text, p, 2);
				arr[i].text[2] = 0;
		//	#ifdef DEBUG
		//		printf("%s ", arr[i].text);
		//	#endif
				++i;
				p += 2;
				if (i >= maxsz) {
				#if 0
					maxsz = carray_reinit();
					arr = *parr;
				#else
				//	DBG("--- toatal = %d ---\n", i);
					return i;
				#endif
				}
			}
		}
		pi++;
	}
/*
#ifdef DEBUG
	printf("\n\n");
	printf("--- toatal = %d ---\n", i);
#endif
*/
	return i;
}

void
py_init(void)
{
	int  i;
	int  k;
	struct py_index *pi;

#if 0
	carray_init();
#endif
	for (i = 0; i < ARR_SZ(pyi_root); ++i) {
		pi = pyi_root[i];
		for (k = 0; k < pyi_len[i]; ++k) {
			pi->len = strlen(pi->py_mb);
			pi++;
		}
	}
}


