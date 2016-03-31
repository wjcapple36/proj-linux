#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QHBoxLayout>
#include <QPushButton>
#include  <QTimer>
#include <QSemaphore>
#include <QTableWidget>

#include "constant.h"
#include "struct.h"
#include "qmanagelinkedbuf.h"
#include "qmanageringbuf.h"
#include "tsk_datadispatch.h"
#include "tsk_otdrmanage.h"
#include "tsk_host_commu.h"
#include "tsk_sockretrysend.h"
#include "tsk_sms_send.h"

//指定用C编译器编译如下文件，因为下面文件本身就是用c语言编写
#ifdef __cplusplus
extern "C" {
#endif
#include "osnet/epollserver.h"
#include "src/ep_app.h"
#include "src/tms_gpio.h"

#ifdef __cplusplus
}
#endif

//定义本类内部使用段常量
#define  QUEUE_LEN_ALARM_MEASUR    2048
#define  QUEUE_LEN_APPOINT_MEASUR    1

namespace Ui {
class MainWindow;
}
struct _tagCountTimer
{
    int countCheckCyclTest;
    int coutUpdateDevType;
    int start_cyc_test;
    QTimer *Timer;
};

int process_data_from_sock(char buf[], int dataLen, int msec, void *ptr_option);
int dlg_send(char devbuf[], char buf[], unsigned int  cmd, unsigned int res_cmd, _tagBufList **pBufHead, int ack_to,\
             int sem_index = SEM_COMMN);
int send_cmd(int fd, char buf[], int cmd, char *opt = NULL);
class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    //主页面显示时间的timer
    _tagCountTimer countTimer;
    //每个槽位的设备类型也即，对话框类型
    int m_dlgType[NUM_CARD];
    //设备配置信息
    _tagDevCfg devCfg;
    //机框组成信息
    _tagSubrackInfo m_subrackInfo;
    //板卡信息
    _tagSubrackCard m_subrackCard[NUM_SUBRACK];
    //每个槽位的响应函数
    QPushButton *m_pushBtnCard[NUM_CARD];
    //接收缓冲区
    char *RcvBuf;
    //接收链表
    _tagBufList *BufList;
    //链表管理类
    QManageLinkedBuf     LinkedBufManager;    

    //每弹出一个对话框++，销毁一个--，为0表示无对话框
    volatile int m_currenShowDlg;
    volatile void * m_ptrCurentDlg;
    //Otdr测量队列
    _tagAlarmTestQueue queAlarmMeasur;
    //点名测量队列
    _tagAppointTestQueue queAppointMeasur;
    //周期测量队列
    _tagAlarmTestQueue queCycleMeasur;
    //设备类型部分
    QStringList m_strlstEqType;
    //comb设备列表框中全部的
    QStringList m_strlstAllEqType;
    //设备端口书目
    QStringList strlstDevPortNum;
    //otdr端口数目
    QStringList strlstOtdrPortNum;
    //olp端口数目
    QStringList strlstOlpPortNum;
    /*
     *******************关联任务********************************
    */
    //数据分发otdr管理tsk
    class tsk_dataDispatch *pDataDispatch;
    //class tsk_OtdrManage *pOtdrManage;
    class tsk_host_commu *pHostCommu;
    //重发任务
    class tsk_SockRetrySend *pSockRetry;
    class tsk_SMS_Send  *pSmsSend;
    class tsk_OtdrManage    *pCtuAsk;
    /*
     ****************************************************************
    */
   // _tagDevType dynamicDevtype[NUM_SUBRACK];
    //控制状态
    _tagCtrlStat m_ctrlStat;
    //控制是否主动上报数据
    _tagCtrActivUpData m_ctrlUpData;
    //当前显示dlg段属性
    _tagDlgAttribute curShowDlgAttr;
    //otdr曲线，参考曲线
    _tagRcvOtdrCurv OtdrCurv;
    _tagRcvOtdrCurv AlarmCurv;
    _tagReferCurv ReferCrv;
    //周期性测量容器
    _tagVectorCycleTest vectorCycleTest;
    //环形缓冲区管理类
    QManageRingBuf      RingBufManager;    
    _tagOtdrCtrl m_otdrCtrl;
    _tagOtdrDevQue m_otdrDevQue; //系统中otdr队列
    _tagOtdrTskQue m_otdrTskQue; //otdr管理线程类
    _tagCardAddr m_addrGsm;
    //每个板卡一个
    _tagObjSynvSem devLock[NUM_SUBRACK][NUM_COMM_CARD];
    //周期性测量数据data buf event buf
    unsigned short cyc_buf[MAX_PT_NUM];
    _tagEvent cyc_event_buf[MAX_EVENT_NUM];
    //板卡通信状态
    _tagDevCommuState  DevCommuState[NUM_SUBRACK][NUM_CARD];

    unsigned short pkid;
    QString strSoftVerson; //软件版本号
    int isGetOtdrPara; //是否获取Otdr模块参数
    char cfg_file_path[FILE_PATH_LEN];
    int mcu_stat[GPIO_PIN_NUM];

    /*
     **********************告警celement***************************************
    */
    _tagTotalOPMAlarm  total_opm_alarm;//总的光功率告警
    _tagOlpActionRecordBuf OlpActionRecordBuf; //OLP切换记录缓存
    _tagHwAlarmBuf  HwAlarmBuf; //硬件告警缓存

    /*
     ********************************************************************************
    */

    /*
     **********************互斥信号量***************************************
    */
    QMutex objDealCycCurv;//周期性测量曲线锁
    QMutex objPkid;//产生流水号的同步
    QMutex objDlgSend;//对话框调用发送时的同步
    QMutex objOperateFrameCard; //初始化板卡组成信息的互斥体
    QMutex objHwAlarm; //硬件告警锁

    QMutex mutexRcvData;//处理接收函数互斥

    QMutex refreshOtdrDev; //正在更新otdr的标志
    //环形缓冲区的信号
    QSemaphore semRingBuf;
    //处理结果同步信号2个,通知dlg作相应段处理
    _tagObjSynvSem objSyncSem[SEM_NUM];
    /*
     ********************************************************************************
    */

