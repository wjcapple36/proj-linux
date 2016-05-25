#include "tsk_sockretrysend.h"

#include "protocol/tmsxx.h"

tsk_SockRetrySend::tsk_SockRetrySend(QObject *parent) :
    QThread(parent)
{
    pMainWindow = ( MainWindow *)parent;
    timer_space = 500; //定时器间隔
    count = 0;
    usr_timer_initial();
    stopped = false;
}
tsk_SockRetrySend::~tsk_SockRetrySend()
{
    if(usr_timer.ptr_timer != NULL)
    {
        delete usr_timer.ptr_timer;
    }
}
//设置用户定时器
void tsk_SockRetrySend::usr_timer_initial()
{
    usr_timer.ptr_timer = new QTimer;

    connect(usr_timer.ptr_timer,SIGNAL(timeout()),this,SLOT(usr_time_out()));

    usr_timer.ptr_timer->start(timer_space);
}
//超时函数
void tsk_SockRetrySend::usr_time_out()
{
    check_is_retry();
    //qDebug(" counter %d", count++);
}
/*
   **************************************************************************************
 *  函数名称：check_is_retry
 *  函数描述：检查是否有重发的数据
 *  入口参数：
 *  返回参数：
 *  作者       ：
 *  日期       ：
 **************************************************************************************
*/

void tsk_SockRetrySend::check_is_retry()
{
    int i, fd;
    glink_addr dst_addr;
    char *buf;
    _tagSockDataCpy DataCpy;
    for(i = 0; i < RetryList.list.size();i++)
    {
        RetryList.list[i].timer -= timer_space;
        DataCpy = RetryList.list[i];
        if(RetryList.list[i].retv != -1) //收到回应
        {

            //如果是周期性测量曲线，需要清除掉该曲线
            if(DataCpy.cmd == ID_RET_OTDR_CYC)
            {
                /*
                 *2016-01-28 收到曲线回应
                 */
                pMainWindow->m_ctrlUpData.IsPermitRet32 = RET_SUCCESS;
                if(DataCpy.buf_type == BUF_TYPE_DYANM)
                    buf = DataCpy.buf_dyanm;
                else
                    buf = DataCpy.buf_fixed;
                 pMainWindow->deal_db_alarm_curv(*((int *)buf), ALARM_CURV_DEL,DataCpy.pkid);
            }
            if(DataCpy.buf_type == BUF_TYPE_DYANM)
                delete []DataCpy.buf_dyanm;
            clear_retry_list(i);
            i--;
        }
        else if(RetryList.list[i].timer < 0) //等待时间到
        {
            //重发次数已经达到极限
            if(RetryList.list[i].retry_send_num >= RetryList.list[i].needed_retry_num)
            {

                //通过短信通知用户
                inform_usr_by_sms(&DataCpy);
                //如果是动态分配内存，那么需要释放掉
                if(DataCpy.buf_type == BUF_TYPE_DYANM)
                    delete []DataCpy.buf_dyanm;
                //重发次数已达到最大
                clear_retry_list(i);
                //因为删除一个元素，索引要保持不变
                i--;
            }
            else //还需要重发
            {
                dst_addr.dst = DataCpy.dst;
                dst_addr.src = DataCpy.src;
                dst_addr.pkid = DataCpy.pkid;
                if(DataCpy.buf_type == BUF_TYPE_DYANM)
                    buf = DataCpy.buf_dyanm;
                else
                    buf = DataCpy.buf_fixed;
                switch(DataCpy.cmd)
                {
                case ID_ALARM_HW: //发送总的硬件告警
                {
                    pMainWindow->send_total_hw_alarm(SEND_TYPE_RETRY,DataCpy.pkid);
                    break;
                }
                case ID_RET_OTDR_CYC:
                {
                    pMainWindow->deal_db_alarm_curv(*((int *)buf), SEND_TYPE_RETRY,DataCpy.pkid);
                    break;
                }
                case ID_RET_OLP_ACTION_LOG:
                {
                    /*
                     *2016-01-28 增加olp 切换记录
                    */
                    pMainWindow->send_olp_action_record_to_host(SEND_TYPE_RETRY,DataCpy.pkid,NULL);
                }
                case ID_RET_TOTAL_OP_ALARM:
                {
                    /*
                     *2016-01-29 增加主动发送光功告警
                    */
                    pMainWindow->send_totoal_opm_alarm(SEND_TYPE_RETRY,DataCpy.pkid,NULL);
                }
                default: //变化的光功告警，olp切换记录
                {
                    fd = tms_SelectFdByAddr(&dst_addr.dst);
                    send_cmd(fd,buf,DataCpy.cmd,(char *)(&dst_addr));
                }
                }
                //重发,并设定超时时间
                RetryList.list[i].retry_send_num++;
                RetryList.list[i].timer = DATA_RETURN_WAIT_TIME;
            }
        }
    }

}
/*
   **************************************************************************************
 *  函数名称：clear_retry_list
 *  函数描述：清空重发队列中的某个或者全部元素
 *  入口参数：
 *  返回参数：
 *  作者       ：
 *  日期       ：
 **************************************************************************************
*/
int tsk_SockRetrySend::clear_retry_list(int index)
{
    RetryList.obj.lock();
    if(index > 0 && index < RetryList.list.size())
        RetryList.list.removeAt(index);
    else
        RetryList.list.clear();
    RetryList.obj.unlock();
}

