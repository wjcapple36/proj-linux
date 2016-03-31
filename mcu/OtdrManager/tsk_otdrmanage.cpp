#include <QDebug>
#include "tsk_otdrmanage.h"
#include "qcurv.h"
#include "math.h"
#include "protocol/tmsxx.h"


tsk_OtdrManage::tsk_OtdrManage(QObject *parent) :
    QThread(parent)
{
    m_pParent = (MainWindow *)parent;
    stopped = false;
    //初始化otdr控制变量
    memset(&m_pParent->m_otdrCtrl, 0, sizeof(m_pParent->m_otdrCtrl));
    memset(&otdrAddr, 0, sizeof(_tagCardAddr));
    //初始化系列资源
    pObjSem = NULL;
    pAppointTstQue = NULL;
    pCycTstQue = NULL;
    pAlarmTstQue = NULL;
    port_state_buf  = NULL;
    osw_rout_buf = NULL;
    pobjComm = NULL;
    pTskWaitCurv = NULL;
    isAllocaResource = TSK_INITIAL_NO;
    run_stat = TSK_NORMAL;
    modulPara.initial = 0;
    otdr_mode = OTDR_MODE_SERRI;
    set_tsk_attribute(TSK_ATTRIBUTE_RTU);
    modulPara.lamdaList.clear();
    //查找告警信号
    connect(this, SIGNAL(findAlarm(unsigned int )), this, \
            SLOT(inspectFiber(unsigned int)),Qt::QueuedConnection);

}
tsk_OtdrManage:: ~ tsk_OtdrManage()
{
    release_resource();
}
/*
   **************************************************************************************
 *  函数名称：alloca_resource
 *  函数描述：每个端口分配一组资源包括：同步信号，告警/点名/周期测量队列
 *  入口参数：端口书目
 *  返回参数：
 *  作者       ：
 *  日期       ：
 **************************************************************************************
*/
int tsk_OtdrManage::alloca_resource(int port_num)
{
    int res_code;
    res_code = RET_SUCCESS;
    //初始化信号，队列资源
    pObjSem = NULL;
    pAppointTstQue = NULL;
    pCycTstQue = NULL;
    pAlarmTstQue = NULL;
    port_state_buf  = NULL;
    osw_rout_buf = NULL;
    pobjComm = NULL;
    pTskWaitCurv = NULL;
    if(port_num > 0 && port_num < 32)
    {
        pObjSem = new _tagObjSynvSem[port_num];
        if(pObjSem == NULL)//为每个通道分配信号量
        {
            res_code = -1;
            goto usr_exit;
        }
        pAppointTstQue = new _tagAppointTestQueue[port_num];
        if(pAppointTstQue == NULL)//为每个通道分配点名测量队列
        {
            res_code = -1;
            goto usr_exit;
        }
        pCycTstQue = new _tagAlarmTestQueue[port_num];
        if(pCycTstQue == NULL)//为每个通道分配周期性测量队列
        {
            res_code = -1;
            goto usr_exit;
        }
        pAlarmTstQue = new _tagAlarmTestQueue[port_num];
        if(pAlarmTstQue == NULL)//为每个通道分配点名测量队列
        {
            res_code = -1;
            goto usr_exit;
        }
        port_state_buf = new _tagPortState[port_num];
        if(port_state_buf == NULL)//为每个通道分配状态zhishi
        {
            res_code = -1;
            goto usr_exit;
        }
        memset(port_state_buf, 0, sizeof(_tagPortState)*port_num);
        osw_rout_buf = new _tagOswRout[port_num];
        if(osw_rout_buf == NULL)//为每个通道分配光开关级联资源
        {
            res_code = -1;
            goto usr_exit;
        }
        pobjComm = new _tagObjSynvSem[port_num];
        if(pobjComm == NULL)//为每个通道分配信号量
        {
            res_code = -1;
            goto usr_exit;
        }
        pTskWaitCurv = new tsk_waitOtdrCurv ::tsk_waitOtdrCurv(this);
        if(pTskWaitCurv == NULL)//为每个通道分配信号量
        {
            res_code = -1;
            goto usr_exit;
        }
        pTskWaitCurv->start();
    }
usr_exit:
    if(res_code == RET_SUCCESS)
    {
        isAllocaResource = TSK_INITIAL_YES;
    }
    else
        isAllocaResource = TSK_INITIAL_NO;
    qDebug("resource allca portnum %d %d", port_num,isAllocaResource);
    qDebug("add otdr frame %d card %d port %d", otdrAddr.frame_no, otdrAddr.card_no, otdrAddr.port);
    return res_code;
}
/*
   **************************************************************************************
 *  函数名称：release_resource
 *  函数描述：释放掉分配的资源
 *  入口参数：
 *  返回参数：
 *  作者       ：
 *  日期       ：
 **************************************************************************************
*/
void tsk_OtdrManage::release_resource()
{
    //首先stop
    if(!stopped)
        stop();
    if(TSK_INITIAL_NO == isAllocaResource)
        return;

    if(pAppointTstQue != NULL)
    {
        pAppointTstQue->objMutex.lock();
        pAppointTstQue->xqueue.clear();
        pAppointTstQue->objMutex.unlock();
        delete []pAppointTstQue;
        pAppointTstQue = NULL;
    }

    if(pCycTstQue != NULL)
    {
        pCycTstQue->objMutex.lock();
        pCycTstQue->xqueue.clear();
        pCycTstQue->objMutex.unlock();
        delete []pCycTstQue;
        pCycTstQue = NULL;
    }

    if(pAlarmTstQue != NULL)
    {
        pAlarmTstQue->objMutex.lock();
        pAlarmTstQue->xqueue.clear();
        pAlarmTstQue->objMutex.unlock();
        delete []pAlarmTstQue;
        pAlarmTstQue = NULL;
    }

    if(port_state_buf != NULL)
    {
        delete []port_state_buf;
        port_state_buf = NULL;
    }

    if(osw_rout_buf != NULL)
    {
        delete []osw_rout_buf;
        osw_rout_buf = NULL;
    }
    if(pObjSem != NULL)
    {
        delete []pObjSem;
        pObjSem = NULL;
    }
    if(pobjComm != NULL)
    {
        delete []pobjComm;
        pobjComm = NULL;
    }
    if(pTskWaitCurv != NULL)
    {
        pTskWaitCurv->stop();
        pTskWaitCurv->wait(1000);
        pTskWaitCurv->terminate();
        delete pTskWaitCurv;
        pTskWaitCurv = NULL;
    }
    isAllocaResource = TSK_INITIAL_NO;
    qDebug("del otdr frame %d card %d port %d", otdrAddr.frame_no, otdrAddr.card_no, otdrAddr.port);
}
/*
   **************************************************************************************
 *  函数名称：input_test_queue
 *  函数描述：将测试端口加入测试队列，成功返回0，否则返回-1
 *  入口参数：
 *  返回参数：返回当前正在测试的时间,不满足条件则返回-1
 *  作者       ：
 *  日期       ：
 **************************************************************************************
*/
int tsk_OtdrManage::input_test_queue(void *pelement, int testType, int port)
{
    int cur_test_time;
    cur_test_time = -1;
    if(pelement == NULL || stopped)
        goto usr_exit;
    if(isAllocaResource == TSK_INITIAL_YES&& port >= 0 && port < otdrAddr.port)
    {
        cur_test_time = port_state_buf[port].test_time;
        switch(testType)
        {
        case OTDR_MOD_ALARM://主控发起的告警测试
        {
            _tagCtrlPortTest portTest;
            memcpy(&portTest, pelement, sizeof(_tagCtrlPortTest));
            pAlarmTstQue[port].objMutex.lock();
            pAlarmTstQue[port].xqueue.enqueue(portTest);
            pAlarmTstQue[port].objMutex.unlock();
            break;
        }
        case OTDR_MOD_APPOINT: //点名测量，网管发出的周期性测量
        {
            _tagCtrlAppointTest appoint_test;
            memcpy(&appoint_test, pelement, sizeof(_tagCtrlAppointTest));
            pAppointTstQue[port].objMutex.lock();
            pAppointTstQue[port].xqueue.enqueue(appoint_test);
            pAppointTstQue[port].objMutex.unlock();
            break;
        }
        case OTDR_MOD_CYC: //断网情况下主控发出的周期性测量
        {
            _tagCtrlPortTest portTest;
            memcpy(&portTest, pelement, sizeof(_tagCtrlPortTest));
            pCycTstQue[port].objMutex.lock();
            pCycTstQue[port].xqueue.enqueue(portTest);
            pCycTstQue[port].objMutex.unlock();
            break;
        }
        default:
        {
            cur_test_time = -1;
            break;
        }
        }
    }
    else
    {
        cur_test_time = -1;
    }
usr_exit:
    qDebug("cur_test_time %d testType %d port %d", cur_test_time, testType,port);
    return cur_test_time;
}

