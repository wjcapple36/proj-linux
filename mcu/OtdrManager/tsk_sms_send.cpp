#include "tsk_sms_send.h"
#include <math.h>
#include "protocol/tmsxx.h"
#include "program_run_log.h"
/****************************************************************************************

//如果要接收短信，使用信号与槽的模式来处理短消息，目前只发送，不读取短消息

*******************************************************************************************/
//构造函数
tsk_SMS_Send::tsk_SMS_Send(QObject *parent) :
    QThread(parent)
{
    stopped = false;
    pMainWindow = (MainWindow *)parent;
    pSerialPort = new CSerialPort ::CSerialPort(this);
    //打开串口
    // /dev/ttySAC3
    if(pSerialPort != NULL && pSerialPort->open_port("/dev/ttySAC3", 9600) == RET_SUCCESS)
        device_state  = 1;
    else
        device_state = 0;
    //设置固定发送参数
    bzero(&smsPara,  sizeof(smsPara)); //短消息中心号码设置为0
    smsPara.TP_DCS = GSM_UCS2; //Unicode 中文编码
    smsPara.TP_PID = 0; //协议标示符号
    qDebug("open com %d", device_state);
    SmsCtrl.reset_eq = 0;
    SmsCtrl.sucess_num = 0;
    SmsCtrl.error_num = 0;
    long_msg_serri = 0;
    objSynSem.commu_stat = SMS_STAT_INITIAL;
}
//析构函数
tsk_SMS_Send::~tsk_SMS_Send()
{
    if(pSerialPort != NULL)
        delete pSerialPort;
}
/*
   **************************************************************************************
 *  函数名称：run
 *  函数描述：短信猫首先初始化，如果初始化10次仍不成功，那么认为短信猫
 *                ：故障，继续初始化。如果成功，跳出，开始发短信
 *  入口参数：
 *  返回参数：
 *  作者       ：
 *  日期       ：2015-11-20
 *  修改日期：
 *  修改内容：
 *                ：
 **************************************************************************************
*/
extern unsigned short wcs2utf16(wchar_t *wcs, unsigned short * utf16, int len);
extern wchar_t  utf162wcs(unsigned short * utf16,wchar_t *wcs,int len);
void tsk_SMS_Send::run()
{    
    unsigned long  wait_time;
    wait_time = 3000000;
    _tagGsmContext  msg_text;
    //char msg[] = "你好阿，abc,123！";
    char msg[] = "鬼探头，专指对道路法规安全意识单薄的非机动车突然横穿马路造成驾驶员反应不及导致的交通意外，这种情况因事发前毫无预兆，所以往往致使驾驶员来不及反应，造成较大损失。那么如何避免这种现象的发生呢？今天我们就来简单聊一聊";
    wchar_t wmsg [512];
    unsigned short utf_16[512];
    int count, retv, initial_num;
    char log_msg[NUM_CHAR_LOG_MSG] = {0};

    mbstowcs(wmsg,msg, strlen(msg));
    count = wcslen(wmsg);
    wcs2utf16(wmsg, utf_16, count);

    msg_text.count = count;
    bzero(msg_text.phone, sizeof(msg_text.phone));
    strcpy(msg_text.phone,"18907832553");
    bzero(msg_text.context, sizeof(msg_text.context));
    memcpy(msg_text.context, utf_16, count*2);

    bzero(smsPara.TPA, sizeof(smsPara.TPA));
    strcpy(smsPara.TPA, msg_text.phone);
    objSynSem.commu_stat = SMS_STAT_INITIAL;
    usr_delay(5);
    GSMQueue.objMutex.lock();
//    GSMQueue.xqueue.enqueue(msg_text);
    GSMQueue.objMutex.unlock();
    initial_num = 0;
    //2016-05-25 输出线程号，与htop中的线程号对应
    printf("%s : Line : %d  thread id %ld \n",  __FILE__, __LINE__,(long int)syscall(224));
    while (!stopped)
    {
        //自检部分
        //self_test:
        while ((objSynSem.commu_stat != SMS_STAT_OK) &&!stopped)
        {
            retv = sms_eq_self_test();
            if(retv == RET_SUCCESS)
            {                
                objSynSem.commu_stat = SMS_STAT_OK;
                //2016-04-21 新增日志功能
                snprintf(log_msg, NUM_CHAR_LOG_MSG, " %s", "sms module initial ok!");
                LOGW(__FUNCTION__,__LINE__, LOG_LEV_FATAL_ERRO,log_msg);
                break;
            }
            else
            {
                initial_num++;
                if(initial_num >= 10)
                {
                    objSynSem.commu_stat = SMS_STAT_ERROR;
                    //2016-04-21 新增日志功能
                    if(initial_num == 10)
                    {
                        snprintf(log_msg, NUM_CHAR_LOG_MSG, " %s", "sms module initial failed !");
                        LOGW(__FUNCTION__,__LINE__, LOG_LEV_FATAL_ERRO,log_msg);
                    }
                }
            }
            usleep(100);
        }
        //循环等待消息
        while(true&&!stopped)
        {
            if(!GSMQueue.xqueue.isEmpty())
            {
                GSMQueue.objMutex.lock();
                msg_text = GSMQueue.xqueue.dequeue();
                GSMQueue.objMutex.unlock();
                if(msg_text.context != NULL&& msg_text.count > 0 &&strlen(msg_text.phone) > 1)
                {
                    //电话前面加86
                    bzero(smsPara.TPA, sizeof(smsPara.TPA));
                    strcpy(smsPara.TPA, "86");
                    strcpy(smsPara.TPA + 2, msg_text.phone);
                    printf("gsm queue len %d \n", GSMQueue.xqueue.size());
                    printf("msg count %d phone %s\n", msg_text.count, smsPara.TPA);
                    break;
                }
            }
            //延迟500ms
            usr_delay(500);
        }
        if(stopped)
            goto usr_exit;

        //开始发送
        retv = send_gsm_msg(&msg_text);
        if(retv != RET_SUCCESS)
        {
            retv = sms_retry_text(msg_text);
            SmsCtrl.error_num++;
        }
        else
        {
            initial_num = 0;
            SmsCtrl.sucess_num++;
        }
        //如果是源地址不是MCU，那么需要回应
        if(msg_text.usr_addr.src != ADDR_MCU)
        {
            res_nm_send_result(&msg_text, retv);
        }

        if(retv != RET_SUCCESS)
        {
            objSynSem.commu_stat = SMS_STAT_INITIAL;
            printf("send sms error, goto self teset ! %d \n", retv);
            SmsCtrl.error_num++;
            //goto self_test;
        }
        else
        {
            SmsCtrl.sucess_num++;
        }

    }
usr_exit:
    stopped = false;
}
/*
   **************************************************************************************
 *  函数名称：res_nm_send_result
 *  函数描述：通知网管发送结果
 *                ：
 *  入口参数：
 *  返回参数：
 *  作者       ：
 *  日期       ：2015-11-16
 *  修改日期：
 *  修改内容：
 *                ：
 **************************************************************************************
*/
int tsk_SMS_Send::res_nm_send_result(_tagGsmContext * pText,int retv)
{
    int fd;
    unsigned int  state;
    glink_addr addr;
    addr.src = ADDR_MCU;
    addr.dst = pText->usr_addr.src;
    addr.pkid = pText->usr_addr.pkid;
    if(retv != RET_SUCCESS)
        state = 1;
    else
        state = 0;
    fd = tms_SelectFdByAddr(&pText->usr_addr.src);
    if(fd > 0)
        tms_RetSMSError(fd, &addr,pText->phone,state);
    printf("tsk sms res NM, fd %d src %d dst %d state %d\n", fd, addr.src, addr.dst, state);
    return 0;
}

