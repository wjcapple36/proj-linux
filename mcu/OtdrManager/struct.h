#ifndef STRUCT_H
#define STRUCT_H
#include "constant.h"
#include <QPoint>
#include <QMutex>
#include <QQueue>
#include <QVector>
#include <QSemaphore>
#include <QStringList>
#include <QTimer>
//#include "protocol/tmsxx.h"
#pragma pack(1) //按1个字节对齐
//总段机框信息
typedef struct
{
    int     numInUse;
    int     numTotal;
    int     curShowNo;
    int     onState;
    char  oprateTime[NUM_SUBRACK][20];
}_tagSubrackInfo;

//每一个机框插卡状态
#define CARD_OPT_NUM    4
typedef struct
{
    int     numInUse;
    int     numTotal;
    int     onState;
    int     type[NUM_CARD];
    int     ports[NUM_CARD];
    int     opt[NUM_CARD][CARD_OPT_NUM];//4个保留位
    char  oprateTime[20];
}_tagSubrackCard;
//曲线信息
typedef struct
{    
    int xStart;             //x轴起点
    int xEnd;             //x轴末点
    int lineA;             //标杆A
    int lineB;             //标杆B
    int zoomTimesH;             //x轴放大倍数
    int zoomTimesV;             //y轴放大倍数
    float yStart;                //y轴起点
    float yEnd;             //y轴末点
    int sampleNum;             //采样点数
    int IsDraw;             //是否画图
    int moveLineA;             //是否移动标杆A
    int xOrigPt;             //原点对应段位置
}_tagDrawCtrl;
//测量参数
typedef struct
{
    int range_m;                                //量程
    int lamda_ns;                              //波长
    int pulseWidth_ns;                      //脉宽
    int measureTime_s;                    //测量时间
    float n;                                            //折射率
    float endThreshold;                        //结束门限
    float NonRelectThreshold;             //非反射门限
    int samplHz;                               //采样率赫兹
    //int rserv[2];
    int test_reason; //测试原因
   unsigned int src_ip; //发出点名测量的IP，CTU-----RTU有用
}_tagMeasurPara;                           //测量参数
//事件点
typedef struct
{
    int pos;                                        //位置
    int type;                                       //类型
    float attenCoef;                           //衰减系数
    float insertLoss;                          //插损
    float reflectLoss;                         //回波损耗
    float totalLoss;                            //累计损耗
}_tagEvent;
//测量结果
typedef struct
{
    float fiberLen;                             //链长
    float fiberLoss;                           //链损耗
    float fiberAtten;                          //链衰减
}_tagMeasurResult;

//告警门限
typedef struct
{
    int seriousAlarmTh;                    //紧急告警门限
    int primaryAlarmTh;                    //主要告警门限
    int slightAlarmTh;                       //次要告警门限
}_tagAlarmTh;

//struct _tag

struct _tagBufList
{
    struct _tagBufList * next;
    struct _tagBufList * pre;
    int bufSize;    //buf大小，字节
    int dataLen;    //缓冲区内字节长度，bufList head有效
    int bufListID; // 此位表明随缓冲区段标志段
    char buf[BUF_LIST_SIZE];
};
struct _tagDataHead
{
    unsigned int cmd;
    int dataLen;
};

//对话框属性，包括机框号，板卡号，设备类型
struct _tagDlgAttribute
{
    int frameNo;
    int cardNo;
    int devType;
};
//处理结果返回结构体
struct _tagResCode
{
    unsigned int cmd;
    int bytes;
    int res_code;
    unsigned  int res_cmd;
    int reserved[4];
};
//板卡版本号
struct _tagCardVersion
{
    int frameNo;
    int cardNo;
    int devType;
    char softV[48];
};
//设备类型，端口数目,动态更新
struct _tagDevType
{
    int type[NUM_CARD];     //设备类型
    int port_num[NUM_CARD]; //端口数目
    int opt[NUM_CARD][CARD_OPT_NUM];    //保留项
};
//控制变量
struct _tagCtrlStat
{
    //设备类型是一个枚举行，最大的那个设备类型
    int minDevType;
    int maxDevType;
    //网管状态
    int PreNMStat; // 网管预状态
    int NMstat; //默认随主网管状态
    int vice_NMstat;    //其他网管是否存在
    int will_reboot;
    int save_cyc_time;    
    int rcvSocData;
    int  io_fd;//io口的状态   
    //通信状态
    int commuStat[NUM_SUBRACK][NUM_CARD];
};
//主网管连接之后上报相关数据
#define PERMIT_SEND            0   //允许发送数据
#define UNPERMIT_SEND  -1       //不允许发送数据
struct _tagCtrActivUpData
{
    int  IsPermitRet32;//允许发送80000*32曲线
    int  IsPermitRet80; //允许主动上报olp切换记录;
    int IsPermitRet77;  //允许主动发送总的光功告警
    int IsPermitRet31;  //允许发送总的硬件告警
};
//告警测试队列内容，根据以下内容查找对应的测量参数
struct _tagPortOSW
{
    //机框号
    int frame;
    //板卡号
    int card;
    //槽位号
    int port;
    //关联设备
    int relate_dev;
    //回应码给谁
    int ack_to;
};
//板卡地址
struct _tagCardAddr
{
    int frame;
    int card;
    int port;
};

