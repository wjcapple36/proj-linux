#ifndef TSK_SOCKRETRYSEND_H
#define TSK_SOCKRETRYSEND_H

#include <QThread>

#include "struct.h"
#include "mainwindow.h"

class tsk_SockRetrySend : public QThread
{
    Q_OBJECT
public:
    explicit tsk_SockRetrySend(QObject *parent = 0);
    ~tsk_SockRetrySend();
public:
    _tagSockRetryList RetryList;
    _tagUsrTimer usr_timer;
    int timer_space;
    int count;
    class MainWindow *pMainWindow;
    _tagObjSynvSem  objSynSem; //同步信号
protected:
    void run();
public:
    void stop();
private:
    volatile bool stopped;
public:
    void usr_timer_initial();
    void check_is_retry();
    int usr_delay(int delay_time_ms);
    int inform_usr_by_sms(_tagSockDataCpy *pDataCpy);
    int input_sms_queue_olp_switch(char buf[]);
    int input_sms_queue_opmAlarm_chang(char buf[]);
    int clear_retry_list(int index = -1); //清空重发队列中的某一个或者全部
    int wait_response(int wait_time_ms = -1);
    int rcv_response();
public slots:
    void usr_time_out() ;

    
signals:
    
public slots:
    
};

#endif // TSK_SOCKRETRYSEND_H