/*
   **************************************************************************************
 *  函数名称：sms_retry_text
 *  函数描述：重发达到一定次数，就返回，总软件复位设备，总会伴随着第一条
 *  函数描述：短信发送不成功，体现总设置短信长度失败
 *  入口参数：
 *  返回参数：
 *  作者       ：
 *  日期       ：
 **************************************************************************************
*/
int tsk_SMS_Send::sms_retry_text(_tagGsmContext text)
{
    int count, retv;
    count = 0;
    while(count <  SMS_RETRY_NUM)
    {
        retv = send_gsm_msg(&text);
        if(retv != RET_SUCCESS)
        {
            SmsCtrl.error_num++;
        }
        else
        {
            SmsCtrl.sucess_num++;
            break;
        }
        count++;
    }

    return retv;
}

/*
   **************************************************************************************
 *  函数名称：sms_eq_self_test
 *  函数描述：短信猫自检。注意：并不是所有的短信猫启动时会主动上报systart
 *                ：
 *  入口参数：
 *  返回参数：
 *  作者       ：
 *  日期       ：2015-11-14
 *  修改日期：
 *  修改内容：
 *                ：
 **************************************************************************************
*/
int tsk_SMS_Send::sms_eq_self_test()
{
    int retv;

    retv = sms_eq_power_reset();
    if (retv != RET_SUCCESS)
        goto usr_exit;

    retv = sms_eq_initial();//发送AT指令
    if (retv != RET_SUCCESS)
        goto usr_exit;

    retv = set_sms_mod_echo_close();  //关闭回显
    if (retv != RET_SUCCESS)
        goto usr_exit;

    retv = set_sms_mod();//设置pdu模式
    if(retv != RET_SUCCESS)
        goto usr_exit;

    printf("self test ok \n");

usr_exit:
    return retv;

}