struct _tagDevComm
{
    int frame_no;
    int card_no;
    int type;
    int port;
};
//端口测试参数
struct _tagPortPara
{
    //光开关端口
    _tagPortOSW osw_port;
    //关联设备地址
    _tagDevComm relate_dev;
    //otdr的地址
    _tagCardAddr otdr;
};
//点名测量选项内容
struct _tagAppointCtrOpt
{
    unsigned int src; //源地址
    unsigned int dst; //目的地址
    unsigned short pkid;//源pkid
};
//点名测量,被插光纤的端口(osw, olp, otdr)
struct _tagCtrlAppointTest
{
    int ack_to;
    unsigned int cmd; //命令    
    _tagDevComm  test_port;
    //测量参数
    _tagMeasurPara para;
    _tagAppointCtrOpt opt;
};
//端口测试信息
struct _tagCtrlPortTest
{
    int ack_to;
    unsigned int cmd; //命令
    int test_reason; //测试原因
    _tagDevComm  test_port; //被测试端口
};
//告警测试队列
struct _tagAlarmTestQueue
{
    QMutex objMutex;
    QQueue<_tagCtrlPortTest> xqueue;
};
//点名测量队列
struct _tagAppointTestQueue
{
    QMutex objMutex;
    QQueue<_tagCtrlAppointTest> xqueue;
};
struct _tagOpticalAlarm
{
    int port;
    int level;
    int curPower;
    char time[TIME_STR_LEN];
};
//板卡属性，如果大小发生变化，须修改CARD_COMPOSITION_SIZE
struct _tagCardAttribute
{
    int frameNo;
    int cardNo;
    int devType;
    int ports;
    int opt[4];
};
//周期性测量端口信息
struct _tagOswCycCfg
{
    int frame;
    int card;
    int type;
    int port;
    int isTest;
    int cycle;
};
//周期性测量头信息
struct _tagCycleTest
{
    int frameNo;
    int cardNo;
    int type;
    int portNum;
};
//记录下次周期性测量时间
struct _tagNextCycleTest
{
    int framNo;
    int cardNo;
    int devType;
    int portNo;
    int iscyc;
    int cycle;
    int next;
};
struct _tagVectorCycleTest
{
    QMutex objMutex;
    QVector <_tagNextCycleTest> xvector;
};
//信号量，同步时使用，cmd表明随哪条命令在等待信号量
struct _tagObjSynvSem
{
    unsigned int sendCmd;//发出的cmd
    unsigned int resCmd; //等待回应段cmd
    int resCode;//回应码
    int resrvd3;//对应ack中的reservd3
    unsigned short pkid;
    _tagBufList *pBufListHead;
    QSemaphore objSem;//同步信号量
    QMutex objMutex; //锁，防止重入
};
//Otdr控制变量
struct _tagOtdrCtrl
{
    _tagCardAddr addr;
    int mod;            //0,空闲，1，点名测量，2，告警测量,3周期性测量
    int resCode;       //设备的回应码 0,表示成功，其他失败码
    int errorNum;      //失败次数
    int curStage;         //当前状态，0，其他，1，第一阶段，回应，2，等待曲线
    int cur_test_time; //当前正在测量的时间
    int cur_usr;        //当前使用者 0 空闲，1网管点名测量，2，周期性测量，3MCU点名量
};
struct _tagAnyOsw
{
    int any_port;
    int frame_no;
    int card_no;
    int type;
    int osw_port;
};
//参考曲线
// OTDR参考曲线头
struct otdr_ref_hdr
{
    int osw_frame;		///< osw机框号
    int osw_slot;		///< osw槽位号
    int osw_type;		///< osw设备类型
    int osw_port;		///< osw端口
    int otdr_port;		///< otdr端口
    int  strid[20];		///< OTDR测量参数的识别ID，

};
struct _tagReferCurv
{
    //光开关信息
    otdr_ref_hdr ref_head;
    _tagMeasurPara measurPara;
    //数据点
    char dpid[12];
    int ptNum;
    unsigned short dataPt[MAX_PT_NUM];
    //事件点信息
    char eventID[12];
    int eventNum;
    _tagEvent eventBuf[MAX_EVENT_NUM];
    //测量结果
    char measurResultID[20];
    _tagMeasurResult measurResult;
    //告警门限
    _tagAlarmTh alarmTh;
};
//Otdr返回曲线
struct _tagOtdrCurv
{
    //光开关port
    _tagDevComm oswPort;
    char measurTime[20];
    _tagDevComm otdrPort;
    //测量参数
    _tagMeasurPara measurPara;
    //数据点信息
    char dpid[12];
    int ptNum;
    unsigned short dataPt[MAX_PT_NUM];
    //事件点信息
    char eventID[12];
    int eventNum;
    _tagEvent eventBuf[MAX_EVENT_NUM];
    //测量结果
    char measurResultID[20];
    _tagMeasurResult measurResult;
};
//otdr curv 加了保护变量
struct _tagRcvOtdrCurv
{
    QMutex mutexObj;
    _tagOtdrCurv curv;
};
//板卡通信状态
#define CARD_ALARM_LOCAL_MAX_VLAUE  999  // 本地能够产生的最大硬件告警原因
#define CARD_ALARM_RECOVER   0  //通信恢复正常
#define CARD_ALARM_LOST             1 //通信中断
#define CARD_ALARM_TYPE_DIFFERENT    2 //板卡类型不一致
#define CARD_ALARM_NEW                 3 //新增板卡
#define CARD_ALARM_PORT_DIFFERENT    4 //端口不一致
#define CARD_ALARM_ATTRIBUT_DIFF        5 //属性发生变化
#define CARD_ALARM_PULL_OUT    6            //板卡拔出
#define CARD_ALARM_POWER_ABNORMAL    7            //电源不正常
#define CARD_ALARM_POWER_NORMAL    8            //电源正常
#define CARD_ALARM_SMS_EQ_ERROR    9            //短信模块不存在或者故障