/*
   **************************************************************************************
 *  函数名称：run
 *  函数描述：检测是否收到回应
 *  入口参数：无
 *  返回参数：无
 *  作者       ：
 *  日期       ：
 **************************************************************************************
*/
void tsk_SockRetrySend::run()
{
    unsigned long  wait_time;
    wait_time = 100;
    int retv;
    //2016-05-25 输出线程号，与htop中的线程号对应
    printf("%s : Line : %d  thread id %ld \n",  __FILE__, __LINE__,(long int)syscall(224));
    while (!stopped)
    {
        retv = wait_response();
        if(retv == RET_SUCCESS)
        {
            rcv_response();
        }
        //2016-05-25防止cpu占用率过高
        usleep(100);
    }

    stopped = false;
}
/*
   **************************************************************************************
 *  函数名称：wait_response
 *  函数描述：等待处理结果，如果阻塞太时间太长，程序退出是，线程无法
 *  函数描述：释放，因此需要分段等待
 *  入口参数：需要等待时间，-1，永久等待
 *  返回参数：
 *  作者       ：
 *  日期       ：
 **************************************************************************************
*/
 int tsk_SockRetrySend::wait_response(int wait_time_ms)
{
    int count, wait_space;
    int retv;
    retv = RET_SEND_CMMD_TIMEOUT;
    bool result;
    wait_space = 200;
    result = false;
    count = 0;
    if(wait_time_ms > 0) //有超时时间
    {
        while(count < wait_time_ms && !stopped)
        {
            result = objSynSem.objSem.tryAcquire(1, wait_space);
            if(result)
            {
                retv = RET_SUCCESS;
                break;
            }
            else
            {
                count += wait_space;
            }


        }
    }
    else //永久阻塞
    {
        while(!stopped)
        {
            result = objSynSem.objSem.tryAcquire(1, wait_space);
            if(result)
            {
                retv = RET_SUCCESS;
                break;
            }
        }
    }

    return retv;
}
 /*
   **************************************************************************************
 *  函数名称：rcv_response
 *  函数描述：收到对端的回应。里面使用了两个锁。第一个锁防止第一个回应没
 *  函数描述：有处理完成，第二个回应又出现；第而个锁随防止重发数据信息因
 *  函数描述：超时而被删除，造成错误的写入。都是属于短时操作。
 *  入口参数：
 *  返回参数：
 *  作者       ：
 *  日期       ：
 **************************************************************************************
*/
 int tsk_SockRetrySend::rcv_response()
 {
     int i,retv;
     retv = -1;
     //收到回应的时候同步信号，另外一个地方是在MainWindow里面收到ACK需要等待该信号
     objSynSem.objMutex.lock();
     //修改重发信息的时候同步信号，另外一个地方是在删除list内容里面的时需要等待该信号
     RetryList.obj.lock();

     for(i = 0;i < RetryList.list.size();i++)
     {
         if(objSynSem.resCmd == RetryList.list[i].cmd && objSynSem.pkid == RetryList.list[i].pkid)
         {
             retv = 0;
             RetryList.list[i].retv = objSynSem.resCode;
         }
     }
     //释放锁
     RetryList.obj.unlock();
     //释放另外一个锁
     objSynSem.objMutex.unlock();
     return 0;
 }