void tsk_SMS_Send::stop()
{
    stopped = true;
    if(pSerialPort != NULL)
        pSerialPort->stop_port();
}
/*
   **************************************************************************************
 *  函数名称：send_gsm_msg
 *  函数描述：发送gs短消息
 *  入口参数：
 *  返回参数：
 *  作者       ：
 *  日期       ：
 **************************************************************************************
*/
int tsk_SMS_Send::send_gsm_msg(_tagGsmContext *pmsg)
{    
    int res_code;
    res_code = -1;

    if(pmsg->count <= 70)
        res_code = send_short_sms(pmsg);
    else
        res_code = send_long_sms(pmsg);

usr_exit:
    //qDebug("send short sms res_code %d", res_code);
    return res_code;
}
/*
   **************************************************************************************
 *  函数名称：send_short_sms
 *  函数描述：发送短消息，字符数小于70
 *  入口参数：
 *  返回参数：
 *  作者       ：
 *  日期       ：
 **************************************************************************************
*/
int tsk_SMS_Send::send_short_sms(_tagGsmContext *pmsg)
{
    char context[48];
    char pdu_packet[512];
    char re_context[48];
    int offset, res_code;
    int total_bytes;

    //设置packet 帧头
    offset = set_pdu_packet_head(&smsPara,pdu_packet);
    offset += set_pdu_packet_len(pdu_packet,pmsg->count*2, offset);
    //填充短信短信内容
    offset += filled_pdu_packet_context(pmsg->context, pdu_packet, offset, pmsg->count);

    total_bytes = get_pdu_packet_cmd_len(pdu_packet);

    bzero(re_context, sizeof(context));
    sprintf(re_context, "\r",total_bytes);
    bzero(context, sizeof(context));
    sprintf(context, "AT+CMGS=%d\r",total_bytes);


    //首先发送短信长度
    res_code = send_sms_context(context, re_context,strlen(context));
    if(res_code != RET_SUCCESS)
        goto usr_exit;

    bzero(re_context, sizeof(context));
    sprintf(re_context, "+CMGF=");
    strcat(pdu_packet, "\x01a");
    res_code = send_sms_context(pdu_packet, re_context,strlen(pdu_packet),30000);
usr_exit:
    qDebug("send short msg %d  %s", res_code, re_context);
    return res_code;
}
/*
   **************************************************************************************
 *  函数名称：send_long_sms
 *  函数描述：发送短消息，字符数小于70
 *  入口参数：
 *  返回参数：
 *  作者       ：
 *  日期       ：
 **************************************************************************************
*/
int tsk_SMS_Send::send_long_sms(_tagGsmContext *pmsg)
{
    char context[48];
    char pdu_packet[512];
    char re_context[48];
    int offset, res_code;
    int total_bytes,head_offset;
    unsigned char total_sec;
    int index,i, cur_sec_utf_16;
    double split_len = SMS_SPLIT_LEN;
    long_msg_serri++;
    total_sec = ceil(pmsg->count / split_len);

    //设置packet 帧头,长短信
    bzero(pdu_packet, sizeof(pdu_packet));
    head_offset = set_pdu_packet_head(&smsPara,pdu_packet,1);
    index = 0;
    printf("long sms count %d total sec %d \n", pmsg->count,total_sec);
    for( i = 0; i < total_sec;i++)
    {
        offset = head_offset;
        memset(pdu_packet + offset, 0, sizeof(pdu_packet) - offset);
        //前面n-1条长度为SMS_SPLIT_LEN，最后一条要特殊计算
        if(i < (total_sec - 1))
            cur_sec_utf_16 = SMS_SPLIT_LEN;
        else
            cur_sec_utf_16 = pmsg->count - (total_sec - 1)*SMS_SPLIT_LEN;
        index = i*SMS_SPLIT_LEN;
        offset +=set_pdu_packet_len(pdu_packet, cur_sec_utf_16*2 + 6, offset);
        offset += set_long_msg_info(pdu_packet,total_sec,(unsigned char)(i + 1),long_msg_serri,offset);

        //填充短信短信内容
        offset += filled_pdu_packet_context(pmsg->context + index, pdu_packet, offset,cur_sec_utf_16);

        total_bytes = get_pdu_packet_cmd_len(pdu_packet);
        printf("long sms total sec %d cur sec %d total utf16 %d  cur utf16 %d \n",\
               total_sec, i + 1,pmsg->count,cur_sec_utf_16);

        bzero(re_context, sizeof(context));
        sprintf(re_context, "\r",total_bytes);
        bzero(context, sizeof(context));
        sprintf(context, "AT+CMGS=%d\r",total_bytes);

        //首先发送短信长度
        res_code = send_sms_context(context, re_context,strlen(context));
        if(res_code != RET_SUCCESS)
            goto usr_exit;

        bzero(re_context, sizeof(context));
        sprintf(re_context, "+CMGF=");
        strcat(pdu_packet, "\x01a");
        res_code = send_sms_context(pdu_packet, re_context,strlen(pdu_packet),30000);
        if(res_code != RET_SUCCESS)
            goto usr_exit;
    }
usr_exit:
    qDebug("send long msg %d  %s", res_code, re_context);
    return res_code;
}
/*
   **************************************************************************************
 *  函数名称：proc_com_data
 *  函数描述：处理从串口收到的数据
 *  入口参数：
 *  返回参数：
 *  作者       ：
 *  日期       ：
 **************************************************************************************
*/
int tsk_SMS_Send::proc_com_data(char RcvBuf[], int rcv_bytes)
{    

    qDebug("proc_com_data: bytes %d rcv %s ", rcv_bytes ,RcvBuf);
    qDebug("objSynSem.rcv_res_data %d ", objSynSem.rcv_res_data);
    if(objSynSem.rcv_res_data == 1 ) //正常的接收发送状态
    {
        objSynSem.rcv_res_data = -1;
        objSynSem.objMutex.lock();

        if(strstr(RcvBuf, "OK") != NULL)
        {
            objSynSem.retval = 0; //成功
        }
        else if(strstr(RcvBuf, "ERROR") != NULL)
        {
            objSynSem.retval = 1; //失败
        }
        else if(strstr(RcvBuf, ">") != NULL)
        {
            objSynSem.retval = 0; //设置短信发送长度
        }
        else if(strstr(RcvBuf, "SYSSTART") != NULL) //系统重启
        {
            objSynSem.retval = 0;
        }
        else
        {
            objSynSem.retval = 3;
        }

        objSynSem.objSem.release();
        objSynSem.objMutex.unlock();
    }
    //PrintfMemory((uint8_t *)RcvBuf,rcv_bytes);
    return 0;
}
//初始化同步变量信息
int tsk_SMS_Send::initial_sync_sem(char context[])
{
    int count;
    objSynSem.objMutex.lock();
    //信号量释放掉
    count = objSynSem.objSem.available();
    if(count > 0)
        objSynSem.objSem.acquire(count);
    //希望获取到的内容
    strcpy(objSynSem.hope_str, context);
    //接收数据不处理
    objSynSem.rcv_res_data = -1;
    objSynSem.objMutex.unlock();
    return count;
}