#define HW_ALARM_REASON      1000   //OLP内部光开关损坏



struct _tagDevCommuState
{   
    int cur_type;           //当前设备类型
    int cur_port;       //当前的端口数目    
    int last_alarm;   //上次的告警状态
    int cur_alarm;    //当前告警的状态
    int card_state; //设备插拔状态
    int pw_state;//电源的状态
    int opt; //如果板卡插入，但一直没有通信，那就开始计数
    int cur_opt[CARD_OPT_NUM]; //属性数目
    char alarm_time[TIME_STR_LEN];
    //int alarm_change;       //告警是否发生变化
};
#define GSM_TEXT_LEN 512
#define GSM_BUF_SIZE         128
#define GSM_PHOME_NUM   16
#define PHONE_ACTUAL_NUM    11
//短信队列
struct _tagGsmContext
{
    _tagAppointCtrOpt usr_addr;
    int count;    
    char phone[GSM_PHOME_NUM];
    unsigned short context[GSM_TEXT_LEN];
};
struct _tagGsmQue
{
    QMutex objMutex;
    QQueue <_tagGsmContext> xqueue;
};
/*
 *路由级联中基本的模块信息
*/
struct _tagOswRoutUnit
{
    int srcIP;
    int dstIP;
    _tagDevComm Dev;
};
/*
 *数据库中一条基本的记录格式
*/
struct _tagRoutRecord
{
    int id;
    int srcIP;
    int dstIP;
    _tagDevComm DevA;
    _tagDevComm DevB;
};
//osw otdr初始化时使用，记录本设备关联的路径
struct _tagRoutBuf
{
    int record_num;
    _tagRoutRecord buf[ROUT_BUF_LEN];
};
/*
 *路由级联中具有可操作端口的设备,级联路由中首位全零，还有公共端口
 *本结构体不包含首位全零和公共端口部分
*/
struct _tagOswRout
{
    int depth;
    _tagOswRoutUnit table_buf[OSW_ROUT_EQ_NUM];
};
struct _tagPortState
{
    int test_time; //正在进行测试时间
    int state;//状态，空闲，或者忙
    int ack_to; //回应码发给谁，网管，本地，不回应
    int port_type;   //当前正在测量的端口的类型，olp, osw, otdr
    int wait_time;//等待的时间
};
//tsk 队列管理
struct _tagOtdrTskQue
{
    QList <void *> xlist;
};
struct _tagOtdrDevQue
{
    QList<_tagDevComm> xlist;
};
struct _tagResPortInfo
{
    int port;
    _tagAppointCtrOpt opt;
};