void tsk_SockRetrySend::stop()
{
    stopped = true;
}
int tsk_SockRetrySend::usr_delay(int delay_time_ms)
{
    int sleep_space, count;
    int sleep_us;
    sleep_us = delay_time_ms*1000;
    count = 0;
    sleep_space = 200000; //200ms
    while(count < sleep_us && !stopped)
    {
        usleep(sleep_space);
        count += sleep_space;
    }
    return 0;

}
/*
   **************************************************************************************
 *  函数名称：inform_usr_by_sms
 *  函数描述：重发失败之后，改用短信的方式发送
 *  入口参数：
 *  返回参数：
 *  作者       ：
 *  日期       ：
 **************************************************************************************
*/
int tsk_SockRetrySend::inform_usr_by_sms(_tagSockDataCpy *pDataCpy)
{
    char *buf;
    if(pDataCpy->buf_type == BUF_TYPE_DYANM)
        buf = pDataCpy->buf_dyanm;
    else
        buf = pDataCpy->buf_fixed;
    switch(pDataCpy->cmd)
    {
    case ID_REPORT_OLP_ACTION:
    {
        input_sms_queue_olp_switch(buf); //olp切换记录通过短信发送
        break;
    }
    case ID_ALARM_OPM_CHANGE:
    {
        input_sms_queue_opmAlarm_chang(buf);//变化的光功告警通过短信发送
        break;
    }    
    case ID_RET_ALARM_HW_CHANGE: //发送变化的硬件告警通过短信发送
        pMainWindow->input_changed_hw_gsm_queue(buf);
        break;
    default:
        break;
    }

    return 0;
}
/*
   **************************************************************************************
 *  函数名称：input_sms_queue_olp_switch
 *  函数描述：光开关发生切换重发次数超时，通过短信发送
 *  入口参数：
 *  返回参数：
 *  作者       ：
 *  日期       ：
 **************************************************************************************
*/
int tsk_SockRetrySend::input_sms_queue_olp_switch(char buf[])
{
    _tagDevComm *pdev;
    QString str, str1;
    char msg[48];
    QByteArray pbuf;
    int offset, switch_type, switch_port;

    offset = 0;
    pdev = (_tagDevComm *)(buf + offset);
    offset += sizeof(_tagDevComm);
    memcpy(&switch_type, buf + offset, sizeof(switch_type));
    offset += sizeof(int);
    memcpy(&switch_port, buf + offset, sizeof(switch_port));
    offset += sizeof(int);
    if(switch_type == OLP_SWITCH_AUTO)
    {
        str = tr("保护切换到端口:");
    }
    else if(switch_type == OLP_SWITCH_ACTION_PERSION)
    {
        str = tr("人工切换到端口:");
    }
    else if(switch_type == OLP_SWITCH_ACTION_BACK)
    {
        str = tr("保护返回到端口:");
    }
    else
        goto usr_exit;
    if(switch_port == 1 || switch_port == 2)
    {
        str1.setNum(switch_port);
    }
    else
        goto usr_exit;

    bzero(msg, sizeof(msg));
    str  = str + str1;
    pbuf = str.toLocal8Bit();
    strcpy(msg, pbuf);
    pMainWindow->input_gsm_queue(ALARM_LEVE_1, ALARM_OLP_SWITCH,NULL,msg);

usr_exit:
    return 0;
}
/*
   **************************************************************************************
 *  函数名称：input_sms_queue_opmAlarm_chang
 *  函数描述：重发变化的光功率告警次数超时，通过sms发送
 *  入口参数：
 *  返回参数：
 *  作者       ：
 *  日期       ：
 **************************************************************************************
*/
int tsk_SockRetrySend::input_sms_queue_opmAlarm_chang(char buf[])
{
    //告警类型，从机号，临时变量
    int alarm_type, frame_no,i;
    int card_no, alarm_num, changed_num;
    int offset, dev_type;
    int res_code;
    int frame_state ,card_state;
    QString str;
    char option[48];
    QByteArray byteArray;
    _tagOpticalAlarm *pOpticalAlarm; //指向光功率告警位
    _tagDevComm alarmDev;           //告警的设备


    char msg[SHORT_MSG_LEN];
    memset(msg, 0, sizeof(char));
    res_code = RET_SUCCESS;
    offset = 0;

    //告警类型
    memcpy(&alarm_type, buf + offset, sizeof(int));
    offset += sizeof(int);
    //机框，槽位编号
    memcpy(&frame_no, buf + offset, sizeof(int));
    offset += sizeof(int);
    memcpy(&card_no, buf + offset, sizeof(int));
    offset += sizeof(int);
    //告警数目
    memcpy(&alarm_num, buf + offset, sizeof(int));
    offset += sizeof(int);
    memcpy(&changed_num, buf + offset, sizeof(int));
    offset += sizeof(int);

    //检查机框号，槽位号是否正确
    pMainWindow->checkFrameCardNo(frame_no, card_no, frame_state, card_state);
    if((frame_state +  card_state )!= RET_SUCCESS)
    {
        if(frame_state != FRAME_CARD_NO_OK)
            res_code =RET_UNMATCH_FRAME;
        else
            res_code = RET_UNMATCH_SLOT;
        printf( " soceket retray opm alarm frame or card  error frame %d card %d ", frame_no, card_no);
        goto usr_exit;
    }

    dev_type = pMainWindow->m_subrackCard[frame_no].type[card_no];
    res_code = pMainWindow->check_dev_type(frame_no, card_no, dev_type);
    if(res_code == RET_SUCCESS)
    {
        for(i = 0; i < changed_num; i++)
        {
            pOpticalAlarm = (_tagOpticalAlarm *)(buf + offset);
            offset += sizeof(_tagOpticalAlarm);
            alarmDev.port = pOpticalAlarm->port;
            res_code = pMainWindow->check_dev_port_no(frame_no, card_no,pOpticalAlarm->port);
            printf("soceket retray opm changed alarm frame %d card%d port %d",\
                   frame_no, card_no,pOpticalAlarm->port);
            if(res_code == RET_SUCCESS)
            {

                bzero(option, sizeof(option));
                str.setNum(pOpticalAlarm->curPower);
                str  =  tr("当前光功率值:") + str;
                byteArray = str.toLocal8Bit();
                strcpy(option, byteArray.data());
                pMainWindow->input_gsm_queue(pOpticalAlarm->level, alarm_type, NULL, msg);

            }
            else
            {
                printf( " soceket retray  changed opm alarm port error port %d !", pOpticalAlarm->port);
                break;
            }
        }
    }
    else
    {
        printf("soceket retray  opm changed alrarm check type error  frame %d card %d", frame_no, card_no);

    }
usr_exit:
    return res_code;
}