/*
   **************************************************************************************
 *  函数名称：initial_sms_modul
 *  函数描述：设在短信发送模式
 *  入口参数：短信内容，ascii，期望收到的字符串，发送的字节数，等待时间
 *  返回参数：处理结果，0代表成功，其他值，失败
 *  作者       ：
 *  日期       ：
 **************************************************************************************
*/
int tsk_SMS_Send::send_sms_context(char context[], char res_context[], int bytes,int wait_time)
{    
    int res_code, send_bytes;

    res_code = -1;
    //初始化等待回应信号量
    initial_sync_sem(res_context);
    //发送数据
    qDebug("\n\n\nsend context: size %d   %s", strlen(context), context);
    send_bytes = pSerialPort->write_port(context, bytes);
    if(send_bytes == bytes)
    {
        objSynSem.rcv_res_data = 1;
        res_code = get_signal(wait_time);
        if(res_code != RET_SUCCESS )
        {
            printf("sms ask respones error %d \n", res_code);
        }

    }
    else
    {
        printf("actual send bytes %d strlen %d ", send_bytes, strlen(context));
    }
    return res_code;
}
/*
   **************************************************************************************
 *  函数名称：set_sms_mod_echo
 *  函数描述：设置短信猫返回发送内容，不建议开启
 *  入口参数：
 *  返回参数：
 *  作者       ：
 *  日期       ：
 **************************************************************************************
*/
int tsk_SMS_Send::set_sms_mod_echo()
{
    char context[48];
    char re_context[48];
    int res_code;
    res_code = -1;
    //设置短信模式 pdu
    bzero(context, sizeof(context));
    strcpy(context, "ATE1\r");
    //回应内容
    bzero(re_context, sizeof(context));
    strcpy(re_context, "ATE1");
    res_code = send_sms_context(context, re_context, strlen(context));
    return res_code;
}
/*
   **************************************************************************************
 *  函数名称：set_sms_mod_echo_close
 *  函数描述：关闭短信猫回显
 *  入口参数：
 *  返回参数：
 *  作者       ：
 *  日期       ：
 **************************************************************************************
*/
int tsk_SMS_Send::set_sms_mod_echo_close()
{
    char context[48];
    char re_context[48];
    int res_code;
    res_code = -1;
    //设置短信模式 pdu
    bzero(context, sizeof(context));
    strcpy(context, "ATE0\r");
    //回应内容
    bzero(re_context, sizeof(context));
    strcpy(re_context, "ATE0");
    res_code = send_sms_context(context, re_context, strlen(context));

    return res_code;
}

