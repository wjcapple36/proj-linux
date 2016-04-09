#ifndef CONSTANT_H
#define CONSTANT_H
//定义工作环境
//#define ARM_BOARD

//unit num power1, power2, mcu, otdr, olc, opc,adn so on
#define     NUM_SUBRACK                         16          //机框数目
#define     NUM_CARD                                16          //板卡数目
#define     NUM_COMM_CARD                   14           //业务板卡数目
#define     NUM_PORT                                 8          //最大的port端口数目
#define     HOST_NO                                    0           //主机编号
#define     MAX_F_DB                                   50       //otdr对话框绘图纵轴最大值
#define     MAX_SELSPAC                            20
//字符串长度
#define     TIME_STR_LEN                            20  //时间字符串长度

//槽位号和板卡号是否合法的状态
#define     FRAME_CARD_NO_OK                   0
#define     FRAME_NO_OVERFLOW                1
#define     CARD_NO_OVERFLOW                2
#define     FRAME_NO_CONFIG                    3
#define     CARD_NO_ONFIG                         4
//板卡类型，枚举
//               0             1         2        3       4         5          6      7       8    9
enum {NONE = 0, PWU, MCU, OPM, OSW, OTDR, OLS,OLP,GSM,TU};
//检查设备状态时,填写到设备类型位，用一表示设备拔出或者插入状态
/*
 *2016-01-12
 *板卡初始状态为0，定位已未定状态
*/
enum{ PULL_IN = 1, PULL_OUT};
//短信告警权限定义以及最低最大值,
enum {MSG_ALARM_FIBER = 1,MSG_ALARM_OPM,MSG_ALARM_HARDWARE,MSG_ALARM_COMMU,\
      MSG_ALARM_OLP_SWITCH};
//检查短信告警类型用到
#define MSG_ALARM_TYPE_MIN       MSG_ALARM_FIBER
#define MSG_ALARM_TYPE_MAX       MSG_ALARM_OLP_SWITCH
//告警类型
enum {ALARM_OPM  = 1,ALARM_LINE,ALARM_HARD_WARE,ALARM_COMMU,ALARM_OLP_SWITCH};
//短信告警级别
enum {MSG_ALARM_LEVE1 = 1,MSG_ALARM_LEVE2, MSG_ALARM_LEVE3,MSG_ALARM_LEVE4};
//告警级别
enum {ALARM_NONE = 0, ALARM_LEVE_1, ALARM_LEVE_2,ALARM_LEVE_3};
//定义电源电压
enum {PWU_V0 = 0,PWU_V48,PWU_V220 };
//检查告警级别用到
#define MSG_ALARM_LEVE_MIN       MSG_ALARM_LEVE1
#define MSG_ALARM_LEVE_MAX       MSG_ALARM_LEVE4


#define     DEV_TYPE_MAX             TU        //最大设备类型
#define     DEV_TYPE_MIN               NONE     //最小设备类型
#define      MAX_PT_NUM                  16000      //最大采样点数
#define      MAX_EVENT_NUM           500        //最大事件点数目
#define      INVALID_VALUE              100     //OTDR计算参数时段无效值
//定义环形缓冲区的长度
#define      RING_BUF_SIZE               0x8000   //长度32k
#define      LINKED_BUF_NUM           128    //链表缓冲区 128个
#define      BUF_LIST_SIZE                  256                     //链表buf
#define      BUF_LINST_ID                   36                      //链表标志
#define      SHORT_MSG_LEN                     256                      //短消息的长度
//定义事件类型 开始，反射，非反射，结束
#define      EVENT_TYPE_START      0
#define      EVENT_TYPE_REFLEC      1
#define      EVENT_TYPE_NO_REFLEC      2
#define      EVENT_TYPE_END                  3

//设备地址
//cu地址
#define  ADDR_CU    0X0000000e
//板卡地址
#define  ADDR_CARD 0x0000000b
//网管地址
#define  ADDR_NET_MANAGER 0x0000003e
//具有管理权限的主机地址
#define  ADDR_HOST_VIP  0x0000003e
//群发地址
#define  ADDR_MASS_SEND      0x30
//mcu地址 2015-10-23重新定义为CU的地址，原因是MCU代替了CU
//#define  ADDR_MCU 0x0000000a
#define  ADDR_MCU ADDR_CU

//Otdr 测量模式

//空闲状态
#define  OTDR_MOD_IDLE                      0
//告警测量
#define  OTDR_MOD_ALARM                      1
//点名测量
#define  OTDR_MOD_APPOINT                     2
//周期性测试
#define   OTDR_MOD_CYC                              3

