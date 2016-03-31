#ifndef TSK_OTDRMANAGE_H
#define TSK_OTDRMANAGE_H
#include <QThread>

#include "mainwindow.h"
#include "struct.h"
#include "constant.h"
#include "tsk_waitotdrcurv.h"
#define OTDR_MODE_SERRI         2   //otdr串行测量
#define OTDR_MODE_PARAL         1   //otdr并行测量
#define  PORT_IDLE                        0 //端口空闲
#define  PORT_BUSY                      1 //端口忙
#define  TSK_ATTRIBUTE_CTU      36 //该任务属于CTU
#define  TSK_ATTRIBUTE_RTU       0  //该人数属于RTU


class tsk_OtdrManage  : public QThread
{
    Q_OBJECT
public:
    explicit tsk_OtdrManage(QObject *parent = 0);
    void stop();
    ~ tsk_OtdrManage();
private:
    //获取任务
    int getTask(_tagCtrlAppointTest *pAppointTest, int *pCmd, int port);

    int switch_osw(_tagPortOSW *posw_port,unsigned int  cmd, int ack_to = ACK_TO_NONE);
    //获取曲线
    int get_otdr_curv(_tagCtrlAppointTest *pAppointTest, _tagDevComm *pEndDev, \
                      unsigned int cmd , unsigned int res_cmd, int ack_to);
    int  tsk_send(char devbuf[] , char buf[], unsigned int  cmd, int res_cmd, int otdr_port,int ack_to);
    int send_cmd(int fd, char buf[],int cmd);


public:
    void set_all_port_busy(); //设在所有端口忙
    void set_all_port_idle(); //设置所有端口空闲
    void notify_olp_end_test(_tagDevComm *polp, unsigned int cmd, int test_port, int ack_to);
    void reset_port_state(int index);
    void notify_otdr_dlg(unsigned int res_cmd, int rescode);
    //切换光开关
    int  alloca_resource(int port_num); //为每个端口分配资源
    void release_resource();//释放资源
    int input_test_queue(void *pelement, int testType, int port);//将测试加入对应的队列
    void sendSignal(int usr_event, unsigned option);
    void setModulPara(QStringList lamdaList);
    int ctuGetRtuFd(int IP); //CTU获取RTU模式
    //获取路由中最后一个模块的fd,RTU是otdr模块的fd，CTU是RTU的ip
    int get_end_dev_fd(_tagCtrlAppointTest *pAppointTest, _tagDevComm *pendDev);
    //发送测试参数
    int send_test_para(_tagCtrlAppointTest *pAppointTest, _tagDevComm *pEndDev, unsigned int cmd, int &waitTime_ms);
    int set_tsk_attribute(int attribute = TSK_ATTRIBUTE_RTU);
public:
    //定义指向父窗口段指针
    class MainWindow  *m_pParent;
    class tsk_waitOtdrCurv *pTskWaitCurv;
    _tagDevComm otdrAddr; //板卡地址
    _tagObjSynvSem *pObjSem; //同步信号量
    _tagObjSynvSem *pobjComm; //光开关，光保护同步变量
    _tagAppointTestQueue *pAppointTstQue; //点名测量队列
    _tagAlarmTestQueue *pCycTstQue; //周期性测量队列
    _tagAlarmTestQueue *pAlarmTstQue; //告警测试队列
    _tagOswRout *osw_rout_buf;
    int isAllocaResource; //是否分配资源的标志
    _tagPortState *port_state_buf;
    _tagWaitCurvPort waitCurvPortQue;//等待回应的队列
    int otdr_mode;//otdr模式，串行，并行
    _tagAppointCtrOpt res_addr; //回应地址
    _tagAppointCtrOpt to_addr; //下行地址
    _tagRcvOtdrCurv AlarmCurv; //存放告警曲线
    _tagReferCurv ReferCrv;//存放参考曲线
    int run_stat;//不允许再次操作了
    _tagOtdrModulPara modulPara;


protected:
    void run();

private:
    volatile bool stopped;
    int tsk_attribute;

signals:
    int findAlarm(unsigned int);
public slots:
    int inspectFiber(unsigned int);//查找告警
};


#endif // TSK_OTDRMANAGE_H