public:   
    //创建对话框
    void creatDlg(int curFrameShow, int cardNo);
    //更新主页面时间
    void timeUpdate();
    //初始化机框和插槽信息
    void InitialSubrackAndCadInfo();
    //初始化插槽类型
    void InitialCardType();
    //初始化每个槽位段按钮指针
    void InitialCardBtn();
    //获取设备类型
    int    GeteqType();
    void setWindowsStyle();
    //将接收到的数据放入环形缓冲区
    int inputDataToRingBuf(char buf[], int dataLen, int msec);
    int processRcvData(int cmd, int dataLen, _tagBufList *pDataHead);

    //获取系统时间,让各个模块调用
    void GetCurrentTime(char timeBuf[]);
    //检查命令是否合法
    int checkCMD(int cmd);
    //mcu返回nm板卡信息
    int mcuToNMCardsType();
    //mcu返回nm响应码
    int mcuToNMResultCode(int code);
    //mcu返回NM线路告警
    int mcuToNMFiberAlarm();
    //mcuNM返回总的OPM告警
    int mcuToNMAllOPMAlarm();
    //    int inspectFiber(_tagReferCurv referCrv, _tagOtdrCurv curOtdrCrv);
    int upload_alarm_cyc_data(void * ptr_alarmline,  _tagOtdrCurv *ptr_curv, unsigned int cmd, void * ptr_opt);
    //获取告警级别
    int getAlarmLev(float deltaLoss, _tagAlarmTh *pAlarmTh);
    //mcu处理异常
    int dealAbnormal(QString str, int isYesNo = 0);
    //直接以cmd段方式处理异常
    void dealAbnormalResCod(int rescode);
    //计算下次周期性测量的时间
    int  isStartCycleTest(_tagNextCycleTest &cycleTest);
    //保存周期性测量时间
    int db_save_cyc_time();  
    //mcu测试数据,数据未来之前，临时测试
    void testRingbuf();
    //从链表缓冲区中拷贝数据
    int cpyDataFromLinkedBuf(char buf[], int dataLen, _tagBufList *pBufHead );
    //检查从机号，槽位号是否处于合理范围内
    void checkFrameCardNo(int framNo, int cardNo, int &framStat, int &cardStat);
    // 处理接收到的命令

    //收到设备类型
    int dealRcvCardDevtype(char buf[], void *ptr_opt);
    //收到网管状态，新建连接还是全部网管消失
    int dealRcvHostNetManagStats(char buf[] , void *ptr_opt);
    //收到opm告警
    int dealRcvCardOpmAlarm(char buf[], void *ptr_opt);
    // 将opm 关联的光开关加入到告警测量队列
    int input_arlarm_gsm_test_queue(_tagDevComm alarmDev, \
                                    int alarm_lev, int alarm_type,int test_reason, char msg[]);
    //收到otdr曲线
    int dealRcvCardOtdrCrv(char buf[], void *ptr_option);
    //分析otdr曲线
    int analyze_otdr_curv(_tagOtdrCurv *pcurv, char buf[]);    
    //收到参考曲线
    int dealRcvNMReferCrv(char buf[], void *ptr_opt);
    //网管查询序列号
    int dealRcvNMAskSerilNum(char buf[], void *ptr_opt);
    //网管查询板卡信息组成
    int getCfgCardForm(void *ptr_opt);
    int getRealCardForm(void *ptr_opt); //查询实时板卡组成信息
    int rcvConfirmCardForm(char buf[], void *ptr_opt); //收到网管确认板卡组成信息
    //收到OSW模块关联信息
    //收到清除OSW模块关联信息
    //主界面上显示时间
    void showTime();
    //初始化周期性测量
    void initialCycleTestTime();
    //读取周期性测量时间
    int db_read_cyc_time();
    //取消周期性测量
    void  cancelCycleTest();
    //发送同步信号
    void sendRespondSignal(int cmd, int resCmd, int erroCode,int dlgType);
    int    process_rcv_otdr_curv( int cmd, char buf[]);
    void process_cmd_ack( char buf[],void *ptr_option);
    int    dispatch_cmd_ack(_tagResCode * pRes,void *ptr_option);
    void get_otdr_card_slot(int is_prompt = 1);
    void refresh_card_slot_config();
    //收到板卡来的短数据
    int send_short_data_2_dlg(unsigned int cmd, _tagBufList *pBufHead);
    int get_dev_type(QString strType);
    int update_dev_type(int frame_no = -1);
    //从数据库读取板卡组成信息
    int get_card_composition_fromdb();
    int get_dev_composition_fromdb();

    //第一次处理数据，不用同步，可立马返回
    int local_process_cmd(char buf[], int dataLen, int msec, void *ptr_opt);
    //保存opm/olp端口与osw端口的关联信息
    int save_port_refer_osw(char buf[], void *ptr_opt);
    //保存osw端口关联光缆信息
    int save_osw_port_cable_infor(char buf[], void *ptr_opt);
    //清除opm/olp端口与osw端口的关联信息
    int clear_port_refer_osw(char buf[], void * ptr_opt);
    //清除osw端口关联光缆信息
    int clear_osw_port_cable_infor(char buf[], void *ptr_opt);   
    //检查端口号是否合法
    int check_dev_port_no( int frame_no,int card_no,int port_no);
    //检查opm/olp --- osw对应关系
    int check_any_port_osw(char buf[]);
    //配置/清除短信权限
    int  save_sms_cfg(char buf[], void *ptr_opt);
    int  clear_sms_cfg(char buf[], void *ptr_opt);
    int  check_sms_cfg(char buf[]);
    //从板卡组成信息中得到机框使用状态
    int get_frame_state_from_card_comp();   
    //写入板卡组成信息
    int wrtie_db_card_comp();
    //检查参考曲线是否合法
    int check_otdr_ref_curv(char buf[]);
    //检查从数据库读取的参考曲线是否正确
    int db_check_otdr_ref(char buf[]);
    //保存周期性测量周期
    int db_save_cyc_cfg(char buf[], void *ptr_opt);
    int db_del_cyc_cfg(char buf[], void *ptr_opt);//删除周期性测量条目，全部删除或者部分删除
    int db_check_cyc_cfg(char buf[]);
    int db_del_otdr_refer_curv(char buf[], void *ptr_opt);//网管全部/逐条删除参考曲线
    //查找发生变化的板卡，槽位
    int find_changed_dev(_tagSubrackInfo *pdstSubrackInfor,_tagSubrackCard dstCardCommp[]);
    //按机框号，槽位号，删除数据库中的记录
    int db_delete_record(int frame, int card = -1);
    int db_delete_osw_map(int frame ,int card); //删除osw port 的所接光缆设备信息
    int db_delete_osw_connect(int frame, int card);//删除olp/opm---osw
    int db_delete_osw_ref_curv(int frame ,int card);//删除 osw参考曲线
    int db_delete_osw_cyc(int frame ,int card);//删除osw周期性测量
    int db_delete_cyc_time(int frame, int card);//删除周期性测量时间
    //数据库测试程序
    int db_test();
    //获取参考曲线
    int db_get_refr_otdr(_tagDevComm *posw, _tagReferCurv *pReferCurv);
    //获取对应端口的测量参数
    int db_get_osw_test_para(_tagDevComm *posw,_tagMeasurPara *pMeasur);
    //网管点名测量
    int dealNMAppointTest(char buf[], void *ptr_opt);
    //查询光开关关联的设备类型,返回关联设备类型地址
    int db_get_osw_connect_dev(_tagDevComm *posw, _tagDevComm *pconnect_dev);
    int input_gsm_queue(int alarm_lev, int alarm_type, _tagDevComm *pusr_dev, char option[] = NULL);
    int send_gsm_msg(wchar_t msg[], int len);
    //等待回应码
    int wait_res_response(unsigned int send_cmd, unsigned int res_cmd, int wait_time_ms,int sem_index = SEM_MCU);
    //根据告警类型，告警级别得到相应的中文信息
    int get_alarm_ch_text(int alarm_leve, int alarm_type, char leve_buf[], char type_buf[], int option = -1);
    int dealRcvGsmRes(char buf[]);
    //保存周期性测量曲线
    int db_save_osw_cyc_curv(char buf[]);
    //保存告警曲线
    int db_save_alarm_curv(char buf[], char alarmBuf[]);
    //发送周期性测量曲线到网管
    int send_cyc_curv();
    //发送告警曲线
    int deal_db_alarm_curv(int db_id , int type = SEND_TYPE_INITIATIVE, int pkid = -1);
    //获取光功率总的告警
    int get_optical_powr_alarm();
    //检查通信状态，判断是否产生通信故障
    int check_card_commu_state(int check_type = 0);   
    //创建otdr管理任务
    int creat_otdr_tsk();
    //将测试端口加入对应的otdr队列
    int dispatch_test_port(void *pTestModel, int mod);
    //产生流水号
    unsigned short creat_pkid();
    //收到otdr和光开关的曲线操作的响应码
    int rcv_odtr_curv_ack(_tagResCode * pRes, unsigned short pkid, int ack_type = 1);
    int rcv_odtr_curv_ack( _tagDevComm dev, unsigned short pkid, int res_code);   
    //获取测试光纤连接的otdr路由
    int db_get_test_port_rout(_tagDevComm testDev, _tagOswRout *ptrRou);
    int mcu_Ack(void * ptr_opt, unsigned int res_cmd, int res_code,int opt = 0);//封装tms_AckEx
    int db_save_A_trigger_B(char buf[], void * ptr_opt);//保存告警触发表
    int db_del_A_trigger_B(char buf[], void * ptr_opt);//删除告警触发表

    //模块路由表操作
    int db_save_module_rout(char buf[], void * ptr_opt); //保存模块路由表
    int db_del_module_rout(char buf[], void * ptr_opt);//删除模块路由表

    void * get_otdr_tsk(_tagDevComm otdrDev);
    //公共函数，每个talble插入item，方便设置格式
    void insert_tablewidget_item(QString str, QTableWidget *ptr_widget, int row, int column);
    int rcv_opm_power(char buf[],void *ptr_opt);//收到光功率,在对话框中显示
    int gsm_send_alarm_NM_state(int lev);//报告网管状态
    int olp_switch(char buf[],void *ptr_opt);//OLP切换
    int send_total_hw_alarm(int send_type = SEND_TYPE_INITIATIVE, int pkid = -1, void *ptr_opt = NULL); //发送总的告警数目

    int input_changed_hw_gsm_queue(char buf[]);//将变化的告警加入短信发送队列
    int  pc_adjust_mcu_time(char buf[], void *ptr_opt);
    int  get_otdr_modul_para(); //获取otdr模块参数
    int rcv_otdr_modul_para(char buf[], void * ptr_opt);//获取otdr模块参数
    int send_verson2PC(char buf[], void * ptr_opt);//发送硬件版本，软件版本到PC
    int refresh_total_opm_alarm(_tagOpticalAlarm *pRcvOpmAlarm, _tagFSOpmAlarm &LOpmAlarm, \
                                int port);//收到变化的光功率告警，更新总的告警
    //收到板卡上报的总的光功告警
    int dealRcvCardOpmAlarmTotal(char buf[], void *ptr_opt);
    //网管查询总的光功告警
    int send_totoal_opm_alarm(int send_type = SEND_TYPE_ASK, short pkid = -1, void *ptr_opt = NULL);
    int new_fs_opm_alarm( _tagFSOpmAlarm LOpmAlarm);//初始化一个新的板卡槽位的光功告警
    _tagFSOpmAlarm get_fs_opm_alarm(int frame, int card, int &index);//获取某机框，某槽位的总的光功告警
    int db_save_total_opm_alarm(); //往数据库中保存总的光功告警
    int  initial_total_opm_alarm();//从数据库中读取总的光功率告警
    int get_opm_alarm_from_buf(char *buf, int len, int opm_num); //从buf中初始化保存的当前告警   
    int input_cpy_sock_retry_queue(const char buf[], int pkid); //加入到发送缓冲区
    int find_alter_alarm(_tagFSOpmAlarm *pFsOpmAlarm, char buf[]); //从当前的槽位的总的告警中查找改变的告警
    int process_alter_alarm_from_total_opm_alarm(char buf[]);//处理总的告警中发现的变化的告警
    int db_save_olp_switch_record();//db保存OLP切换记录    
    int input_new_olp_switch_record(_tagOlpActionRecordCell record);//新收到OLP切换记录，将其保存到缓冲区中
    int initial_olp_switch_record_buf();//初始化olp切换记录的缓冲
    int db_read_comm_record(unsigned int ID); //读取通用的记录
    int initial_hw_alarm_buf();
    int send_olp_action_record_to_host(int send_type = SEND_TYPE_ASK, int pkid = -1, void *popt = NULL);//发送olp切换记录到主机
    int send_hw_alarm(void *popt, int pkid = -1, int alarm_type = 0, int is_retry = 0);//发送的总的硬件告警
    int db_save_total_hw_alarm();//保存全部的硬件告警
    int dealRcvTuReportInfor(char buf[], void *ptropt);//TU报告板卡插拔状态 
    int dealRcvCardHwAlarm(char buf[], void *ptr_opt);
    int gpio_init(); //初始化gpio
    int get_io_val(int index, int &val); //获取io val
    int set_io_val(int index, int val);//设置io的值
    int read_all_io_val(int val[]);//读取通用io状态值
    int refresh_mcu_card_state();//更新主控板卡状态
    int creat_other_tsk(); //主线程运行时创建其他任务
    int destroy_other_tsk();//住进成退出时候销毁其他任务
    void soft_reboot(int reboot_type = 0);//重启操作系统
    int ctrl_mcu_alarm_sound(char buf[], void * ptr_opt);//控制mcu告警声音
    int nm_get_alarm_sound_state(char *buf, void *ptr_opt);
    int mcu_get_fd_by_ip(int IP);//通过ip获取IP
    int mcu_send_otdr_curv_to_remote(int fd, char buf[], void * ptr_opt);
    int ctu_get_curv_dst(int pkid);
    int save_data_befor_reboot();//系统重启前调用该函数
    //保存配置文件
    int save_dev_cfg();
    //读取配置文件
    int read_dev_cfg();
    //创建sn
    int create_sn(char sn[]);
    //检测网卡
    int detect_net_card(unsigned int *wlan_IP = NULL, unsigned int *lan_IP = NULL);
    //收到上层短信请求
    int RcvNMShorMsg(char buf[], void * ptr_opt);
    int clear_usr_link(int frame, int card);
    //olp发生故障，请求进行测试
    int olp_ask_test(char buf[], void *ptr_opt);       

    /*
     *检查参数是否合法函数集
    */
    //j检查uuid
    int check_dev_uuid();
    int check_rcv_opm_alarm_change(const char buf[]);//收到变化的光功告警，首先进行检查参数是否合法
    int check_rcv_opm_alarm_total(const char buf[]);//收到总的光功告警，首先进行检查参数是否合法
    //检查机框，板卡范围
    int check_frame_card_range(int frame_no, int card_no);
    //检查设备类型，端口数目
    int check_dev_type_port_num(int dev_type, int port_num);
    //检查设备类型是否合法
    int check_dev_type( int frame_no,int card_no,int dev_type);
    //检查收到的硬件告警是否合法
    int check_rcv_hw_alarm(const char buf[]);

    int db_check_module_rout(char buf[]);//检查模块路由表
    int db_check_A_trigger_B(char buf[]);//检查告警触发表是否正确
    int db_check_cyc_curv(char buf[]);
    //检查板卡组成信息
    int check_card_comp(_tagSubrackCard *pSubRackCard);
    //初始化发送信息
    int get_dst_host_addr_info(char *dst_addr,int send_type, int pkid = -1, void *ptr_context = NULL);
    //检查硬件告警是否存在
    int check_is_hw_alarm(int alarm_reason);
    //更新总的光功告警池子
    int refresh_opm_alarm();
    //更新olp切换记录
     int refresh_olp_action_log();
     /*
      **********************调试输出函数************************************
     */
     void   show_db_saved_alarm();
     void   show_total_hw_alarm();
     void   show_total_opm_alarm();
     void   show_olp_swtich_log();
     /*
      **********************网管查询数据库内容，返回数据库内容*********************************
     */
     int ret_sms_authority(char buf[], void *ptr_opt); //返回告警短信权限
     int ret_port_infor(char buf[],void *ptr_opt);//返回关联信息
     int ret_refer_curv(char buf[],void *ptr_opt);//返回参考曲线
     int ret_module_cascade_table(void *ptr_opt);//返回模块级联表
     int ret_port_trigger_table(char buf[], void *ptr_opt);//返回端口触发表
     int ret_cyc_test_table(char buf[], void *ptr_opt);//返回周期性测量表

     /*
      *************************************************************************
     */






