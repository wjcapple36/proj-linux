#ifndef TSK_SMS_SEND_H
#define TSK_SMS_SEND_H

#include <QThread>
#include "struct.h"
#include "cserialport.h"
#include "sms_protocol.h"
#include "mainwindow.h"

//信号量，同步时使用，cmd表明随哪条命令在等待信号量
#define RES_STR_LEN     36 //返回字符串的长度
#define EQ_OK               0
#define EQ_ERROR        1
#define SMS_REBOOT_TIME_MS         15000   //系统重启时间，测算10s左右
#define SMS_RETRY_NUM           3  //如果发送不成功，发送三次

#define SMS_STAT_INITIAL     0        //短信猫正在初始化
#define SMS_STAT_OK      1       //短信猫正常
#define SMS_STAT_ERROR   2       //短信猫故障
struct _tagSmsSynvSem
{
    int commu_stat; //通信状态
    int retval; //返回值
    int rcv_res_data; //状态，1表示正在等待数据
    char hope_str[RES_STR_LEN];

    QSemaphore objSem;//同步信号量
    QMutex objMutex; //锁，防止重入
};
//短信发送控制变量
struct _tagSendSmsCtrl
{
    int error_num; //失败次数
    int sucess_num; //成功次数
    int reset_eq; //重启设备
};

class tsk_SMS_Send : public QThread
{
    Q_OBJECT
public:
    explicit tsk_SMS_Send(QObject *parent = 0);
    ~tsk_SMS_Send();
public:
    _tagObjSynvSem  semSynch;
    _tagGsmQue GSMQueue;    //短消息队列
    struct CSerialPort *pSerialPort;
    _tagSmsSynvSem objSynSem;
    _SMParam smsPara; //参数设置
    _tagSendSmsCtrl   SmsCtrl; //短信发送控制参数
    class MainWindow *pMainWindow;
     int device_state; //设备状态
    unsigned char  long_msg_serri;// 长短信的序号0~255
private:
    volatile bool stopped;
protected:
    void run();
public:
    void stop();
    int send_gsm_msg(_tagGsmContext *pmsg);
    int get_signal(int wait_time_ms);
    int proc_com_data(char RcvBuf[], int rcv_bytes);
    int send_sms_context(char context[], char res_context[], int bytes, int wait_time = 10000); //发送消息
    int initial_sync_sem(char context[]);    
    int send_short_sms(_tagGsmContext *pmsg);//发送短消息
    int send_long_sms(_tagGsmContext *pmsg);//发送常消息
    int set_sms_mod_echo();//设置短信猫回显
    int set_sms_mod_echo_close();//关闭短信猫回显
    int sms_eq_self_test();//设备自检
    int set_sms_mod(int mod = 0);//设置工作模式,默认是pdu模式
    int  sms_eq_soft_restart();//重启sms设备
    int sms_eq_power_reset();//断电重启
    int sms_eq_initial();
    int sms_retry_text(_tagGsmContext text);
    int usr_delay(int delay_time_ms);//线程内延时    
    int res_nm_send_result(_tagGsmContext *pText, int retv);


    
signals:
    
public slots:
    
};

#endif // TSK_SMS_SEND_H