//mainwindow信号量的使用宏
#define  SEM_NUM                            5
#define  SEM_OTDR_TSK                 0
#define  SEM_OTDR_DLG                 1
#define  SEM_T_OSW_OLP               2
#define  SEM_COMMN                      3
#define   SEM_MCU                           4
//tsk_otdrManage 测量时需要知道回应码发给谁
#define  ACK_TO_NONE                0
#define  ACK_TO_NM                     1
#define  ACK_TOT_DLG                 2

//定义立即返回的数据等待的时间
#define  DATA_RETURN_WAIT_TIME     3000

//Otdr测量阶段
//测量未开始
#define OTDR_MEASUR_STAGE_0             0
//等待回应
#define OTDR_MEASUR_STAGE_1             1
//等待曲线
#define OTDR_MEASUR_STAGE_2             2

//定义网管存在或者消失
#define  NM_PRE_LOST                              -1
#define  NM_LOST                                        0
#define  NM_EXIST                                       1
//命令码，通知对话框更新板卡组成信息, 内部通信使用
#define  CMD_REFRES_FRAME_CARD    1
//数据库自定义ID
#define DB_COMM_ID_FRAME_COMMP    0x3000000     //机框组成信息
#define DB_COMM_ID_CARD_COMMP    0x3000001      //板卡组成信息
#define DB_COMM_ID_OPM_ALARM        0x3000002      //总的光功告警
#define DB_COMM_ID_OLP_ACTION       0x3000003      //OLP切换记录
#define DB_COMM_ID_HW_ALARM        0x3000004      //总的硬件告警

//操作数据库回调函数错误码
#define DB_ID_ERROR                 1           //ID错误
#define DB_LEN_ERROR               2           //数据长度错误

#define     DB_RET_PTR_EERO                 -3
#define     DB_RET_EMPTY                        -2
#define     DB_RET_OTHSER_ERROR       -1
#define     DB_RET_SUCESS                       0

//定义命令码
////////////////////////////////////////////////////////////////////////////////
// 命令类型:网管与MCU之间的通信，命令格式，id--src--dst--cmd
//NM net manager  mcu 工控机 card 板卡
#define ID_NM_MCU_GET_CARD_TYPE             0x80000005
#define ID_MCU_NM_RE_CARD_TYPE               0x80000006
#define ID_MCU_NM_RE_CARD_VERSION        0x80000036
//MCU与板卡通信
#define ID_MCU_CARD_GET_DEVTYPE               0x60000000	///<工控板查询设备板卡类型信息
#define ID_CARD_MCU_RET_DEVTYPE               0x60000001	///<返回设备类型信息
#define ID_CARD_MCU_RET_OPM_ALARM          0x80000029	///<返回设备类型信息


//#define ID_CU_NOTE_NET               0x60000002///<工控板与CU通信的网段选择切换通知
//#define ID_CU_NOTE_MANAGE		0x60000003///<CU通知工控板网管的连接状态

//错误应答码
#define ID_MCU_NM_RCODE_OK                   0       //一切顺利，ok
#define ID_MCU_NM_RCODE_NO_CARD                   1       //板卡不存在
#define ID_MCU_NM_RCODE_COMMU_EROR                   2       //通信异常
#define ID_MCU_NM_RCODE_FRAME_EROR                   3       //从机号错误
#define ID_MCU_NM_RCODE_CARD_EROR                   4       //槽位号错误
#define ID_MCU_NM_RCODE_DEV_TYPE_EROR                   5       //设备类型不一致
#define ID_MCU_NM_RCODE_PARA_EROR                   6       //参数非法
#define ID_MCU_NM_RCODE_PARA_NOT_SAVE                   7       //命令参数无法保存
#define ID_MCU_NM_RCODE_CMD_EXIST                   8       //已经存在一条命令
#define ID_MCU_NM_RCODE_OTDR_EROR                   9       //测量异常
#define ID_MCU_NM_RCODE_OTDR_TIMEOUT                   10       //测量超时
#define ID_MCU_NM_RCODE_UPDATA_EROR                   11       //升级异常
#define ID_MCU_NM_RCODE_CMD_INVALIDE                   12       //无效命令


//#define ID_CMD_SMS_ERROR		27	///<RTU返回短信操作返回码

//#define ID_ALARM_MEASUR			0x80000035	//OPM告警之后的otdr测量
//#define ID_ALARM_CURV			0x80000035	///告警测量得到的曲线
//#define ID_APPOINT_MEASUR			0x80000023	// 点名测量
//#define ID_APPOINT_CURV			0x80000024	//店名测量得到的曲线

#define USR_DEBUG  1        //  如果处调试模式，将会输出调试信息，正式发布的时候，调试信息需要屏蔽

//通信故障类型
#define COMMU_STATE_CHANGE                  1

#define ALARM_HW_COMMU_OK                  0
#define ALARM_HW_COMMU_ERROR                  1
#define ALARM_HW_CARD_TYPE_UN_MATCH                 2
#define NM_LOST_JUD_TIME                    15

