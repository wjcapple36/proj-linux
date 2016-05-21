#ifndef TSK_HOST_COMMU_H
#define TSK_HOST_COMMU_H

#include <QThread>
#include "mainwindow.h"
#include "struct.h"
#include "constant.h"
struct timer_counter
{
    int commCounter;
//    int nmStateCounter;
//    int IOStateCounter;
};
//LED灯状态
struct GPIO_CTRL
{
    int run_state;
    int alarm_state;
    int alarm_output;//告警声音输出
    int alarm_led_val;//告警led
};
#define     LED_RUN_GREEN    0
#define     LED_RUN_RED         1
#define     LED_RUN_FLICKER  2
#define     ALARM_OUTPUT_ALARM      1
#define     ALARM_OUTPUT_NORMAL     0
class tsk_host_commu : public QThread
{
    Q_OBJECT
public:
    explicit tsk_host_commu(QObject *parent = 0);
public:
    //定义指向父窗口段指针
    class MainWindow  *pMainWidow;
    volatile bool get_op_alarm;   //读取光功率
    volatile bool up_cyc_curv;   //上传周期性测量曲线
    _tagObjSynvSem  semSynch;
   // _tagGsmQue GSMQueue;    //短消息队列
    _tagUsrTimer usr_timer;
    int mcu_stat[GPIO_PIN_NUM];
    int mcu_stat_last[GPIO_PIN_NUM];
    int timer_space;
    int count;
    int led_run_state;
    int fdWatchdog;
    GPIO_CTRL gpioCtrl;
    timer_counter  usr_counter;
public:
    void usr_timer_initial();
    void read_all_io();
    void refresh_nm_state();
    void check_commu_state();
    void check_alarm_stat();
    void usr_ctrl_io();
    //喂看门狗
    void feed_watch_dog();
    //停止喂狗，准备重启
    void stop_feed_dog();
public slots:
    void usr_time_out() ;
protected:
    void run();
public:
    void stop();
private:
    volatile bool stopped;
    volatile bool stop_dog;
signals:
    
public slots:
    
};

#endif // TSK_HOST_COMMU_H