struct _tagWaitCurvPort
{
    QMutex obj;
    QList<_tagResPortInfo> xlist;
};
struct _tagCallbackPara
{
    int operate_type;   //操作类型：0，获取条目；1获取记录
    int record_num; //条目
    int operate_num;
    _tagDevComm *buf;
};
/*
 *2016-03-31
 *该结构体与_tagCallbackPara作用相同，后期增加了可查询表记录数目的函数
 *并且使用char 型的buf，更有通用性
*/
struct _tagDBCallbackPara
{
    int  list_num;
    int  index;
    char *dst_buf;
};

//数据库读取曲线的类型
#define CURV_TYPE_CYC   0
#define CURV_TYPE_ALARM 1
struct _tagDbReadCurv
{
    int curv_type;
    void * ptr;
};
//otdr 模块参数
struct _tagOtdrModulPara
{
    int initial;
    QStringList lamdaList;
};
struct _tagUsrTimer
{
    QTimer *ptr_timer;
};
#define OPM_PORT_NUM 8
struct  _tagLOpmAlarmUit
{
    int alarm_chang;//告警是否发生变化，作用时间有限
    int lev;    //告警级别0，表示无告警，其他表示告警级别
    int power;//光功率
    char come_time[TIME_STR_LEN]; //产生时间
    char fade_time[TIME_STR_LEN];//消失时间
};
struct _tagFSOpmAlarm
{
    int frame;
    int card;
    int type;
    int port_num;
    int cur_alarm_num; //告警总的数目
    //_tagLOpmAlarmUit alarm[OPM_PORT_NUM];
    QVector<_tagLOpmAlarmUit> alarmArray;
};
struct _tagTotalOPMAlarm
{
    QMutex mutexBoj;
    QList<_tagFSOpmAlarm> OpmList;
};
//根据应用场景，使用最频繁的是变化的光功告警，按8个端口全部变化，最长字节276
//将来OPM端口发生编号，要随之发生改变
#define SOCK_DATA_CPY_BUF_LEN   20 + 8*32
#define RETRY_NUM               3
#define BUF_TYPE_FIXED         0
#define BUF_TYPE_DYANM         1
#define RETRY_LIST_LEN             512
//主动上报的数据需要用户回应，否则重发
struct _tagSockDataCpy
{
    unsigned int cmd;  //帧标志
    int src;    //源地址，一般设为MCU本地地址
    int dst;    //目的地址
    int pkid;   //pkid流水号
    int timer;  //计时器
    int  retv;  //返回值
    int retry_send_num;
    int needed_retry_num; //重发次数
    int buf_type;   //使用固定存储器还是动态分配，如果是动态分配的话，将会使用结束的释放
    char buf_fixed[SOCK_DATA_CPY_BUF_LEN]; //存储的是cmd data_len 之外的数据
    char *buf_dyanm;
};
//重发数据管理List
struct _tagSockRetryList
{
    QMutex obj;
    QList<_tagSockDataCpy> list;
};
#define HW_ALARM_MAX_NUM    256 //16个机框，每个机框16个槽位，足够
//总的硬件告警
struct _tagTotalHwAlarmCell
{
    int is_change;  //是否发生变化
    int lev;    //告警级别
    int frame;  //机框号
    int card;   //槽位号
    int reason; //告警原因
    char time[TIME_STR_LEN];    //告警发生时间
};
struct _tagHwAlarmBuf //总的硬件告警总数
{
    QMutex obj;
    QList<_tagTotalHwAlarmCell> list;
};
//OLP切换动作
struct _tagOlpActionRecordCell
{
    int frame; //机框
    int card;   //槽位
    int type;   //设备类型
    int action_type; //切换类型
    int to_port;    //切换到的端口
    char time[TIME_STR_LEN]; //切换时间
};
//olp切换记录最大条数500条，超过500条循环覆盖
#define OLP_ACTION_MAX_NUM      500 //保存总的OLP切换记录条数
struct _tagOlpActionRecordBuf //总的硬件告警总数
{
    QMutex obj;
    int cur_total_record; //当前缓冲区内已经存在的切换记录条目
    int cur_index;  //当前索引
    int buf_size; //缓冲区size
    QVector<_tagOlpActionRecordCell> list;
};
//设备信息，板卡设备一旦连接上，就发送该消息，设备
struct _tagDevInfo
{
    int frame;
    int card;
    int type;
    int port_num;
    int reservd[CARD_OPT_NUM];
};
#define GPIO_ALARM_OPEN   1
#define GPIO_ALARM_CLOSE    0
#define SN_LEN 128
struct _tagDevCfg
{
    char sn[SN_LEN];//SN序列号
    int gpioAlarm;//告警输出状态
    char buzzing_time[TIME_STR_LEN];
};
#pragma pack(0) //按1个字节对齐
#endif // STRUCT_H
