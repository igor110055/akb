#include <string.h>
#include "gui.h"
#include "pinyin.h"

typedef struct py_index
{
    char *py;
    char *py_mb;
	int   len;
} py_index_t;

//"ƴ�����뷨�������б�,���(mb)"
char PY_mb_a[]     ={"����"};
char PY_mb_ai[]    ={"��������������������������"};
char PY_mb_an[]    ={"������������������"};
char PY_mb_ang[]   ={"������"};
char PY_mb_ao[]    ={"�������������°İ�"};
char PY_mb_ba[]    ={"�˰ͰȰǰɰṴ̋ưʰΰϰѰаӰְհ�"};
char PY_mb_bai[]   ={"�װٰ۰ذڰܰݰ�"};
char PY_mb_ban[]   ={"�����߰����������"};
char PY_mb_bang[]  ={"������������������"};
char PY_mb_bao[]   ={"������������������������������������"};
char PY_mb_bei[]   ={"������������������������������"};
char PY_mb_ben[]   ={"����������"};
char PY_mb_beng[]  ={"�����±ñű�"};
char PY_mb_bi[]    ={"�ƱǱȱ˱ʱɱұرϱձӱѱݱбֱԱͱױ̱αڱܱ�"};
char PY_mb_bian[]  ={"�߱�ޱ���������"};
char PY_mb_biao[]  ={"�����"};
char PY_mb_bie[]   ={"�����"};
char PY_mb_bin[]   ={"����������"};
char PY_mb_bing[]  ={"������������������"};
char PY_mb_bo[]    ={"����������������������������������������"};
char PY_mb_bu[]    ={"��������������������"};
char PY_mb_ca[]    ={"��"};
char PY_mb_cai[]   ={"�²ŲĲƲòɲʲǲȲ˲�"};
char PY_mb_can[]   ={"�βͲвϲѲҲ�"};
char PY_mb_cang[]  ={"�ֲײԲղ�"};
char PY_mb_cao[]   ={"�ٲڲܲ۲�"};
char PY_mb_ce[]    ={"���޲��"};
char PY_mb_ceng[]  ={"�����"};
char PY_mb_cha[]   ={"������������ɲ"};
char PY_mb_chai[]  ={"����"};
char PY_mb_chan[]  ={"�������������������"};
char PY_mb_chang[] ={"������������������������"};
char PY_mb_chao[]  ={"��������������������"};
char PY_mb_che[]   ={"������������"};
char PY_mb_chen[]  ={"�������������³��ĳ�"};
char PY_mb_cheng[] ={"�Ƴųɳʳгϳǳ˳ͳ̳γȳѳҳ�"};
char PY_mb_chi[]   ={"�Գճڳس۳ٳֳ߳޳ݳܳ����"};
char PY_mb_chong[] ={"������"};
char PY_mb_chou[]  ={"�������������"};
char PY_mb_chu[]   ={"����������������������������������"};
char PY_mb_chuai[] ={"��"};
char PY_mb_chuan[] ={"��������������"};
char PY_mb_chuang[]={"����������"};
char PY_mb_chui[]  ={"����������"};
char PY_mb_chun[]  ={"��������������"};
char PY_mb_chuo[]  ={"��"};
char PY_mb_ci[]    ={"�ôʴĴɴȴǴŴƴ˴δ̴�"};
char PY_mb_cong[]  ={"�ѴӴҴдϴ�"};
char PY_mb_cou[]   ={"��"};
char PY_mb_cu[]    ={"�ִٴ״�"};
char PY_mb_cuan[]  ={"�ڴܴ�"};
char PY_mb_cui[]   ={"�޴ߴݴ�����"};
char PY_mb_cun[]   ={"����"};
char PY_mb_cuo[]   ={"�������"};
char PY_mb_da[]    ={"�������"};
char PY_mb_dai[]   ={"������������������������"};
char PY_mb_dan[]   ={"������������������������������"};
char PY_mb_dang[]  ={"����������"};
char PY_mb_dao[]   ={"������������������������"};
char PY_mb_de[]    ={"�õµ�"};
char PY_mb_deng[]  ={"�Ƶǵŵȵ˵ʵ�"};
char PY_mb_di[]    ={"�͵̵εҵϵеӵѵյ׵ֵصܵ۵ݵڵ޵�"};
char PY_mb_dian[]  ={"���ߵ�������������"};
char PY_mb_diao[]  ={"�����������"};
char PY_mb_die[]   ={"��������������"};
char PY_mb_ding[]  ={"������������������"};
char PY_mb_diu[]   ={"��"};
char PY_mb_dong[]  ={"��������������������"};
char PY_mb_dou[]   ={"����������������"};
char PY_mb_du[]    ={"�����������¶ĶöʶŶǶȶɶ�"};
char PY_mb_duan[]  ={"�˶̶ζ϶ж�"};
char PY_mb_dui[]   ={"�ѶӶԶ�"};
char PY_mb_dun[]   ={"�ֶضն׶ܶ۶ٶ�"};
char PY_mb_duo[]   ={"��߶�޶��������"};
char PY_mb_e[]     ={"����������������"};
char PY_mb_en[]    ={"��"};
char PY_mb_er[]    ={"����������������"};
char PY_mb_fa[]    ={"����������������"};
char PY_mb_fan[]   ={"����������������������������������"};
char PY_mb_fang[]  ={"���������������·÷ķ�"};
char PY_mb_fei[]   ={"�ɷǷȷƷʷ˷̷ͷϷзη�"};
char PY_mb_fen[]   ={"�ַԷ׷ҷշӷطڷٷ۷ݷܷ޷߷�"};
char PY_mb_feng[]  ={"����������������"};
char PY_mb_fo[]    ={"��"};
char PY_mb_fou[]   ={"��"};
char PY_mb_fu[]    ={"������󸥷�����������������������������������������������������������������������������"};
char PY_mb_ga[]    ={"�¸�"};
char PY_mb_gai[]   ={"�øĸƸǸȸ�"};
char PY_mb_gan[]   ={"�ɸʸ˸θ̸͸ѸϸҸи�"};
char PY_mb_gang[]  ={"�Ըոڸٸظ׸ָ۸�"};
char PY_mb_gao[]   ={"�޸�߸�ݸ�����"};
char PY_mb_ge[]    ={"����������������������"};
char PY_mb_gei[]   ={"��"};
char PY_mb_gen[]   ={"����"};
char PY_mb_geng[]  ={"��������������"};
char PY_mb_gong[]  ={"������������������������������"};
char PY_mb_gou[]   ={"������������������"};
char PY_mb_gu[]    ={"�����ù¹��������ŹȹɹǹƹĹ̹ʹ˹�"};
char PY_mb_gua[]   ={"�Ϲιйѹҹ�"};
char PY_mb_guai[]  ={"�Թչ�"};
char PY_mb_guan[]  ={"�ع۹ٹڹ׹ݹܹ�߹��"};
char PY_mb_guang[] ={"����"};
char PY_mb_gui[]   ={"������������������"};
char PY_mb_gun[]   ={"������"};
char PY_mb_guo[]   ={"������������"};
char PY_mb_ha[]    ={"���"};
char PY_mb_hai[]   ={"��������������"};
char PY_mb_han[]   ={"��������������������������������������"};
char PY_mb_hang[]  ={"������"};
char PY_mb_hao[]   ={"���������úºźƺ�"};
char PY_mb_he[]    ={"�ǺȺ̺ϺκͺӺҺ˺ɺԺкʺغֺպ�"};
char PY_mb_hei[]   ={"�ں�"};
char PY_mb_hen[]   ={"�ۺܺݺ�"};
char PY_mb_heng[]  ={"��ߺ���"};
char PY_mb_hong[]  ={"����������"};
char PY_mb_hou[]   ={"��������"};
char PY_mb_hu[]    ={"������������������������������������"};
char PY_mb_hua[]   ={"������������������"};
char PY_mb_huai[]  ={"����������"};
char PY_mb_huan[]  ={"�����������û»�������������"};
char PY_mb_huang[] ={"�ĻŻʻ˻ƻ̻ͻȻǻɻлλѻ�"};
char PY_mb_hui[]   ={"�һֻӻԻջػ׻ڻܻ������߻޻�ݻٻ�"};
char PY_mb_hun[]   ={"�������"};
char PY_mb_huo[]   ={"�������������"};
char PY_mb_ji[]    ={"���������������������������������������������������������������������ƼǼ��ͼ˼ɼ��ʼ����ȼü̼żļ¼�������"};
char PY_mb_jia[]   ={"�ӼмѼϼҼμԼռ׼ּؼۼݼܼټ޼�Ю"};
char PY_mb_jian[]  ={"����߼����������������������������������������������������"};
char PY_mb_jiang[] ={"��������������������������"};
char PY_mb_jiao[]  ={"���������������������ǽƽʽȽýŽ½��˽ɽнνϽ̽ѽ;���"};
char PY_mb_jie[]   ={"�׽Խӽսҽֽڽٽܽ��ݽ޽ؽ߽����������"};
char PY_mb_jin[]   ={"���������������������������������"};
char PY_mb_jing[]  ={"��������������������������������������������������"};
char PY_mb_jiong[] ={"����"};
char PY_mb_jiu[]   ={"�������žþľ��¾ƾɾʾ̾ξǾȾ;�"};
char PY_mb_ju[]    ={"�ӾоѾԾҾϾֽ۾վ׾ھپؾ�޾ܾ߾����ݾ��۾�"};
char PY_mb_juan[]  ={"��������"};
char PY_mb_jue[]   ={"��������������"};
char PY_mb_jun[]   ={"����������������������"};
char PY_mb_ka[]    ={"������"};
char PY_mb_kai[]   ={"����������"};
char PY_mb_kan[]   ={"��������������"};
char PY_mb_kang[]  ={"��������������"};
char PY_mb_kao[]   ={"��������"};
char PY_mb_ke[]    ={"�����¿ƿÿſĿǿȿɿʿ˿̿Ϳ�"};
char PY_mb_ken[]   ={"�Ͽѿҿ�"};
char PY_mb_keng[]  ={"�Կ�"};
char PY_mb_kong[]  ={"�տ׿ֿ�"};
char PY_mb_kou[]   ={"�ٿڿۿ�"};
char PY_mb_ku[]    ={"�ݿ޿߿����"};
char PY_mb_kua[]   ={"������"};
char PY_mb_kuai[]  ={"�����"};
char PY_mb_kuan[]  ={"���"};
char PY_mb_kuang[] ={"�����������"};
char PY_mb_kui[]   ={"����������������������"};
char PY_mb_kun[]   ={"��������"};
char PY_mb_kuo[]   ={"��������"};
char PY_mb_la[]    ={"��������������"};
char PY_mb_lai[]   ={"������"};
char PY_mb_lan[]   ={"������������������������������"};
char PY_mb_lang[]  ={"��������������"};
char PY_mb_lao[]   ={"������������������"};
char PY_mb_le[]    ={"������"};
char PY_mb_lei[]   ={"����������������������"};
char PY_mb_leng[]  ={"������"};
char PY_mb_li[]    ={"��������������������������������������������������������������������"};
char PY_mb_lian[]  ={"����������������������������"};
char PY_mb_liang[] ={"������������������������"};
char PY_mb_liao[]  ={"������������������������"};
char PY_mb_lie[]   ={"����������"};
char PY_mb_lin[]   ={"������������������������"};
char PY_mb_ling[]  ={"����������������������������"};
char PY_mb_liu[]   ={"����������������������"};
char PY_mb_long[]  ={"��������¡��¤¢£"};
char PY_mb_lou[]   ={"¦¥§¨ª©"};
char PY_mb_lu[]    ={"¶¬®«¯­±²°³½¼¸¹»µ·¾º´"};
char PY_mb_luan[]  ={"������������"};
char PY_mb_lue[]   ={"����"};
char PY_mb_lun[]   ={"��������������"};
char PY_mb_luo[]   ={"������������������������"};
char PY_mb_lv[]    ={"��¿������������������������"};
char PY_mb_ma[]    ={"������������������"};
char PY_mb_mai[]   ={"������������"};
char PY_mb_man[]   ={"����������á������"};
char PY_mb_mang[]  ={"æâäãçå"};
char PY_mb_mao[]   ={"èëìéêîíïðóñò"};
char PY_mb_me[]    ={"ô"};
char PY_mb_mei[]   ={"ûöõü÷ýúøùÿ��þ��������"};
char PY_mb_men[]   ={"������"};
char PY_mb_meng[]  ={"����������������"};
char PY_mb_mi[]    ={"����������������������������"};
char PY_mb_mian[]  ={"������������������"};
char PY_mb_miao[]  ={"����������������"};
char PY_mb_mie[]   ={"����"};
char PY_mb_min[]   ={"������������"};
char PY_mb_ming[]  ={"������������"};
char PY_mb_miu[]   ={"��"};
char PY_mb_mo[]    ={"����ġģĤĦĥĢħĨĩĭİĪįĮīĬ"};
char PY_mb_mou[]   ={"Ĳıĳ"};
char PY_mb_mu[]    ={"ĸĶĵķĴľĿ��ļĹĻ��Ľĺ��"};
char PY_mb_na[]    ={"��������������"};
char PY_mb_nai[]   ={"����������"};
char PY_mb_nan[]   ={"������"};
char PY_mb_nang[]  ={"��"};
char PY_mb_nao[]   ={"����������"};
char PY_mb_ne[]    ={"��"};
char PY_mb_nei[]   ={"����"};
char PY_mb_nen[]   ={"��"};
char PY_mb_neng[]  ={"��"};
char PY_mb_ni[]    ={"����������������������"};
char PY_mb_nian[]  ={"��������������"};
char PY_mb_niang[] ={"����"};
char PY_mb_niao[]  ={"����"};
char PY_mb_nie[]   ={"��������������"};
char PY_mb_nin[]   ={"��"};
char PY_mb_ning[]  ={"��š������Ţ"};
char PY_mb_niu[]   ={"ţŤŦť"};
char PY_mb_nong[]  ={"ũŨŧŪ"};
char PY_mb_nu[]    ={"ūŬŭ"};
char PY_mb_nuan[]  ={"ů"};
char PY_mb_nue[]   ={"űŰ"};
char PY_mb_nuo[]   ={"ŲŵųŴ"};
char PY_mb_nv[]    ={"Ů"};
char PY_mb_o[]     ={"Ŷ"};
char PY_mb_ou[]    ={"ŷŹŸŻżźŽ"};
char PY_mb_pa[]    ={"ſž����������"};
char PY_mb_pai[]   ={"������������"};
char PY_mb_pan[]   ={"����������������"};
char PY_mb_pang[]  ={"����������"};
char PY_mb_pao[]   ={"��������������"};
char PY_mb_pei[]   ={"������������������"};
char PY_mb_pen[]   ={"����"};
char PY_mb_peng[]  ={"����������������������������"};
char PY_mb_pi[]    ={"��������������Ƥ��ƣơ��ƢƥƦƨƧƩ"};
char PY_mb_pian[]  ={"Ƭƫƪƭ"};
char PY_mb_piao[]  ={"ƯƮưƱ"};
char PY_mb_pie[]   ={"ƲƳ"};
char PY_mb_pin[]   ={"ƴƶƵƷƸ"};
char PY_mb_ping[]  ={"ƹƽ��ƾƺƻ��ƿƼ"};
char PY_mb_po[]    ={"����������������"};
char PY_mb_pou[]   ={"��"};
char PY_mb_pu[]    ={"������������������������������"};
char PY_mb_qi[]    ={"������������������������������������������������������������������������"};
char PY_mb_qia[]   ={"��ǡǢ"};
char PY_mb_qian[]  ={"ǧǪǤǨǥǣǦǫǩǰǮǯǬǱǭǳǲǴǷǵǶǸ"};
char PY_mb_qiang[] ={"ǺǼǹǻǿǽǾ��"};
char PY_mb_qiao[]  ={"������������������������������"};
char PY_mb_qie[]   ={"����������"};
char PY_mb_qin[]   ={"����������������������"};
char PY_mb_qing[]  ={"��������������������������"};
char PY_mb_qiong[] ={"����"};
char PY_mb_qiu[]   ={"����������������"};
char PY_mb_qu[]    ={"����������������ȡȢȣȥȤ"};
char PY_mb_quan[]  ={"ȦȫȨȪȭȬȩȧȮȰȯ"};
char PY_mb_que[]   ={"Ȳȱȳȴȸȷȵȶ"};
char PY_mb_qun[]   ={"ȹȺ"};
char PY_mb_ran[]   ={"ȻȼȽȾ"};
char PY_mb_rang[]  ={"ȿ��������"};
char PY_mb_rao[]   ={"������"};
char PY_mb_re[]    ={"����"};
char PY_mb_ren[]   ={"��������������������"};
char PY_mb_reng[]  ={"����"};
char PY_mb_ri[]    ={"��"};
char PY_mb_rong[]  ={"��������������������"};
char PY_mb_rou[]   ={"������"};
char PY_mb_ru[]    ={"��������������������"};
char PY_mb_ruan[]  ={"����"};
char PY_mb_rui[]   ={"������"};
char PY_mb_run[]   ={"����"};
char PY_mb_ruo[]   ={"����"};
char PY_mb_sa[]    ={"������"};
char PY_mb_sai[]   ={"��������"};
char PY_mb_san[]   ={"����ɡɢ"};
char PY_mb_sang[]  ={"ɣɤɥ"};
char PY_mb_sao[]   ={"ɦɧɨɩ"};
char PY_mb_se[]    ={"ɫɬɪ"};
char PY_mb_sen[]   ={"ɭ"};
char PY_mb_seng[]  ={"ɮ"};
char PY_mb_sha[]   ={"ɱɳɴɰɯɵɶɷ��"};
char PY_mb_shai[]  ={"ɸɹ"};
char PY_mb_shan[]  ={"ɽɾɼ��ɺɿ������ɻ������������դ"};
char PY_mb_shang[] ={"����������������"};
char PY_mb_shao[]  ={"����������������������"};
char PY_mb_she[]   ={"������������������������"};
char PY_mb_shen[]  ={"��������������������������������ʲ"};
char PY_mb_sheng[] ={"��������ʤ����ʡʥʢʣ"};
char PY_mb_shi[]   ={"��ʬʧʦʭʫʩʨʪʮʯʱʶʵʰʴʳʷʸʹʼʻʺʿ��������ʾʽ������������������������������������"};
char PY_mb_shou[]  ={"��������������������"};
char PY_mb_shu[]   ={"������������������������������������������������������ˡ����������"};
char PY_mb_shua[]  ={"ˢˣ"};
char PY_mb_shuai[] ={"˥ˤ˦˧"};
char PY_mb_shuan[] ={"˩˨"};
char PY_mb_shuang[]={"˫˪ˬ"};
char PY_mb_shui[]  ={"˭ˮ˰˯"};
char PY_mb_shun[]  ={"˱˳˴˲"};
char PY_mb_shuo[]  ={"˵˸˷˶"};
char PY_mb_si[]    ={"˿˾˽˼˹˻˺����������������"};
char PY_mb_song[]  ={"����������������"};
char PY_mb_sou[]   ={"��������"};
char PY_mb_su[]    ={"����������������������"};
char PY_mb_suan[]  ={"������"};
char PY_mb_sui[]   ={"����������������������"};
char PY_mb_sun[]   ={"������"};
char PY_mb_suo[]   ={"����������������"};
char PY_mb_ta[]    ={"����������̡̢̤̣"};
char PY_mb_tai[]   ={"̨̧̥̦̫̭̬̩̪"};
char PY_mb_tan[]   ={"̸̵̷̶̴̮̰̯̲̱̳̹̻̺̼̾̿̽"};
char PY_mb_tang[]  ={"��������������������������"};
char PY_mb_tao[]   ={"����������������������"};
char PY_mb_te[]    ={"��"};
char PY_mb_teng[]  ={"��������"};
char PY_mb_ti[]    ={"������������������������������"};
char PY_mb_tian[]  ={"����������������"};
char PY_mb_tiao[]  ={"������������"};
char PY_mb_tie[]   ={"������"};
char PY_mb_ting[]  ={"��͡����ͤͥͣͦͧ͢"};
char PY_mb_tong[]  ={"ͨͬͮͩͭͯͪͫͳͱͰͲʹ"};
char PY_mb_tou[]   ={"͵ͷͶ͸"};
char PY_mb_tu[]    ={"͹ͺͻͼͽͿ;��������"};
char PY_mb_tuan[]  ={"����"};
char PY_mb_tui[]   ={"������������"};
char PY_mb_tun[]   ={"��������"};
char PY_mb_tuo[]   ={"����������������������"};
char PY_mb_wa[]    ={"��������������"};
char PY_mb_wai[]   ={"����"};
char PY_mb_wan[]   ={"����������������������������������"};
char PY_mb_wang[]  ={"��������������������"};
char PY_mb_wei[]   ={"Σ��΢ΡΪΤΧΥΦΨΩάΫΰαβγέίή��δλζηθξνιμεοκ"};
char PY_mb_wen[]   ={"��������������������"};
char PY_mb_weng[]  ={"������"};
char PY_mb_wo[]    ={"������������������"};
char PY_mb_wu[]    ={"����������������������������������������������������������"};
char PY_mb_xi[]    ={"Ϧϫ����ϣ������Ϣ��Ϥϧϩ����ϬϡϪ��Ϩ����ϥϰϯϮϱϭϴϲϷϵϸ϶"};
char PY_mb_xia[]   ={"ϺϹϻ��Ͽ��ϾϽϼ������������������{"};
char PY_mb_xian[]  ={"ϳ�������������������������������������������������������������޺�����ϳ"};
char PY_mb_xiang[] ={"�����������������������������������������������߽�"};
char PY_mb_xiao[]  ={"����������������������С��ТФ��ЧУЦХ���������"};
char PY_mb_xie[]   ={"��ЩШЪЫЭавбгЯЬдйкжмелизѪв�����ߢ�����"};
char PY_mb_xin[]   ={"����о����п��н�������ܰ�ݷ"};
char PY_mb_xing[]  ={"��������������������������������ʡߩ"};
char PY_mb_xiong[] ={"��������������"};
char PY_mb_xiu[]   ={"�����������������������������"};
char PY_mb_xu[]    ={"������������������������������������������������ޣ"};
char PY_mb_xuan[]  ={"������������ѡѢѤѣ�����������"};
char PY_mb_xue[]   ={"��ѥѦѨѧѩѪ��"};
char PY_mb_xun[]   ={"ѫѬѰѲѮѱѯѭѵѶѴѸѷѳ޹��Ѥ�"};
char PY_mb_ya[]    ={"ѾѹѽѺѻѼ��ѿ��������������������"};
char PY_mb_yan[]   ={"����������������������������������������������������������������������������������������"};
char PY_mb_yang[]  ={"��������������������������������������������"};
char PY_mb_yao[]   ={"��������ҢҦҤҥҡң��ҧҨҩҪҫԿ����ߺ����"};
char PY_mb_ye[]    ={"ҬҭүҮҲұҰҵҶҷҳҹҴҺҸ��������"};
char PY_mb_yi[]    ={"һ����ҽ��ҿҼҾ����߽����������������������������������������������������������������������������������������������������ܲ���߮���������"};
char PY_mb_yin[]   ={"������������������������������ӡ�����ط�����"};
char PY_mb_ying[]  ={"ӦӢӤӧӣӥӭӯӫӨөӪӬӮӱӰӳӲ��ݺ�������������۫���"};
char PY_mb_yo[]    ={"Ӵ�"};
char PY_mb_yong[]  ={"ӶӵӸӹӺӷ��ӽӾ��ӿ��Ӽӻ��ٸ�ܭ�"};
char PY_mb_you[]   ={"�������������������������������������������������������"};
char PY_mb_yu[]    ={"������������������������������������������������������Ԧ����������ԡԤ������Ԣ��ԣ������ԥ���ع��������������"};
char PY_mb_yuan[]  ={"ԩԧԨԪԱ԰ԫԭԲԬԮԵԴԳԯԶԷԹԺԸ����"};
char PY_mb_yue[]   ={"ԻԼ��������Ծ��Խ�h"};
char PY_mb_yun[]   ={"������������������������ܿ�����۩"};
char PY_mb_za[]    ={"������զ"};
char PY_mb_zai[]   ={"������������������"};
char PY_mb_zan[]   ={"�������������"};
char PY_mb_zang[]  ={"������갲���"};
char PY_mb_zao[]   ={"����������������������������"};
char PY_mb_ze[]    ={"������������"};
char PY_mb_zei[]   ={"��"};
char PY_mb_zen[]   ={"����"};
char PY_mb_zeng[]  ={"���������"};
char PY_mb_zha[]   ={"������������բագէթըե��դ߸"};
char PY_mb_zhai[]  ={"իժլ��խծկ"};
char PY_mb_zhan[]  ={"մձճղհնչյոշռսջվ��տպ��"};
char PY_mb_zhang[] ={"��������������������������������"};
char PY_mb_zhao[]  ={"��������������������צ����گ����"};
char PY_mb_zhe[]   ={"���������������������ņ��������ѵ�"};
char PY_mb_zhen[]  ={"��������������������������������֡���������������"};
char PY_mb_zheng[] ={"֣��������������������֤��֢���ں"};
char PY_mb_zhi[]   ={"ְֱֲֳִֵֶַָֹֺֻּֽ֧֥֦֪֭֮֨֯֫֬֩��־������������ֿ����������������������������"};
char PY_mb_zhong[] ={"����������������������ڣ��"};
char PY_mb_zhou[]  ={"�����������������������������������"};
char PY_mb_zhu[]   ={"����ף����������������������������ס��ע��פ�����������������"};
char PY_mb_zhua[]  ={"ץ"};
char PY_mb_zhuai[] ={"ק�J"};
char PY_mb_zhuan[] ={"רשת׫׭׬�����"};
char PY_mb_zhuang[]={"ױׯ׮װ׳״��ײ"};
char PY_mb_zhui[]  ={"׷׵׶׹׺׸�����"};
char PY_mb_zhun[]  ={"׻׼��"};
char PY_mb_zhuo[]  ={"׿׾׽������������������ߪ�Ê����"};
char PY_mb_zi[]    ={"������������������������������֨������������������"};
char PY_mb_zong[]  ={"������������������"};
char PY_mb_zou[]   ={"����������"};
char PY_mb_zu[]    ={"������������������"};
char PY_mb_zuan[]  ={"׬����߬��"};
char PY_mb_zui[]   ={"���������"};
char PY_mb_zun[]   ={"��������ߤ��"};
char PY_mb_zuo[]   ={"������������������"};
char PY_mb_space[] = {""};

/*"ƴ�����뷨��ѯ���,������ĸ������(index)"*/
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

/*��������ĸ������*/
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