/*
   **************************************************************************************
 *  函数名称：sms_eq_initial
 *  函数描述：设备自检，发送自检指令
 *  入口参数：
 *  返回参数：
 *  作者       ：
 *  日期       ：
 **************************************************************************************
*/
int tsk_SMS_Send::sms_eq_initial()
{
    char context[48];
    char re_context[48];
    int res_code;
    res_code = -1;
    //设置短信模式 pdu
    bzero(context, sizeof(context));
    strcpy(context, "AT\r");
    //回应内容
    bzero(re_context, sizeof(context));
    strcpy(re_context, "OK");
    res_code = send_sms_context(context, re_context, strlen(context));
    return res_code;
}
/*
   **************************************************************************************
 *  函数名称：set__sms_pdu_mod
 *  函数描述：设置工作总pdu模式
 *  入口参数：
 *  返回参数：
 *  作者       ：
 *  日期       ：
 **************************************************************************************
*/
int tsk_SMS_Send::set_sms_mod(int mod)
{
    char context[48];
    char re_context[48];
    int res_code, count;
    res_code = -1;

    //设置短信模式 pdu
    bzero(context, sizeof(context));
    strcpy(context, "AT+CMGF=0\r");
    //回应内容
    bzero(re_context, sizeof(context));
    strcpy(re_context, "OK");
    res_code = send_sms_context(context, re_context, strlen(context));

    //重启之后，设置字节长度回出错一般出错四次
    if(res_code != RET_SUCCESS)
        goto usr_exit;
    //测试
    bzero(context, sizeof(context));
    sprintf(context, "AT+CMGS=%d\r",36);

usr_exit:
    return res_code;
}
/*
   **************************************************************************************
 *  函数名称：sms_eq_power_reset
 *  函数描述：设备硬重启,断电之后，必须软件重启方可有效
 *  入口参数：
 *  返回参数：
 *  作者       ：
 *  日期       ：2015-11-13
 **************************************************************************************
*/
int tsk_SMS_Send::sms_eq_power_reset()
{
    int res_code,ioval;
    //断电
    ioval = -1;
    pMainWindow->set_io_val(PIN_SMS_HARD_RESET,0);
    usr_delay(1500);
    res_code = pMainWindow->get_io_val(PIN_SMS_HARD_RESET,ioval);
    pMainWindow->set_io_val(PIN_SMS_HARD_RESET,1);
    printf("sms power down , ioval %d \n",ioval);
    res_code = sms_eq_soft_restart();
    return res_code;
}
/*
   **************************************************************************************
 *  函数名称：sms_eq_soft_restart
 *  函数描述：直接控制独立的短信猫模块，不能用IO的方式，控制MCU上面
 *                ：的短信猫可以使用IO口的方式
 *  入口参数：
 *  返回参数：
 *  作者       ：
 *  日期       ：2015-11-13
 *  修改日期：
 *  修改内容：
 *                ：
 **************************************************************************************
*/
int tsk_SMS_Send::sms_eq_soft_restart()
{
    char context[48];
    char re_context[48];
    int res_code;
    res_code = -1;
    bzero(context, sizeof(context));
    strcpy(context, "AT+CFUN=1,1\r");
    //回应内容
    bzero(re_context, sizeof(context));
    strcpy(re_context, "SYSSTART");
    //软件置位----适用独立的短信猫模块
    /*
    res_code = send_sms_context(context, re_context, strlen(context));
    if(res_code != RET_SUCCESS)
        goto usr_exit;
    */
    //软重启-----适合MCU集成短信猫
    pMainWindow->set_io_val(PIN_SMS_SOFT_RESET,0);
    usr_delay(150);
    pMainWindow->set_io_val(PIN_SMS_SOFT_RESET,1);

    //等待重启
    printf("Wait sms eq reboot !\n");
    /*
    initial_sync_sem(re_context);
    objSynSem.rcv_res_data = 1;
    res_code = get_signal(SMS_REBOOT_TIME_MS);
    printf("Sms eq reboot %d! \n", res_code);
    */
    //sleep 5s，确保重启后下面完成初始化
    usr_delay(15000);
    res_code = 0;
usr_exit:
    return res_code;
}

/*
   **************************************************************************************
 *  函数名称：wait_response
 *  函数描述：等待处理结果，如果阻塞太时间太长，程序退出是，线程无法
 *  函数描述：释放，因此需要分段等待
 *  入口参数：
 *  返回参数：
 *  作者       ：
 *  日期       ：
 **************************************************************************************
*/
int tsk_SMS_Send::get_signal(int wait_time_ms)
{
    int count, wait_space;
    int retv;
    retv = RET_SEND_CMMD_TIMEOUT;
    bool result;
    wait_space = 200;
    result = false;
    count = 0;
    while(count < wait_time_ms && !stopped)
    {
        result = objSynSem.objSem.tryAcquire(1, wait_space);
        if(result)
        {
            retv = objSynSem.retval;
            break;
        }
        else
        {
            count += wait_space;
        }

    }

    return retv;
}
/*
   **************************************************************************************
 *  函数名称：usr_delay
 *  函数描述：延迟函数，线程一次sleep太久，退出是回异常
 *  入口参数：延迟时间
 *  返回参数：
 *  作者       ：
 *  日期       ：
 **************************************************************************************
*/
int tsk_SMS_Send::usr_delay(int delay_time_ms)
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