/*
   **************************************************************************************
 *  函数名称：getTask
 *  函数描述：从队列中获取测试任务。
 *                ：
 *  入口参数：
 *  返回参数：
 *  作者       ：
 *  日期       ：
 *  修改日期：2015-11-05
 *  修改内容：设置测量参数src_ip: CTU/RTU ip/0;检查级联路由最后一个模块
 *                ：是否合法;设置测量参数test_reason
 **************************************************************************************
*/
int tsk_OtdrManage::getTask(_tagCtrlAppointTest *pAppointTest, int *pCmd, int port)

{
    //保存出队的元素，从机号，槽位，端口
    _tagCtrlPortTest curTestPort;
    //点名测量成员
    _tagCtrlAppointTest curAppointTest;
    _tagDevComm testPort;
    int res_db;
    int res_code;
    res_code = -1;
    memset(&curAppointTest, 0, sizeof(curAppointTest));
    memset(&curTestPort, 0, sizeof(curTestPort));
    //资源未分配，或者端口号搞错，直接返回
    if(TSK_INITIAL_NO == isAllocaResource)
        goto usr_exit;
    if(port >= otdrAddr.port) // 0~port-1
        goto usr_exit;
    res_db = 0;
    //点名测量带了测量参数，其他与告警测量结构体相同：ack_to, port
    if(!pAppointTstQue[port].xqueue.isEmpty()) //点名测量队列不空
    {
        pAppointTstQue[port].objMutex.lock();
        curAppointTest = pAppointTstQue[port].xqueue.dequeue();
        pAppointTstQue[port].objMutex.unlock();
        //点名测量，是dlg还是网管的由osw_port中的ack_to决定
        memcpy(pAppointTest, &curAppointTest, sizeof(curAppointTest));
        *pCmd = pAppointTest->cmd;
        res_code = RET_SUCCESS;
    }
    //告警测试
    else if(!pAlarmTstQue[port].xqueue.isEmpty())
    {
        pAlarmTstQue[port].objMutex.lock();
        curTestPort = pAlarmTstQue[port].xqueue.dequeue();
        pAlarmTstQue[port].objMutex.unlock();
        //根据光开关获取底参考曲线的参数
        memcpy(&testPort, &curTestPort.test_port, sizeof(_tagDevComm));
        res_db = m_pParent->db_get_osw_test_para(&testPort, &pAppointTest->para);
        qDebug("alarm test frame %d card %d port %d res_db %d",testPort.frame_no,testPort.card_no,testPort.port,res_db);
        qDebug("alarm para range %d lamda %d pl %d time %d n %f  end th %f non th %f",\
               pAppointTest->para.range_m,pAppointTest->para.lamda_ns,\
               pAppointTest->para.pulseWidth_ns,pAppointTest->para.measureTime_s,\
               pAppointTest->para.n,pAppointTest->para.endThreshold, pAppointTest->para.NonRelectThreshold);
        if(res_db != RET_SUCCESS)
            res_code = -1;
        else
            res_code = RET_SUCCESS;
        memcpy(&pAppointTest->test_port, &curTestPort.test_port, sizeof(_tagDevComm)); //拷贝了测试端口，端口信息
        pAppointTest->opt.src = ADDR_MCU;
        pAppointTest->opt.dst = ADDR_CARD;
        pAppointTest->ack_to = curTestPort.ack_to;
        pAppointTest->cmd =  curTestPort.cmd;
        pAppointTest->opt.pkid = m_pParent->creat_pkid();
        *pCmd = pAppointTest->cmd;
        //增加告警测试原因
        pAppointTest->para.test_reason = curTestPort.test_reason;

    }
    //周期性测试
    else if(!pCycTstQue[port].xqueue.isEmpty()) //周期测量队列不空
    {
        pCycTstQue[port].objMutex.lock();
        curTestPort = pCycTstQue[port].xqueue.dequeue();
        pCycTstQue[port].objMutex.unlock();
        //根据光开关获取底参考曲线的参数
        res_db = m_pParent->db_get_osw_test_para(&curTestPort.test_port, &pAppointTest->para);
        /*qDebug("cyc test frame %d card %d port %d res_db %d",oswAddr.frame_no,oswAddr.card_no,oswAddr.port,res_db);
            qDebug("cyc para range %d lamda %d pl %d time %d n %f  end th %f non th %f",\
                   pAppointTest->para.range_m,pAppointTest->para.lamda_ns,\
                   pAppointTest->para.pulseWidth_ns,pAppointTest->para.measureTime_s,\
                   pAppointTest->para.n,pAppointTest->para.endThreshold, pAppointTest->para.NonRelectThreshold);*/

        memcpy(&pAppointTest->test_port, &curTestPort.test_port, sizeof(_tagDevComm)); //拷贝链ack_to, port
        pAppointTest->opt.src = ADDR_MCU;
        pAppointTest->opt.dst = ADDR_CARD;
        pAppointTest->ack_to = curTestPort.ack_to;
        pAppointTest->cmd =  curTestPort.cmd;
        *pCmd = pAppointTest->cmd;
        //增加告警测试原因
        pAppointTest->para.test_reason = curTestPort.test_reason;
        if(res_db != RET_SUCCESS)
            res_code = -1;
        else
            res_code = RET_SUCCESS;

    }
    //如果本任务属性为CTU,那么设置本机外部IP
    /*
    *2016-03-10 send_test_para 处理
    */
    /*
    if(tsk_attribute == TSK_ATTRIBUTE_CTU)
        pAppointTest->para.src_ip = 1;
    else
        pAppointTest->para.src_ip = 0;
    */
    if(res_code == RET_SUCCESS)
    {
        port_state_buf[port].ack_to = pAppointTest->ack_to;
        //读取数据库,得到测试端口的路由信息
        res_code = m_pParent->db_get_test_port_rout(pAppointTest->test_port,&osw_rout_buf[port]);
        if(res_code != RET_SUCCESS)
        {
            printf("tsk otdr manage get rout error! \n");
            goto usr_exit;
        }
        /*
         *CTU模式，路由中的最后一个模块的ip必须是外部ip
         *RTU模式，路由模块中最后一个必须是内部ip,且设备类型必须是OTDR
         *
        */
        if(tsk_attribute == TSK_ATTRIBUTE_CTU)
        {
            if(osw_rout_buf[port].table_buf[osw_rout_buf[port].depth - 1].dstIP == 0)
            {
                res_code = RET_MODULE_ROUTE_ILLEGALT;
                goto usr_exit;
            }
        }
        else
        {
            if(osw_rout_buf[port].table_buf[osw_rout_buf[port].depth - 1].dstIP != 0 || \
                    osw_rout_buf[port].table_buf[osw_rout_buf[port].depth - 1].Dev.type != OTDR)
            {
                res_code = RET_MODULE_ROUTE_ILLEGALT;
                goto usr_exit;
            }
        }
        qDebug(" test port  frame %d card %d port %d",pAppointTest->test_port.frame_no,pAppointTest->test_port.card_no,\
               pAppointTest->test_port.port);
        qDebug("test para range %d lamda %d pl %d time %d n %f  end th %f non th %f",\
               pAppointTest->para.range_m,pAppointTest->para.lamda_ns,\
               pAppointTest->para.pulseWidth_ns,pAppointTest->para.measureTime_s,\
               pAppointTest->para.n,pAppointTest->para.endThreshold, pAppointTest->para.NonRelectThreshold);
    }

usr_exit:
    return res_code;
}
//设置所有端口忙
void tsk_OtdrManage::set_all_port_busy()
{
    int i;
    if(isAllocaResource == TSK_INITIAL_YES)
    {
        for(i = 0; i < otdrAddr.port;i++)
            port_state_buf[i].state = PORT_BUSY;
    }
}
//设置所有端口空闲
void tsk_OtdrManage::set_all_port_idle()
{
    if(isAllocaResource == TSK_INITIAL_YES)
    {
        memset(port_state_buf, 0, sizeof(_tagPortState)*otdrAddr.port);
    }
}
/*
   **************************************************************************************
 *  函数名称：run
 *  函数描述：处理流程：1、获取空闲通道；2、读取参数；3、通知olp，切换
 *  函数描述：osw；4、发送测量参数；5、等待测量结果；6、善后操作
 *  入口参数：
 *  返回参数：
 *  作者       ：
 *  日期       ：
 **************************************************************************************
*/
void tsk_OtdrManage::run()
{   

    int cmd, otdr_port; //命令码，端口编号
    int rout_depth; //otdr在路由表中的位置
    int ack_to;
    //返回的字节数,发送的命令码
    _tagResCode res_code;
    //发送的源地址和目的地址
    glink_addr send_addr;
    tms_devbase devbase;
    //处理结果
    int result, i;
    //点名测量
    _tagCtrlAppointTest AppointTest;
    _tagDevComm *ptest_port;
    _tagDevComm *pEndDev;
    _tagDevComm *polp;
    _tagDevComm *posw;
    //ptest_port = (_tagDevComm *)&AppointTest.test_port;


    memset(&res_code, 0, sizeof(res_code));
    res_code.cmd = ID_CMD_ACK;

    send_addr.src = ADDR_MCU;
    send_addr.dst = ADDR_NET_MANAGER;
    otdr_port = 0;
    while (!stopped)
    {
        //如果资源还未分配，那就重复等待
        if(isAllocaResource == TSK_INITIAL_NO)
        {
            usleep(10000);
            continue;
        }
        //首先等待，直到队列不为空
        result = -1;
        //获取测量任务,该任务处于阻塞中
        memset(&AppointTest, 0, sizeof(AppointTest));
        //求余，保证i不会超出端口数目，轮流检查每个端口的队列
        otdr_port = otdr_port % otdrAddr.port;
        for(; otdr_port < otdrAddr.port;otdr_port++)
        {
            result = -1;
            if(port_state_buf[otdr_port].state== PORT_IDLE)
            {
                result =  getTask(&AppointTest, &cmd,otdr_port);
                if(result == RET_SUCCESS)
                {
                    if(otdr_mode == OTDR_MODE_PARAL)
                        port_state_buf[otdr_port].state = PORT_BUSY;
                    else
                    {
                        set_all_port_busy();
                    }
                    break;
                }
            }
        }

        //如果是被外部stop，那么要要返回
        if(stopped)
            break;
        //轮询一遍，如果没有找到空闲端口或者找不到测量参数
        if(result != RET_SUCCESS)
        {
            usleep(10000);
            continue;
        }
        //下面是具体的操作，通知olp，切换光开关，发送测量参数
        res_code.res_cmd = cmd;
        res_code.res_code = -1;
        //类成员变量，发送引用
        //回应地址
        res_addr.dst = AppointTest.opt.src;
        res_addr.src = ADDR_MCU;
        res_addr.pkid = AppointTest.opt.pkid;
        //下行地址
        to_addr.dst = AppointTest.opt.dst;
        to_addr.src = AppointTest.opt.src;
        to_addr.pkid = AppointTest.opt.pkid;
        result = RET_SUCCESS;
        //输出调试信息
        qDebug("otdr tsk  frame %d card %d  port_num %d cur_port %d otdr_mode %d, attribute %d", \
               otdrAddr.frame_no, otdrAddr.card_no,\
               otdrAddr.port, otdr_port, otdr_mode, tsk_attribute);
        qDebug("otdr tsk  frame %d card %d  cmd %x ,src %x dst %x  pkid %d", otdrAddr.frame_no, otdrAddr.card_no,cmd,\
               res_addr.src, res_addr.dst, res_addr.pkid);

        rout_depth = osw_rout_buf[otdr_port].depth;
        pEndDev = &osw_rout_buf[otdr_port].table_buf[rout_depth - 1].Dev;
        ptest_port = &AppointTest.test_port;
        /*
         *第一个要切换端口根据测试端口来确定，osw_rout_buf里面存放第二级开始
         *rout_depth，存放的是osw_rout_buf有级联模块的数目
         *如果osw关联的是olp,需要通知olp
        */
        ack_to = port_state_buf[otdr_port].ack_to;
        if(ptest_port->type == OLP)
        {
            polp = ptest_port;
            tms_GetDevBaseByLocation(polp->frame_no, polp->card_no,&devbase);
            result = tsk_send((char *)&devbase, (char *)polp,ID_CMD_OLP_START_OTDR,cmd, otdr_port, ack_to);
            qDebug("notify olp start %d", result);
        }
        else if(ptest_port->type == OSW)
        {
            if(result == RET_SUCCESS)
            {
                tms_GetDevBaseByLocation(ptest_port->frame_no, ptest_port->card_no,&devbase);
                result = tsk_send((char *)&devbase, (char *)ptest_port,ID_CMD_OSW_SWITCH, cmd, otdr_port, ack_to);

            }
            else
            {
                goto usr_exit;
            }
        }
        //rout_depth - 1关联的是otdr
        for(i = 0; i < (rout_depth - 1); i++)
        {
            posw = &osw_rout_buf[otdr_port].table_buf[i].Dev;
            //切换光保护成功，切换光开关
            if(result == RET_SUCCESS)
            {
                tms_GetDevBaseByLocation(posw->frame_no, posw->card_no,&devbase);
                result = tsk_send((char *)&devbase, (char *)posw,ID_CMD_OSW_SWITCH, cmd, otdr_port, ack_to);
            }
            else
            {
                goto usr_exit;
            }
        }
        //切换光开关成功，发送测量参数
        if(result == RET_SUCCESS)
        {
            port_state_buf[otdr_port].test_time = AppointTest.para.measureTime_s *2;
            result = get_otdr_curv(&AppointTest,pEndDev,cmd,cmd + 1, ack_to);
        }
usr_exit:
        //如果发送测量参数不成功置位通道状态，
        if(result != RET_SUCCESS)
        {
            if(ptest_port->type == OLP) //如果是对olp进行测试，那么就要通知olp
                notify_olp_end_test(ptest_port, cmd, otdr_port,ack_to);
            //并行otdr设备本通道闲，串行otdr全部闲
            if(otdr_mode == OTDR_MODE_PARAL)
            {
                //port_state_buf[otdr_port].state = PORT_IDLE;
                bzero(&port_state_buf[otdr_port], sizeof(_tagPortState));
            }
            else
            {
                set_all_port_idle();
            }
        }
        continue;
    }
    //    stopped = false; //stop即终止，不能重新拿start
}
/*
   **************************************************************************************
 *  函数名称：notify_olp_end_test
 *  函数描述：提供给其他函数调用
 *  入口参数：
 *  返回参数：
 *  作者       ：
 *  日期       ：
 **************************************************************************************
*/
void tsk_OtdrManage::notify_olp_end_test(_tagDevComm *polp, unsigned int cmd, int test_port,int ack_to)
{
    //如果是olp，那么还要通其知测量结束
    int result;
    tms_devbase devbase;
    tms_GetDevBaseByLocation(polp->frame_no, polp->card_no,&devbase);
    result = tsk_send((char *)&devbase, (char *)polp,ID_CMD_OLP_FINISH_OTDR,cmd,test_port,ack_to);
    qDebug("notify olp end %d", result);

}
/*
   **************************************************************************************
 *  函数名称：reset_port_state
 *  函数描述：端口状态复位
 *  入口参数：
 *  返回参数：
 *  作者       ：
 *  日期       ：
 **************************************************************************************
*/
void tsk_OtdrManage::reset_port_state(int index)
{
    if(index == -1)
    {
        memset(port_state_buf, 0, sizeof(_tagPortState)*otdrAddr.port);
    }
    else if(index > -1 && index < otdrAddr.port)
    {
        memset(&port_state_buf[index], 0, sizeof(_tagPortState));
    }
}