public slots :
    void countTimeout() ;
    int inspectFiber();
    int insepectCycleTime();
    int s_update_dev_type();

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
signals:
    //cmd, dataLen,存放数据段链表
    void dataToDlg(int, int, _tagBufList *);
    void sendMsgToDlg(int ,int, int);
    int findAlarm();
    int checkCycleTime();
    int updateDevType();
    
private slots:

    void on_pushBtnCard_1_clicked();    

    void on_pushBtnCard_2_clicked();

    void on_pushBtnCard_3_clicked();

    void on_pushBtnCard_4_clicked();

    void on_pushBtnCard_5_clicked();

    void on_pushBtnCard_6_clicked();

    void on_pushBtnCard_7_clicked();

    void on_pushBtnCard_8_clicked();

    void on_pushBtnCard_9_clicked();

    void on_pushBtnCard_10_clicked();

    void on_pushBtnCard_13_clicked();

    void on_pushBtnCard_14_clicked();

    void on_pushBtnCard_15_clicked();

    void on_pushBtnCard_16_clicked();

    void on_pushBtnCard_11_clicked();

    void on_pushBtnCard_12_clicked();

private:
    Ui::MainWindow *ui;
public:


public:
    //    void loadBtnIco();
};

#endif // MAINWINDOW_H