//光开关级联深度3，不包含接有光纤的端,比协议增加了一级冗余度
#define OSW_CONNECT_DEPTH               3
//级联的末端是otdr，开始有可能是otdr,olp,osw
#define OSW_ROUT_EQ_NUM                    OSW_CONNECT_DEPTH + 2
//查询模块路由时BUF长度
#define ROUT_BUF_LEN            OSW_CONNECT_DEPTH*2 + 1
//定义光开关的公共端口
#define COMN_COM        0xEFFFFFFF
//数据库操作
#define DB_GET_RECORD_NUM       0
#define DB_GET_RECORD                 1
//主控单元的机框，槽位
#define MCU_FRAME           0
#define MCU_CARD        NUM_COMM_CARD - 1
#define TU_CARD          MCU_CARD
//定义用户事件
#define USR_EVENT_FIND_ALARM  0

#define TSK_WILL_DEAD  1
#define TSK_NORMAL  1
#define TSK_INITIAL_NO     0
#define TSK_INITIAL_YES     1
//对某端口进行测量，首先检查该端口是否是某个关联关系中的一环，如果是，则不能进行测量
#define  PORT_CHECK     1
#define  PORT_MEASUR  0
//olp切换类型
#define OLP_SWITCH_MANUE    1    //人工切换
#define OLP_SWITCH_AUTO         2 //自动保护切换
#define OLP_SWITCH_RETURN  3    //保护返回
//板卡组成信息buf,需要根据_tagCardAttribute的size进行调整
#define CARD_COMPOSITION_SIZE   NUM_CARD*NUM_SUBRACK*32
#define SOFT_VERSON_LEN         48
//文件路径长度和文件名字长度
#define FILE_PATH_LEN       128
#define FILE_NAME_LEN       32
//电源个数
#define POWER_NUM   2
#ifdef GPIO_EDITION_12

#define CONFIG_BOARD  12
//定义GPIO口
#define GPIO_PIN_NUM            30 //IO口数目
#define PIN_IP_SET                  0       //ip段 0 192.168.1.x网段 1 192.168.0.1.x网段
#define PIN_ALARM_SILENCE       20  //告警消音按键
#define PIN_LED_ALARM_TOTAL         21  //总的告警灯，亮，表示有告警
#define PIN_LED_RUN         22                      //运行灯
#define PIN_LED_ALARM_COMM          24 //通信告警灯
#define PIN_SMS_SOFT_RESET     25   //短信软重启
#define PIN_SMS_HARD_RESET     26  //短信猫硬重启
#define PIN_SWITCH_RESET                  27    //交换机重启
#define PIN_ALARM_BUZZER                  29    //告警蜂鸣器
#else
#define CONFIG_BOARD  11
#define GPIO_PIN_NUM            27 //IO口数目
#define PIN_IP_SET                  0       //ip段 0 192.168.1.x网段 1 192.168.0.1.x网段
#define PIN_ALARM_SILENCE       20  //告警消音按键
#define PIN_LED_ALARM_TOTAL         21  //总的告警灯，亮，表示有告警
#define PIN_LED_RUN         22                      //运行灯
#define PIN_LED_ALARM_COMM          23 //通信告警灯
#define PIN_SMS_SOFT_RESET     24   //短信软重启
#define PIN_SMS_HARD_RESET     25  //短信猫硬重启
#define PIN_SWITCH_RESET                  26    //交换机重启
#define PIN_ALARM_BUZZER                  PIN_LED_ALARM_TOTAL    //告警蜂鸣器和告警灯相同
#endif

#define PIN_PW_NUM  6           //电源状态IO口数量
#define PIN_CARD_NUM           13 //MCU端口永远是插入状态，不会有拔出状态的
#define PW_STATE_NORMAL     0
#define PW_STATE_ERROR       1


//定义一个未定义错误
#define  UNDEFINED_ERROR            0x100000ff
//重启类型
#define  REBOOT_TYPE_NORMAL     0
#define  REBOOT_TYPE_ABNORMAL   1

//定义发送类型，包括查询，主动上报，重发
#define SEND_TYPE_ASK   0       //查询
#define SEND_TYPE_INITIATIVE   1 //主动上报
#define SEND_TYPE_RETRY         2 //重发
//定义告警曲线处理类型
#define ALARM_CURV_DEL                 3
//OTDR测试原因：周期性测量，光功告警测量，光功告警消失测量,OLP切换
#define TEST_CYC       0
#define TEST_OPM_ALARM_COME       1
#define TEST_OPM_ALARM_AWAY       2
#define  TEST_OLP_ACTION                   3
//告警延迟时间
#define ALARM_DELAY_TIME_S          90
//无效fd
#define INVALID_FD  0

#endif // CONSTANT_H
