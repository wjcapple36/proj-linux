#include "tsk_waitotdrcurv.h"
#include "protocol/tmsxx.h"

/*
   **************************************************************************************
 *  函数名称：tsk_waitOtdrCurv
 *  函数描述：构造函数，获取父对象
 *  入口参数：
 *  返回参数：
 *  作者       ：
 *  日期       ：
 **************************************************************************************
*/
tsk_waitOtdrCurv::tsk_waitOtdrCurv(QObject *parent) :
    QThread(parent)
{
    pOtdrManager = (tsk_OtdrManage *)parent;
    stopped = false;
}
/*
   **************************************************************************************
 *  函数名称：tsk_waitOtdrCurv
 *  函数描述：处理函数
 *  入口参数：
 *  返回参数：
 *  作者       ：
 *  日期       ：
 **************************************************************************************
*/
void tsk_waitOtdrCurv:: run()
{
    int i, port,wait_time;
    bool result, is_deal;
    _tagWaitCurvPort *pWaitPortQue;
    _tagPortState *ptrPortStateBuf;
    _tagObjSynvSem *ptrObjSem;    
    tms_devbase devbase;
    _tagResPortInfo portResInfo;
    tms_ack ack;
    i = 0;
    pWaitPortQue = &pOtdrManager->waitCurvPortQue;
    ptrPortStateBuf = pOtdrManager->port_state_buf;
    ptrObjSem = pOtdrManager->pObjSem;
    wait_time = 100;//每次等待的时间
    qDebug("waitOtdrCurv LINE : %d wordking frame %d card %d", __LINE__,\
           pOtdrManager->otdrAddr.frame_no, pOtdrManager->otdrAddr.card_no);
    //2016-05-25 输出线程号，与htop中的线程号对应
    printf("%s : Line : %d  thread id %ld \n",  __FILE__, __LINE__,(long int)syscall(224));
    while(!stopped)
    {
//        qDebug("list count %d frame %d card %d", pWaitPortQue->xlist.count(),pOtdrManager->otdrAddr.frame_no, \
//               pOtdrManager->otdrAddr.card_no);
        pWaitPortQue->obj.lock();
        pWaitPortQue->obj.unlock();
        if(pWaitPortQue->xlist.count() > 0)
        {
            for(i = 0; i < pWaitPortQue->xlist.count();i++)
            {                
                portResInfo = pWaitPortQue->xlist.at(i);
                port = portResInfo.port;
                result = ptrObjSem[port].objSem.tryAcquire(1,wait_time);
                is_deal = false;
                if(result)
                {
                    is_deal = true;
                }
                else
                {
                    ptrPortStateBuf[port].wait_time -=  wait_time;
                    if(ptrPortStateBuf[port].wait_time <= 0)
                    {
                        is_deal = true;
                        ptrObjSem[port].resCode = RET_OTDR_TIMEOUT;//超时
                    }
                }
                if(is_deal)
                {
                    //等到信号
                    ptrPortStateBuf[port].wait_time = 0;
                    qDebug(" wait queue deal, port %d, res_cmd 0x%x , retv %d,ack to %d",\
                           port, ptrObjSem[port].resCmd, ptrObjSem[port].resCode,ptrPortStateBuf[port].ack_to);
                    //清除掉本级按钮
                    pWaitPortQue->obj.lock();
                    pWaitPortQue->xlist.removeAt(i);
                    pWaitPortQue->obj.unlock();
                    if(ptrPortStateBuf[port].ack_to == ACK_TOT_DLG)//本地对话框
                    {
                        pOtdrManager->notify_otdr_dlg( ptrObjSem[port].resCmd, ptrObjSem[port].resCode);
                    }                   
                    else if(ptrPortStateBuf[port].ack_to == ACK_TO_NM &&ptrObjSem[port].resCode != RET_SUCCESS) //回应给网管，需要知道回给哪个网管
                    {                        
                        //如果需要加机框槽位也是可以，创建该任务的任务的机框，槽位即时回应ack的槽位号
                        devbase.fd = tms_SelectFdByAddr(&portResInfo.opt.dst);
                        bzero(&ack, sizeof(tms_ack));
                        ack.cmdid = ptrObjSem[port].resCmd;
                        ack.errcode =  ptrObjSem[port].resCode;
                        //reserve1 高16位机框，低16位槽位 reserve2 高16 类型，低16 端口
                        ack.reserve1 = pOtdrManager->otdrAddr.frame_no << 16 & 0xFFFF0000;
                        ack.reserve1 = ack.reserve1 | (pOtdrManager->otdrAddr.card_no | 0x0000FFFF);
                        ack.reserve2 = pOtdrManager->otdrAddr.type << 16 & 0xFFFF0000;
                        ack.reserve2 = ack.reserve2 | (pOtdrManager->otdrAddr.port | 0x0000FFFF);

                        tms_AckEx(devbase.fd, (glink_addr *)(&portResInfo.opt),&ack);
                    }

                    if(pOtdrManager->otdr_mode == OTDR_MODE_SERRI)
                        pOtdrManager->set_all_port_idle();
                    else
                    {
                        bzero(&ptrPortStateBuf[port], sizeof(_tagPortState));
                        //ptrPortStateBuf[port].state = PORT_IDLE;
                    }
                }
            }
        }
        //2016-05-25 防止CPU占有率过高
        usleep(1000);

    }
}
/*
   **************************************************************************************
 *  函数名称：stop
 *  函数描述：如果外界要终止本线程，那么首先要stop
 *  入口参数：
 *  返回参数：
 *  作者       ：
 *  日期       ：
 **************************************************************************************
*/
void tsk_waitOtdrCurv::stop()
{
    stopped = true;
}