void tsk_OtdrManage::stop()
{
    stopped = true;
    pTskWaitCurv->stop();
    run_stat = TSK_WILL_DEAD;
}
//通知点名测量对话框
void tsk_OtdrManage::notify_otdr_dlg(unsigned int res_cmd, int rescode)
{
    if(m_pParent->m_currenShowDlg > 0&&\
            res_cmd == m_pParent->objSyncSem[SEM_OTDR_DLG].resCmd)
    {
        m_pParent->objSyncSem[SEM_OTDR_DLG].resCode = rescode;
        m_pParent->objSyncSem[SEM_OTDR_DLG].objSem.release();
    }
    qDebug("res cmd %x code %d",res_cmd,rescode);
    qDebug("obj cmd %x code %d",\
           m_pParent->objSyncSem[SEM_OTDR_DLG].resCmd,m_pParent->objSyncSem[SEM_OTDR_DLG].resCode);
}
/*
//获取otdr曲线
int tsk_OtdrManage::get_otdr_curv(_tagCtrlAppointTest *pAppointTest,  _tagDevComm *pEndDev, unsigned int cmd , unsigned int res_cmd, int ack_to)
{
    glink_addr send_addr;
    tms_devbase devbase;
    _tagDevComm *posw;
    _tagResPortInfo portResInfo;
    int return_val, res_code;
    int wait_curv_time;
    bool result, send_parar_ok;
    char short_msg[SHORT_MSG_LEN];
    int otdr_port;
    otdr_port = pEndDev->port;
    //回应信息
    portResInfo.port = otdr_port;
    portResInfo.opt.dst = res_addr.dst; //命令的源地址
    portResInfo.opt.src = res_addr.src;//本机地址
    portResInfo.opt.pkid = res_addr.pkid;//命令所带的pkid

    result = false;
    return_val = -1;
    send_parar_ok = false;
    posw = &pAppointTest->test_port;

    send_addr.dst  = ADDR_CARD;
    send_addr.src = ADDR_MCU;
    send_addr.pkid = res_addr.pkid;
    //通过机框，槽位获取fd
    tms_GetDevBaseByLocation(pEndDev->frame_no, pEndDev->card_no,&devbase);

    if(devbase.fd <= 0)
    {  //fd error
        res_code = RET_COMMU_ABORT;
        strcpy(short_msg,"otdr test wait curv  fd error!");
        goto usr_exit;
    }


    //发送测量参数,返回码是8**0038
    pObjSem[otdr_port].sendCmd = cmd;
    pObjSem[otdr_port].resCmd = cmd;
    pObjSem[otdr_port].pkid = res_addr.pkid;
    //等待曲线的时间
    wait_curv_time = pAppointTest->para.measureTime_s *3000;
    //发送参数，需要根据目的地址，源地址区分曲线发到哪里
    qDebug("get otdr curv src 0x%x dst 0x%x pkid %d", to_addr.src, to_addr.dst, to_addr.pkid);
    return_val = tms_AnyGetOTDRTest( devbase.fd,(glink_addr*)(&to_addr),posw->frame_no,posw->card_no, \
                                     posw->type, posw->port, pEndDev->port, (tms_getotdr_test_param *)&pAppointTest->para, cmd);
    //发送成功，等待回应
    if(return_val >= 0)
    {
        result = pObjSem[otdr_port].objSem.tryAcquire(1,DATA_RETURN_WAIT_TIME);
        //如果等来信号
        if(result)
        {
            res_code = pObjSem[otdr_port].resCode;
            //第一次回应，要通知对方，参数发送如何
            if(ack_to == ACK_TOT_DLG)
            {
                notify_otdr_dlg(cmd,res_code );
            }
            else if(ack_to == ACK_TO_NM &&res_code != RET_SUCCESS )
            {
                devbase.fd =  tms_SelectFdByAddr(&res_addr.dst);
                tms_Ack(devbase.fd,(glink_addr*)(&res_addr),res_code,cmd);

            }
            //等待回应，参数正确
            if(res_code == RET_SUCCESS)
            {
                send_parar_ok = true;
                pObjSem[otdr_port].resCmd = res_cmd;
                port_state_buf[otdr_port].wait_time = wait_curv_time;
                //第二次等待专门的线程完成
                waitCurvPortQue.obj.lock();
                waitCurvPortQue.xlist.append(portResInfo);
                waitCurvPortQue.obj.unlock();
                qDebug("apend wait queue port %d list len %d size %d empty %d", otdr_port,waitCurvPortQue.xlist.count(), \
                       waitCurvPortQue.xlist.size(),waitCurvPortQue.xlist.isEmpty());
            }
            //发送测量参数超时
            else
            {
                res_code = RET_SEND_CMMD_TIMEOUT;
                strcpy(short_msg, "otdr test send cmd time out");
            }
        }
        else
        {
            //等待命令回应超时
            res_code = RET_SEND_CMMD_TIMEOUT;
            strcpy(short_msg,"otdr test wait res cmmd time out!");
            return_val = -1;
        }
    }
    else
    {
        //通信异常
        strcpy(short_msg,"otdr test send cmmd faile !");
        res_code = RET_COMMU_ABORT;
        return_val = -1;
    }



    if(send_parar_ok)
        goto usr_exit;
    //如果失败，那么处理返回结果，成功
    if(ack_to == ACK_TOT_DLG)
    {
        notify_otdr_dlg(cmd,res_code );
    }
    else if(ack_to == ACK_TO_NM&&res_code != RET_SUCCESS) //网管要求成功时不返回回应码
    {
        devbase.fd =  tms_SelectFdByAddr(&res_addr.dst);
        tms_Ack(devbase.fd,(glink_addr*)(&res_addr),res_code,cmd);
    }
    //是否输出调试信息
    if(res_code != RET_SUCCESS )
    {
        qDebug("%s",short_msg);
    }
    pObjSem[otdr_port].sendCmd = -1;
    pObjSem[otdr_port].resCmd = -1;

usr_exit :    
    return res_code;
}
*/
//获取otdr曲线
int tsk_OtdrManage::get_otdr_curv(_tagCtrlAppointTest *pAppointTest,  _tagDevComm *pEndDev, unsigned int cmd , unsigned int res_cmd, int ack_to)
{
    _tagResPortInfo portResInfo;
    int  res_code;
    int wait_curv_time;
    int otdr_port;
    if(tsk_attribute == TSK_ATTRIBUTE_CTU)
        otdr_port = 0;
    else
        otdr_port = pEndDev->port;
    //回应信息
    portResInfo.port = otdr_port;
    portResInfo.opt.dst = res_addr.dst; //命令的源地址
    portResInfo.opt.src = res_addr.src;//本机地址
    portResInfo.opt.pkid = res_addr.pkid;//命令所带的pkid

    res_code =  send_test_para(pAppointTest, pEndDev,cmd,wait_curv_time);

    if(res_code == RET_SUCCESS)
    {
        pObjSem[otdr_port].resCmd = res_cmd;
        port_state_buf[otdr_port].wait_time = wait_curv_time;
        //第二次等待专门的线程完成
        waitCurvPortQue.obj.lock();
        waitCurvPortQue.xlist.append(portResInfo);
        waitCurvPortQue.obj.unlock();
        qDebug("apend wait queue port %d list len %d size %d empty %d", otdr_port,waitCurvPortQue.xlist.count(), \
               waitCurvPortQue.xlist.size(),waitCurvPortQue.xlist.isEmpty());
    }
    else
    {
        pObjSem[otdr_port].sendCmd = -1;
        pObjSem[otdr_port].resCmd = -1;
    }


usr_exit :
    return res_code;
}
//通知osw开始/测量完成
int tsk_OtdrManage :: tsk_send(char devbuf[] , char buf[], unsigned int  cmd, int res_cmd, int otdr_port,int ack_to)
{
    //发送地址
    glink_addr send_addr;
    tms_devbase *pdevbase;
    tms_ack ack;

    int return_val, res_code;
    bool result;
    char short_msg[64];
    result = false;
    res_code = RET_SUCCESS;
    pdevbase =  (tms_devbase *)devbuf;
    send_addr.dst  = ADDR_CARD;
    send_addr.src = ADDR_MCU;
    send_addr.pkid = res_addr.pkid;
    //通过机框，槽位获取fd
    /*
     *2016-01-19 外面已经调用tms_GetDevBaseByLocation，重复调用会有问题
    */
   // tms_GetDevBaseByLocation(pdevbase->frame, pdevbase->slot,pdevbase);
    if(pdevbase->fd > 0)
    {
        //设置要等待的回应码
        pobjComm[otdr_port].sendCmd = cmd;
        pobjComm[otdr_port].resCmd = cmd;
        pobjComm[otdr_port].pkid = res_addr.pkid;
        return_val = send_cmd(pdevbase->fd, buf,cmd);
        //发送成功，等待回应
        if(return_val >= 0)
        {
            result = pobjComm[otdr_port].objSem.tryAcquire(1,DATA_RETURN_WAIT_TIME);
            if(result)
            {
                res_code = pobjComm[otdr_port].resCode;
            }
            else
            {
                //等待命令回应超时
                res_code = RET_SEND_CMMD_TIMEOUT;
                strcpy(short_msg,"switch osw wait res cmmd time out!");
            }
        }
        else
        {
            //通信异常
            strcpy(short_msg,"switch osw send cmmd faile !");
            res_code = RET_COMMU_ABORT;
        }
    }
    else
    {
        //fd error
        res_code = RET_COMMU_ABORT;
        strcpy(short_msg,"switch osw  fd error!");
    }

    //处理结果
    if(res_code != RET_SUCCESS)
    {
        if(ack_to == ACK_TOT_DLG)
        {
            notify_otdr_dlg(res_cmd,res_code );
        }
        //向网管汇报
        else if(ack_to == ACK_TO_NM)
        {
            //获取网管地址
            pdevbase->fd = tms_SelectFdByAddr(&res_addr.dst);
            ack.reserve1 = otdrAddr.frame_no << 16 & 0xffff0000;
            ack.reserve1 = ack.reserve1 | (otdrAddr.card_no | 0xffff0000);
            ack.reserve2 = otdrAddr.type << 16 & 0xffff0000;
            ack.reserve2 = ack.reserve2 | (otdrAddr.port | 0xffff0000);
            ack.cmdid = res_cmd;
            ack.errcode = res_code;
            tms_AckEx(pdevbase->fd, (glink_addr*)(&res_addr),&ack);
        }
        //        else
        //            tms_Trace(&send_addr,short_msg,strlen(short_msg),1);
        qDebug("%s",short_msg );
    }
    //恢复初始值
    pobjComm[otdr_port].sendCmd = -1;
    pobjComm[otdr_port].resCmd = -1;
    return res_code;
}
//根据命令选择发送函数
int tsk_OtdrManage::send_cmd(int fd, char buf[],int cmd)
{
    _tagDevComm *posw_port;
    glink_addr card_addr;
    _tagDevComm *polp;
    int return_val;
    card_addr.dst = ADDR_CARD;
    card_addr.src = ADDR_MCU;
    card_addr.pkid = res_addr.pkid;
    switch (cmd)
    {
    //通知光开关进行切换
    case ID_CMD_OSW_SWITCH:
    {
        posw_port = (_tagDevComm*)buf;
        //通知光开关切换
        return_val = tms_MCU_OSWSwitch (fd, &card_addr,posw_port->frame_no, \
                                        posw_port->card_no,posw_port->port);
        break;
    }
        //通知olp,otdr开始测量
    case ID_CMD_OLP_START_OTDR:
    {
        polp = (_tagDevComm *)buf;
        return_val = tms_MCU_OLPStartOTDRTest(fd, &card_addr,polp->frame_no, \
                                              polp->card_no, polp->port);
        qDebug("olp start frame %d card %d port%d",\
               polp->frame_no,polp->card_no,polp->port);
        break;
    }
        //通知olp,otdr完成测量
    case ID_CMD_OLP_FINISH_OTDR:
    {
        polp = (_tagDevComm *)buf;
        return_val = tms_MCU_OLPFinishOTDRTest(fd, &card_addr,polp->frame_no, \
                                               polp->card_no,polp->port);
        qDebug("olp end start frame %d card %d port%d",\
               polp->frame_no,polp->card_no,polp->port);
        break;
    }
    default:
        return_val = -1;
        m_pParent->objSyncSem[SEM_T_OSW_OLP].resCmd = -1;
    }
    return return_val;
}
int tsk_OtdrManage::inspectFiber(unsigned int cmd)
{

    int eventNumRefer, eventNumCur;
    int alarmLev,i, j,m;
    int xSpace, alarmPos;
    float sample_Mhz;
    int fd, res_code;
    unsigned short * pDataPtCur;
    unsigned short * pDataPtRef;
    char msg[SHORT_MSG_LEN];
    _tagEvent *pEventBufRefer, *pEventBufCur;
    _tagAlarmTh *pAlarmTh;
    _tagDevComm *posw_port;
    tms_alarm_line_hdr alarm_line;
    //发送函数接口部分
    tms_retotdr_test_hdr   *ptest_hdr;
    tms_retotdr_test_param *ptest_param;
    tms_retotdr_data_hdr   *pdata_hdr;
    tms_retotdr_data_val   *pdata_val;
    tms_retotdr_event_hdr  *pevent_hd;
    tms_retotdr_event_val  *pevent_val;
    tms_retotdr_chain      *pchain;
    glink_addr dst_addr;
    float deltaLoss;
    QCurv drawCurv;
    float div_distance;
    int pos_cur, pos_ref;

    //初始化相关的指针
    ptest_hdr = (tms_retotdr_test_hdr*)&AlarmCurv.curv.oswPort;
    ptest_param = (tms_retotdr_test_param*)&AlarmCurv.curv.measurPara;
    pdata_hdr = (tms_retotdr_data_hdr*)&AlarmCurv.curv.dpid;
    pdata_val = (tms_retotdr_data_val*)AlarmCurv.curv.dataPt;
    pevent_hd = (tms_retotdr_event_hdr*)AlarmCurv.curv.eventID;
    pevent_val = (tms_retotdr_event_val*)AlarmCurv.curv.eventBuf;
    pchain = (tms_retotdr_chain*)AlarmCurv.curv.measurResultID;
    fd = tms_GetManageFd();  //获取网管fd
    dst_addr.src = ADDR_MCU;
    dst_addr.dst = ADDR_NET_MANAGER;
    alarmLev = 0;
    deltaLoss = 0;
    alarmPos = 0;
    memset(msg, 0 , sizeof(msg));
    AlarmCurv.mutexObj.lock();
    posw_port = (_tagDevComm *)(&AlarmCurv.curv.oswPort);
    res_code = m_pParent->db_get_refr_otdr(posw_port, &ReferCrv);
    if( res_code != RET_SUCCESS) //获取参考曲线失败
    {
        sprintf(msg, "inspect fiber alarm get otdr ref_otdr error");
        goto usr_exit;
    }

    pAlarmTh = &ReferCrv.alarmTh;
    sample_Mhz = ReferCrv.measurPara.samplHz / pow(10,6);
    m = ReferCrv.measurPara.pulseWidth_ns * sample_Mhz / pow(10,3);

    //参考曲线和当前otdr曲线的事件点数目
    eventNumRefer = ReferCrv.eventNum;
    eventNumCur = AlarmCurv.curv.eventNum;
    pEventBufRefer = ReferCrv.eventBuf;
    pEventBufCur = AlarmCurv.curv.eventBuf;
    pDataPtCur = AlarmCurv.curv.dataPt;
    pDataPtRef = ReferCrv.dataPt;

    //初始化告警结构体
    memset(&alarm_line, 0, sizeof(tms_alarm_line_hdr));
    alarm_line.alarm_type = ALARM_LINE;
    alarm_line.frame = AlarmCurv.curv.oswPort.frame_no;
    alarm_line.slot = AlarmCurv.curv.oswPort.card_no;
    alarm_line.port = AlarmCurv.curv.oswPort.port;
    m_pParent->GetCurrentTime((char *)(alarm_line.time));
    //测试原因
    alarm_line.reserve0 = AlarmCurv.curv.measurPara.test_reason;

    div_distance = drawCurv.Get_xCoord(2*m, ptest_param->sample,ptest_param->gi);

    qDebug("div distace %f, m %d, ns %d", div_distance, m, ReferCrv.measurPara.pulseWidth_ns);
    qDebug("sample %d  gi %f referLen %f cur_chain %f pchain->range ",\
           ptest_param->sample,ptest_param->gi, ReferCrv.measurResult.fiberLen, pchain->range);

    if(ReferCrv.measurResult.fiberLen > (pchain->range + div_distance))
    {
        alarmLev = 1;
        alarmPos =  pchain->range;
    }
    else
    {
        //开始比较事件点
        for(i = 0; i < eventNumCur;i++)
        {
            deltaLoss = 0;
            for(j = 0; j < eventNumRefer;j++)
            {
                xSpace = abs(pEventBufCur[i].pos - pEventBufRefer[j].pos);
                if(xSpace < 2*m)
                    break;
            }
            //相同位置的事件点
            if(j < eventNumRefer)
            {
                //插入损耗均存在
                if(pEventBufCur[i].insertLoss < INVALID_VALUE \
                        && pEventBufRefer[j].insertLoss < INVALID_VALUE)
                {
                    deltaLoss = fabs(pEventBufCur[i].insertLoss - pEventBufRefer[j].insertLoss);
                }
                //参考曲线的插损不存在
                else if(pEventBufCur[i].insertLoss < INVALID_VALUE)
                {
                    deltaLoss = pEventBufCur[i].insertLoss;
                }
                else //当前曲线插损不存在，用对应点的损耗代替
                {
                    pos_cur = pEventBufCur[i].pos;
                    pos_ref = pEventBufRefer[j].pos;
                    deltaLoss = fabs(pDataPtCur[pos_cur] - pDataPtRef[pos_ref]);
                    qDebug("插损不存在，当前/参考 事件序号%d %d",i,j);
                }
            }
            //新增事件，如果超出参考曲线结束事件点之后，不处理
            else if(abs(pEventBufCur[i].pos - pEventBufRefer[eventNumRefer - 1].pos) < 2*m)
            {
                if(pEventBufCur[i].insertLoss < INVALID_VALUE)
                    deltaLoss = pEventBufCur[i].insertLoss;
                //当前曲线插损不存在
                else //当前曲线插损不存在，用对应点的损耗代替
                {
                    pos_cur = pEventBufCur[i].pos;
                    deltaLoss = fabs(pDataPtCur[pos_cur] - pDataPtRef[pos_cur]);
                    qDebug("插损不存在，当前/参考 事件序号%d %d",i,j);
                }
            }
            alarmLev = m_pParent->getAlarmLev(deltaLoss, pAlarmTh);
            //告警级别大于0，结束循环
            if(alarmLev > 0)
            {

                alarmPos =  drawCurv.Get_xCoord(pEventBufCur[i].pos, ptest_param->sample,ptest_param->gi);
                break;
            }
        }
    }
    //发送线路告警
    alarm_line.alarm_level = alarmLev;
    alarm_line.alarm_position = alarmPos;
    if(m_pParent->m_ctrlStat.NMstat == NM_EXIST)
    {
        qDebug("inspect fiber send   arlarm curv to pc ");
        tms_AlarmLine(fd,&dst_addr,&alarm_line,ptest_hdr,ptest_param,pdata_hdr,pdata_val,\
                      pevent_hd,pevent_val,pchain,ID_RET_OTDR_CYC);
    }
    else if(m_pParent->m_ctrlStat.NMstat != NM_EXIST)
    {
        qDebug("inspect fiber save  arlarm curv db ");
        m_pParent->input_gsm_queue(alarmLev, ALARM_LINE,posw_port);
        m_pParent->db_save_alarm_curv((char *)(&AlarmCurv.curv), (char *)(&alarm_line));
    }
#if USR_DEBUG
    qDebug("inspect fiber alarm lev %d", alarmLev);
    /*
    qDebug("ptest_hdr  osw frame %d otdr port %d", ptest_hdr->osw_frame, ptest_hdr->otdr_port);
    qDebug("ptest_param  range %d  reserv1 %d", ptest_param->rang, ptest_param->reserve1);
    qDebug("pdata_hdr  dpid %s data num %d", pdata_hdr->dpid, pdata_hdr->count);
    qDebug("pevent_hd  eventid  %s event num %d", pevent_hd->eventid, pevent_hd->count);
    qDebug("pchain  inf %s att %f ", pchain->inf, pchain->att);
    */
#endif

usr_exit:
    AlarmCurv.mutexObj.unlock();
    qDebug("inspect fiber proce arlarm curv");
    return res_code;
}
/*
   **************************************************************************************
 *  函数名称：sendSignal
 *  函数描述：提供给外部调用的发送接口
 *  入口参数：事件类型，其他参数
 *  返回参数：
 *  作者       ：
 *  日期       ：
 **************************************************************************************
*/
void tsk_OtdrManage:: sendSignal(int usr_event, unsigned option)
{
    switch(usr_event)
    {
    case USR_EVENT_FIND_ALARM:
    {
        qDebug(" otdr tsk  emmit");
        emit(findAlarm(option));
    }
    default:
        break;
    }

}
/*
   **************************************************************************************
 *  函数名称：setModulPara
 *  函数描述：提供给外面调用设置波长信息
 *  入口参数：
 *  返回参数：
 *  作者       ：
 *  日期       ：
 **************************************************************************************
*/
void tsk_OtdrManage:: setModulPara(QStringList lamdaList)
{
    modulPara.lamdaList.clear();
    modulPara.lamdaList = lamdaList;
    modulPara.initial = 1;
}
/*
   **************************************************************************************
 *  函数名称：set_tsk_attribute
 *  函数描述：设置任务是RTU还是CTU，默认的是RTU，CTU必须显式设置
 *                ：如果入口参数不是CTU或者RTU，返回参数非法，同时设置为RTU
 *  入口参数：
 *  返回参数：
 *  作者       ：
 *  日期       ：2015-11-05
 *  修改日期：
 *  修改内容：
 *                ：
 **************************************************************************************
*/
int tsk_OtdrManage::set_tsk_attribute(int attribute)
{
    int retv;
    if(attribute == TSK_ATTRIBUTE_RTU || attribute == TSK_ATTRIBUTE_CTU)
    {
        tsk_attribute = attribute;
        retv = RET_SUCCESS;
    }
    else
    {
        tsk_attribute = TSK_ATTRIBUTE_CTU;
        retv = RET_PARAM_INVALID;
    }
    return retv;
}
/*
   **************************************************************************************
 *  函数名称：ctuGetRtuFd
 *  函数描述：CTU任务获取RTU的fd
 *                ：
 *  入口参数：RTU IP
 *  返回参数：FD
 *  作者       ：
 *  日期       ：2015-11-05
 *  修改日期：
 *  修改内容：
 *                ：
 **************************************************************************************
*/
extern struct ep_t ep;
int tsk_OtdrManage::ctuGetRtuFd(int IP)
{
    struct ep_con_t client;
    char *pstrAddr;
    struct in_addr ip_addr;
    int fd;
    fd = m_pParent->mcu_get_fd_by_ip(IP);
    if(fd < 0)
    {
        memcpy(&ip_addr, &IP, sizeof(int));
        pstrAddr = inet_ntoa(ip_addr);
        if (0 == ep_Connect(&ep,&client, pstrAddr, 6000))
        {
            printf("ok, very good");
            fd = client.sockfd;
        }

    }

    return fd;
}
/*
   **************************************************************************************
 *  函数名称：get_end_dev_fd
 *  函数描述：获取级联路由最后一个模块的fd,ctu获取到的是RTU 的fd, RTU任务
 *                ：获取到的是OTDR模块的fd
 *  入口参数：
 *  返回参数：
 *  作者       ：
 *  日期       ：2015-11-06
 *  修改日期：
 *  修改内容：
 *                ：
 **************************************************************************************
*/
int tsk_OtdrManage::get_end_dev_fd(_tagCtrlAppointTest *pAppointTest, _tagDevComm *pendDev)
{
    tms_devbase devbase;
    int fd;
    fd = -1;
    if(tsk_attribute == TSK_ATTRIBUTE_CTU)
        fd = ctuGetRtuFd(pAppointTest->para.src_ip);
    else if(tsk_attribute ==TSK_ATTRIBUTE_RTU)
    {
        tms_GetDevBaseByLocation(pendDev->frame_no, pendDev->card_no,&devbase);
        fd = devbase.fd;
    }

    return fd;
}
/*
   **************************************************************************************
 *  函数名称：send_test_para
 *  函数描述：CTU任务中向RTU点名测量;RTU向OTDR发送测试指令
 *                ：
 *  入口参数：
 *  返回参数：
 *  作者       ：
 *  日期       ：2015-11-05
 *  修改日期：
 *  修改内容：
 *                ：
 **************************************************************************************
*/
int tsk_OtdrManage::send_test_para(_tagCtrlAppointTest *pAppointTest, \
                                   _tagDevComm *pEndDev, unsigned int cmd,int &waitTime_ms)
{
    glink_addr send_addr;
    _tagDevComm *pTestDev;
    struct sockaddr_in sin;
    int return_val, res_code;
    bool result, send_parar_ok;
    int otdr_port, fd;
    int ack_to;
    if(tsk_attribute == TSK_ATTRIBUTE_CTU)
        otdr_port = 0;
    else
        otdr_port = pEndDev->port;
    ack_to = pAppointTest->ack_to;
    result = false;
    send_parar_ok = false;
    //根据任务模式选择目的地址
    if(tsk_attribute == TSK_ATTRIBUTE_CTU)
    {
        send_addr.dst  = ADDR_MCU;
        /*
         *2016-03-08 如果是CTU模式，需要将本机IP，加入到测量参数中
         *作为RTU的MCU根据IP选择对应的通信端.此处需要完善
        */
        pAppointTest->para.src_ip = 0;
        m_pParent->detect_net_card(&pAppointTest->para.src_ip);
        sin.sin_addr.s_addr = pAppointTest->para.src_ip;
        printf("%s: Line : %d local wlan ip %d str ip %s",__FUNCTION__, __LINE__,\
               pAppointTest->para.src_ip,inet_ntoa(sin.sin_addr));
        if( pAppointTest->para.src_ip <= 0)
        {
            res_code = RET_GET_LOCAL_WLAN_IP_FAIL;
            goto usr_exit;
        }

    }
    else
    {
        /*
         *2016-03-08 如果是RTU模式，参数中对应的ip设置成0
        */
        send_addr.dst  = ADDR_CARD;
        pAppointTest->para.src_ip = 0;
    }

    send_addr.src = ADDR_MCU;
    send_addr.pkid = res_addr.pkid;
    fd = get_end_dev_fd( pAppointTest, pEndDev);
    if(fd <= 0 )
    {
        res_code = RET_COMMU_ABORT;
        goto usr_exit;
    }

    //发送测量参数,返回码是8**0038
    pObjSem[otdr_port].sendCmd = cmd;
    pObjSem[otdr_port].resCmd = cmd;
    pObjSem[otdr_port].pkid = res_addr.pkid;
    pObjSem[otdr_port].resrvd3 = 0;
    pTestDev = &(pAppointTest->test_port);
    //发送参数，需要根据目的地址，源地址区分曲线发到哪里    
    printf("%s: Line : %d src 0x%x dst 0x%x pkid 0x%x",__FUNCTION__, __LINE__,\
            to_addr.src, to_addr.dst, to_addr.pkid);
    return_val = tms_AnyGetOTDRTest( fd,(glink_addr*)(&to_addr),pTestDev->frame_no,pTestDev->card_no, \
                                     pTestDev->type, pTestDev->port, otdr_port, \
                                     (tms_getotdr_test_param *)&pAppointTest->para, cmd);
    waitTime_ms = pAppointTest->para.measureTime_s *3000;
    //发送成功，等待回应
    if(return_val >= 0)
    {
        result = pObjSem[otdr_port].objSem.tryAcquire(1,DATA_RETURN_WAIT_TIME);
        //如果等来信号
        if(result)
        {
            res_code = pObjSem[otdr_port].resCode;
            //等待曲线的时间
            if(tsk_attribute == TSK_ATTRIBUTE_CTU)
                waitTime_ms  += pObjSem[otdr_port].resrvd3 ;

        }
        //发送测量参数超时
        else
        {
            res_code = RET_SEND_CMMD_TIMEOUT;
        }
    }
    else
    {
        res_code = RET_COMMU_ABORT;;
    }

usr_exit:
    if(ack_to == ACK_TOT_DLG)
    {
        notify_otdr_dlg(cmd,res_code );
    }
    else if(ack_to == ACK_TO_NM &&res_code != RET_SUCCESS )
    {
        fd =  tms_SelectFdByAddr(&res_addr.dst);
        tms_Ack(fd,(glink_addr*)(&res_addr),res_code,cmd);

    }
    printf("%s: Line : %d  res_code %d", __FUNCTION__, __LINE__, res_code);
    return res_code;
}
