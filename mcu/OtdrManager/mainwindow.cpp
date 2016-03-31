#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>
#include <QPalette>
#include <QIcon>
#include <QPixmap>
#include <QString>
#include <QDateTime>
#include <QMessageBox>
#include <QUuid>

#include <stdio.h>
#include <ifaddrs.h>
#include <arpa/inet.h>
#include "dlgotdr.h"
#include "dlgmcu.h"
#include "math.h"
#include "dlggsm.h"
#include "dlgolp.h"
#include "dlgosw.h"
#include "dlgopm.h"
#include "protocol/tmsxx.h"
#include "src/tmsxxdb.h"
#include "string.h"

//修改成全局的，方便通信类调用

MainWindow *pmain_window = NULL;
extern struct ep_t ep;
/*
   **************************************************************************************
 *  函数名称：process_data_from_sock
 *  函数描述：提供给通信线程调用的接口函数，收到数据之后，放入缓冲区
 *  函数描述：如果收到曲线，或者升级文件，那么不使用环形缓存
 *  函数描述：本函数要防止重入
 *  入口参数：buf, dataLen,最长的等待时间，毫秒
 *  返回参数：
 *  日期版本：
 *  作者       ：
 **************************************************************************************
*/
int  process_data_from_sock(char buf[], int dataLen, int msec, void *ptr_option)
{
    //如果mainwindow还没有建立就返回
    int returnValue;
    returnValue = -1;
    if (NULL == pmain_window)
        goto usr_exit;
    /*
     *2016-02-03 如果准备好开始接收数据，那么就接收
    */
    if(1 == pmain_window->m_ctrlStat.will_reboot ||\
            0 == pmain_window->m_ctrlStat.rcvSocData)
        goto usr_exit;
    //防止重入
    pmain_window->mutexRcvData.lock();
    returnValue = pmain_window->local_process_cmd(buf,  dataLen,  msec,ptr_option);
    pmain_window->mutexRcvData.unlock();

usr_exit:    return returnValue;
}
/*
   **************************************************************************************
 *  函数名称：dlg_send,全局函数，因为所有对话框都要调用，所以定义成全局
 *  函数描述：调用此函数回阻塞，知道结果返回，超时，或者返回处理结果
 *  入口参数：
 *  返回参数：
 *  作者       ：
 *  日期       ：
 **************************************************************************************
*/
int dlg_send(char devbuf[] , char buf[], unsigned int  cmd, unsigned int res_cmd, _tagBufList **pBufHead, int ack_to, int sem_index)
{
    //发送地址
    glink_addr send_addr;
    tms_devbase *pdevbase;

    int return_val, res_code;
    bool result;
    char short_msg[SHORT_MSG_LEN];

    if(pmain_window == NULL)
        return -1;
    //加锁
    pmain_window->objDlgSend.lock();
    result = false;
    res_code = -1;
    pdevbase =  (tms_devbase *)devbuf;
    send_addr.dst  = ADDR_CARD;
    send_addr.src = ADDR_MCU;
    send_addr.pkid = pmain_window->creat_pkid();
    //通过机框，槽位获取fd
    tms_GetDevBaseByLocation(pdevbase->frame, pdevbase->slot,pdevbase);
    if(pdevbase->fd > 0)
    {
        //设置要等待的回应码
        pmain_window->objSyncSem[sem_index].resCmd = res_cmd;
        pmain_window->objSyncSem[sem_index].sendCmd = cmd;
        pmain_window->objSyncSem[sem_index].pkid = send_addr.pkid;
        return_val = send_cmd(pdevbase->fd, buf,cmd);
        //发送成功，等待回应
        if(return_val >= 0)
        {
            result = pmain_window->objSyncSem[sem_index].objSem.tryAcquire(1,DATA_RETURN_WAIT_TIME);
            if(result)
            {
                res_code = pmain_window->objSyncSem[sem_index].resCode;
            }
            else
            {
                //等待命令回应超时
                res_code = RET_SEND_CMMD_TIMEOUT;
                sprintf(short_msg, "0x%x wait res cmmd time out!", cmd);
            }
        }
        else
        {
            //通信异常
            sprintf(short_msg, "0x%x send cmmd faile !", cmd);
            res_code = RET_COMMU_ABORT;
        }
    }
    else
    {
        //fd error
        res_code = RET_COMMU_ABORT;
        sprintf(short_msg, "0x%x  error!", cmd);
    }
    pmain_window->objSyncSem[sem_index].resCmd = -1;
    pmain_window->objSyncSem[sem_index].sendCmd = -1;
    //处理结果
    if(res_code != RET_SUCCESS)
    {
        if(ack_to == ACK_TOT_DLG)
        {
            pmain_window->dealAbnormalResCod(res_code);
        }
        //向网管汇报
        else if(ack_to == ACK_TO_NM)
        {
            //获取网管地址
            send_addr.dst  = ADDR_HOST_VIP;
            pdevbase->fd = tms_SelectFdByAddr(&send_addr.dst);
            tms_Ack(pdevbase->fd,&send_addr,res_code,res_cmd);
        }
        qDebug("%s",short_msg );
    }
    else
    {
        *pBufHead = pmain_window->objSyncSem[sem_index].pBufListHead;
    }
    pmain_window->objSyncSem[sem_index].resCmd = -1;
    //解锁
    pmain_window->objDlgSend.unlock();
    return res_code;
}
/*
   **************************************************************************************
 *  函数名称：send_cmd
 *  函数描述：将选择命令发送出去
 *  入口参数：fd,数据信息，opt，存放地址信息 glink_addr
 *  返回参数：发送结果
 *  作者       ：
 *  日期       ：
 **************************************************************************************
*/
int send_cmd(int fd, char buf[], int cmd, char *opt)
{
    glink_addr dst_addr;
    _tagDlgAttribute *pdev_type;
    int return_val;
    return_val = -1;
    if (pmain_window == NULL)
        goto usr_exit;
    return_val = RET_SUCCESS;
    //兼容以前的版本
    if(opt ==  NULL)
    {
        dst_addr.dst = ADDR_CARD;
        dst_addr.src = ADDR_MCU;
        dst_addr.pkid = pmain_window->creat_pkid();
    }
    else
    {
        memcpy(&dst_addr,opt, sizeof(glink_addr));
    }
    //开始发送
    switch (cmd)
    {

    case ID_GET_VERSION: //获取软件版本
    {

        pdev_type = (_tagDlgAttribute *)buf;
        return_val = tms_GetVersion(fd,&dst_addr,pdev_type->frameNo,pdev_type->cardNo,\
                                    pdev_type->devType);
        break;
    }
    case ID_GET_OPM_OP://获取光功率
    {
        pdev_type = (_tagDlgAttribute *)buf;
        return_val = tms_GetOPMOP ( fd, &dst_addr, pdev_type->frameNo,pdev_type->cardNo);
        break;
    }
    case ID_GET_DEV_PRODUCE : //获取硬件信息
    {
        pdev_type = (_tagDlgAttribute *)buf;
        return_val = tms_GetDevProduce ( fd, &dst_addr, pdev_type->frameNo,pdev_type->cardNo, pdev_type->devType);
        break;
    }
    case ID_GET_OTDR_PARAM://otdr参数
    {
        pdev_type = (_tagDlgAttribute *)buf;
        return_val = tms_MCU_GetOTDRParam( fd, &dst_addr, pdev_type->frameNo,pdev_type->cardNo);
        break;
    }
    case ID_REPORT_OLP_ACTION: //OLP切换动作
    {
        /*
         *机框
         *槽位
         *类型
         *切换类型
         *切换端口
         */
        int swtype, swport;
        int offset;
        pdev_type = (_tagDlgAttribute *)buf;
        offset = sizeof(_tagDlgAttribute);
        memcpy(&swtype, buf + offset, sizeof(swtype));
        offset += sizeof(swtype);
        memcpy(&swport, buf + offset, sizeof(swport));
        offset += sizeof(swport);

        tms_ReportOLPAction(fd,&dst_addr, pdev_type->frameNo, pdev_type->cardNo,swtype,swport);
    }
    case ID_ALARM_OPM_CHANGE: //变化的光功率告警
    {
        int alarm_count, count;
        int offset;
        tms_alarm_opm_val *list;
        /*
         *告警类型
         *机框
         *槽位
         *告警数目
         *变化的告警数目
         *告警内容-----端口，级别，光功率，时间
         */
        offset = 4; //跳过告警类型
        pdev_type = (_tagDlgAttribute *)(buf + 4); //取机框槽位
        offset += 8;
        memcpy(&alarm_count, buf + offset, sizeof(alarm_count));//总的告警数目
        offset += sizeof(alarm_count);
        memcpy(&count, buf + offset, sizeof(count));//变化的告警数目
        offset += sizeof(alarm_count);
        list = (tms_alarm_opm_val *) (buf + offset);
        return_val = tms_AlarmOPMChange(fd,&dst_addr,pdev_type->frameNo,pdev_type->cardNo,\
                                        alarm_count,count,list);
    }
    case ID_RET_ALARM_HW_CHANGE: //变化的硬件告警
    {
        /*
         *告警类型------int
         *告警数目------int
         *变化的告警数目----int
         *告警内容-----级别，机框，槽位，原因，时间
         */
        int total_num, alarm_num;
        int offset;
        offset = 4; //跳过告警类型
        memcpy(&total_num, buf + offset, sizeof(int));//总的告警数目
        offset += sizeof(int);
        memcpy(&alarm_num, buf + offset, sizeof(int));//变化的告警数目
        offset += sizeof(int);
        tms_RetAlarmHWChange(fd,&dst_addr,total_num,alarm_num,\
                             ( tms_alarm_hw_change_val *)(buf + offset));
    }
    default:
        return_val = -1;
    }
usr_exit:
    return return_val;
}
/*
   **************************************************************************************
 *  函数名称：wcs2utf16
 *  函数描述：将ucs-4 转换成ucs-2
 *  入口参数：ucs-4,ucs-2
 *  返回参数：utf16 buf 长度
 *  作者       ：
 *  日期       ：
 **************************************************************************************
*/
unsigned short wcs2utf16(wchar_t *wcs, unsigned short * utf16, int len)
{
    int wc;
    wchar_t *end = wcs + len;
    unsigned short count;
    count = 0;
    while(wcs < end)
    {
        wc = *(wcs++);
        if (wc > 0xFFFF)
        {
            wc -= 0x0010000UL;
            *utf16++ = 0xD800 | (wc >> 10);
            *utf16++ = 0xDC00 | (wc & 0x03FF);
        }
        else
        {
            *utf16++ = wc;
        }
        count++;
    }
    return count;
}
/*
   **************************************************************************************
 *  函数名称：wcs2utf16
 *  函数描述：将ucs-2转换成ucs-4
 *  入口参数：
 *  返回参数：
 *  作者       ：
 *  日期       ：
 **************************************************************************************
*/
wchar_t  utf162wcs(unsigned short * utf16,wchar_t *wcs,int len)
{
    int utf;
    unsigned short h;
    unsigned short l;
    int count;
    count = 0;
    while(utf = *utf16++)
    {
        if(utf<0xD800||utf>0xDFFF)
        {
            *wcs++ = utf;
        }
        else
        {
            h = utf - 0xD800;
            l = *(utf16++) - 0xDC00;
            *wcs++ =  ((h<<10) | l ) + 0x10000;
        }
        count++;
    }
    return count;
}
//utf16,有多少个字符
int xwcslen(unsigned short *utf16)
{
    int utf;
    int ret = 0;
    while(utf = *utf16++)
        ret += ((utf < 0xD800) || (utf > 0xDFFF)) ? 2 :1;
    return ret;
}
//检查wcs,需要多少utf-16保存wcs中的字符的unicode
int utf16len(wchar_t *wcs)
{
    int wc;
    int ret = 0;
    while(wc = *wcs++)
        ret += wc > 0xFFFF ? 2 : 1;
    return ret;
}
/*
   **************************************************************************************
 *  函数名称：db_read_card_commp
 *  函数描述：从数据库中读取板卡组成信息的回调函数
 *  入口参数：output数据源，ptr 目的地址 len 数据长度
 *  返回参数：-1 代表成功，0，需要继续查找，其他为错误码
 *  作者       ：
 *  日期       ：
 **************************************************************************************
*/
int db_read_card_commp(tdb_common_t *output, void *ptr, int len)
{
    int return_val;
    return_val = -1;
    //检查数据信息
    if(output->val1 != DB_COMM_ID_CARD_COMMP )
        return_val = DB_ID_ERROR;
    else if (len != sizeof(_tagSubrackCard)*NUM_SUBRACK)
        return_val = DB_LEN_ERROR;
    else
        memcpy(ptr, output->pdata, len);
    qDebug( "db_read_card_commp len %d return value %d", len,return_val);
    return return_val;
}
/*
   **************************************************************************************
 *  函数名称：db_read_card_commp
 *  函数描述：从数据库中读取板卡组成信息的回调函数
 *  入口参数：output数据源，ptr 目的地址 len 数据长度
 *  返回参数：-1 代表成功，0，需要继续查找，其他为错误码
 *  作者       ：
 *  日期       ：
 **************************************************************************************
*/
int db_read_opm_alarm_commp(tdb_common_t *output, void *ptr, int len)
{
    int return_val;
    return_val = -1;

    //检查数据信息
    if(output->val1 != DB_COMM_ID_OPM_ALARM )
        return_val = DB_ID_ERROR;
    else if(pmain_window != NULL &&output->val2 < 256) //不认为系统中的opm的个数回超过256
    {
        return_val = pmain_window->get_opm_alarm_from_buf((char *)(output->pdata), len, output->val2);
        printf("%s(): Line : %d OK  ID 0x %x opm / olp num %d \n",  \
               __FUNCTION__, __LINE__,output->val1,output->val2);
    }
    else
    {
        printf("%s(): Line : %d  error ID 0x %x opm / olp num %d \n",  \
               __FUNCTION__, __LINE__,output->val1,output->val2);
    }
    return return_val;
}

/*
   **************************************************************************************
 *  函数名称：db_read_olp_action_commp
 *  函数描述：从数据库中读取OLP切换记录
 *  入口参数：output数据源，ptr 目的地址 len 数据长度
 *  返回参数：-1 代表成功，0，需要继续查找，其他为错误码
 *  作者       ：
 *  日期       ：
 **************************************************************************************
*/
int db_read_olp_action_commp(tdb_common_t *output, void *ptr, int len)
{
    int return_val, res_code;
    int i, offset, type;
    _tagOlpActionRecordCell olpAction;
    _tagOlpActionRecordBuf *pRecordBuf;
    char *buf;
    return_val = -1;
    offset = 0;

    //检查数据信息
    if(output->val1 != DB_COMM_ID_OLP_ACTION )
        return_val = DB_ID_ERROR;
    //切换记录条数必须小于最大条目
    else if(pmain_window!= NULL&&ptr != NULL && output->val2 <= OLP_ACTION_MAX_NUM&&output->val2 >= 0\
            &&output->val3 < OLP_ACTION_MAX_NUM&&output->val3 >=0)
    {
        pRecordBuf = (_tagOlpActionRecordBuf *)ptr;
        pRecordBuf->cur_total_record = output->val2; //数据库中的记录条目数
        pRecordBuf->cur_index = output->val3;   //当前的索引
        offset = 0;
        buf = (char *)output->pdata;
        for(i = 0; i < output->val2;i++)
        {
            memcpy(&olpAction, buf + offset, sizeof(_tagOlpActionRecordCell));
            offset += sizeof(_tagOlpActionRecordCell);
            //检查机框，槽位范围
            res_code = pmain_window->check_frame_card_range(olpAction.frame, olpAction.card);
            if(res_code != RET_SUCCESS)
                continue;
            //检查 机框槽位是否配置过
            //type = pmain_window->m_subrackCard[olpAction.frame].type[olpAction.card];
            res_code = pmain_window->check_dev_type(olpAction.frame, olpAction.card, OLP);
            if(res_code != RET_SUCCESS)
                continue;
            pRecordBuf->list.replace(i,olpAction);
        }
        printf("%s(): Line : %d  OK ID 0x %x log num %d \n",  \
               __FUNCTION__, __LINE__,output->val1,output->val2);
    }
    else
    {
        printf("%s(): Line : %d   error ID 0x %x log num %d \n",  \
               __FUNCTION__, __LINE__,output->val1,output->val2);
    }

    return return_val;
}


/*
   **************************************************************************************
 *  函数名称：db_read_hw_alarm_commp
 *  函数描述：从数据库中读取硬件告警信息
 *  入口参数：output数据源，ptr 目的地址 len 数据长度
 *  返回参数：-1 代表成功，0，需要继续查找，其他为错误码
 *  作者       ：
 *  日期       ：
 **************************************************************************************
*/
int db_read_hw_alarm_commp(tdb_common_t *output, void *ptr, int len)
{
    int return_val;
    int offset;
    char *buf;
    return_val = -1;
    offset = 0;

    //检查数据信息
    if(output->val1 != DB_COMM_ID_HW_ALARM )
        return_val = DB_ID_ERROR;
    //切换记录条数必须小于最大条目
    else if(ptr != NULL&&output->lenpdata == sizeof(pmain_window->DevCommuState))
    {
        buf = (char *)output->pdata;
        memcpy(ptr, buf, output->lenpdata);
        printf("%s(): Line : %d  OK ID 0x %x size %d \n",  \
               __FUNCTION__, __LINE__,output->val1,output->lenpdata);
    }
    else
    {
        printf("%s(): Line : %d  error ID 0x %x size %d \n",  \
               __FUNCTION__, __LINE__,output->val1,output->lenpdata);
    }
    return return_val;
}

/*
   **************************************************************************************
 *  函数名称：read_refotdr(tdb_otdr_ref_t *output, void *ptr)
 *  函数描述：从数据库总读取otdr 参考曲线的回调函数
 *  入口参数：
 *  返回参数：-1 代表成功，0，需要继续查找，其他为错误码
 *  作者       ：
 *  日期       ：
 **************************************************************************************
*/
int db_read_refotdr(tdb_otdr_ref_t *output, void *ptr)
{
    int return_val, res_code;
    _tagReferCurv *prefotdr;
    prefotdr = (_tagReferCurv *)(ptr);
    return_val = -1;
    if(pmain_window != NULL && ptr != NULL)
    {
        res_code = pmain_window->db_check_otdr_ref((char *)(output));
        if(res_code == RET_SUCCESS)
        {
            //测量参数
            memcpy( &prefotdr->measurPara, output->ptest_param,sizeof(_tagMeasurPara));
            //告警门限
            memcpy( &prefotdr->alarmTh, output->pref_data,sizeof(_tagAlarmTh));
            //事件点
            prefotdr->eventNum = output->pevent_hdr->count;
            memcpy(prefotdr->eventBuf, output->pevent_val, sizeof(_tagEvent)*prefotdr->eventNum);
            //测量结果
            memcpy(&prefotdr->measurResult, &output->pchain->range, sizeof(_tagMeasurResult));
            //光开关信息
            memcpy(&prefotdr->ref_head, output->pref_hdr, sizeof(otdr_ref_hdr));
            //数据点
            prefotdr->ptNum = output->pdata_hdr->count;
            memcpy(prefotdr->dataPt, output->pdata_val, sizeof(short int )*prefotdr->ptNum);
        }
        else
        {
            return_val = res_code;
        }

    }
    return return_val;
}
/*
   **************************************************************************************
 *  函数名称： db_read_cyc_curv
 *  函数描述： 从数据库中读取周期性测量曲线
 *  入口参数：
 *  返回参数：-1 代表成功，0，需要继续查找，其他为错误码
 *  日期版本：
 *  作者       ：
 **************************************************************************************
*/
int db_read_curv_cyc(tdb_otdr_his_data_t *db_output, void *ptr)
{
    tdb_otdr_alarm_data_t *pcyc_curv, *output;
    _tagDbReadCurv *pdb_read_curv;
    pdb_read_curv = (_tagDbReadCurv *)ptr;
    output = (tdb_otdr_alarm_data_t *)db_output;
    int return_val, res_code;
    return_val = -1;
    if(pmain_window != NULL && ptr != NULL)
    {
        //osw--otdr data id  data buf event id /buf result
        res_code = pmain_window->db_check_cyc_curv((char *)output);
        if (res_code != RET_SUCCESS)
        {
            return_val = res_code;
            goto usr_exit;
        }
        pcyc_curv = (tdb_otdr_alarm_data_t *)pdb_read_curv->ptr;
        pcyc_curv->id = output->id;
        memcpy(pcyc_curv->ptest_hdr, output->ptest_hdr, sizeof(tms_retotdr_test_hdr));
        memcpy(pcyc_curv->ptest_param, output->ptest_param, sizeof(tms_retotdr_test_param));
        memcpy(pcyc_curv->pdata_hdr, output->pdata_hdr, sizeof(tms_retotdr_data_hdr));
        memcpy(pcyc_curv->pdata_val, output->pdata_val, \
               output->pdata_hdr->count*sizeof(tms_retotdr_data_val));
        memcpy(pcyc_curv->pevent_hdr, output->pevent_hdr, sizeof(tms_retotdr_event_hdr));
        memcpy(pcyc_curv->pevent_val, output->pevent_val,\
               output->pevent_hdr->count*sizeof(tms_retotdr_event_val));
        memcpy(pcyc_curv->pchain, output->pchain, sizeof(tms_retotdr_chain));
    }
usr_exit:
    qDebug("return_val %d ", return_val);
    return return_val;
}
/*
   **************************************************************************************
 *  函数名称： db_read_cyc_curv
 *  函数描述： 从数据库中读取周期性测量曲线
 *  入口参数：
 *  返回参数：-1 代表成功，0，需要继续查找，其他为错误码
 *  日期版本：
 *  作者       ：
 **************************************************************************************
*/
int db_read_curv_alarm(tdb_otdr_alarm_data_t *output, void *ptr)
{
    tdb_otdr_alarm_data_t *pcyc_curv;
    _tagDbReadCurv *pdb_read_curv;
    pdb_read_curv = (_tagDbReadCurv *)ptr;
    int return_val, res_code;
    return_val = -1;
    if(pmain_window != NULL && ptr != NULL)
    {
        //osw--otdr data id  data buf event id /buf result
        res_code = pmain_window->db_check_cyc_curv((char *)output);
        if (res_code != RET_SUCCESS)
        {
            return_val = res_code;
            goto usr_exit;
        }
        pcyc_curv = (tdb_otdr_alarm_data_t *)pdb_read_curv->ptr;
        pcyc_curv->id = output->id;
        memcpy(pcyc_curv->ptest_hdr, output->ptest_hdr, sizeof(tms_retotdr_test_hdr));
        memcpy(pcyc_curv->ptest_param, output->ptest_param, sizeof(tms_retotdr_test_param));
        memcpy(pcyc_curv->pdata_hdr, output->pdata_hdr, sizeof(tms_retotdr_data_hdr));
        memcpy(pcyc_curv->pdata_val, output->pdata_val, \
               output->pdata_hdr->count*sizeof(tms_retotdr_data_val));
        memcpy(pcyc_curv->pevent_hdr, output->pevent_hdr, sizeof(tms_retotdr_event_hdr));
        memcpy(pcyc_curv->pevent_val, output->pevent_val,\
               output->pevent_hdr->count*sizeof(tms_retotdr_event_val));
        memcpy(pcyc_curv->pchain, output->pchain, sizeof(tms_retotdr_chain));
        if(pdb_read_curv->curv_type == CURV_TYPE_ALARM)
        {
            memcpy(pcyc_curv->palarm, output->palarm, sizeof(tms_alarm_line_hdr));
        }
    }
usr_exit:
    qDebug("return_val %d ", return_val);
    return return_val;
}
/*
   **************************************************************************************
 *  函数名称：read_refotdr(tdb_otdr_ref_t *output, void *ptr)
 *  函数描述：读取参考曲线的测量参数
 *  入口参数：
 *  返回参数：返回-1，从数据库中直接返回，返回0，数据库继续查找，其他为错误码
 *  作者       ：
 *  日期       ：
 **************************************************************************************
*/
int db_read_refotdr_test_para(tdb_otdr_ref_t *output, void *ptr)
{
    int return_val, res_code;
    _tagMeasurPara *pTestPara;
    pTestPara = (_tagMeasurPara *)(ptr);
    return_val = -1;
    if(pmain_window != NULL)
    {
        res_code = pmain_window->db_check_otdr_ref((char *)(output));
        if(res_code == RET_SUCCESS)
        {
            memcpy( pTestPara, output->ptest_param,sizeof(_tagMeasurPara));
            /* qDebug("range %d pl %d time %d lamda %d n %f end th %f non %f samphz %d ",
                   output->ptest_param->rang,output->ptest_param->pw,output->ptest_param->time,output->ptest_param->wl\
                   ,output->ptest_param->gi, output->ptest_param->end_threshold,
                   output->ptest_param->none_reflect_threshold,output->ptest_param->sample);
            qDebug("range %d pl %d time %d lamda %d n %f end th %f non %f samphz %d ",
                   pTestPara->range_m,pTestPara->pulseWidth_ns, pTestPara->measureTime_s\
                  ,pTestPara->lamda_ns,pTestPara->n,pTestPara->endThreshold,
                   pTestPara->NonRelectThreshold,pTestPara->samplHz);*/

        }
        else
            return_val = res_code;
        qDebug("db read ref para res_code %d",res_code);
    }
    return return_val;
}
/*
   **************************************************************************************
 *  函数名称：db_read_a_trigger_b
 *  函数描述：读取告警触发的回掉函数,第一次获取记录条目，第二次获具体的记录
 *  入口参数：返回-1，从数据库中直接返回，返回0，数据库继续查找，其他为错误码
 *  返回参数：
 *  作者       ：
 *  日期       ：
 **************************************************************************************
*/
int db_read_a_trigger_b(tdb_a_trigger_b_t *output, void *ptr)
{
    int return_val;
    int i;
    _tagDevComm *input;
    _tagCallbackPara *pCallbackPara;
    pCallbackPara = (_tagCallbackPara *)(ptr);
    return_val = -1;
    if(pmain_window != NULL && ptr != NULL)
    {
        if(pCallbackPara->operate_type == DB_GET_RECORD_NUM)
        {
            return_val = 0;
            pCallbackPara->record_num++;
            pCallbackPara->operate_num++;
        }
        else if(pCallbackPara->operate_type == DB_GET_RECORD)
        {
            return_val = 0;
            i = pCallbackPara->operate_num++;
            if(i < pCallbackPara->record_num)
            {
                input = pCallbackPara->buf;
                memcpy(&input[i], &output->frame_b,sizeof(_tagDevComm));

            }
            else
            {
                return_val = -1;
            }
        }
    }
usr_exit:
    return return_val;
}
/*
   **************************************************************************************
 *  函数名称：db_read_all_rout
 *  函数描述：从数据库里面读取全部的路由信息的回调函数
 *                ：
 *  入口参数：数据库查询到的记录，查询参数结构体指针:条目总数，当前操作次数
 *  返回参数：返回-1，从数据库中直接返回，返回0，数据库继续查找，其他为错误码
 *  作者       ：
 *  日期       ：
 *  修改日期：
 *  修改内容：
 *                ：
 **************************************************************************************
*/
int db_read_all_rout(tdb_route_t *output, void *ptr)
{
    _tagDBCallbackPara *pDBCallbackPara;
    tms_route *prout;
    int alreadyRead, retv;
    retv = 0;
    pDBCallbackPara = (_tagDBCallbackPara *)ptr;
    alreadyRead = pDBCallbackPara->index;

    if(pDBCallbackPara->index < pDBCallbackPara->list_num){
        prout = (tms_route *)(pDBCallbackPara->list_num + sizeof(tms_route)*alreadyRead);
        memcpy(prout, &output->ip_src, sizeof(tms_route));
        pDBCallbackPara->index++;
    }
    else{
        printf("%s(): Line : %d read db overflow  list num %d read num %d \n",  __FUNCTION__, __LINE__,\
               pDBCallbackPara->list_num, pDBCallbackPara->index);
    }


    return retv;

}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    int retv;
    RcvBuf = new char[RING_BUF_SIZE];
    if(NULL != RcvBuf)
        RingBufManager.attachBufManaged(RcvBuf,RING_BUF_SIZE);
    else
    {
        qDebug()<<"RcvBuf alloc failed, reboot";
        sleep(5);
        soft_reboot(REBOOT_TYPE_NORMAL);
    }
    BufList = new _tagBufList[LINKED_BUF_NUM];
    if(NULL != BufList)
        LinkedBufManager.attachBuf(BufList, LINKED_BUF_NUM, BUF_LIST_SIZE);
    else
    {
        qDebug()<<"BufList alloc failed, reboot";
        sleep(5);
        soft_reboot(REBOOT_TYPE_NORMAL);
    }
    //设定告警测量队列的缓冲区
    queAlarmMeasur.xqueue.reserve(QUEUE_LEN_ALARM_MEASUR);
    //点名测量队列，队列长度是1
    queAlarmMeasur.xqueue.reserve(QUEUE_LEN_APPOINT_MEASUR);
    //设定告板卡名字
    m_strlstEqType <<tr("NONE")<<tr("OPM")<<tr("OSW")<<tr("OTDR")\
                  <<tr("OLS")<<tr("OLP")<<tr("GSM");
    m_strlstAllEqType <<tr("NONE")<<tr("PWU")<<tr("MCU")<<tr("OPM")<<tr("OSW")<<tr("OTDR")\
                     <<tr("OLS")<<tr("OLP")<<tr("GSM")<<("TU");
    //初始化设备端口数目，一般的是8～128 otdr,olp比较特殊，单独定义
    strlstDevPortNum <<"8"<<"16"<<"32"<<"64"<<"128";
    strlstOtdrPortNum <<"1";
    strlstOlpPortNum <<"3";
    strSoftVerson = "V16.02.03.12";
    bzero(cfg_file_path, sizeof(cfg_file_path));
#ifdef ARM_BOARD
    snprintf(cfg_file_path, FILE_PATH_LEN,"/etc/dev.cfg");
#else
    snprintf(cfg_file_path, FILE_PATH_LEN,"/home/wjc/src-example/mcu/dev.cfg");
#endif

    memset(&curShowDlgAttr, 0, sizeof(curShowDlgAttr));
    //关联信号，队列形式
    //传递的不是指针，因此需要注册为原类型
    //    qRegisterMetaType<_tagReferCurv>("_tagReferCurv");
    //    qRegisterMetaType<_tagOtdrCurv>("_tagOtdrCurv");
    connect(this, SIGNAL(findAlarm( )), this, \
            SLOT(inspectFiber()),Qt::QueuedConnection);

    //检查周期性测量
    connect(this, SIGNAL(checkCycleTime()), this, \
            SLOT(insepectCycleTime()),Qt::QueuedConnection);
    //更新设备
    connect(this, SIGNAL(updateDevType()), this, \
            SLOT(s_update_dev_type()),Qt::QueuedConnection);
    //设置minDevType
    memset(&m_ctrlStat, 0,sizeof(m_ctrlStat));
    memset(&m_ctrlUpData,0,sizeof(m_ctrlUpData));
    m_ctrlStat.minDevType = DEV_TYPE_MIN; //设备类型范围
    m_ctrlStat.maxDevType  = DEV_TYPE_MAX;//
    m_ctrlStat.NMstat = NM_PRE_LOST;//首先处于预失去连接状态
    memset(&m_otdrCtrl, 0, sizeof(_tagOtdrCtrl));
    //bzero(&dynamicDevtype, sizeof(dynamicDevtype));
    m_ptrCurentDlg =  NULL;
    pkid = 0x1000;

    ui->setupUi(this);
    //设置页面风格
    setWindowsStyle();
    //设在秒表
    timeUpdate();
    //初始化槽位类型
    get_dev_composition_fromdb();
    //InitialSubrackAndCadInfo();
    InitialCardBtn();
    InitialCardType();
    //initialCycleTestTime();
    //获取系统中的otdr gsm 地址
    get_otdr_card_slot(0);
    //check_dev_uuid();
    refresh_card_slot_config();
    read_dev_cfg();

    retv = creat_other_tsk();
    if(retv != RET_SUCCESS)
    {
        qDebug("creat other tsk error, will reboot system");
        sleep(10);
        soft_reboot();
    }
    creat_otdr_tsk();
    /*
     *2016-02-03 读数据库的时候需要用此指针
    */
    pmain_window = this;
    initial_hw_alarm_buf(); //读取硬件告警信息
    initial_olp_switch_record_buf();//读取olp切换记录
    initial_total_opm_alarm();//读取光功告警
    show_db_saved_alarm();
    gpio_init();
    /*
     *2016-02-03 增加一个控制接收数据的变量
    */
    m_ctrlStat.rcvSocData = 1;
}
/*
   **************************************************************************************
 *  函数名称：soft_reboot
 *  函数描述：软件重启整个系统
 *  入口参数：reboot_type,重启类型，0，代表正常重启，其它为遇到问题重启
 *  返回参数：
 *  作者       ：
 *  日期       ：2015-10-29
 *  修改日期：
 *  修改内容：
 **************************************************************************************
*/
void MainWindow::soft_reboot(int reboot_type)
{   
    printf("%s(): Line : %d reboot type %d \n",  __FUNCTION__, __LINE__,reboot_type);
    system("sync");
    sleep(5);
    system("reboot");
}

/*
   **************************************************************************************
 *  函数名称：creat_other_tsk
 *  函数描述：创建其他一些必要任务
 *  入口参数：
 *  返回参数：如果返回非0，系统将不能正常工作
 *  作者       ：
 *  日期       ：2015-10-30
 *  修改日期：
 *  修改内容：
 **************************************************************************************
*/
int MainWindow::creat_other_tsk()
{
    int retv;
    retv = UNDEFINED_ERROR;
    //数据分发
    pDataDispatch = new tsk_dataDispatch ::tsk_dataDispatch(this);
    if(pDataDispatch == NULL)
        goto usr_exit;
    //与host通信相关
    pHostCommu = new tsk_host_commu:: tsk_host_commu(this);
    if(pHostCommu == NULL)
        goto usr_exit;
    //网络数据重发任务
    pSockRetry = new tsk_SockRetrySend::tsk_SockRetrySend(this);
    if(pSockRetry == NULL)
        goto usr_exit;
    //短信发送任务
    pSmsSend = new tsk_SMS_Send::tsk_SMS_Send(this);
    if(pSmsSend == NULL)
        goto usr_exit;
    //CTU请求任务
    pCtuAsk = new tsk_OtdrManage::tsk_OtdrManage(this);
    if(pCtuAsk == NULL)
        goto usr_exit;
    pCtuAsk->set_tsk_attribute(TSK_ATTRIBUTE_CTU);
    pCtuAsk->alloca_resource(1);
    pCtuAsk->otdr_mode = OTDR_MODE_SERRI;
    //设置为MCU的板卡地址
    pCtuAsk->otdrAddr.frame_no = MCU_FRAME;
    pCtuAsk->otdrAddr.card_no = MCU_CARD;
    pCtuAsk->otdrAddr.type = OTDR;
    pCtuAsk->otdrAddr.port = 1;

    //启动两个线程
    pDataDispatch->start();
    pHostCommu->start();
    pSockRetry->start();
    pSmsSend->start();
    pCtuAsk->start();
    retv = RET_SUCCESS;

usr_exit:
    return retv;
}
/*
   **************************************************************************************
 *  函数名称：destroy_other_tsk
 *  函数描述：程序退出时候，销毁创建一些任务
 *  入口参数：
 *  返回参数：
 *  作者       ：
 *  日期       ：2015-10-30
 *  修改日期：
 *  修改内容：
 **************************************************************************************
*/
int MainWindow::destroy_other_tsk()
{

    pDataDispatch->stop();
    pSockRetry->stop();
    pHostCommu->stop();
    pSmsSend->stop();
    pCtuAsk->stop();

    sleep(3);
    //pSockRetry->wait(1000);
    pSockRetry->terminate();



    //pDataDispatch->wait(1000);
    //pHostCommu->wait(1000);
    pDataDispatch->terminate();

    pHostCommu->terminate();
    m_otdrDevQue.xlist.clear();

    //pSmsSend->wait(1000);
    pSmsSend->terminate();
    pCtuAsk->terminate();

    return 0;

}
/*
   **************************************************************************************
 *  函数名称：check_dev_uuid
 *  函数描述：检查设备的
 *  入口参数：
 *  返回参数：
 *  作者       ：
 *  日期       ：
 **************************************************************************************
*/
int MainWindow :: check_dev_uuid()
{
    int row;
    tdb_sn_t input, mask;
    tdb_sn_t *ppout;
    QUuid id = QUuid::createUuid();
    QString strId;
    QByteArray byte_arry ;
    char *id_buf;

    bzero(&mask,  sizeof(tdb_sn_t));
    bzero(&input,  sizeof(tdb_sn_t));

    //    tmsdb_Delete_sn(&input, &mask);
    mask.sn[0] = 1;
    row = 0;
    row = tmsdb_Select_sn(&input, &mask, &ppout);
    if(row  < 1)
    {
        bzero(&mask,  sizeof(tdb_sn_t));
        bzero(&input,  sizeof(tdb_sn_t));
        mask.sn[0] = 1;
        strId = id.toString();
        byte_arry = strId.toLatin1();
        id_buf = byte_arry.data();
        memcpy(input.sn, id_buf, 128);
        tmsdb_Insert_sn(&input, &mask, 1);
        qDebug(" Insert sn %s", input.sn);
    }
    else
    {
        qDebug("read sn %s len %d", ppout[0].sn,strlen((char *)ppout[0].sn));
        free(ppout);
    }
    //qDebug("row %d len %d", row, strlen((char *)ppout[0].sn));
    return 0;
}

void MainWindow::timeUpdate()
{
    countTimer.Timer = new QTimer;
    ui->lcdNumTime->setNumDigits(19);
    ui->lcdNumTime->setSegmentStyle(QLCDNumber::Flat);
    ui->lcdNumTime->display(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
    connect(countTimer.Timer,SIGNAL(timeout()),this,SLOT(countTimeout()));

    countTimer.countCheckCyclTest = 0;
    countTimer.coutUpdateDevType = 0;
    countTimer.start_cyc_test = 0;
    countTimer.Timer->start(1000);
}
void MainWindow::countTimeout()
{
    showTime();
    countTimer.countCheckCyclTest++;
    countTimer.coutUpdateDevType++;

    if(countTimer.countCheckCyclTest == 60) //60 s检查一次周期性测量
    {
        countTimer.countCheckCyclTest = 0;
        if(m_ctrlStat.NMstat == NM_LOST)
            emit(checkCycleTime());
    }
    if(countTimer.coutUpdateDevType >29 &&\
            (countTimer.coutUpdateDevType % 30) == 0 )//30s更新一次设备类型
    {
        emit(updateDevType());
    }
    if(m_ctrlStat.NMstat == NM_PRE_LOST)//如果网管断开超过10分钟，开始启动周期性测量
        countTimer.start_cyc_test++;
    else
        countTimer.start_cyc_test = 0;
    if(NM_LOST_JUD_TIME == countTimer.start_cyc_test) //延迟一段时间，如果没有连接仍判断为断开
    {
        qDebug("net manager discard !");
        m_ctrlStat.NMstat = NM_LOST;
        gsm_send_alarm_NM_state(ALARM_LEVE_1);
        initialCycleTestTime();
    }
}
/*
   **************************************************************************************
 *  函数名称：save_data_befor_reboot
 *  函数描述：在自检启动前，将相关数据保存到
 *                ：
 *  入口参数：
 *  返回参数：
 *  作者       ：
 *  日期       ：
 *  修改日期：
 *  修改内容：
 *                ：
 **************************************************************************************
*/
int MainWindow::save_data_befor_reboot()
{
    db_save_total_hw_alarm();
    db_save_total_opm_alarm();
    db_save_cyc_time();
    /*
     *2016-02-04 增加了即使更新动作
    */
    system("sync");
}

/*
   **************************************************************************************
 *  函数名称：gsm_send_alarm_NM_state
 *  函数描述：通过短信告知网管状态
 *  入口参数：0,至少有一个网管，1，网管丢失
 *  返回参数：
 *  作者       ：
 *  日期       ：
 **************************************************************************************
*/
int MainWindow::gsm_send_alarm_NM_state(int lev)
{
    char buf[48];
    int res_code;
    res_code = -1;
    if(lev == ALARM_LEVE_1)
        strcpy(buf,"网管与主控通信故障! ");
    else if(lev == ALARM_NONE)
        strcpy(buf,"网管与主控通信恢复! ");
    else
        goto usr_exit;
    res_code = input_gsm_queue(lev, ALARM_COMMU,NULL,buf);
usr_exit:
    return 0;
}

void MainWindow::showTime()
{
    QString strTime, strHour;
    QDateTime currenTime;
    strTime = currenTime.currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    strHour = currenTime.currentDateTime().toString("hh:mm:ss");
#ifdef PRIOD_REBOOT
    if(strHour >= "03:00:00" &&strHour < "03:02:00"&& m_ctrlStat.will_reboot == 0)
    {
        m_ctrlStat.will_reboot = 1;
    }
    else if(m_ctrlStat.will_reboot  == 1&& strHour >= "03:02:00" )
    {
        save_data_befor_reboot();
        soft_reboot();
    }
#endif
    //qDebug()<<strHour.simplified();
    ui->lcdNumTime->display(strTime);
    //    testRingbuf();
}

MainWindow::~MainWindow()
{

    destroy_other_tsk();
    delete []RcvBuf;
    delete []BufList;
    delete ui;
}
/*
   **************************************************************************************
 *  函数名称：InitialSubrackAndCadInfo
 *  函数描述：初始化级联以及插卡信息
 *  入口参数：
 *  返回参数：
 *  作者       ：
 *  日期       ：
 **************************************************************************************
*/
void MainWindow::InitialSubrackAndCadInfo()
{
    int i, j, state;
    QString str;
    //模拟初始化参数
    m_subrackInfo.curShowNo = HOST_NO;
    m_subrackInfo.numInUse = 2;
    m_subrackInfo.numTotal =NUM_SUBRACK;
    memset(m_subrackInfo.oprateTime, 0, sizeof(m_subrackInfo.oprateTime));
    memset(m_subrackCard, 0, sizeof(m_subrackCard));
    //1 和16 号机框
    m_subrackInfo.onState = 0x0001;
    //模拟0机全装满
    m_subrackCard[HOST_NO].onState = 0xf80;
    i = 0;
    m_subrackCard[i].numInUse = 5;
    m_subrackCard[i].numTotal = 12;
    m_subrackCard[i].type[7] = GSM;
    m_subrackCard[i].type[8] = OTDR;
    m_subrackCard[i].type[9] = MCU;
    m_subrackCard[i].type[10] = PWU;
    m_subrackCard[i].type[11] = PWU;

    m_subrackCard[i].ports[7] = 0;
    m_subrackCard[i].ports[8] = 1;
    m_subrackCard[i].ports[9] = 0;
    m_subrackCard[i].ports[10] = 0;
    m_subrackCard[i].ports[11] = 0;


#if 0
    //模拟给启用的每个从机赋值
    for(i = 0; i < NUM_SUBRACK; i++)
    {
        state = m_subrackInfo.onState;
        if(1 ==  ( 1&(state >> i))
                )
        {
            //qDebug()<<"state = "<<state<<i;
            m_subrackCard[i].numInUse = 3;
            m_subrackCard[i].onState = 0xfff;
            m_subrackCard[i].numTotal = NUM_CARD;
            for(j = 0; j < NUM_COMM_CARD; j++)
            {
                state = m_subrackCard[i].onState;
                if(1 ==  ( 1 & (state >> j))
                        )
                {
                    if(j < 6)
                        m_subrackCard[i].type[j] = OPM + j;
                    else
                        m_subrackCard[i].type[j] = OSW;

                    //确定端口
                    if(OSW == m_subrackCard[i].type[j])
                        m_subrackCard[i].ports[j] = NUM_PORT;
                    else if(OLP == m_subrackCard[i].type[j])
                        m_subrackCard[i].ports[j] = 4;
                    else
                        m_subrackCard[i].ports[j] = 0;


                }
            }
            //9--11槽位固定的设备类型
            m_subrackCard[i].type[0] = OSW;
            m_subrackCard[i].ports[0] = 4;
            m_subrackCard[i].type[9] = MCU;
            m_subrackCard[i].type[10] = PWU;
            m_subrackCard[i].type[11] = PWU;
        }
    }
#endif
    //在主界面上显示当前段机框编号
    str.setNum(m_subrackInfo.curShowNo + 1);
    str = tr("当前显示框号") + str;
    ui->labelCurFrameNo->setText(str);

}
/*
   **************************************************************************************
 *  函数名称： 下面的函数是槽位响应函数
 *  函数描述： 第一个插槽响应函数
 *  入口参数：
 *  返回参数：
 *  日期       ： 2015-03-20
 **************************************************************************************
*/
void MainWindow::on_pushBtnCard_1_clicked()
{
    creatDlg(m_subrackInfo.curShowNo, 0);
}

void MainWindow::on_pushBtnCard_2_clicked()
{
    creatDlg(m_subrackInfo.curShowNo, 1);
}

void MainWindow::on_pushBtnCard_3_clicked()
{
    creatDlg(m_subrackInfo.curShowNo, 2);
}

void MainWindow::on_pushBtnCard_4_clicked()
{
    creatDlg(m_subrackInfo.curShowNo, 3);
}

void MainWindow::on_pushBtnCard_5_clicked()
{
    creatDlg(m_subrackInfo.curShowNo, 4);
}

void MainWindow::on_pushBtnCard_6_clicked()
{
    creatDlg(m_subrackInfo.curShowNo, 5);
}

void MainWindow::on_pushBtnCard_7_clicked()
{
    creatDlg(m_subrackInfo.curShowNo, 6);
}

void MainWindow::on_pushBtnCard_8_clicked()
{
    creatDlg(m_subrackInfo.curShowNo, 7);
}

void MainWindow::on_pushBtnCard_9_clicked()
{
    creatDlg(m_subrackInfo.curShowNo, 8);
}
/*
   **************************************************************************************
 *  函数名称：InitialCardType
 *  函数描述：初始化每个槽位设备类型
 *  入口参数：
 *  返回参数：
 *  作者       ：
 *  日期       ：
 **************************************************************************************
*/
void MainWindow:: InitialCardType()
{
    int i, state, subrackNo;
    subrackNo = m_subrackInfo.curShowNo;
    state = m_subrackCard[subrackNo].onState;
    //初始化槽位的设备类型
    for(i = 0;i < NUM_CARD;i++)
    {
        if(1 ==  ( 1&(state >> i))
                )
        {
            m_dlgType[i] = m_subrackCard[subrackNo].type[i];
        }
        else
        {
            m_dlgType[i] = NONE;
        }
    }
    //每个板卡的名字
    for(i = 0;i < NUM_CARD;i++)
    {
        switch(m_dlgType[i])
        {
        case NONE:
        {
            m_pushBtnCard[i]->setText(tr("空"));
            break;
        }
        case PWU:
        {
            if(m_subrackCard[subrackNo].opt[i][0] == PWU_V220)
                m_pushBtnCard[i]->setText(tr("电\n源\n220"));
            else  if(m_subrackCard[subrackNo].opt[i][0] == PWU_V48)
                m_pushBtnCard[i]->setText(tr("电\n源\n48"));
            else
                m_pushBtnCard[i]->setText(tr("电\n源\n220"));
            break;
        }
        case MCU:
        {
            m_pushBtnCard[i]->setText(tr("M\nC\nU"));
            break;
        }
        case TU:
        {
            m_pushBtnCard[i]->setText(tr("T\nU"));
            break;
        }
        case OPM:
        {
            m_pushBtnCard[i]->setText(tr("光\n功"));
            break;
        }
        case OSW:
        {
            m_pushBtnCard[i]->setText(tr("光\n开\n关"));
            break;
        }
        case OTDR:
        {
            m_pushBtnCard[i]->setText(tr("O\nT\nD\nR"));
            break;
        }
        case OLS:
        {
            m_pushBtnCard[i]->setText(tr("光\n源"));
            break;
        }
        case OLP:
        {
            m_pushBtnCard[i]->setText(tr("光\n保\n护"));
            break;
        }
        case GSM:
        {
            m_pushBtnCard[i]->setText(tr("短\n信\n单\n元"));
            break;
        }
        default:
        {
            m_pushBtnCard[i]->setText(tr("空"));
            break;
        }
        }
    }
    update();
}
/*
   **************************************************************************************
 *  函数名称：InitialCardBtn
 *  函数描述：将每个槽位按钮指针放到一个数组内部，方便处理
 *  入口参数：
 *  返回参数：
 *  作者       ：
 *  日期       ：
 **************************************************************************************
*/
void MainWindow:: InitialCardBtn()
{
    int i;
    i = 0;
    m_pushBtnCard[i++] = ui->pushBtnCard_1;
    m_pushBtnCard[i++] = ui->pushBtnCard_2;
    m_pushBtnCard[i++] = ui->pushBtnCard_3;
    m_pushBtnCard[i++] = ui->pushBtnCard_4;
    m_pushBtnCard[i++] = ui->pushBtnCard_5;
    m_pushBtnCard[i++] = ui->pushBtnCard_6;
    m_pushBtnCard[i++] = ui->pushBtnCard_7;
    m_pushBtnCard[i++] = ui->pushBtnCard_8;
    m_pushBtnCard[i++] = ui->pushBtnCard_9;
    m_pushBtnCard[i++] = ui->pushBtnCard_10;
    m_pushBtnCard[i++] = ui->pushBtnCard_11;
    m_pushBtnCard[i++] = ui->pushBtnCard_12;
    m_pushBtnCard[i++] = ui->pushBtnCard_13;
    m_pushBtnCard[i++] = ui->pushBtnCard_14;
    m_pushBtnCard[i++] = ui->pushBtnCard_15;
    m_pushBtnCard[i++] = ui->pushBtnCard_16;
}
//设置窗口背景色和风格
void MainWindow:: setWindowsStyle()
{
    //exe not show in task bar
    //let Layout adjust to mainwindow
#ifdef ARM_BOARD
    setWindowFlags(windowFlags()|Qt::FramelessWindowHint|Qt::WindowTitleHint);
#endif
    setCentralWidget(ui->verticalLayoutWidget);
    //set background
    setAutoFillBackground(true);
    QPalette paletteBack = this->palette();
    paletteBack.setColor(QPalette::Background,QColor (125,175,200) );
    setPalette(paletteBack);
}
/*
   **************************************************************************************
 *  函数名称：creatDlg
 *  函数描述：根据对话框类型创建对应的对话框，主界面上的btn调用
 *  函数描述：MCU，PWU，有专门的响应函数，NONE，不用响应
 *  入口参数：
 *  返回参数：
 *  作者       ：
 *  日期       ：
 **************************************************************************************
*/
void MainWindow::creatDlg(int curFrameShow, int cardNo)
{
    switch(m_dlgType[cardNo])
    {
    case OPM:
    {
        dlgOpm *pDlgOpm = new dlgOpm(this, curFrameShow,cardNo,OPM);
        pDlgOpm->setAttribute(Qt::WA_DeleteOnClose);
        pDlgOpm->showMaximized();
        break;
    }
    case OSW:
    case OTDR://光开关和otdr做到一起
    {
        QDlgOtdr *pDlgOtdr = new QDlgOtdr(this, curFrameShow,cardNo,m_dlgType[cardNo]);
        pDlgOtdr->setAttribute(Qt::WA_DeleteOnClose);
        pDlgOtdr->showMaximized();
        break;
    }
    case OLS:
    {
        break;
    }
    case OLP:
    {

        dlgOlp *pDlgOlp = new dlgOlp(this, curFrameShow,cardNo,OLP);
        pDlgOlp->setAttribute(Qt::WA_DeleteOnClose);
        pDlgOlp->showMaximized();
        break;
    }
    case GSM:
    {
        dlgGsm *pdlgGsm = new dlgGsm(this, curFrameShow,cardNo,GSM);
        pdlgGsm->setAttribute(Qt::WA_DeleteOnClose);
        pdlgGsm->showMaximized();
        break;
    }
    case MCU:
    case TU:
    {
        QDlgMcu *pDlgMcu = new QDlgMcu(this);
        pDlgMcu->setAttribute(Qt::WA_DeleteOnClose);
        pDlgMcu->showMaximized();
    }
    default:
    {
        break;
    }
    }
    return;
}

//检查是否是合法命令
int MainWindow:: checkCMD(int cmd)
{
    int return_val;
    return_val = 1;
    switch(cmd)
    {
    case ID_GET_DEVTYPE :
        break;
    default:
        break;
    }
    return return_val;
}
/*
   **************************************************************************************
 *  函数名称：processRcvData
 *  函数描述：从环形缓冲区中取出之后，分类处理数据，如果需要前台显示，
 *  函数描述：则通过发送信号段方式，需要后台处理的，直接处理，然后释放链表
 *  入口参数：
 *  返回参数：
 *  作者       ：
 *  日期       ：
 **************************************************************************************
*/
int MainWindow::processRcvData(int cmd, int dataLen, _tagBufList *pBufHead)
{
    int bufNum;
    switch(cmd)
    {
    case ID_RET_VERSION: //软件版本
    case ID_RET_DEV_PRODUCE://硬件信息
    case ID_RET_OTDR_PARAM://otdr参数
    {
        send_short_data_2_dlg( cmd, pBufHead); //板卡---->返回设备类型
        break;
    }
    default:
        if(pBufHead != NULL)
        {
            bufNum = LinkedBufManager.freeBuf(pBufHead);
            break;
        }
    }
    //输出相关信息
    qDebug("processRcvData 收到cmd 0x%x datalen %d ", \
           cmd, dataLen );
    qDebug("processRcvData buf_head 0x%x linked buf num %d", \
           pBufHead,LinkedBufManager.freeBufNum);
    return 0;
}
/*
   **************************************************************************************
 *  函数名称：deal_rcv_card_short_data
 *  函数描述：从板卡收到较短数据，需要对话框及时处理
 *  入口参数：
 *  返回参数：
 *  作者       ：
 *  日期       ：
 **************************************************************************************
*/
int MainWindow::send_short_data_2_dlg(unsigned int cmd, _tagBufList *pBufHead)
{
    //如果宿主对话框已经消失，那么本地释放
    int local_free;
    local_free = 1;
    if(objSyncSem[SEM_COMMN].resCmd == cmd)
    {
        if(pBufHead != NULL && m_currenShowDlg > 0)
        {
            objSyncSem[SEM_COMMN].resCode = RET_SUCCESS;
            objSyncSem[SEM_COMMN].pBufListHead = pBufHead;
            objSyncSem[SEM_COMMN].objSem.release();
            local_free = 0;
        }
    }
    //本地释放
    if(local_free == 1 && pBufHead  != NULL)
        LinkedBufManager.freeBuf(pBufHead);
    return 0;
}
//mcu返回nm板卡信息
int MainWindow::mcuToNMCardsType()
{
    //从数据库里面查询设备以及板卡类型然后发送给mcu
    return 0;
}
/*
   **************************************************************************************
 *  函数名称：inspectFiber
 *  函数描述：检查光纤有无告警
 *  入口参数：参考曲线，当前otdr曲线，不管有无告警，都要返回曲线
 *  返回参数：
 *  作者       ：
 *  日期       ：
 **************************************************************************************
*/
/*
   **************************************************************************************
 *  函数名称：inspectFiber
 *  函数描述：由光功告警引起的测试，查找告警
 *                ：
 *  入口参数：
 *  返回参数：
 *  作者       ：
 *  日期       ：2015-03
 *  修改日期：2015-12-08
 *  修改内容：改为群发
 *                ：
 **************************************************************************************
*/
int MainWindow::inspectFiber()
{

    int eventNumRefer, eventNumCur;
    int alarmLev,i, j,m;
    int samplHz ,xSpace, alarmPos;
    int fd, res_code;
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

    //初始化相关的指针
    ptest_hdr = (tms_retotdr_test_hdr*)&AlarmCurv.curv.oswPort;
    ptest_param = (tms_retotdr_test_param*)&AlarmCurv.curv.measurPara;
    pdata_hdr = (tms_retotdr_data_hdr*)&AlarmCurv.curv.dpid;
    pdata_val = (tms_retotdr_data_val*)AlarmCurv.curv.dataPt;
    pevent_hd = (tms_retotdr_event_hdr*)AlarmCurv.curv.eventID;
    pevent_val = (tms_retotdr_event_val*)AlarmCurv.curv.eventBuf;
    pchain = (tms_retotdr_chain*)AlarmCurv.curv.measurResultID;
    /*
     *2015-12-09 修改为群发
    */
    fd = 0;  //获取网管fd
    dst_addr.src = ADDR_MCU;
    dst_addr.dst = ADDR_MASS_SEND;
    alarmLev = 0;
    deltaLoss = 0;
    alarmPos = 0;
    memset(msg, 0 , sizeof(msg));
    AlarmCurv.mutexObj.lock();
    posw_port = (_tagDevComm *)(&AlarmCurv.curv.oswPort);
    res_code = db_get_refr_otdr(posw_port, &ReferCrv);
    if( res_code != RET_SUCCESS) //获取参考曲线失败
    {
        sprintf(msg, "inspect fiber alarm get otdr ref_otdr error");
        goto usr_exit;
    }

    pAlarmTh = &ReferCrv.alarmTh;
    samplHz = ReferCrv.measurPara.samplHz;
    m = ReferCrv.measurPara.pulseWidth_ns * samplHz * pow(10,-9);

    //参考曲线和当前otdr曲线的事件点数目
    eventNumRefer = ReferCrv.eventNum;
    eventNumCur = AlarmCurv.curv.eventNum;
    pEventBufRefer = ReferCrv.eventBuf;
    pEventBufCur = AlarmCurv.curv.eventBuf;

    //初始化告警结构体
    memset(&alarm_line, 0, sizeof(tms_alarm_line_hdr));
    alarm_line.alarm_type = ALARM_LINE;
    alarm_line.frame = AlarmCurv.curv.oswPort.frame_no;
    alarm_line.slot = AlarmCurv.curv.oswPort.card_no;
    alarm_line.port = AlarmCurv.curv.oswPort.port;
    GetCurrentTime((char *)(alarm_line.time));
    div_distance = drawCurv.Get_xCoord(2*m, ptest_param->sample,ptest_param->gi);
    qDebug("div distace %f", div_distance);
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
                    deltaLoss = pEventBufCur[i].insertLoss;
                //当前曲线插损不存在
                else
                    qDebug("当前曲线插损不存在，当前/参考 事件序号%d %d",i,j);
            }
            else
            {

                if(pEventBufCur[i].insertLoss < INVALID_VALUE)
                    deltaLoss = pEventBufCur[i].insertLoss;
                //当前曲线插损不存在
                else
                    qDebug("当前曲线插损不存在，当前/参考 事件序号%d %d",i,j);
            }
            alarmLev = getAlarmLev(deltaLoss, pAlarmTh);
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
    if(m_ctrlStat.NMstat == NM_EXIST)
    {
        tms_AlarmLine(fd,&dst_addr,&alarm_line,ptest_hdr,ptest_param,pdata_hdr,pdata_val,\
                      pevent_hd,pevent_val,pchain,ID_ALARM_LINE);
    }
    else if(m_ctrlStat.NMstat != NM_EXIST)
    {
        input_gsm_queue(alarmLev, ALARM_LINE,posw_port);
    }
#if USR_DEBUG
    qDebug("inspect fiber alarm lev %d", alarmLev);
    qDebug("ptest_hdr  osw frame %d otdr port %d", ptest_hdr->osw_frame, ptest_hdr->otdr_port);
    qDebug("ptest_param  range %d  reserv1 %d", ptest_param->rang, ptest_param->reserve1);
    qDebug("pdata_hdr  dpid %s data num %d", pdata_hdr->dpid, pdata_hdr->count);
    qDebug("pevent_hd  eventid  %s event num %d", pevent_hd->eventid, pevent_hd->count);
    qDebug("pchain  inf %s att %f ", pchain->inf, pchain->att);
#endif

usr_exit:
    AlarmCurv.mutexObj.unlock();
    return res_code;
}
/*
   **************************************************************************************
 *  函数名称：网管存在的情况下上传opm光功率告警曲线，网管点名曲线
 *  函数描述：
 *  入口参数：
 *  返回参数：
 *  作者       ：
 *  日期       ：
 **************************************************************************************
*/
/*
   **************************************************************************************
 *  函数名称：upload_alarm_cyc_data
 *  函数描述：上传光功告警曲线
 *                ：
 *  入口参数：
 *  返回参数：
 *  作者       ：
 *  日期       ：
 *  修改日期：2015-12-09
 *  修改内容：修改为群发
 *                ：
 **************************************************************************************
*/
int MainWindow::upload_alarm_cyc_data(void * ptr_alarmline,  _tagOtdrCurv *ptr_curv, unsigned int cmd, void *ptr_opt)
{
    tms_alarm_line_hdr *parlarmline;
    //发送函数接口部分
    tms_retotdr_test_hdr   *ptest_hdr;
    tms_retotdr_test_param *ptest_param;
    tms_retotdr_data_hdr   *pdata_hdr;
    tms_retotdr_data_val   *pdata_val;
    tms_retotdr_event_hdr  *pevent_hd;
    tms_retotdr_event_val  *pevent_val;
    tms_retotdr_chain      *pchain;
    glink_addr dst_addr;
    tms_context *popt;
    int res_code;
    popt = (tms_context *)ptr_opt;
    int fd;
    ptest_hdr = (tms_retotdr_test_hdr*)&ptr_curv->oswPort;
    ptest_param = (tms_retotdr_test_param*)&ptr_curv->measurPara;
    pdata_hdr = (tms_retotdr_data_hdr*)&ptr_curv->dpid;
    pdata_val = (tms_retotdr_data_val*)ptr_curv->dataPt;
    pevent_hd = (tms_retotdr_event_hdr*)ptr_curv->eventID;
    pevent_val = (tms_retotdr_event_val*)ptr_curv->eventBuf;
    pchain = (tms_retotdr_chain*)ptr_curv->measurResultID;
    parlarmline = (tms_alarm_line_hdr *)ptr_alarmline;
    bzero(parlarmline, sizeof(tms_alarm_line_hdr));
    fd = 0;  //获取网管fd
    dst_addr.src = ADDR_MCU;
    dst_addr.dst = ADDR_MASS_SEND; //群发
    dst_addr.pkid = popt->pgb->pkid;
    res_code = tms_AlarmLine(fd,&dst_addr,parlarmline, ptest_hdr, ptest_param, pdata_hdr, pdata_val,\
                             pevent_hd, pevent_val, pchain,ID_ALARM_LINE);
    //qDebug("upload_alarm_cyc_data cmd %x res_code %d", cmd, res_code);

    return 0;
}

/*
   **************************************************************************************
 *  函数名称：input_gsm_queue
 *  函数描述：通过gsm发送告警
 *  入口参数：
 *  返回参数：
 *  日期版本：
 *  作者       ：
 **************************************************************************************
*/
int MainWindow::input_gsm_queue(int alarm_lev, int alarm_type, _tagDevComm *pusr_dev, char option[])
{
    //短信告警权限
    tdb_sms_t *smsout = NULL;
    tdb_sms_t cfg_sms, mask_sms;
    //光开关关联地理信息
    tdb_dev_map_t *map_buf = NULL;
    tdb_dev_map_t cfg_map, mask_map;
    int wday, row, i;
    int map_row;
    int count;
    int offset,  wch_num;
    bool is_map_info;

    char msg[GSM_TEXT_LEN *3]; //全部是中文，一个中文占3个字节
    char lev_buf[32]; //需要经过计算方得到32的长度
    char type_buf[32];
    char ch_time[20];
    wchar_t wmsg[GSM_TEXT_LEN]; //存放ucs-4编码
    wchar_t wmsg_1[64]; //存放ucs-4编码
    unsigned short msg_code[GSM_TEXT_LEN + 16]; //存放ucs-2编码
    unsigned short *ptr_uint16;
    time_t now;
    _tagGsmContext gsm_context;
    struct tm* ptr ;
    //要放在这里，否则，mbstowcs不起作用的
    if(NULL == setlocale (LC_ALL, "zh_CN.UTF-8"))
    {
        qDebug("setlocale zh_CN.UTF-8 fail");
    }
    //初始化
    memset(msg, 0, sizeof(msg));
    memset(wmsg, 0, sizeof(wmsg));
    memset(msg_code, 0, sizeof(msg_code));
    gsm_context.usr_addr.src = ADDR_MCU;
    gsm_context.usr_addr.dst = ADDR_MCU;
    gsm_context.usr_addr.pkid = creat_pkid();

    get_alarm_ch_text(alarm_lev, alarm_type, lev_buf, type_buf);
    //拷贝告警类型，告警级别 最长32*3 = 96B

    strcpy(msg, type_buf);
    offset = strlen(type_buf);
    strcpy(msg + offset, lev_buf);
    offset += strlen(lev_buf);
    wch_num = wcslen(wmsg);
    mbstowcs(wmsg + wch_num,msg, strlen(msg));


    is_map_info = false;
    //查询关联信息 64*4B = 256B
    if(pusr_dev != NULL && alarm_type == ALARM_LINE)
    {
        bzero(&mask_map, sizeof(tdb_dev_map_t));
        bzero(&cfg_map, sizeof(tdb_dev_map_t));
        memcpy(&cfg_map.frame, pusr_dev, sizeof(_tagDevComm));
        mask_map.frame = 1;
        mask_map.slot = 1;
        mask_map.port = 1;
        mask_map.type = 1;
        map_row = tmsdb_Select_dev_map(&cfg_map, &mask_map, &map_buf);
        if( map_row > 0&&map_buf != NULL)
        {
            is_map_info = true;
            if(is_map_info)
            {
                ptr_uint16 = (unsigned short *)map_buf[map_row - 1].start_name; //局站
                wch_num = wcslen(wmsg);
                utf162wcs(ptr_uint16, wmsg + wch_num, 0);
                /*
                bzero(wmsg_1, sizeof(wmsg_1));
                utf162wcs(ptr_uint16, wmsg_1, 0);
                wcstombs(msg, wmsg_1,sizeof(msg));
                qDebug("start name %s",msg);
                PrintfMemory(map_buf[map_row - 1].start_name, 64);
                */

                ptr_uint16 = (unsigned short *)map_buf[map_row - 1].end_name; //末端局站
                wch_num = wcslen(wmsg);
                utf162wcs(ptr_uint16, wmsg + wch_num, 0);

                /*
                bzero(wmsg_1, sizeof(wmsg_1));
                utf162wcs(ptr_uint16, wmsg_1, 0);
                wcstombs(msg, wmsg_1,sizeof(msg));
                qDebug("end name %s",msg);
                PrintfMemory(map_buf[map_row - 1].end_name, 64);
                */

                wch_num = wcslen(wmsg);
                ptr_uint16 = (unsigned short *)map_buf[map_row - 1].dev_name;//设备
                utf162wcs(ptr_uint16, wmsg + wch_num, 0);
                /*
                bzero(wmsg_1, sizeof(wmsg_1));
                utf162wcs(ptr_uint16, wmsg_1, 0);
                wcstombs(msg, wmsg_1,sizeof(msg));
                qDebug("dev name %s",msg);
                PrintfMemory(map_buf[map_row - 1].dev_name, 64);
                */

                wch_num = wcslen(wmsg);
                ptr_uint16 = (unsigned short *)map_buf[map_row - 1].cable_name;//光缆名称
                utf162wcs(ptr_uint16, wmsg + wch_num, 0);
                /*
                bzero(wmsg_1, sizeof(wmsg_1));
                utf162wcs(ptr_uint16, wmsg_1, 0);
                wcstombs(msg, wmsg_1,sizeof(msg));
                qDebug("cable name %s",msg);
                PrintfMemory(map_buf[map_row - 1].cable_name, 64);
                */

            }
        }
        /*
         *2016-03-11 将map_buf释放掉，避免内存泄漏
        */
        if(map_buf != NULL)
            free(map_buf);
    }
    //拷贝其他内容
    if(option != NULL)
    {
        wch_num = wcslen(wmsg);
        mbstowcs(wmsg + wch_num,option, strlen(option));
    }
    //拷贝时间
    bzero(ch_time, sizeof(ch_time));
    GetCurrentTime(ch_time);

    wch_num = wcslen(wmsg);
    mbstowcs(wmsg + wch_num,ch_time, strlen(ch_time));

    wch_num = wcslen(wmsg); //获取宽字符的个数
    count = utf16len(wmsg); //获取ucs-16编码个数
    if(count > (GSM_TEXT_LEN + 16))
        count = GSM_TEXT_LEN;
    wcs2utf16(wmsg, msg_code, wch_num);

    //重新转换输出一遍，看看
    bzero(wmsg, sizeof(wmsg));
    bzero(msg, sizeof(msg));
    utf162wcs(msg_code, wmsg, 0);
    wcstombs(msg, wmsg,sizeof(msg));
    qDebug("gsm msg %s xxxx msg len %d  utf6 count %d wchar count %d", msg, strlen(msg), count, wch_num);

    //查询告警权限 now = time(NULL);
    ptr = localtime(&now);
    wday = ptr->tm_wday; //从星期1算起，0～6
    memset(&mask_sms, 0, sizeof(tdb_sms_t));
    memset(&cfg_sms, 0, sizeof(tdb_sms_t));

    cfg_sms.level = alarm_lev;
    cfg_sms.type = alarm_type;
    cfg_sms.time = wday;
    mask_sms.level = 1;
    mask_sms.type = 1;
    mask_sms.time = 1;

    row = tmsdb_Select_sms(&cfg_sms, &mask_sms, &smsout);
    if(row > 0)
    {
        for(i = 0; i < row;i++)
        {
            bzero(gsm_context.phone, sizeof(gsm_context.phone));
            memcpy(&gsm_context.phone, (char *)(smsout[i].phone), sizeof(gsm_context.phone));
            //非字符数
            gsm_context.count = wch_num;
            if(gsm_context.count > GSM_TEXT_LEN)
                gsm_context.count = GSM_TEXT_LEN;
            memcpy(&gsm_context.context, msg_code, count* sizeof(unsigned short));
            pSmsSend->GSMQueue.objMutex.lock();
            if(pSmsSend->GSMQueue.xqueue.size() < GSM_BUF_SIZE)
            {
                pSmsSend->GSMQueue.xqueue.enqueue(gsm_context);
            }
            else
            {
                //短消息队列已满，去掉时间最长的一个，然后添加一个
                pSmsSend->GSMQueue.xqueue.dequeue();
                pSmsSend->GSMQueue.xqueue.enqueue(gsm_context);
            }
            pSmsSend->GSMQueue.objMutex.unlock();


#if USR_DEBUG
            qDebug("gsm insert queue %s row %d queue len %d", \
                   smsout[i].phone, row, pSmsSend->GSMQueue.xqueue.size());
            qDebug("%s", msg);
#endif
        }
        free(smsout);
    }

    return 0;
}
int MainWindow::wait_res_response(unsigned int send_cmd, unsigned int res_cmd, int wait_time_ms,int sem_index)
{
    bool result;
    int res_code;
    result = false;
    objSyncSem[sem_index].sendCmd = send_cmd;
    objSyncSem[sem_index].resCmd = res_cmd;
    result = objSyncSem[sem_index].objSem.tryAcquire(1,wait_time_ms);
    if(result)
        res_code = objSyncSem[sem_index].resCode;
    else
        res_code = RET_SEND_CMMD_TIMEOUT;
    return res_code;
}
/*
   **************************************************************************************
 *  函数名称：get_alarm_ch_text
 *  函数描述：通过告警级别，告警类型，返回相应的中文提示
 *  入口参数：
 *  返回参数：
 *  作者       ：
 *  日期       ：
 **************************************************************************************
*/
int MainWindow::get_alarm_ch_text(int alarm_leve, int alarm_type, char leve_buf[], char type_buf[],int option)
{
    //告警级别
    QString strLev,strType;
    QByteArray buf;
    switch(alarm_leve)
    {
    case ALARM_NONE:
        strLev = tr("告警消警,");
        break;
    case ALARM_LEVE_1:
        strLev = tr("紧急告警,");
        break;
    case ALARM_LEVE_2:
        strLev = tr("主要告警,");
        break;
    case ALARM_LEVE_3:
        strLev = tr("次要告警,");
        break;
    default:
        qDebug("invalid alarm lev %d", alarm_leve);
        break;
    }
    //告警类型
    switch(alarm_type)
    {
    case ALARM_LINE:
        strType = tr("线路告警:");
        break;
    case ALARM_OPM:
        strType = tr("光功率告警:");
        break;
    case ALARM_HARD_WARE:
        strType = tr("硬件告警:");
        break;
    case ALARM_COMMU:
        strType = tr("通信告警:");
        break;
    case ALARM_OLP_SWITCH:
        strType = tr("OLP切换告警:");
        break;
    default:
        qDebug("invalid alarm type %d", alarm_type);
        break;
    }
    if(!strLev.isEmpty())
    {
        buf = strLev.toLocal8Bit();
        strcpy(leve_buf, buf);
    }
    if(!strType.isEmpty())
    {
        buf = strType.toLocal8Bit();
        strcpy(type_buf, buf);
    }
    return 0;
}

/*
   **************************************************************************************
 *  函数名称：db_get_refr_otdr
 *  函数描述：获取osw对应端口的参考曲线
 *  入口参数：
 *  返回参数：
 *  作者       ：
 *  日期       ：
 **************************************************************************************
*/
int MainWindow::db_get_refr_otdr(_tagDevComm *posw, _tagReferCurv *pReferCurv)
{

    tdb_otdr_ref_t input, mask;
    struct tms_otdr_ref_hdr dev_osw,mask_osw;
    int res_db;
    res_db = -1;
    if(posw ==  NULL || pReferCurv == NULL)
        goto usr_exit;

    memset(&input, 0, sizeof(tdb_otdr_ref_t));
    memset(&mask, 0, sizeof(tdb_otdr_ref_t));
    bzero(&mask, sizeof(tdb_otdr_ref_t));
    bzero(&mask_osw, sizeof( tms_otdr_ref_hdr));
    input.pref_hdr = &dev_osw;
    mask.pref_hdr  = &mask_osw;

    mask.id = 0;
    dev_osw.osw_frame = posw->frame_no;
    dev_osw.osw_slot = posw->card_no;
    dev_osw.osw_port = posw->port;
    dev_osw.osw_type = OSW;

    mask_osw.osw_frame = 1;
    mask_osw.osw_slot = 1;
    mask_osw.osw_port = 1;
    mask_osw.osw_type = 1;
    memset(&ReferCrv, 0, sizeof(ReferCrv));
    res_db = tmsdb_Select_otdr_ref(&input, &mask, db_read_refotdr, pReferCurv);
usr_exit:
    return res_db;

}

/*
   **************************************************************************************
 *  函数名称：db_get_cyc_measur
 *  函数描述：获取周期性测量的参数
 *  入口参数：
 *  返回参数：
 *  作者       ：
 *  日期       ：
 **************************************************************************************
*/
int MainWindow::db_get_osw_test_para(_tagDevComm *posw,_tagMeasurPara *pMeasur)
{
    tdb_otdr_ref_t input, mask;
    struct tms_otdr_ref_hdr dev_osw,mask_osw;
    int res_db;

    memset(&input, 0, sizeof(tdb_otdr_ref_t));
    memset(&mask, 0, sizeof(tdb_otdr_ref_t));
    bzero(&mask, sizeof(tdb_otdr_ref_t));
    bzero(&mask_osw, sizeof( tms_otdr_ref_hdr));
    input.pref_hdr = &dev_osw;
    mask.pref_hdr  = &mask_osw;

    mask.id = 0;
    dev_osw.osw_frame = posw->frame_no;
    dev_osw.osw_slot = posw->card_no;
    dev_osw.osw_port = posw->port;
    dev_osw.osw_type = OSW;

    mask_osw.osw_frame = 1;
    mask_osw.osw_slot = 1;
    mask_osw.osw_port = 1;
    mask_osw.osw_type = 1;
    res_db = tmsdb_Select_otdr_ref(&input, &mask, db_read_refotdr_test_para, (void *)pMeasur);

    return res_db;
}

//获取告警级别,返回告警级别
int MainWindow:: getAlarmLev(float deltaLoss, _tagAlarmTh *pAlarmTh)
{
    int alarmLev;
    int idelatLoss;
    idelatLoss = deltaLoss *10;
    alarmLev = 0;
    if(idelatLoss > pAlarmTh->seriousAlarmTh )
        alarmLev = 1;
    else if(idelatLoss > pAlarmTh->seriousAlarmTh )
        alarmLev = 2;
    else if(idelatLoss > pAlarmTh->slightAlarmTh )
        alarmLev = 3;
    else
        alarmLev = 0;
    return alarmLev;
}
//处理异常提示,供各个子dlg调用
int MainWindow::dealAbnormal(QString str, int isYesNo)
{
    int return_yes;
    return_yes = 1;
    QMessageBox::StandardButton reply;
    if(isYesNo > 0)
    {
        reply = QMessageBox::information(NULL, tr("提示"), str,
                                         QMessageBox::Yes, QMessageBox::No);
        if(reply != QMessageBox::Yes)
        {
            return_yes = 0;
        }
    }
    else
    {
        reply = QMessageBox::information(this, tr("提示"), str,
                                         QMessageBox::Yes);
    }
    return return_yes;

}
//临时测试
void MainWindow::testRingbuf()
{
    char buf[512];
    memset(buf, 0, sizeof(buf));
    _tagDataHead *pdataHead;
    _tagCardVersion *pcardVersion;
    pdataHead = (_tagDataHead *)buf;
    pdataHead->cmd = ID_MCU_NM_RE_CARD_VERSION;
    pdataHead->dataLen = 274;
    pcardVersion = (_tagCardVersion *)(buf + sizeof(_tagDataHead));
    pcardVersion->frameNo = 0;
    pcardVersion->cardNo = 2;
    pcardVersion->devType = 4;
    memset(pcardVersion->softV, 0, sizeof(pcardVersion->softV));
    strcpy(pcardVersion->softV, "V14.0912.01");
    process_data_from_sock(buf,pdataHead->dataLen,1000, NULL);
}
//从链表缓冲中拷贝数据,返回拷贝的字节数
int MainWindow::cpyDataFromLinkedBuf(char buf[], int dataLen, _tagBufList *pBufHead )
{
    int cpyedBytes, leftBytes;
    _tagBufList *pBufIndex;
    cpyedBytes = 0;
    leftBytes = dataLen;
    pBufIndex = pBufHead;
    while(cpyedBytes < dataLen)
    {
        if(leftBytes < pBufIndex->bufSize)
        {
            memcpy(buf + cpyedBytes, pBufIndex->buf, leftBytes);
            cpyedBytes += leftBytes;
        }
        else
        {
            memcpy(buf + cpyedBytes, pBufIndex->buf, pBufIndex->bufSize);
            leftBytes += pBufHead->bufSize;
        }
        if(pBufIndex->next != NULL)
            pBufIndex = pBufIndex->next;
        else
            break;
    }
    return cpyedBytes;

}

/*
   **************************************************************************************
 *  函数名称：dealRcvCardDevtype
 *  函数描述：板卡上报设备类型，mcu收到该信息后，更新设备类型，该设备类型
 *  函数描述：作为将来mcu新增设备判断依据.本命令数据长度不会超过256
 *  入口参数：
 *  返回参数：
 *  作者       ：
 *  日期       ：
 **************************************************************************************
*/
int MainWindow::dealRcvCardDevtype(char buf[], void *ptr_opt )
{
    int returnVal, frameNo, cardNo,devType, portNum;
    int offset;
    int opt[4];
    unsigned int cmd;
    _tagDataHead *pDataHead;
    tms_context *pcontext;

    pDataHead = (_tagDataHead *)buf;
    cmd = pDataHead->cmd;
    returnVal = RET_SUCCESS;
    memset(buf, 0, sizeof(buf));
    portNum = 0;
    frameNo = 0;
    cardNo = 0;


    if(ID_RET_DEVTYPE == cmd)
    {
        offset = 8;
        frameNo = *(int*)(buf + offset);
        offset += 4;
        cardNo  = *(int*)(buf + offset);
        offset += 4;
        devType = *(int*)(buf + offset);
        offset += 4;
        portNum =  *(int*)(buf + offset);
        offset += 4;
        //检查参数是否正常
        if(frameNo < 0 || frameNo >= NUM_SUBRACK)
        {
            returnVal =  RET_UNMATCH_FRAME;
            goto usr_exit;
        }
        else if(cardNo < 0 || cardNo >= NUM_CARD)
        {

            returnVal =  RET_UNMATCH_SLOT;
            goto usr_exit;
        }
        else if(devType < m_ctrlStat.minDevType || devType > m_ctrlStat.maxDevType)
        {
            returnVal =  RET_UNMATCH_TYPE;
            goto usr_exit;
        }
        else if(portNum < 0 || portNum > NUM_PORT)
        {
            returnVal = RET_PARAM_INVALID;
            goto usr_exit;
        }
        //全部正确
        else
        {
            memcpy(opt, buf + offset, sizeof(int) * 4);
            update_dev_type(frameNo);
        }

    }
    else
    {
        returnVal = ID_MCU_NM_RCODE_CMD_INVALIDE;

    }

usr_exit:
    //如果不成功，输出错误信息
    qDebug("rcv card type res code %d, frame %d card %d type %d port %d  opt 0-3 %d %d %d %d",\
           returnVal, frameNo, cardNo,devType,portNum, opt[0], opt[1], opt[2], opt[3]);
    mcu_Ack(ptr_opt, cmd, returnVal);
    //如果随OTDR获取参数信息
    if(devType == OTDR)
    {
        _tagDlgAttribute otdrDev;
        otdrDev.frameNo = frameNo;
        otdrDev.cardNo = cardNo;
        otdrDev.devType = OTDR;
        pcontext = (tms_context *)ptr_opt;
        send_cmd(pcontext->fd, (char *)&otdrDev,ID_GET_OTDR_PARAM);
    }
    return returnVal;
}
/*
   **************************************************************************************
 *  函数名称：dealRcvHostNetManagStats
 *  函数描述：如果网管建立链接，那么将要发送总的告警给网管，如果网管（全部）丢失，
 *  函数描述：那么将来产生段告警将通过短信发送.本条命令数据长度不会超过256
 *  入口参数：
 *  返回参数：
 *  作者       ：
 *  日期       ：
 **************************************************************************************
*/
int MainWindow::dealRcvHostNetManagStats(char buf[], void *ptr_opt)
{
    int offset, NMState;
    int res_code;
    _tagDataHead *pDataHead;
    glink_addr dst_addr;
    tms_context *popt;
    popt = (tms_context *)ptr_opt;
    dst_addr.src =  ADDR_MCU;
    dst_addr.dst = popt->pgb->src;
    pDataHead = (_tagDataHead *)(buf);
    offset = sizeof(_tagDataHead);
    res_code = RET_SUCCESS;

    memcpy(&NMState, buf + offset, sizeof(int));
    //网管建立
    if(NM_EXIST == NMState)
    {
        cancelCycleTest();
        m_ctrlStat.NMstat = NM_EXIST;
        //发送查询告警命令 OPM,OLP
        pHostCommu->get_op_alarm = true;
        pHostCommu->up_cyc_curv = true;
        gsm_send_alarm_NM_state(ALARM_NONE);

    }
    //网管消失
    else if(NM_LOST == NMState)
    {
        m_ctrlStat.NMstat = NM_PRE_LOST;
    }
    else
        res_code = RET_PARAM_INVALID;

    dst_addr.pkid = popt->pgb->pkid;
    mcu_Ack(ptr_opt,pDataHead->cmd , res_code);
    qDebug("nm state %d ", NMState);
    return res_code;

}
/*
   **************************************************************************************
 *  函数名称：check_rcv_opm_alarm_change
 *  函数描述：收到变化的光功率告警，首先检查，如果无误，进行下一步处理
 *  入口参数：
 *  返回参数：检查结果，0，成功；1，失败
 *  作者       ：
 *  日期       ：
 **************************************************************************************
*/
int MainWindow::check_rcv_opm_alarm_change(const char buf[])
{
    //告警类型，从机号，临时变量
    int alarm_type, frame_no,i;
    int card_no, alarm_num, changed_num;
    int offset, dev_type;
    int res_code;
    int frame_state ,card_state;
    _tagOpticalAlarm *pOpticalAlarm; //指向光功率告警位
    _tagDevComm alarmDev;           //告警的设备

    char msg[SHORT_MSG_LEN];
    memset(msg, 0, sizeof(char));
    res_code = RET_SUCCESS;
    offset = sizeof(_tagDataHead);

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
    qDebug("opm alarm change frame %d card %d alarm_num %d change_num %d", \
           frame_no, card_no,alarm_num,changed_num);
    //检查机框号，槽位号是否正确
    checkFrameCardNo(frame_no, card_no, frame_state, card_state);
    if((frame_state +  card_state )!= RET_SUCCESS)
    {
        if(frame_state != FRAME_CARD_NO_OK)
            res_code =RET_UNMATCH_FRAME;
        else
            res_code = RET_UNMATCH_SLOT;
        sprintf(msg, "frame or state error frame %d card %d ", frame_no, card_no);
        goto usr_exit;
    }

    dev_type = m_subrackCard[frame_no].type[card_no];
    res_code = check_dev_type(frame_no, card_no, dev_type);
    if(res_code == RET_SUCCESS)
    {
        for(i = 0; i < changed_num; i++)
        {
            pOpticalAlarm = (_tagOpticalAlarm *)(buf + offset);
            offset += sizeof(_tagOpticalAlarm);
            alarmDev.port = pOpticalAlarm->port;
            res_code = check_dev_port_no(frame_no, card_no,pOpticalAlarm->port);
            if(res_code != RET_SUCCESS)
            {
                qDebug("opm changed alarm frame %d card %d port %d lev %d power %d time %s",\
                       frame_no, card_no,pOpticalAlarm->port,pOpticalAlarm->level,pOpticalAlarm->curPower,\
                       pOpticalAlarm->time);
                sprintf(msg, " rcv changed opm alarm port error port %d !", pOpticalAlarm->port);
                break;
            }
        }

    }
    else
    {
        sprintf(msg, "opm changed alrarm check type error  frame %d card %d", frame_no, card_no);

    }
usr_exit:
    if(res_code != RET_SUCCESS)
    {
        qDebug("%s", msg);
    }
    return res_code;
}
/*
   **************************************************************************************
 *  函数名称：input_cpy_sock_retry_queue
 *  函数描述：将需要重发的数据加入到重发队列中
 *  入口参数：buf 存放的是cmd---data
 *  返回参数：0，加入是否成功；其他，加入失败
 *  作者       ：
 *  日期       ：2015-10-28
 *  修改       ：增加判断，如果主网管不存在无须重发了
 **************************************************************************************
*/
int MainWindow::input_cpy_sock_retry_queue(const char buf[], int pkid)
{
    _tagSockDataCpy SockDatcpy;
    _tagDataHead *pDataHead;
    int data_bytes, offset, retv;
    int size_old, size_new;
    bool is_full;
    pDataHead = (_tagDataHead *)buf;
    offset = sizeof(_tagDataHead);
    //重发参数
    SockDatcpy.cmd = pDataHead->cmd;
    SockDatcpy.dst = ADDR_HOST_VIP;
    SockDatcpy.src = ADDR_MCU;
    SockDatcpy.pkid = pkid;
    SockDatcpy.retv = -1;
    SockDatcpy.timer = DATA_RETURN_WAIT_TIME;
    SockDatcpy.needed_retry_num = RETRY_NUM;
    SockDatcpy.retry_send_num = 0;


    //确定使用静态内存还是动态分配
    data_bytes = pDataHead->dataLen - sizeof(_tagDataHead);
    /*
     *2016-01-28 首先定义成固定的类型，因为数据长度有可能为0
     *此时不需要复制操作
    */
    SockDatcpy.buf_type = BUF_TYPE_FIXED;
    if(data_bytes > SOCK_DATA_CPY_BUF_LEN)
    {
        SockDatcpy.buf_type = BUF_TYPE_DYANM;
        SockDatcpy.buf_dyanm = new char [data_bytes];
        memcpy(SockDatcpy.buf_dyanm, buf + offset, data_bytes);
    }
    else if(data_bytes > 0)
    {
        memcpy(SockDatcpy.buf_fixed, buf + offset, data_bytes);
    }

    is_full = true;
    size_new = 0;

    //加入队列之前上锁，如果队列长度已满，则不加入
    pSockRetry->RetryList.obj.lock();
    size_old = pSockRetry->RetryList.list.size();
    if(pSockRetry->RetryList.list.size() < RETRY_LIST_LEN)
    {
        pSockRetry->RetryList.list.append(SockDatcpy);
        size_new =  pSockRetry->RetryList.list.size();
        is_full = false;
    }
    pSockRetry->RetryList.obj.unlock();



    //判断是否加入成功
    retv = -1;
    if(size_new > size_old)
    {
        retv = RET_SUCCESS;
    }
    else if(is_full)
    {
        size_old = pSockRetry->RetryList.list.size();
        size_new = size_old;
        printf("Retry List queue full, len %d \n", size_old);
    }
    else
    {
        printf("input retry list queue fail \n");
    }
    qDebug("input retry  queue len  new / old %d  / %d", \
           size_new, size_old);
    qDebug("input retry queue cmd 0x%x, data len %d dst %d pkid 0x%x", \
           SockDatcpy.cmd, data_bytes,SockDatcpy.dst,SockDatcpy.pkid);

    return retv;
}

/*
   **************************************************************************************
 *  函数名称：dealRcvCardOpmAlarm
 *  函数描述：收到变化的光功告警，首先检查参数是否合法，回应板卡，
 *  函数描述：加入到重发队列中去，查找数据库，找到对应的触发关系
 *  函数描述：，如果网络不存在，加入到短信发送队列中去。
 *  函数描述：模块级联表，启动otdr测试
 *  入口参数：
 *  返回参数：
 *  作者       ：
 *  日期       ：
 **************************************************************************************
*/
int MainWindow::dealRcvCardOpmAlarm(char buf[] ,void * ptr_opt)
{
    //告警类型，从机号，临时变量

    int offset;
    int res_code;

    int locale_index; //当前总告警在队列中的位置
    int  refresh_total_alarm;
    _tagDataHead *pDataHead;
    char msg[SHORT_MSG_LEN];
    tms_context *ptr_context;

    ptr_context = (tms_context *)(ptr_opt);
    refresh_total_alarm = 0;
    locale_index = 0;
    memset(msg, 0, sizeof(char));
    pDataHead = (_tagDataHead *)(buf);
    offset = sizeof(_tagDataHead);

    //首先检查信息是否正确
    res_code = check_rcv_opm_alarm_change(buf);
    //回应板卡
    mcu_Ack(ptr_opt, pDataHead->cmd,res_code);
    //2015-10-28  收到之后回应以下，然后跳出，不做出来，二期约定只发总的告警
    qDebug("rcv changed opm alarm cmd 0x%x res_code %d", pDataHead->cmd, res_code);

    return res_code;
}
int MainWindow::check_rcv_opm_alarm_total(const char buf[])
{
    //告警类型，从机号，临时变量
    int alarm_type, frame_no,i;
    int card_no, alarm_num;
    int offset, dev_type;
    int res_code;
    int frame_state ,card_state;

    _tagOpticalAlarm *pOpticalAlarm; //指向光功率告警位
    _tagDevComm alarmDev;           //告警的设备
    _tagDataHead *pDataHead;
    res_code = RET_SUCCESS;

    pDataHead = (_tagDataHead *)(buf);
    offset = sizeof(_tagDataHead);

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

    //检查机框号，槽位号是否正确
    checkFrameCardNo(frame_no, card_no, frame_state, card_state);
    if((frame_state +  card_state )!= RET_SUCCESS)
    {
        if(frame_state != FRAME_CARD_NO_OK)
            res_code =RET_UNMATCH_FRAME;
        else
            res_code = RET_UNMATCH_SLOT;
        printf(" total opm alarm error frame or state error frame %d card %d\n", frame_no, card_no);
        goto usr_exit;
    }

    dev_type = m_subrackCard[frame_no].type[card_no];
    res_code = check_dev_type(frame_no, card_no, dev_type);
    if(dev_type != OPM && dev_type != OLP) //设备类型如果不是OPM，OLP,出错
    {
        res_code = RET_PARAM_INVALID;
        goto usr_exit;
    }
    if(res_code == RET_SUCCESS)
    {
        for(i = 0; i < alarm_num; i++)
        {
            pOpticalAlarm = (_tagOpticalAlarm *)(buf + offset);
            offset += sizeof(_tagOpticalAlarm);
            alarmDev.port = pOpticalAlarm->port;
            res_code = check_dev_port_no(frame_no, card_no, pOpticalAlarm->port);
            if(res_code != RET_SUCCESS)
            {
                printf("opm total alarm frame %d card%d port %d \n",frame_no, card_no, pOpticalAlarm->port);
                break;
            }
        }

    }
    else
    {
        printf("opm total alrarm check type error  frame %d card %d \n", frame_no, card_no);
    }
usr_exit:
    return res_code;
}

/*
   **************************************************************************************
 *  函数名称：dealRcvCardOpmAlarmTotal
 *  函数描述：收到总的光功率告警
 *  入口参数：
 *  返回参数：
 *  作者       ：
 *  日期       ：
 **************************************************************************************
*/
int MainWindow::dealRcvCardOpmAlarmTotal(char buf[], void *ptr_opt)
{

    //告警类型，从机号，临时变量
    int alarm_type, frame_no,i;
    int card_no, alarm_num;
    int offset, dev_type;
    int res_code, j;
    bool is_new_total_alarm;
    bool is_deal;
    is_deal = false;
    is_new_total_alarm = false;

    _tagFSOpmAlarm LOpmAlarm;
    int locale_index; //当前总告警在队列中的位置
    int result, refresh_total_alarm;
    _tagOpticalAlarm *pOpticalAlarm; //指向光功率告警位
    _tagDevComm alarmDev;           //告警的设备
    _tagDataHead *pDataHead;


    char *alter_alarm_buf;

    unsigned int cmd;
    int  data_len, alter_alarm_num;
    char cur_time[TIME_STR_LEN];

    GetCurrentTime(cur_time);
    alter_alarm_buf = NULL;

    refresh_total_alarm = 0;
    locale_index = -1;
    res_code = RET_SUCCESS;
    //首先检查总的光功告警是否正确
    res_code = check_rcv_opm_alarm_total(buf);
    pDataHead = (_tagDataHead *)(buf);
    offset = sizeof(_tagDataHead);

    //回应板卡
    mcu_Ack(ptr_opt, pDataHead->cmd, res_code);

    //根据返回结果分开处理
    if(res_code != RET_SUCCESS)
    {
        goto usr_exit;
    }

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

    dev_type = m_subrackCard[frame_no].type[card_no];

    if(res_code == RET_SUCCESS)
    {
        alarmDev.frame_no = frame_no;
        alarmDev.card_no = card_no;
        alarmDev.type = dev_type;
        //找到总的告警对应的槽位
        result = -1;
        LOpmAlarm = get_fs_opm_alarm(frame_no, card_no,locale_index);
        //如果总的告警结构体不存在，需要创建
        if(locale_index == -1)
        {
            //新建一个总的告警结构体
            LOpmAlarm.frame = alarmDev.frame_no;
            LOpmAlarm.card = alarmDev.card_no;
            LOpmAlarm.type = alarmDev.type;
            LOpmAlarm.port_num = m_subrackCard[frame_no].ports[card_no];
            LOpmAlarm.cur_alarm_num = 0;
            res_code = new_fs_opm_alarm(LOpmAlarm);
            if(res_code == RET_SUCCESS)
            {
                //如果是新建的，肯定是加在了末尾
                LOpmAlarm = get_fs_opm_alarm(frame_no, card_no,locale_index);
                if(locale_index == -1) //如果不成功
                {
                    qDebug("new_fs_opm_alarm sucees, but out value fail ");
                    goto usr_exit;
                }
                is_new_total_alarm = true;
                /*
                qDebug("1 cur total  alarm %d ",LOpmAlarm.cur_alarm_num);
                */
            }
            else
            {
                qDebug("new_fs_opm_alarm error");
                goto usr_exit;
            }
        }
        //开始查找总的告警
        pOpticalAlarm = (_tagOpticalAlarm *)(buf + offset);
        for(j = 0; j < LOpmAlarm.port_num;j++ )
        {
            is_deal = false;
            LOpmAlarm.alarmArray[j].alarm_chang = 0;//之前有漏洞，必须先设置成没变化
            for(i = 0; i < alarm_num; i++)
            {
                //用该单元保存的告警遍历收到的总的告警
                if(j == pOpticalAlarm[i].port)
                {
                    is_deal = true;
                    //告警发生变化
                    if(pOpticalAlarm[i].level != LOpmAlarm.alarmArray[j].lev)
                    {
                        //如果之前的没有告警，那么告警次数++
                        if(LOpmAlarm.alarmArray[j].lev == ALARM_NONE)
                            LOpmAlarm.cur_alarm_num++;
                        LOpmAlarm.alarmArray[j].lev = pOpticalAlarm[i].level;
                        LOpmAlarm.alarmArray[j].power = pOpticalAlarm[i].curPower;
                        memcpy(LOpmAlarm.alarmArray[j].come_time, cur_time, TIME_STR_LEN);
                        LOpmAlarm.alarmArray[j].alarm_chang = 1;
                        refresh_total_alarm++;
                    }
                    break;
                }
            }
            //如果没有找到对应的端口，说明当前该端口无告警，如果之前有告警需要消警
            if(!is_deal && LOpmAlarm.alarmArray[j].lev != ALARM_NONE)
            {
                LOpmAlarm.alarmArray[j].lev = ALARM_NONE;
                memcpy(LOpmAlarm.alarmArray[j].fade_time, cur_time, TIME_STR_LEN);
                LOpmAlarm.alarmArray[j].alarm_chang = 1;
                LOpmAlarm.alarmArray[j].power =  0;
                refresh_total_alarm++;
                LOpmAlarm.cur_alarm_num--;
            }
        }

        if(refresh_total_alarm > 0 && locale_index > -1) //更新总的告警
        {
            //确保当前告警次数有效
            LOpmAlarm.cur_alarm_num =\
                    LOpmAlarm.cur_alarm_num < 0 ? 0 :  LOpmAlarm.cur_alarm_num;
            LOpmAlarm.cur_alarm_num = LOpmAlarm.cur_alarm_num > LOpmAlarm.port_num ?\
                        LOpmAlarm.port_num :  LOpmAlarm.cur_alarm_num;
            //更新以下总的告警
            total_opm_alarm.mutexBoj.lock();
            total_opm_alarm.OpmList.replace(locale_index, LOpmAlarm);
            total_opm_alarm.mutexBoj.unlock();
            //分配变化的告警存储空间cmd--data len----协议
            data_len = 8 + 20 + refresh_total_alarm*sizeof(_tagOpticalAlarm);
            alter_alarm_buf = new char [data_len];
            if(alter_alarm_buf == NULL)
                goto usr_exit;
            bzero(alter_alarm_buf, data_len);
            cmd = ID_ALARM_OPM_CHANGE;
            memcpy(alter_alarm_buf, &cmd, sizeof(cmd));
            offset = sizeof(cmd);
            memcpy(alter_alarm_buf + offset, &data_len, sizeof(data_len));
            offset += sizeof(data_len);

            alter_alarm_num = find_alter_alarm(&LOpmAlarm, alter_alarm_buf + offset);
            if(alter_alarm_num >  0)
                process_alter_alarm_from_total_opm_alarm(alter_alarm_buf);

        }
    }
    else
    {
        printf( "opm total alrarm check type error  frame %d card %d \n", frame_no, card_no);

    }
usr_exit:
    if(alter_alarm_buf != NULL)
        delete []alter_alarm_buf;
    /*
    qDebug("2 cur total  alarm %d ",LOpmAlarm.cur_alarm_num);
    */
    return res_code;
}
/*
   **************************************************************************************
 *  函数名称：get_fs_opm_alarm
 *  函数描述：获取某机框某槽位的光功告警
 *  入口参数：机框，槽位编号，从0开始
 *  返回参数：
 *  作者       ：
 *  日期       ：2015-10-20
 **************************************************************************************
*/
_tagFSOpmAlarm MainWindow::get_fs_opm_alarm(int frame, int card, int &index)
{
    _tagFSOpmAlarm LOpmAlarm;
    int i;
    index = -1;
    for(i = 0; i < total_opm_alarm.OpmList.size();i++)
    {
        //用[]访问速度快，不用担心访问速度慢了，前面的size()已经保证不会越界了
        if(total_opm_alarm.OpmList[i].frame == frame &&\
                total_opm_alarm.OpmList[i].card == card)
        {
            LOpmAlarm = total_opm_alarm.OpmList[i];
            index = i;
            break;
        }
    }
    return LOpmAlarm;
}

/*
   **************************************************************************************
 *  函数名称：process_alter_alarm_from_total_opm_alarm
 *  函数描述：在总的告警中发现变化的告警，首先发送网管，然后启动关联测量
 *  入口参数：
 *  返回参数：
 *  作者       ：
 *  日期       ：
 **************************************************************************************
*/
int MainWindow::process_alter_alarm_from_total_opm_alarm(char buf[])
{
    //告警类型，从机号，临时变量
    int alarm_type, i, fd;
    unsigned int  vip_host_addr;
    int changed_num,alarm_num;
    int offset, card_no, frame_no;
    int res_code, gsm_num;
    int nm_count, test_reason;
    QString str;
    char option[48];
    QByteArray byteArray;

    _tagOpticalAlarm *pOpticalAlarm; //指向光功率告警位
    _tagDevComm alarmDev;           //告警的设备

    glink_addr to_addr;

    //首先检查信息是否正确
    res_code = check_rcv_opm_alarm_change(buf);
    //告警类型
    offset = sizeof(_tagDataHead);
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
    if(res_code == RET_SUCCESS)
    {
        alarmDev.frame_no = frame_no;
        alarmDev.card_no = card_no;
        alarmDev.type = m_subrackCard[frame_no].type[card_no];
        //找到总的告警对应的槽位
        gsm_num = 0;
        for(i = 0; i < changed_num; i++)
        {
            pOpticalAlarm = (_tagOpticalAlarm *)(buf + offset);
            offset += sizeof(_tagOpticalAlarm);
            alarmDev.port = pOpticalAlarm->port;
            qDebug("opm changed alarm frame %d card %d port %d lev %d power %d time %s",\
                   frame_no, card_no,pOpticalAlarm->port,pOpticalAlarm->level,pOpticalAlarm->curPower,\
                   pOpticalAlarm->time);
            if(pOpticalAlarm->level == ALARM_NONE)
                test_reason = TEST_OPM_ALARM_AWAY;
            else
                test_reason = TEST_OPM_ALARM_COME;
            if(m_ctrlStat.NMstat != NM_EXIST)
            {
                gsm_num++;
                bzero(option, sizeof(option));
                str.setNum(pOpticalAlarm->curPower);
                str  =  tr("当前光功率值:") + str + tr(" ");
                byteArray = str.toLocal8Bit();
                strcpy(option, byteArray.data());
                input_arlarm_gsm_test_queue(alarmDev, pOpticalAlarm->level, alarm_type,test_reason,option);
            }
            else
            {
                input_arlarm_gsm_test_queue(alarmDev,pOpticalAlarm->level, alarm_type ,test_reason,NULL);
            }



        }
        nm_count = tms_ManageCount();
        if(nm_count <= 0)
        {
            printf("opm change alarm error nm count  %d \n", nm_count);
            goto usr_exit;
        }
        /*
        *如果主网管一直不存在，那么gsm_num == changed_num
        *如果主网管一直存在，gsm_num = 0
        *如果gsm_num != 0,且满足下面条件，说明,主网管总上面过程中发生了变化
        *部分告警会重复发送
        *2016-01-17 应该先加入重发队列，然后再发送。否则，如果网管的回应来的快，
        *但队列中还没有重发数据，那么就会出现逻辑错误
        */

        //即使主网管不存在，有其他网管，也要发送
        to_addr.src = ADDR_MCU;
        to_addr.pkid =  creat_pkid();
        to_addr.dst = ADDR_HOST_VIP;
        fd = tms_SelectFdByAddr(&to_addr.dst);
        to_addr.dst = ADDR_MASS_SEND;
        if(fd > 0)
        {
            input_cpy_sock_retry_queue(buf, to_addr.pkid);
        }
        offset = sizeof(_tagDataHead) + sizeof(int)*5;
        tms_AlarmOPMChange(0,&to_addr,frame_no, card_no,alarm_num,changed_num,\
                           (tms_alarm_opm_val *)(buf + offset));

    }
    else
    {
        printf("opm changed alrarm check type error  frame %d card %d \n", frame_no, card_no);

    }
usr_exit:

    return res_code;
}

/*
   **************************************************************************************
 *  函数名称：find_alter_alarm
 *  函数描述：从当前变化的告警中查找改变的告警
 *  入口参数：当前槽位的总的告警对象指针
 *  返回参数：
 *  作者       ：
 *  日期       ：
 **************************************************************************************
*/
int MainWindow::find_alter_alarm(_tagFSOpmAlarm *pFsOpmAlarm, char buf[])
{
    int alter_alarm_num;
    int i, offset, alarm_type;
    alter_alarm_num = 0;
    if(pFsOpmAlarm == NULL || buf == NULL)
        goto usr_exit;
    _tagOpticalAlarm *pOpticalAlarm;

    offset = 20; //预留头信息，如果有变化的告警方填入
    for(i = 0; i < pFsOpmAlarm->alarmArray.size();i++)
    {
        if(pFsOpmAlarm->alarmArray[i].alarm_chang == 1)
        {
            pOpticalAlarm = (_tagOpticalAlarm *)(buf + offset);
            offset += sizeof(_tagOpticalAlarm);
            pOpticalAlarm->port = i;
            pOpticalAlarm->level = pFsOpmAlarm->alarmArray[i].lev;
            pOpticalAlarm->curPower = pFsOpmAlarm->alarmArray[i].power;
            if(pOpticalAlarm->level == ALARM_NONE)
                memcpy(pOpticalAlarm->time, pFsOpmAlarm->alarmArray[i].fade_time, TIME_STR_LEN);
            else
                memcpy(pOpticalAlarm->time, pFsOpmAlarm->alarmArray[i].come_time, TIME_STR_LEN);
            alter_alarm_num++;
        }
    }
    //如果变化的告警数目大于0，填充头信息，frame ,card ,total alarm,alter alarm
    if(alter_alarm_num > 0)
    {
        alarm_type = ALARM_OPM;
        offset = 0;
        memcpy(buf, &alarm_type, sizeof(int));
        offset += sizeof(int);
        memcpy(buf + offset, &pFsOpmAlarm->frame, sizeof(int));
        offset += sizeof(int);
        memcpy(buf + offset, &pFsOpmAlarm->card, sizeof(int));
        offset += sizeof(int);
        memcpy(buf + offset, &pFsOpmAlarm->cur_alarm_num, sizeof(int));
        offset += sizeof(int);
        memcpy(buf + offset, &alter_alarm_num, sizeof(int));
        offset += sizeof(int);
        qDebug("find alter alarm, num %d frame %d card %d \n",alter_alarm_num,pFsOpmAlarm->frame,\
               pFsOpmAlarm->card);
    }
usr_exit:
    return alter_alarm_num;
}

/*
   **************************************************************************************
 *  函数名称：refresh_total_opm_alarm
 *  函数描述：将收到的告警与之前保存的告警进行比对
 *  入口参数：
 *  返回参数：0 代有变化，-1 没有变化，-2出错
 *  作者       ：
 *  日期       ：
 *  修改       ：由于处理逻辑改变，该函数不再使用
 **************************************************************************************
*/
int MainWindow::refresh_total_opm_alarm(_tagOpticalAlarm * pRcvOpmAlarm,  _tagFSOpmAlarm &LOpmAlarm, int port)
{
    _tagLOpmAlarmUit OpmAlarmPort;
    int res_code, cur_alarm_num;
    res_code = -2;
    if(port < 0 || port > LOpmAlarm.alarmArray.size()) //数目不对，返回
        goto usr_exit;
    res_code = RET_SUCCESS;
    OpmAlarmPort.lev = -1;
    OpmAlarmPort = LOpmAlarm.alarmArray.value(port, OpmAlarmPort);
    cur_alarm_num = LOpmAlarm.cur_alarm_num;
    if(OpmAlarmPort.lev == -1)
    {
        qDebug("refresh_total_opm_alarm out value error size %d port %d",LOpmAlarm.alarmArray.size() , port);
        res_code = -2;
        goto usr_exit;
    }
    if(OpmAlarmPort.lev != pRcvOpmAlarm->level) //发生变化
    {
        OpmAlarmPort.alarm_chang = 1;
        if(pRcvOpmAlarm->level == ALARM_NONE) //告警消失
        {
            OpmAlarmPort.lev = pRcvOpmAlarm->level;
            OpmAlarmPort.power = pRcvOpmAlarm->curPower;
            GetCurrentTime(OpmAlarmPort.fade_time);
            cur_alarm_num--;
        }
        else if(OpmAlarmPort.lev == ALARM_NONE) //告警产生
        {
            OpmAlarmPort.lev = pRcvOpmAlarm->level;
            GetCurrentTime(OpmAlarmPort.come_time);//告警产生时间
            bzero(OpmAlarmPort.fade_time, TIME_STR_LEN);
            OpmAlarmPort.power = pRcvOpmAlarm->curPower;
            cur_alarm_num++;
        }
        else//只是告警级别发生变化
        {
            OpmAlarmPort.lev = pRcvOpmAlarm->level;
            GetCurrentTime(OpmAlarmPort.come_time);//更新告警产生时间
            bzero(OpmAlarmPort.fade_time, TIME_STR_LEN);
            OpmAlarmPort.power = pRcvOpmAlarm->curPower;
            //cur_alarm_num++; //当前告警数目不发生改变
        }
        if(res_code == RET_SUCCESS) //本端口告警有变化，需要更新
        {
            //检查当告警端口数目是否有效
            cur_alarm_num = cur_alarm_num < 0 ? 0 : cur_alarm_num;
            cur_alarm_num = cur_alarm_num > LOpmAlarm.port_num ? LOpmAlarm.port_num : cur_alarm_num;
            if(cur_alarm_num != LOpmAlarm.cur_alarm_num)
                LOpmAlarm.cur_alarm_num = cur_alarm_num;
            LOpmAlarm.alarmArray.replace(port, OpmAlarmPort);
        }
    }
    else
    {
        OpmAlarmPort.alarm_chang = 0;
    }
usr_exit:
    return res_code;
}
/*
   **************************************************************************************
 *  函数名称：initial_fs_opm_alarm
 *  函数描述：如果某个槽位总的光功告警之前不存在，那么要新建一个
 *  入口参数：
 *  返回参数：
 *  作者       ：
 *  日期       ：
 **************************************************************************************
*/
int MainWindow:: new_fs_opm_alarm( _tagFSOpmAlarm LOpmAlarm)
{
    _tagLOpmAlarmUit opmAlarmUit;
    _tagFSOpmAlarm FSOpmAlarm;
    int i, size;
    int res_code;

    res_code = -1;
    FSOpmAlarm = LOpmAlarm;
    FSOpmAlarm.alarmArray.reserve(FSOpmAlarm.port_num);
    bzero(&opmAlarmUit, sizeof(_tagLOpmAlarmUit));
    //检查端口数目是否正确
    res_code = check_dev_port_no(LOpmAlarm.frame, LOpmAlarm.card, (LOpmAlarm.port_num - 1));
    if(res_code != RET_SUCCESS)
        goto usr_exit;
    //将槽位上的告警级别全部清零，
    FSOpmAlarm.alarmArray.reserve(LOpmAlarm.port_num);
    for(i = 0; i < LOpmAlarm.port_num;i++)
    {
        size = FSOpmAlarm.alarmArray.size();
        FSOpmAlarm.alarmArray.append(opmAlarmUit);
        if(size >= FSOpmAlarm.alarmArray.size())
        {
            FSOpmAlarm.alarmArray.clear();
            qDebug("port append opm alarm fail,frame %d card%d  type%d port_num %d",\
                   LOpmAlarm.frame, LOpmAlarm.card, LOpmAlarm.type, LOpmAlarm.port_num);
            res_code = -1;
            goto usr_exit;
        }
    }
    //首先保存修改前的大小
    size = total_opm_alarm.OpmList.size();
    total_opm_alarm.mutexBoj.lock();
    total_opm_alarm.OpmList.append(FSOpmAlarm);
    total_opm_alarm.mutexBoj.unlock();

    if(size < total_opm_alarm.OpmList.size())
    {
        res_code = RET_SUCCESS;
    }
    else //插入失败
    {
        qDebug("frame card append opm alarm fail,frame %d card%d  type%d port_num %d",\
               LOpmAlarm.frame, LOpmAlarm.card, LOpmAlarm.type, LOpmAlarm.port_num);
        qDebug("frame card append opm alarm fail,total_opm_alarm size %d",total_opm_alarm.OpmList.size());
    }

usr_exit:
    return res_code;

}
/*
   **************************************************************************************
 *  函数名称：db_save_total_opm_alarm
 *  函数描述：将当前的告警状态保存到数据库中
 *  入口参数：
 *  返回参数：
 *  作者       ：
 *  日期       ：
 **************************************************************************************
*/
int MainWindow::db_save_total_opm_alarm()
{
    int byte_size, count, i;
    int res_code, offset,temp;
    char  *buf;
    _tagFSOpmAlarm frameCardOpm;
    _tagLOpmAlarmUit *pOpmAlarmUit;
    int res_db;
    tdb_common_t input, mask;
    total_opm_alarm.mutexBoj.lock();
    count = total_opm_alarm.OpmList.size();
    byte_size = 0; //先遍历一遍，获取当前光功告警的数目
    buf = NULL;
    for(i = 0; i < count; i++)
    {
        frameCardOpm.frame = -1;
        frameCardOpm = total_opm_alarm.OpmList.value(i, frameCardOpm);
        if(frameCardOpm.frame == -1) //读取出错
        {
            qDebug("total_opm_alarm value at %d error", i);
            continue;
        }
        res_code = check_dev_port_no(frameCardOpm.frame, frameCardOpm.card, frameCardOpm.port_num - 1);
        if(res_code != RET_SUCCESS)
        {
            qDebug("db save total opm alarm error frame %d card %d port_num %d", \
                   frameCardOpm.frame, frameCardOpm.card, frameCardOpm.port_num);
            goto usr_exit;
        }
        byte_size += (sizeof(int) * 5) + sizeof(_tagLOpmAlarmUit) * frameCardOpm.alarmArray.size();
    }

    if(byte_size > 0 )
    {
        buf = new char [byte_size];
    }
    if( buf == NULL)
    {
        qDebug("db save total opm alarm allocal buf error size %d", byte_size);
        goto usr_exit;
    }
    //开始拷贝数据
    offset = 0;
    for(i = 0; i < count; i++)
    {
        frameCardOpm.frame = -1;
        frameCardOpm = total_opm_alarm.OpmList.value(i, frameCardOpm);
        if(frameCardOpm.frame == -1) //多心了，检查一下
        {
            qDebug("total_opm_alarm value at %d error", i);
            continue;
        }
        res_code = check_dev_port_no(frameCardOpm.frame, frameCardOpm.card, frameCardOpm.port_num - 1);
        if(res_code != RET_SUCCESS)
        {
            qDebug("db save total opm alarm error frame %d card %d port_num %d", \
                   frameCardOpm.frame, frameCardOpm.card, frameCardOpm.port_num);
            goto usr_exit;
        }
        memcpy(buf + offset, &frameCardOpm.frame, 5*sizeof(int)); //头信息
        offset += 5*sizeof(int);
        temp = frameCardOpm.alarmArray.size(); //希望端口数目与size相同
        pOpmAlarmUit = frameCardOpm.alarmArray.data();
        memcpy(buf + offset, pOpmAlarmUit, sizeof(_tagLOpmAlarmUit)*temp);
        offset += sizeof(_tagLOpmAlarmUit)*temp;
    }

    //开始保存数据库
    memset(&input, 0, sizeof(tdb_common_t));
    memset(&mask, 0, sizeof(tdb_common_t));

    input.val1 = DB_COMM_ID_OPM_ALARM;
    input.val2 = total_opm_alarm.OpmList.size(); //有几个opm
    mask.val1 = 1;
    input.lenpdata = byte_size;
    input.pdata = buf;
    res_db =  tmsdb_Delete_common(&input, &mask);
    if(res_db >= 0)
        res_db = tmsdb_Insert_common(&input, &mask, 1);
    if(res_db < 0)
    {
        qDebug("db save opm alarm error");
    }
usr_exit:
    total_opm_alarm.mutexBoj.unlock();
    if(buf != NULL) //释放分配的内存
        delete []buf;
    return res_code;
}
/*
   **************************************************************************************
 *  函数名称：  initial_total_opm_alarm
 *  函数描述：从数据库中读取保存的当前光功率告警
 *  入口参数：
 *  返回参数：
 *  作者       ：
 *  日期       ：
 **************************************************************************************
*/
int MainWindow:: initial_total_opm_alarm()
{
    int res_code, res_db;
    tdb_common read, mask;
    res_code = RET_SUCCESS;
    //开始读取数据库
    memset(&read, 0, sizeof(tdb_common_t));
    memset(&mask, 0, sizeof(tdb_common_t));

    read.val1 = DB_COMM_ID_OPM_ALARM;
    mask.val1 = 1;

    res_db = tmsdb_Select_common(&read, &mask, db_read_opm_alarm_commp, NULL);
    printf("%s(): Line : %d  res_db %d \n",  __FUNCTION__, __LINE__,res_db);
    return res_db;
}
/*
   **************************************************************************************
 *  函数名称： db_read_comm_record
 *  函数描述：读取通用表的函数，读取板卡组成信息，总的光功告警已经单独写好
 *  函数描述：故而不在此函数再次实现
 *  入口参数：
 *  返回参数：
 *  作者       ：
 *  日期       ：
 **************************************************************************************
*/
int MainWindow::db_read_comm_record(unsigned int ID)
{
    int res_code, res_db;
    tdb_common read, mask;
    res_code = RET_SUCCESS;
    //开始读取数据库
    memset(&read, 0, sizeof(tdb_common_t));
    memset(&mask, 0, sizeof(tdb_common_t));

    read.val1 = ID;
    mask.val1 = 1;
    switch(ID)
    {
    //读取OLP切换记录
    case DB_COMM_ID_OLP_ACTION:
    {
        res_db = tmsdb_Select_common(&read, &mask, db_read_olp_action_commp, &OlpActionRecordBuf);
        break;
    }
        //读取硬件告警记录
    case DB_COMM_ID_HW_ALARM:
    {
        res_db = tmsdb_Select_common(&read, &mask, db_read_hw_alarm_commp, &DevCommuState[0][0]);
        break;
    }
    default:
        res_db = -1;
        break;
    }
    return res_db;
}
/*
   **************************************************************************************
 *  函数名称：db_save_olp_switch_record
 *  函数描述：将OLP切换记录保存到数据库中
 *  入口参数：
 *  返回参数：
 *  作者       ：
 *  日期       ：2015-10-13
 **************************************************************************************
*/
int MainWindow::db_save_olp_switch_record()
{
    int byte_size, count, i;
    int res_code, offset;
    char  *buf;
    int res_db;
    tdb_common_t input, mask;
    //枷锁
    OlpActionRecordBuf.obj.lock();
    count = OlpActionRecordBuf.cur_total_record;//当前保存的记录数目
    byte_size = sizeof(_tagOlpActionRecordCell) * count;
    buf = new char [byte_size];

    if( buf == NULL)
    {
        qDebug("db save olp action allocal buf error size %d", byte_size);
        goto usr_exit;
    }
    //开始拷贝数据
    offset = 0;
    for(i = 0; i < count; i++)
    {
        memcpy(buf + offset,  &OlpActionRecordBuf.list[i], sizeof(_tagOlpActionRecordCell));
        offset += sizeof(_tagOlpActionRecordCell);
    }

    //开始保存数据库
    memset(&input, 0, sizeof(tdb_common_t));
    memset(&mask, 0, sizeof(tdb_common_t));

    input.val1 = DB_COMM_ID_OLP_ACTION;
    mask.val1 = 1;
    input.lenpdata = byte_size;
    input.pdata = buf;

    res_code = -1;
    res_db =  tmsdb_Delete_common(&input, &mask);

    if(res_db >= 0)
    {
        //设定当前索引号，总的记录数
        input.val2 = OlpActionRecordBuf.cur_index;
        input.val3 = OlpActionRecordBuf.cur_total_record;
        res_db = tmsdb_Insert_common(&input, &mask, 1);
        if(res_db >= 0)
            res_code = RET_SUCCESS;
    }
    if(res_db <= 0)
    {
        qDebug("db save olp action  error");
    }
usr_exit:
    //解锁
    OlpActionRecordBuf.obj.unlock();
    if(buf != NULL) //释放分配的内存
        delete []buf;
    return res_code;
}
/*
   **************************************************************************************
 *  函数名称：db_save_total_hw_alarm
 *  函数描述：将当前的总的硬件告警保存到数据库中,保存的也即是设备通信状态
 *  入口参数：
 *  返回参数：
 *  作者       ：
 *  日期       ：2015-10-15
 **************************************************************************************
*/
int MainWindow::db_save_total_hw_alarm()
{

    int res_code;
    //枷锁
    objHwAlarm.lock();
    int res_db;
    tdb_common_t input, mask;
    //开始保存数据库
    memset(&input, 0, sizeof(tdb_common_t));
    memset(&mask, 0, sizeof(tdb_common_t));

    input.val1 = DB_COMM_ID_HW_ALARM;
    mask.val1 = 1;

    res_code = -1;
    res_db =  tmsdb_Delete_common(&input, &mask);

    if(res_db >= 0)
    {
        input.lenpdata = sizeof(DevCommuState);
        input.pdata = DevCommuState;
        res_db = tmsdb_Insert_common(&input, &mask, 1);
        if(res_db >= 0)
            res_code = RET_SUCCESS;
    }
    if(res_db < 0)
    {
        qDebug("db save total hw alarm  error");
    }
usr_exit:
    objHwAlarm.unlock();

    return res_code;
}
/*
   **************************************************************************************
 *  函数名称：get_opm_alarm_from_buf
 *  函数描述：从数据库里面读取当前告警并进行初始化
 *  入口参数：
 *  返回参数：
 *  作者       ：
 *  日期       ：
 **************************************************************************************
*/
int MainWindow::get_opm_alarm_from_buf(char *buf, int len, int opm_num)
{
    int res_code,offset;
    _tagFSOpmAlarm DevOpm;
    total_opm_alarm.mutexBoj.lock();
    total_opm_alarm.OpmList.clear();
    total_opm_alarm.OpmList.reserve(opm_num);
    res_code = 0;
    offset = 0;
    while(offset < len)
    {
        memcpy(&DevOpm, buf + offset, 5*sizeof(int));
        offset += 5*sizeof(int); //首先拷贝frame, card type port_num,alarm_num
        res_code = check_dev_port_no(DevOpm.frame, DevOpm.card, DevOpm.port_num - 1);

        if(res_code == RET_SUCCESS)
        {
            DevOpm.alarmArray.resize(DevOpm.port_num);
            if(DevOpm.alarmArray.size() >= DevOpm.port_num)
            {
                memcpy(DevOpm.alarmArray.data(), buf + offset, DevOpm.port_num * sizeof(_tagLOpmAlarmUit));
                total_opm_alarm.OpmList.append(DevOpm);
            }
            else
            {
                DevOpm.alarmArray.clear();
            }
        }
        offset += DevOpm.port_num * sizeof(_tagLOpmAlarmUit);
    }
    total_opm_alarm.mutexBoj.unlock();
    return res_code;
}

/*
   **************************************************************************************
 *  函数名称：input_arlarm_gsm_test_queue
 *  函数描述：收到光功告警，加入测试队列，同时发送短信
 *                ：
 *  入口参数：
 *  返回参数：
 *  作者       ：
 *  日期       ：
 *  修改日期：2015-12-04
 *  修改内容：不再把测试端口填充到短信序列
 *                ：
 **************************************************************************************
*/
int MainWindow::input_arlarm_gsm_test_queue(_tagDevComm alarmDev, int alarm_lev, \
                                            int alarm_type, int test_reason, char msg[])
{
    int connect_num, i;
    int db_res;

    tdb_a_trigger_b_t db_check,mask;
    _tagCallbackPara callbackPara;
    //指向端口测量的指针，增加代码可读性
    _tagCtrlPortTest ctrlPortTest;

    //初始化查询信息
    bzero(&db_check, sizeof(tdb_a_trigger_b_t));
    bzero(&mask, sizeof(tdb_a_trigger_b_t));
    db_check.frame_a = alarmDev.frame_no;
    db_check.slot_a = alarmDev.card_no;
    db_check.type_a = alarmDev.type;
    db_check.port_a = alarmDev.port;

    mask.frame_a = alarmDev.frame_no;
    mask.slot_a = alarmDev.card_no;
    mask.type_a = alarmDev.type;
    mask.port_a = alarmDev.port;
    callbackPara.buf = NULL;

    callbackPara.operate_type = DB_GET_RECORD_NUM;
    connect_num = 0;
    bzero(&callbackPara, sizeof(_tagCallbackPara));
    db_res = tmsdb_Select_a_trigger_b(&db_check,&mask,db_read_a_trigger_b,(void *)&callbackPara);
    if(db_res >= RET_SUCCESS && callbackPara.record_num < 1000)
    {
        callbackPara.operate_type = DB_GET_RECORD;
        callbackPara.operate_num = 0;
        callbackPara.buf = new _tagDevComm[callbackPara.record_num];
        if(callbackPara.buf == NULL)
            goto usr_exit;

        db_res = tmsdb_Select_a_trigger_b(&db_check,&mask,db_read_a_trigger_b,(void *)&callbackPara);
        connect_num = callbackPara.record_num;
    }

    qDebug("connect num %d frame %d card %d port %d num %d", connect_num, \
           alarmDev.frame_no, alarmDev.card_no,alarmDev.port, callbackPara.record_num);

    if(connect_num > 0)
    {

        for(i = 0; i < connect_num; i++)
        {
            ctrlPortTest.test_port = callbackPara.buf[i];
            ctrlPortTest.ack_to = ACK_TO_NONE;
            ctrlPortTest.cmd = ID_GET_ALARM_TEST;
            /*
             *记录告警测试的原因，告警消失或者告警产生
            */
            /*
            if(alarm_lev == ALARM_NONE)
                ctrlPortTest.test_reason = TEST_OPM_ALARM_AWAY;
            else
                ctrlPortTest.test_reason = TEST_OPM_ALARM_COME;
            */
            ctrlPortTest.test_reason = test_reason;
            //加入告警测量的队列
            dispatch_test_port((void *)(&ctrlPortTest),OTDR_MOD_ALARM);

            /*
             *之前为什么有这一句，感到疑惑 2015-12-04
            */
            /*
            if(msg != NULL && m_ctrlStat.NMstat != NM_EXIST)
            {
                osw_addr = ctrlPortTest.test_port;
                input_gsm_queue(alarm_lev, alarm_type, &osw_addr, msg);
            }
            */
        }

    }
    //发短信，如果短信不空，就发短信
    if(msg != NULL && m_ctrlStat.NMstat != NM_EXIST)
    {
        input_gsm_queue(alarm_lev, alarm_type, &alarmDev, msg);
    }

usr_exit:
    //释放资源
    if(callbackPara.buf != NULL)
        delete []callbackPara.buf;
    return connect_num;
}

/*
   **************************************************************************************
 *  函数名称：checkFrameCardNo
 *  函数描述：检查槽位号和板卡号是否合法0，合法，1，槽位号
 *  入口参数：
 *  返回参数：
 *  作者       ：
 *  日期       ：
 **************************************************************************************
*/
void MainWindow::checkFrameCardNo(int frameNo, int cardNo, int &frameStat, int &cardStat)
{
    int state;
    //检查机框编号
    state = m_subrackInfo.onState;
    //是否超出范围
    if(frameNo < 0 || frameNo >= NUM_SUBRACK)
        frameStat  = FRAME_NO_OVERFLOW;
    //是否被配置过
    else if(1 != (1 & (state >> frameNo)))
        frameStat = FRAME_NO_CONFIG;
    else
        frameStat = FRAME_CARD_NO_OK;
    //槽位号是否超出范围
    if(cardNo < 0 || cardNo >= NUM_CARD)
        cardStat = CARD_NO_OVERFLOW;
    //检查槽位号是否被配置过
    else if(FRAME_CARD_NO_OK == frameStat)
    {
        state = m_subrackCard[frameNo].onState;
        if(1 == (1&(state >> cardNo)))
            cardStat = FRAME_CARD_NO_OK;
        else
            cardStat = CARD_NO_ONFIG;
    }
    else
        cardStat = CARD_NO_ONFIG;
    return;
}
/*
   **************************************************************************************
 *  函数名称：GetCurrentTime
 *  函数描述：获取系统时间
 *  入口参数：
 *  返回参数：
 *  作者       ：
 *  日期       ：
 **************************************************************************************
*/
void MainWindow::GetCurrentTime(char timeBuf[])
{
    time_t now;
    struct tm* ptr ;
    //    char szBuffer[64];
    const char* pFormat = "%Y-%m-%d %H:%M:%S";
    now = time(NULL);
    ptr = localtime(&now);
    strftime(timeBuf, 20, pFormat, ptr);
}
//网管查询序列码
int MainWindow::dealRcvNMAskSerilNum(char buf[], void *ptr_opt)
{
    glink_addr dst_addr;
    int res_code, fd;
    unsigned char sn[128];
    tms_context *ptr_context;
    _tagDataHead *pDataHead;
    res_code = RET_SUCCESS;
    pDataHead = (_tagDataHead*)(buf);
    //目的地址，源地址倒换，使用原来的pkid
    ptr_context = (tms_context *)ptr_opt;
    dst_addr.src = ADDR_MCU;
    dst_addr.dst = ptr_context->pgb->src;
    dst_addr.pkid = ptr_context->pgb->pkid;
    /*
    int row;
    tdb_sn_t input, mask;
    tdb_sn_t *ppout;

    bzero(&mask,  sizeof(tdb_sn_t));
    bzero(&input,  sizeof(tdb_sn_t));
    mask.sn[0] = 1;
    row = tmsdb_Select_sn(&input, &mask, &ppout);
    if(row  < 1)
    {
        res_code = RET_UNEXIST_ROW;
        mcu_Ack(ptr_opt,pDataHead->cmd, res_code);
    }
    else
    {
        fd = tms_SelectFdByAddr(&dst_addr.dst);
        memcpy(sn, &ppout[0].sn, sizeof(sn));
        tms_RetSerialNumber ( fd,&dst_addr,&sn);
        free(ppout);
    }
    */
    fd = tms_SelectFdByAddr(&dst_addr.dst);
    memcpy(sn, devCfg.sn, sizeof(sn));
    tms_RetSerialNumber ( fd,&dst_addr,&sn);
    return res_code;
}
/*
   **************************************************************************************
 *  函数名称：getRealCardForm
 *  函数描述：网管获取实时板卡信息,一期获取不到ols，mcu，pwu的真实信息
 *  入口参数：
 *  返回参数：
 *  作者       ：
 *  日期       ：
 *  修改日期：2015-10-29
 *  修改内容：从DevCommuState中获取，MCU，PW,均为实际情况
 **************************************************************************************
*/
int MainWindow::getRealCardForm(void *ptr_opt)
{
    int frameNum, devNum;
    int i, j, fd;
    int isHaveDev;
    char buf[CARD_COMPOSITION_SIZE];
    _tagCardAttribute *pCardAttribute;
    glink_addr addr;
    tms_context *ptr_context;
    //目的地址，源地址倒换，使用原来的pkid
    ptr_context = (tms_context *)ptr_opt;
    addr.src = ADDR_MCU;
    addr.dst = ptr_context->pgb->src;
    addr.pkid = ptr_context->pgb->pkid;
    //首先更新设备类型,用最新的打包上去， 为了与告警一致，暂时不用最新的
    // update_dev_type();
    //开始打包
    frameNum = 0;
    devNum = 0;
    bzero(buf, sizeof(buf));
    isHaveDev = 0;
    objHwAlarm.lock();
    for(i = 0; i < NUM_SUBRACK; i++)
    {
        frameNum++;
        isHaveDev = 0;
        for(j = 0; j < NUM_CARD; j++)
        {
            /*
             *2016-01-12
             *单板业务板不会上报插拔插拔状态，因此只要不是板卡处于拔出状态，均上报
            */
            if(DevCommuState[i][j].cur_type  != NONE && DevCommuState[i][j].card_state != PULL_OUT)
            {
                isHaveDev = 1;
                pCardAttribute = (_tagCardAttribute *)(buf + sizeof(_tagCardAttribute)*devNum);
                pCardAttribute->frameNo = i;
                pCardAttribute->cardNo = j;
                pCardAttribute->devType = DevCommuState[i][j].cur_type;
                pCardAttribute->ports = DevCommuState[i][j].cur_port;
                memcpy(pCardAttribute->opt, DevCommuState[i][j].cur_opt, sizeof(int)*CARD_OPT_NUM);
                devNum++;
            }
            //按照当前设计思路，MCU，PW，都可以正确给出，不需要人为处理2015-10-29
            /*
            //如果该机框有设备存在，那么返回主控和全部电源
            if(isHaveDev == 1&&j == NUM_COMM_CARD)
            {
                pCardAttribute = (_tagCardAttribute *)(buf + sizeof(_tagCardAttribute)*devNum);
                pCardAttribute->frameNo = i;
                pCardAttribute->cardNo = j;
                pCardAttribute->ports = dynamicDevtype[i].port_num[j];
                memcpy(pCardAttribute->opt, dynamicDevtype[i].opt[j], sizeof(int)*CARD_OPT_NUM);
                pCardAttribute->devType = MCU;
                devNum++;
            }
            else if(isHaveDev == 1&&j > NUM_COMM_CARD)
            {
                pCardAttribute = (_tagCardAttribute *)(buf + sizeof(_tagCardAttribute)*devNum);
                pCardAttribute->frameNo = i;
                pCardAttribute->cardNo = j;
                pCardAttribute->ports = dynamicDevtype[i].port_num[j];
                memcpy(pCardAttribute->opt, dynamicDevtype[i].opt[j], sizeof(int)*CARD_OPT_NUM);
                pCardAttribute->devType = PWU;
                devNum++;
            }
            */
        }

    }
    objHwAlarm.unlock();
    fd = tms_SelectFdByAddr(&addr.dst);
    tms_RetDeviceCompositionRT ( fd, &addr,devNum, (tms_dev_composition_val *)buf);
    return 0;
}

//查询已配置板卡组成
int MainWindow::getCfgCardForm( void *ptr_opt)
{
    int frameNum, devNum;
    int i, j, fd, state;
    char buf[CARD_COMPOSITION_SIZE];
    _tagCardAttribute *pCardAttribute;
    glink_addr addr;
    tms_context *ptr_context;
    //目的地址，源地址倒换，使用原来的pkid
    ptr_context = (tms_context *)ptr_opt;
    addr.src = ADDR_MCU;
    addr.dst = ptr_context->pgb->src;
    addr.pkid = ptr_context->pgb->pkid;
    bzero(buf, sizeof(buf));
    //开始打包
    frameNum = 0;
    devNum = 0;
    objOperateFrameCard.lock();
    for(i = 0; i < NUM_SUBRACK; i++)
    {
        state = m_subrackInfo.onState;
        //检查是否启用
        if(1 ==  ( 1&(state >> i))
                )
        {
            frameNum++;
            for(j = 0; j < NUM_CARD; j++)
            {
                state = m_subrackCard[i].onState;
                if(1 ==  ( 1 & (state >> j))
                        )
                {
                    pCardAttribute = (_tagCardAttribute *)(buf + sizeof(_tagCardAttribute)*devNum);
                    pCardAttribute->frameNo = i;
                    pCardAttribute->cardNo = j;
                    pCardAttribute->devType = m_subrackCard[i].type[j];
                    pCardAttribute->ports = m_subrackCard[i].ports[j];
                    pCardAttribute->opt[0] = m_subrackCard[i].opt[j][0];
                    devNum++;
                }
            }

        }
    }
    objOperateFrameCard.unlock();
    fd = tms_SelectFdByAddr(&addr.dst);
    tms_RetDeviceComposition ( fd, &addr,devNum, (tms_dev_composition_val *)buf);
    return 0;
}
/*
   **************************************************************************************
 *  函数名称：rcvConfirmCardForm
 *  函数描述：收到网管确认组成信息
 *  入口参数：
 *  返回参数：
 *  作者       ：
 *  日期       ：
 **************************************************************************************
*/
int MainWindow::rcvConfirmCardForm(char buf[], void *ptr_opt)
{
    int i, count, offset, state;
    int res_code, frame_no, card_no;
    char cur_time[TIME_STR_LEN];
    _tagSubrackCard cardForm[NUM_SUBRACK];
    _tagSubrackInfo subrackInfo;
    _tagCardAttribute *pcardForm;
    _tagDataHead *pDataHead;

    pDataHead = (_tagDataHead *)(buf);

    res_code = RET_SUCCESS;
    offset = sizeof(_tagDataHead);
    memcpy(&count, buf + offset, sizeof(int));
    if(count < 0 || count > NUM_SUBRACK*NUM_CARD)
    {
        res_code = RET_PARAM_INVALID;
        goto usr_exit;
    }
    bzero(cardForm, sizeof(_tagSubrackCard) * NUM_SUBRACK);
    bzero(&subrackInfo, sizeof(_tagSubrackInfo) );
    GetCurrentTime(cur_time); //获取当前系统时间
    subrackInfo.numTotal = NUM_SUBRACK;
    offset += sizeof(int);
    for(i = 0; i < count;i++)
    {
        pcardForm = (_tagCardAttribute *)(buf + offset);
        offset += sizeof(_tagCardAttribute) ;

        if(check_frame_card_range(pcardForm->frameNo, pcardForm->cardNo) < 0)
        {
            qDebug("range error frame %d card %d type%d", pcardForm->frameNo, pcardForm->cardNo, pcardForm->devType);
            res_code = RET_PARAM_INVALID;
            goto usr_exit;
        }
        else if(pcardForm->devType < m_ctrlStat.minDevType || pcardForm->devType > m_ctrlStat.maxDevType)
        {
            qDebug("type error frame %d card %d type %d", pcardForm->frameNo, pcardForm->cardNo, pcardForm->devType);
            res_code = RET_PARAM_INVALID;
            goto usr_exit;
        }
        else
        {
            cardForm[i].numTotal = NUM_CARD;
            frame_no = pcardForm->frameNo;
            card_no = pcardForm->cardNo;
            //设置类型，端口数目，选项
            cardForm[frame_no].type[card_no] = pcardForm->devType;
            cardForm[frame_no].ports[card_no] = pcardForm->ports;//
            memcpy(&cardForm[frame_no].opt[card_no],pcardForm->opt, sizeof(int)*CARD_OPT_NUM);
            //更新机框状态
            state = cardForm[frame_no].onState;
            if(((state >> card_no) & 1) == 0)
            {
                cardForm[frame_no].onState = (1 << card_no) |cardForm[frame_no].onState;
                cardForm[frame_no].numInUse++;
                memcpy(cardForm[frame_no].oprateTime, cur_time, sizeof(char)*TIME_STR_LEN);
            }
            //更新总的机框状态
            state = subrackInfo.onState;
            if(((state >> frame_no) & 1) == 0)
            {
                subrackInfo.onState = (1 << frame_no) |subrackInfo.onState ;
                subrackInfo.numInUse++;
                memcpy(subrackInfo.oprateTime, cur_time, sizeof(char)*TIME_STR_LEN);
            }
        }

    }
    if(res_code == RET_SUCCESS)
    {
        objOperateFrameCard.lock();
        memcpy(m_subrackCard, cardForm, sizeof(_tagSubrackCard)*NUM_SUBRACK);
        memcpy(&m_subrackInfo, &subrackInfo, sizeof(_tagSubrackInfo));
        res_code = wrtie_db_card_comp();
        objOperateFrameCard.unlock();
        InitialCardType();
        emit(updateDevType());
    }
usr_exit:
    mcu_Ack(ptr_opt, pDataHead->cmd,res_code);
    return res_code;
}

//收到Otdr曲线
int MainWindow::dealRcvCardOtdrCrv(char buf[], void * ptr_option)
{
    int res_code, fd, offset;
    _tagDataHead *pDataHead;
    _tagDevComm osw;
    _tagDevComm otdr;
    tsk_OtdrManage *pOtdrTsk;
    char cur_time[TIME_STR_LEN];
    _tagMeasurPara *pTestPara;
    int dst;


    unsigned short  pkid;
    tms_context *ptr_context;
    pDataHead = (_tagDataHead *)buf;
    res_code = RET_SUCCESS;

    ptr_context = (tms_context *)ptr_option;
    pkid = ptr_context->pgb->pkid; //命令唯一标示号
    GetCurrentTime(cur_time);
    /*
    *cmd dataLen
    *frame card type port (osw)
    *time (20B)
    *frame card type port (otdr)
    */
    offset =  8 + 16;
    memcpy(buf + offset, cur_time, TIME_STR_LEN);
    offset += TIME_STR_LEN;
    memcpy(&otdr, buf + offset , sizeof(_tagDevComm));
    offset += sizeof(_tagDevComm);
    pTestPara = (_tagMeasurPara *)(buf + offset);
    fd = -2;
    //目的地址不是MCU，即发送的目的地址是网管
    if(ptr_context->pgb->dst != ADDR_MCU)
        fd = tms_SelectFdByAddr(&ptr_context->pgb->dst);
    //目的地址是MCU，但源地址是板卡，并且参数的IP选项不为0，数据须发送给CTU
    else if(ptr_context->pgb->src == ADDR_CARD&&pTestPara->src_ip != 0)
    {
        fd = mcu_get_fd_by_ip(pTestPara->src_ip);
    }
    //源地址目的地址均为MCU，表明是CTU收到数据，但要确定该数据是须上报网管还是本地处理
    else if(ptr_context->pgb->src == ADDR_MCU && ptr_context->pgb->dst == ADDR_MCU)
    {
        dst = ctu_get_curv_dst(ptr_context->pgb->pkid);
        if(dst != ADDR_MCU)
            fd = tms_SelectFdByAddr((unsigned int *)&dst);
    }
    //通过fd的值是否发生改变判断是否需要发送
    if(fd != -2)
    {
        res_code = RET_SUCCESS;
        /*
         *2016-03-10 如果曲线是发送给网管，底层根据目的地址进行了发送
         *此处将来可能需要修改
        */
        if(fd > 0&&pTestPara->src_ip != 0)
        {
            /*
             *2016-03-10 在发送曲线前需要将地址位清零
            */
            pTestPara->src_ip = 0;
            mcu_send_otdr_curv_to_remote(fd,buf,ptr_option);
        }
        //转发完成，结束
        goto usr_exit;
    }

    /*
    *接下来处理 b----->e且src_ip == 0, RTU正常的接收模式
    *或者e----->e CTU工作模式 曲线接收方为本地
    */
    //下述周期测试，告警测试肯定是本机发起的，如果不是，根据前面判断已经发走了
    if(ID_RET_ALARM_TEST ==  pDataHead->cmd || ID_RET_OTDR_TEST_CYC == pDataHead->cmd)
    {
        tms_alarm_line_hdr hdr;
        AlarmCurv.mutexObj.lock();
        res_code = analyze_otdr_curv(&AlarmCurv.curv, buf);
        AlarmCurv.mutexObj.unlock();
        //检查错误，直接返回
        if(res_code != RET_SUCCESS)
            goto usr_exit;
        if(m_ctrlStat.NMstat == NM_EXIST)//网管存在，直接将曲线发送上去，网管不存在，则找告警，保存数据库
        {
            bzero(&hdr, sizeof(tms_alarm_line_hdr));
            if(ID_RET_ALARM_TEST ==  pDataHead->cmd )
                upload_alarm_cyc_data((void *)(&hdr), &AlarmCurv.curv,pDataHead->cmd,ptr_option);
        }
        else //查找告警
        {
            pOtdrTsk = NULL;
            //b------->e RTU
            if(ptr_context->pgb->src == ADDR_CARD)
                pOtdrTsk = (tsk_OtdrManage *)get_otdr_tsk(otdr);
            //e-------->e CTU
            else if(ptr_context->pgb->src == ADDR_MCU)
                pOtdrTsk = pCtuAsk;
            qDebug("rcv otdr curv pOtdrTsk 0x%x", pOtdrTsk);
            if(pOtdrTsk != NULL && pOtdrTsk->run_stat == TSK_NORMAL && pOtdrTsk->isAllocaResource == TSK_INITIAL_YES)
            {
                qDebug("rcv otdr curv emmit");
                pOtdrTsk->AlarmCurv.mutexObj.lock();
                memcpy(&pOtdrTsk->AlarmCurv.curv, &AlarmCurv.curv, sizeof(AlarmCurv.curv));
                pOtdrTsk->sendSignal(USR_EVENT_FIND_ALARM, pDataHead->cmd);
                pOtdrTsk->AlarmCurv.mutexObj.unlock();

            }
        }
    }
    //点名测量Otdr曲线，如果对话框打开，并且是otdr对话框
    else if(ID_RET_OTDR_TEST == pDataHead->cmd)
    {
        OtdrCurv.mutexObj.lock();
        res_code = analyze_otdr_curv(&OtdrCurv.curv, buf);
        memcpy(&osw, &OtdrCurv.curv.oswPort, sizeof(_tagDevComm));
        OtdrCurv.mutexObj.unlock();
    }
usr_exit:
    //通知tsk作相应的处理
    rcv_odtr_curv_ack(otdr, pkid, res_code);
    mcu_Ack(ptr_option,pDataHead->cmd, res_code);

    return 0;
}
/*
   **************************************************************************************
 *  函数名称：ctu_get_curv_dst
 *  函数描述：CTU对外测量时,实际上是点名测量,收到返回曲线，需要根据保存
 *                ：的dst地址进行处理
 *  入口参数：
 *  返回参数：
 *  作者       ：
 *  日期       ：
 *  修改日期：
 *  修改内容：
 *                ：
 **************************************************************************************
*/
int MainWindow::ctu_get_curv_dst(int pkid)
{
    int dst;
    int i;
    //说明该命令还处于有效状态
    dst = -1;
    if(pCtuAsk-> pObjSem[0].pkid == pkid)
    {
        pCtuAsk->waitCurvPortQue.obj.lock();
        for(i = 0; i < pCtuAsk->waitCurvPortQue.xlist.size();i++)
        {
            if(pkid == pCtuAsk->waitCurvPortQue.xlist[i].opt.pkid)
                dst = pCtuAsk->waitCurvPortQue.xlist[i].opt.src;
            break;

        }
        pCtuAsk->waitCurvPortQue.obj.unlock();
        //释放信号
        pCtuAsk-> pObjSem[0].objSem.release();
    }
    return dst;
}

/*
   **************************************************************************************
 *  函数名称：mcu_send_otdr_curv_to_remote
 *  函数描述：b----->3e(网管点名),b---->e(src_ip != 0 即0RTU ---->CTU)
 *                ：e------>e 都需要对外发送
 *  入口参数：
 *  返回参数：
 *  作者       ：
 *  日期       ：2015-11-06
 *  修改日期：
 *  修改内容：
 *                ：
 **************************************************************************************
*/
int MainWindow::mcu_send_otdr_curv_to_remote(int fd, char buf[], void * ptr_opt)
{
    int res_code,  offset;
    _tagDataHead *pDataHead;
    glink_addr dst_addr;
    struct tms_retotdr_test_hdr   *ptest_hdr;
    struct tms_retotdr_test_param *ptest_param;
    struct tms_retotdr_data_hdr   *pdata_hdr;
    struct tms_retotdr_data_val   *pdata_va;
    struct tms_retotdr_event_hdr  *pevent_hdr;
    struct tms_retotdr_event_val  *pevent_val;
    struct tms_retotdr_chain      *pchain;

    tms_context *ptr_context;
    pDataHead = (_tagDataHead *)buf;
    res_code = RET_SUCCESS;
    ptr_context = (tms_context *)ptr_opt;
    //通过fd的值是否发生改变判断是否需要发送


    res_code = RET_SUCCESS;
    if(fd > 0)
    {
        offset = 0;
        ptest_hdr = (struct tms_retotdr_test_hdr   *)(buf + offset);
        offset += sizeof(tms_retotdr_test_hdr);

        ptest_param = (struct tms_retotdr_test_param   *)(buf + offset);
        offset += sizeof(tms_retotdr_test_param);

        pdata_hdr = (struct tms_retotdr_data_hdr   *)(buf + offset);
        offset += sizeof(tms_retotdr_data_hdr);

        pdata_va = (struct tms_retotdr_data_val   *)(buf + offset);
        offset += sizeof(tms_retotdr_data_val);

        pevent_hdr = (struct tms_retotdr_event_hdr   *)(buf + offset);
        offset += sizeof(tms_retotdr_event_hdr);

        pevent_val = (struct tms_retotdr_event_val   *)(buf + offset);
        offset += sizeof(tms_retotdr_event_val);

        pchain = (struct tms_retotdr_chain   *)(buf + offset);
        offset += sizeof(tms_retotdr_chain);



        dst_addr.src = ADDR_MCU;
        dst_addr.dst = ptr_context->pgb->dst;
        dst_addr.pkid = ptr_context->pgb->pkid;
        tms_AnyRetOTDRTest(fd,&dst_addr,ptest_hdr,ptest_param,pdata_hdr,pdata_va,pevent_hdr,\
                           pevent_val,pchain,pDataHead->cmd);
    }

    return 0;
}

/*
   **************************************************************************************
 *  函数名称：db_save_osw_cyc_curv
 *  函数描述：保存周期性测量曲线，只保存曲线部分，
 *                    此函数由保存告警曲线函数代替
 *  入口参数：
 *  返回参数：
 *  作者       ：
 *  日期       ：
 **************************************************************************************
*/
int MainWindow::db_save_osw_cyc_curv(char buf[])
{
    int res_code, res_db;
    _tagOtdrCurv curv;
    tdb_otdr_his_data_t input;
    memset(&input, 0, sizeof(tdb_otdr_his_data_t));
    res_code = analyze_otdr_curv(&curv,  buf);
    res_db = -1;
    if(res_code == RET_SUCCESS)
    {
        input.ptest_hdr = (tms_retotdr_test_hdr *)(&curv.oswPort);
        input.ptest_param = (tms_retotdr_test_param*)(&curv.measurPara);
        input.pdata_hdr = (tms_retotdr_data_hdr*)(&curv.dpid);
        input.pdata_val = (tms_retotdr_data_val*)(curv.dataPt);
        input.pevent_hdr = (tms_retotdr_event_hdr*)(&curv.eventID);
        input.pevent_val = (tms_retotdr_event_val *)(&curv.eventBuf);
        input.pchain = (tms_retotdr_chain*)(&curv.measurResultID);
        res_db = tmsdb_Insert_otdr_his_data(&input,NULL,1);
        //qDebug("save cyc curv ! %d", res_db);
        if(res_db < 0)
            res_code = RET_IGNORE_SAVE;
    }
    qDebug("save cyc curv  res_code %d res_db %d ! ", res_code, res_db);
    return res_code;
}
/*
   **************************************************************************************
 *  函数名称：db_save_alarm_curv
 *  函数描述：断网情况下，opm告警触发的，周期性测量的曲线都需要保存
 *                    除了曲线之外，又增加了告警部分
 *  入口参数：
 *  返回参数：
 *  作者       ：
 *  日期       ：
 **************************************************************************************
*/
int MainWindow::db_save_alarm_curv(char buf[], char alarmBuf[])
{
    int res_code, res_db;
    _tagOtdrCurv *pcurv;
    tdb_otdr_alarm_data_t input;
    memset(&input, 0, sizeof(tdb_otdr_his_data_t));
    res_code = RET_SUCCESS;

    res_db = -1;
    pcurv = (_tagOtdrCurv *)buf;
    input.ptest_hdr = (tms_retotdr_test_hdr *)(&pcurv->oswPort);
    input.ptest_param = (tms_retotdr_test_param*)(&pcurv->measurPara);
    input.pdata_hdr = (tms_retotdr_data_hdr*)(&pcurv->dpid);
    input.pdata_val = (tms_retotdr_data_val*)(pcurv->dataPt);
    input.pevent_hdr = (tms_retotdr_event_hdr*)(&pcurv->eventID);
    input.pevent_val = (tms_retotdr_event_val *)(&pcurv->eventBuf);
    input.pchain = (tms_retotdr_chain*)(&pcurv->measurResultID);
    input.palarm = (tms_alarm_line_hdr *)alarmBuf;
    res_db = tmsdb_Insert_otdr_alarm_data(&input,NULL,1);
    if(res_db < 0)
        res_code = RET_IGNORE_SAVE;

    qDebug("save alarm curv  res_code %d res_db %d ! ", res_code, res_db);
    return res_code;
}
/*
   **************************************************************************************
 *  函数名称：analyze otdr curv
 *  函数描述：将otdr曲线从buf里面按照结构体赋值
 *  入口参数：
 *  返回参数：
 *  作者       ：
 *  日期       ：
 **************************************************************************************
*/
int MainWindow:: analyze_otdr_curv(_tagOtdrCurv *pcurv, char buf[])
{
    int offset, cpyed_bytes;
    int res_code;
    char msg[SHORT_MSG_LEN];
    glink_addr dst_addr;
    memset(msg, 0, sizeof(msg));
    res_code = RET_PARAM_INVALID;
    offset = sizeof(_tagDataHead);
    cpyed_bytes = sizeof(_tagDevComm) + 20 + sizeof(_tagDevComm) + sizeof(_tagMeasurPara) + 12\
            + sizeof(int);// osw, measurtime, otdr ,test para ,data id ,data num
    memcpy(pcurv, buf + offset, cpyed_bytes);
    offset += cpyed_bytes;

    if(pcurv->ptNum < 0 || pcurv->ptNum > MAX_PT_NUM || strcmp(pcurv->dpid, "OTDRData") != 0)
    {
        sprintf(msg, "rcv card curv data error data id %s, data num %d", pcurv->dpid, pcurv->ptNum);
        goto usr_exit;
    }

    cpyed_bytes = pcurv->ptNum * sizeof(unsigned short); //data pt
    memcpy(pcurv->dataPt, buf + offset, cpyed_bytes);
    offset += cpyed_bytes;

    cpyed_bytes = 12 + sizeof(int); //eventid event num
    memcpy(pcurv->eventID, buf + offset , cpyed_bytes);
    offset += cpyed_bytes;

    if(pcurv->eventNum < 1 || pcurv->eventNum > MAX_EVENT_NUM || strcmp(pcurv->eventID, "KeyEvents") != 0)
    {
        sprintf(msg, "rcv card curv event error data id %s, event num %d", pcurv->eventID, pcurv->eventNum);
        goto usr_exit;
    }

    cpyed_bytes = pcurv->eventNum * sizeof(_tagEvent);
    memcpy(pcurv->eventBuf, buf + offset, cpyed_bytes);
    offset += cpyed_bytes;

    cpyed_bytes = 20 + sizeof(_tagMeasurResult); //测量结果
    memcpy(pcurv->measurResultID, buf + offset, cpyed_bytes);

    if(strcmp(pcurv->measurResultID, "OTDRTestResultInfo") != 0)
    {
        sprintf(msg, "rcv card curv data error test result id %s", pcurv->measurResultID);
        goto usr_exit;
    }
    res_code = RET_SUCCESS;
usr_exit:
    if(res_code !=  RET_SUCCESS)
    {
        dst_addr.src = ADDR_MCU;
        dst_addr.dst = ADDR_NET_MANAGER;
        //tms_Trace(&dst_addr,msg, strlen(msg), 1);
        qDebug("%s", msg);
        memset(pcurv, 0, sizeof(_tagOtdrCurv));
    }
    return res_code;
}

//收到参考曲线
int MainWindow::dealRcvNMReferCrv(char buf[], void * ptr_opt)
{
    int offset, res_code,res_db;
    //要些数据库
    tdb_otdr_ref_t otdr_ref;
    tdb_otdr_ref_t mask;
    tms_otdr_ref_hdr mask_osw;
    glink_addr dst_addr;
    _tagDataHead *pDataHead;
    tms_context *popt;
    popt = (tms_context *)ptr_opt;

    pDataHead = (_tagDataHead *)(buf);
    offset = sizeof(_tagDataHead);
    //回应码的目的地址
    dst_addr.src =  ADDR_MCU;
    dst_addr.dst = popt->pgb->src;
    res_code = check_otdr_ref_curv(buf);

    if(res_code == RET_SUCCESS)
    {
        //设置光开关的屏蔽项
        mask_osw.osw_frame = 1;
        mask_osw.osw_port = 1;
        mask_osw.osw_slot = 1;
        mask_osw.osw_type = 1;
        memset(&mask, 0, sizeof(tdb_otdr_ref_t));
        memset(&otdr_ref, 0, sizeof(tdb_otdr_ref_t));
        mask.pref_hdr = &mask_osw;
        pDataHead = (_tagDataHead *)(buf);
        offset = sizeof(_tagDataHead);
        //光开关的头
        otdr_ref.pref_hdr = (tms_otdr_ref_hdr*)(buf + offset);
        offset += sizeof(tms_otdr_ref_hdr);
        //测量参数
        otdr_ref.ptest_param = (tms_retotdr_test_param*)(buf + offset);
        offset += sizeof(tms_retotdr_test_param);
        //data id count
        otdr_ref.pdata_hdr = (tms_retotdr_data_hdr*)(buf + offset);
        offset += sizeof(tms_retotdr_data_hdr);
        //data buf
        otdr_ref.pdata_val = (tms_retotdr_data_val*)(buf + offset);
        offset += otdr_ref.pdata_hdr->count * sizeof(short int);
        //事件点 id count
        otdr_ref.pevent_hdr = (tms_retotdr_event_hdr*)(buf + offset);
        offset += sizeof(tms_retotdr_event_hdr);
        //事件点buf
        otdr_ref.pevent_val = (tms_retotdr_event_val*)(buf + offset);
        offset += sizeof(tms_retotdr_event_val) *otdr_ref.pevent_hdr->count ;
        //测量结果
        otdr_ref.pchain = (tms_retotdr_chain*)(buf + offset);
        offset += sizeof(tms_retotdr_chain);
        //告警门限
        otdr_ref.pref_data = (tms_cfg_otdr_ref_val*)(buf + offset);
        offset += sizeof(tms_cfg_otdr_ref_val);
        res_db = tmsdb_Delete_otdr_ref(&otdr_ref,& mask);
        if(res_db > -1)
            res_db = tmsdb_Insert_otdr_ref(&otdr_ref,& mask, 1);

        if(res_db > -1)
            res_code = RET_SUCCESS;
        else
            res_code = RET_IGNORE_SAVE;

    }
    if(res_code != RET_SUCCESS)
        qDebug("Insert otdr ref curv error res_code %d", res_code);
    dst_addr.pkid = popt->pgb->pkid;
    mcu_Ack(ptr_opt,pDataHead->cmd, res_code);
    return res_code;
}
/*
   **************************************************************************************
 *  函数名称：db_del_otdr_refer_curv
 *  函数描述：逐条或者全部删除记录
 *  入口参数：
 *  返回参数：
 *  作者       ：
 *  日期       ：
 **************************************************************************************
*/
int MainWindow::db_del_otdr_refer_curv(char buf[], void *ptr_opt)
{

    tdb_otdr_ref_t cfg_refotdr, mask_refotdr;
    tms_otdr_ref_hdr cfg_hdr, mask_hdr;

    glink_addr dst_addr;
    _tagDataHead *pDataHead;
    _tagDevComm *pDelDev;
    tms_context *popt;
    int res_code, count, i;
    int offset, res_db, fd;
    popt = (tms_context *)ptr_opt;
    dst_addr.src =  ADDR_MCU;
    dst_addr.dst = popt->pgb->src;
    pDataHead = (_tagDataHead *)(buf);

    //先初始化
    memset(&cfg_refotdr, 0, sizeof(tdb_otdr_ref_t));
    memset(&mask_refotdr, 0, sizeof(tdb_otdr_ref_t));

    memset(&cfg_hdr, 0, sizeof(tms_otdr_ref_hdr));
    memset(&mask_hdr, 0, sizeof(tms_otdr_ref_hdr));
    cfg_refotdr.pref_hdr = & cfg_hdr;
    mask_refotdr.pref_hdr = &mask_hdr;
    res_code = RET_PARAM_INVALID;
    if(pDataHead->cmd == ID_DELALL_TBOTDRREFDATA)
    {
        res_code = RET_SUCCESS;
        //mask_refotdr全零意味着全部删除
        res_db = tmsdb_Delete_otdr_ref(&cfg_refotdr,&mask_refotdr);
        if(res_db < 0)
        {
            res_db = RET_UNEXIST_ROW;
        }
    }
    else
    {
        res_code = RET_SUCCESS;
        //按照机框，槽位，类型，端口来删除
        mask_hdr.osw_frame = 1;
        mask_hdr.osw_slot = 1;
        mask_hdr.osw_type = 1;
        mask_hdr.otdr_port = 1;
        offset = sizeof(_tagDataHead);
        memcpy(&count, buf + offset, sizeof(int));
        offset += sizeof(int);
        for(i = 0; i < count;i++)
        {
            pDelDev = (_tagDevComm *)(buf + offset);
            offset += sizeof(_tagDevComm);
            memcpy(&cfg_hdr, pDelDev, sizeof(_tagDevComm));
            res_db = tmsdb_Delete_otdr_ref(&cfg_refotdr,&mask_refotdr);
            //如果某条删除失败，不跳出，继续删除，但记录下发生过错误
            if(res_db < 0)
                res_code = RET_UNEXIST_ROW;
        }
    }
    //mcu回应码
    dst_addr.pkid = popt->pgb->pkid;
    mcu_Ack(ptr_opt,pDataHead->cmd, res_code);
    return res_code;
}

/*
   **************************************************************************************
 *  函数名称：mcu_Ack
 *  函数描述：回应板卡或者网管
 *                ：
 *  入口参数：opt 点名测量的时候填充正在测量的时间
 *  返回参数：
 *  作者       ：
 *  日期       ：2015-11-16
 *  修改日期：
 *  修改内容：
 *                ：
 **************************************************************************************
*/
int MainWindow:: mcu_Ack(void * ptr_opt, unsigned int res_cmd, int res_code,int opt)
{
    int fd;
    tms_ack ack;
    glink_addr dst_addr;
    tms_context *popt;
    popt = (tms_context *)ptr_opt;
    dst_addr.dst = popt->pgb->src;
    dst_addr.src = ADDR_MCU;
    dst_addr.pkid = popt->pgb->pkid;
    //fd = tms_SelectFdByAddr(&dst_addr.dst);
    fd = popt->fd;
    bzero(&ack, sizeof(tms_ack));
    ack.cmdid = res_cmd;
    ack.errcode = res_code;
    ack.reserve1 = MCU_CARD&0x0000FFFF;
    ack.reserve1 = (MCU_FRAME<<16&0xFFFF0000)|ack.reserve1;
    ack.reserve2 = MCU<<16&0xFFFF0000;
    ack.reserve3 = opt;
    tms_AckEx(fd, &dst_addr,&ack);
#if USR_DEBUG
    qDebug("mcu_ack fd %d src 0x%x dst 0x%x res_code %d cmd 0x%x",\
           fd, dst_addr.src, dst_addr.dst, res_code, res_cmd);
#endif
    return 0;
}

/*
   **************************************************************************************
 *  函数名称：check_otdr_ref_curv
 *  函数描述：检查参考曲线是否合法:检查osw frame card port type 结构体中的id
 *  入口参数：
 *  返回参数：
 *  作者       ：
 *  日期       ：
 **************************************************************************************
*/
int MainWindow::check_otdr_ref_curv(char buf[])
{
    int offset, res_code;
    char msg[SHORT_MSG_LEN];
    tdb_otdr_ref_t otdr_ref;
    memset(&otdr_ref, 0, sizeof(tdb_otdr_ref_t));
    offset = sizeof(_tagDataHead);
    otdr_ref.pref_hdr = (tms_otdr_ref_hdr*)(buf + offset);
    offset += sizeof(tms_otdr_ref_hdr);
    //检查类型是否是osw
    res_code = check_dev_type(otdr_ref.pref_hdr->osw_frame, otdr_ref.pref_hdr->osw_slot,\
                              otdr_ref.pref_hdr->osw_type);

    //检查端口号
    if(res_code == RET_SUCCESS)
    {
        res_code = check_dev_port_no(otdr_ref.pref_hdr->osw_frame, otdr_ref.pref_hdr->osw_slot,\
                                     otdr_ref.pref_hdr->osw_port); //检查端口号
    }
    else
    {
        memset(msg, 0, sizeof(msg));
        sprintf(msg, "check otdr  ref  type error, frame %d card %d type %d port %d",\
                otdr_ref.pref_hdr->osw_frame, otdr_ref.pref_hdr->osw_slot,otdr_ref.pref_hdr->osw_type,otdr_ref.pref_hdr->osw_port);
        goto usr_exit;
    }
    //数据点的id count
    if(res_code == RET_SUCCESS)
    {
        otdr_ref.ptest_param = (tms_retotdr_test_param *)(buf + offset);
        offset += sizeof(tms_retotdr_test_param);
        otdr_ref.pdata_hdr = (tms_retotdr_data_hdr *)(buf + offset);
        offset += sizeof(tms_retotdr_data_hdr);
        if(strcmp((char *)(otdr_ref.pdata_hdr->dpid), "OTDRData") == 0 && otdr_ref.pdata_hdr->count > 0&&\
                otdr_ref.pdata_hdr->count <= MAX_PT_NUM)
        {
            otdr_ref.pdata_val = (tms_retotdr_data_val*)(buf + offset);
            offset += otdr_ref.pdata_hdr->count * sizeof(short int);
            res_code = RET_SUCCESS;
        }
        else
        {
            memset(msg, 0, sizeof(msg));
            sprintf(msg, "check otdr  ref  data id error,count %d id %s",otdr_ref.pdata_hdr->count,otdr_ref.pdata_hdr->dpid);
            res_code = RET_PARAM_INVALID;
            goto usr_exit;

        }
    }
    else
    {
        memset(msg, 0, sizeof(msg));
        sprintf(msg, "check otdr  ref  port error, frame %d card %d type %d port %d",\
                otdr_ref.pref_hdr->osw_frame, otdr_ref.pref_hdr->osw_slot,otdr_ref.pref_hdr->osw_type,otdr_ref.pref_hdr->osw_port);
        goto usr_exit;
    }
    //检查数据点的事件类型
    if(res_code == RET_SUCCESS)
    {
        otdr_ref.pevent_hdr = (tms_retotdr_event_hdr *)(buf + offset);
        offset += sizeof(tms_retotdr_event_hdr);
        if(strcmp((char *)otdr_ref.pevent_hdr->eventid, "KeyEvents") == 0 && otdr_ref.pevent_hdr->count > 0&&\
                otdr_ref.pevent_hdr->count <= MAX_EVENT_NUM)
        {
            otdr_ref.pevent_val = (tms_retotdr_event_val *)(buf + offset);
            offset += sizeof(tms_retotdr_event_val)* otdr_ref.pevent_hdr->count;
        }
        else
        {
            memset(msg, 0, sizeof(msg));
            sprintf(msg, "check otdr  ref  event  id  or count error, count %d id%s",otdr_ref.pevent_hdr->count,\
                    otdr_ref.pevent_hdr->eventid);
            res_code = RET_PARAM_INVALID;
            goto usr_exit;
        }
    }

    //检查测量结果
    if(res_code == RET_SUCCESS)
    {
        otdr_ref.pchain = (tms_retotdr_chain *)(buf + offset);
        offset += sizeof(tms_retotdr_chain);
        if(strcmp((char *)otdr_ref.pchain->inf, "OTDRTestResultInfo") == 0)
            res_code = RET_SUCCESS;
        else
        {
            memset(msg, 0, sizeof(msg));
            sprintf(msg, "check otdr  ref  test result  id error,  id %s",otdr_ref.pchain->inf );
            res_code = RET_PARAM_INVALID;
            goto usr_exit;
        }
    }
    //检查告警门限
    if(res_code == RET_SUCCESS)
    {
        otdr_ref.pref_data = (tms_cfg_otdr_ref_val *)(buf + offset);
        offset += sizeof(tms_cfg_otdr_ref_val);
        if(otdr_ref.pref_data->leve2 > 0 && otdr_ref.pref_data->leve1 > otdr_ref.pref_data->leve2&&\
                otdr_ref.pref_data->leve0 > otdr_ref.pref_data->leve1)
            res_code = RET_SUCCESS;
        else
        {
            memset(msg, 0, sizeof(msg));
            sprintf(msg, "check otdr  ref  alarm lev error,  leve0 %d leve1 %d leve2 %d",otdr_ref.pref_data->leve0, otdr_ref.pref_data->leve1,
                    otdr_ref.pref_data->leve2);
            res_code = RET_PARAM_INVALID;
            goto usr_exit;
        }
    }

usr_exit:    if(res_code != RET_SUCCESS)
        qDebug("%s", msg);
    return res_code;
}

/*
   **************************************************************************************
 *  函数名称：isStartCycleTest
 *  函数描述：检查是否开始周期性测量，如果开始，那么将周期性测量
 *  函数描述：如果开始，加入周期性测量队列，同时将下一次进行周期性测
 *  函数描述：开始的时间写入数据库。
 *  入口参数：
 *  返回参数：0，没开始周期性测量，1开始周期性测量
 *  日期版本：
 *  作者       ：
 **************************************************************************************
*/
int MainWindow::isStartCycleTest(_tagNextCycleTest &cycleTest)
{
    time_t now;
    int spaceHour, res_code;
    _tagDevComm test_port;
    int mod;
    _tagCtrlPortTest ctrlPortTest;
    mod = OTDR_MOD_CYC;
    ctrlPortTest.ack_to = ACK_TO_NONE;
    ctrlPortTest.cmd = ID_GET_OTDR_TEST_CYC;
    ctrlPortTest.test_reason = TEST_CYC;

    now = time(NULL);
    spaceHour =  now - cycleTest.next;
    res_code = 0;
    if(spaceHour >= 0 )
    {
        //cycleTest.next = now + cycleTest.cycle*3600;
        cycleTest.next = now + cycleTest.cycle*60;
        //测试端口信息
        test_port.frame_no = cycleTest.framNo;
        test_port.card_no = cycleTest.cardNo;
        test_port.port = cycleTest.portNo;
        test_port.type = cycleTest.devType;

        //检查设备类型，设备端口
        res_code = check_dev_type(test_port.frame_no, test_port.card_no,test_port.type);
        if(res_code != RET_SUCCESS)
            goto usr_exit;
        res_code = check_dev_port_no(test_port.frame_no, test_port.card_no,test_port.port);
        if(res_code != RET_SUCCESS)
            goto usr_exit;
        memcpy(&ctrlPortTest.test_port, &test_port, sizeof(_tagDevComm));
        //查找到合适的端口并加入到对应的队列
        dispatch_test_port((void *)&ctrlPortTest,  mod);
        qDebug("insert cyc queque");
        res_code = 1;
    }
usr_exit:
    return res_code;
}
/*
   **************************************************************************************
 *  函数名称：dispatch_test_port
 *  函数描述：将要测试的端口加入对应的otdr对应的端口队列
 *  入口参数：需要测试的端口，osw，olp,otdr
 *  返回参数：负值为求反为错误码，正值为当前测量时间
 *  作者       ：
 *  日期       ：
 *  修改日期：2015-11-04
 *  修改内容：支持CTU模式的级联路由，其特点：倒数第二个B设备ip非本地
 *                ：切设备类型为osw,或者otdr;RTU特点：最后一个设备是OTDR
 **************************************************************************************
*/
int MainWindow::dispatch_test_port(void *pTestModel, int mod)
{
    int res_code;
    _tagDevComm endDev;
    _tagDevComm testPort;
    tsk_OtdrManage *pOtdrTsk;
    bool bresult;
    _tagCtrlPortTest *pCtrlPortTest;
    _tagCtrlAppointTest *pCtrlAppointTest;
    _tagOswRout rout;
    _tagOswRoutUnit otdrInfo;
    int res_db;
    res_code = 0 - RET_MODULE_ROUTE_NO_EXIST;
    bresult = false;
    res_db = 0;
    if(mod == OTDR_MOD_CYC || mod == OTDR_MOD_ALARM)//断网时周期测试和告警测试，不带参数
    {
        pCtrlPortTest = (_tagCtrlPortTest *)pTestModel;
        memcpy(&testPort, &pCtrlPortTest->test_port, sizeof(_tagDevComm));
    }
    else if(mod == OTDR_MOD_APPOINT)//网管发起点名测量，周期测量，带端口和参数
    {
        pCtrlAppointTest = (_tagCtrlAppointTest *)pTestModel;
        memcpy(&testPort, &pCtrlAppointTest->test_port, sizeof(_tagDevComm));
    }
    else
    {
        printf("dispatch test mod ill mod %d \n",mod);
        goto usr_exit;
    }
    /*
     *查询整条路由,如果返回成功,根据最末端模块的ip判断是否是本地模块
     *如果不是本地模块，将其加入到特殊的任务中，此种模式认为是CTU
     *如果是本地模块，加入到本地
     *otdr管理任务中该模式是RTU
    */
    res_code = db_get_test_port_rout(testPort,&rout);
    if(res_code == RET_SUCCESS)
    {
        //如果出现此错误，是程序逻辑设计错误，重点检查程序
        if(rout.depth < 1)
        {
            res_code = 0 - RET_MODULE_ROUTE_ILLEGALT;
            printf("dispatch test port rout.depth erro %d \n!", rout.depth);
        }

        otdrInfo = rout.table_buf[rout.depth - 1];
        endDev.frame_no = otdrInfo.Dev.frame_no;
        endDev.card_no = otdrInfo.Dev.card_no;
        endDev.port = otdrInfo.Dev.port;
        endDev.type = otdrInfo.Dev.type;
        //本地ip，最有一个设备必须是OTDR否则非法
        if(otdrInfo.dstIP == 0)
        {
            if(otdrInfo.Dev.type != OTDR)
            {
                res_code = 0 - RET_MODULE_ROUTE_ILLEGALT;
                printf("dispatch test port rout illegalt! \n");
                goto usr_exit;
            }
        }
        else
        {
            //加入到CTU_ASK
            if(otdrInfo.Dev.type == OSW ||otdrInfo.Dev.type == OTDR )
            {
                res_code = pCtuAsk->input_test_queue(pTestModel, mod,endDev.port);;
                goto usr_exit;
            }
            else
            {
                res_code = 0 - RET_MODULE_ROUTE_ILLEGALT;
                printf("dispatch test port rout illegalt! \n");
                goto usr_exit;
            }
        }
    }
    else
    {
        res_code = 0 - res_code;
        goto usr_exit;
    }


    //查找otdr及端口
    pOtdrTsk = (tsk_OtdrManage* ) get_otdr_tsk(endDev);
    if(pOtdrTsk != NULL && endDev.port >= 0 && endDev.port < pOtdrTsk->otdrAddr.port)
    {
        //如果是点名测量，并且点名测量队列已经存在一个等待测试，那么直接返回已存在一个测试命令
        if(mod == OTDR_MOD_APPOINT&&!pOtdrTsk->pAppointTstQue[endDev.port].xqueue.isEmpty())
        {
            res_code =  0 - RET_WAIT;
            goto usr_exit;
        }
        //其他返回正在测试时间
        res_code = pOtdrTsk->input_test_queue(pTestModel, mod,endDev.port);
    }
    else //返回板卡（otdr）不存在
    {
        res_code = 0 - RET_MODULE_ROUTE_NO_EXIST;
    }

usr_exit:
    qDebug("input test queue cyc test %d", res_code);
    return res_code;
}

/*
   **************************************************************************************
 *  函数名称：db_save_cyc_time
 *  函数描述：如果到链周期性测量时间，就将下一次周期性测量时间保存到数据库
 *  入口参数：
 *  返回参数：
 *  作者       ：
 *  日期       ：
 **************************************************************************************
*/
int MainWindow::db_save_cyc_time()
{
    tdb_osw_cyc_bak_t cyc_time, mask;
    _tagNextCycleTest nextCycleTest;
    int i, res_db;
    bzero(&mask, sizeof(tdb_osw_cyc_t));
    bzero(&cyc_time, sizeof(tdb_osw_cyc_t));
    /*
     *2016-02-04 如果当前还没有开始周期性测量那么就不用保存
    */
    if(vectorCycleTest.xvector.size() <= 0)
        goto usr_exit;

    res_db = tmsdb_Delete_osw_cyc_bak(&cyc_time, &mask);
    if(res_db < 0)
        goto usr_exit;

    mask.frame = 1;
    mask.slot = 1;
    mask.type = 1;
    mask.port = 1;
    vectorCycleTest.objMutex.lock();

    for (i = 0; i < vectorCycleTest.xvector.size();i++)
    {
        nextCycleTest = vectorCycleTest.xvector.at(i);
        memcpy(&cyc_time.frame, &nextCycleTest, sizeof(_tagNextCycleTest));

        if(res_db >= 0)
            tmsdb_Insert_osw_cyc_bak(&cyc_time, &mask, 1);
        if(res_db < 0)
            break;
    }
    vectorCycleTest.objMutex.unlock();
usr_exit:
    printf("%s(): Line : %d  list size  %d \n",  \
           __FUNCTION__, __LINE__,vectorCycleTest.xvector.size());
    return res_db;
}

/*
   **************************************************************************************
 *  函数名称：insepectCycleTime
 *  函数描述：检查周期性测量是否到时间，每30秒检查一次，精确到分钟
 *  入口参数：
 *  返回参数：
 *  作者       ：
 *  日期       ：
 **************************************************************************************
*/
int MainWindow::insepectCycleTime()
{
    int i;
    _tagNextCycleTest nextCycleTest;

    if(m_ctrlStat.save_cyc_time == 1)
    {
        m_ctrlStat.save_cyc_time = 0;
        db_save_cyc_time();
    }
    else
    {
        vectorCycleTest.objMutex.lock();
        for (i = 0; i < vectorCycleTest.xvector.size();i++)
        {
            nextCycleTest = vectorCycleTest.xvector.at(i);
            if(1 == isStartCycleTest(nextCycleTest))
            {
                vectorCycleTest.xvector.replace(i, nextCycleTest);
            }
        }
        vectorCycleTest.objMutex.unlock();
    }

    return 0;
}
/*
   **************************************************************************************
 *  函数名称：initialCycleTestTime
 *  函数描述：初始化周期性测量时间。断网超过30分钟，才启用该函数，
 *  函数描述：一旦连接上，马上终止，并清空周期性测量队列
 *  入口参数：
 *  返回参数：
 *  作者       ：
 *  日期       ：
 **************************************************************************************
*/
void MainWindow::initialCycleTestTime()
{
    int i, frame_no,card_no, port_no;
    int cyc_num, res_code;
    time_t now;

    _tagNextCycleTest nextCycleTest;
    tdb_osw_cyc_t *cfg_cyc_buf;
    tdb_osw_cyc_t cfg_cyc, mask_cyc;
    if(RET_SUCCESS == db_read_cyc_time())
        goto usr_exit;
    //从数据库中读取需要周期性测量的端口
    cfg_cyc_buf = NULL;
    cyc_num = 0;
    memset(&cfg_cyc, 0, sizeof(tdb_osw_cyc_t));
    memset(&mask_cyc, 0, sizeof(tdb_osw_cyc_t));
    cfg_cyc.type =  OSW;
    mask_cyc.type = 1;
    cyc_num = tmsdb_Select_osw_cyc(&cfg_cyc, &mask_cyc, &cfg_cyc_buf);
    if(cyc_num > 0)
    {
        //需要进行周期测量的队列
        queCycleMeasur.objMutex.lock();
        queCycleMeasur.xqueue.clear();
        queCycleMeasur.xqueue.reserve(cyc_num);
        queCycleMeasur.objMutex.unlock();

        //需要进行周期性测量的端口，包含了next的测量时间
        vectorCycleTest.objMutex.lock();
        vectorCycleTest.xvector.clear();
        vectorCycleTest.xvector.reserve(cyc_num);

        now = time(NULL);   //获取当前时间
        for(i = 0; i < cyc_num; i++)
        {
            if(/*1 == cfg_cyc_buf[i].iscyc*/true) //后来改为保留项，发下来的都须测试
            {
                frame_no = cfg_cyc_buf[i].frame;
                card_no = cfg_cyc_buf[i].slot;
                port_no = cfg_cyc_buf[i].port;
                res_code = check_dev_port_no(frame_no,card_no,port_no);
                if(res_code == RET_SUCCESS)
                {
                    memcpy(&nextCycleTest, &cfg_cyc_buf[i].frame, sizeof(tdb_osw_cyc_t) - 4);
                    nextCycleTest.next = now +nextCycleTest.cycle *60; //*分钟测试一次
                    vectorCycleTest.xvector.append(nextCycleTest);
                    qDebug("cyc test frame %d card %d port %d cyc %d next %d", nextCycleTest.framNo, nextCycleTest.cardNo, nextCycleTest.portNo\
                           ,nextCycleTest.cycle, nextCycleTest.next);
                }
                else
                {
                    qDebug("read cyc test from db error frame %d card %d port%d", frame_no,card_no, port_no);
                }
            }
        }
        vectorCycleTest.objMutex.unlock();
        free(cfg_cyc_buf);
    }
    qDebug("Cycle Test  cyc num %d  frame %d card %d port %d", cyc_num,nextCycleTest.framNo,\
           nextCycleTest.cardNo, nextCycleTest.portNo);
usr_exit:
    return ;
}
/*
   **************************************************************************************
 *  函数名称：db_read_cyc_time
 *  函数描述：从数据库中读取周期性测量时间
 *  入口参数：
 *  返回参数：
 *  作者       ：
 *  日期       ：
 **************************************************************************************
*/
int MainWindow:: db_read_cyc_time()
{
    int i;
    int cyc_num, res_code;

    _tagNextCycleTest nextCycleTest;
    tdb_osw_cyc_bak_t *cfg_cyc_buf;
    tdb_osw_cyc_bak_t cfg_cyc, mask_cyc;
    //从数据库中读取需要周期性测量的端口
    cfg_cyc_buf = NULL;
    cyc_num = 0;
    memset(&cfg_cyc, 0, sizeof(tdb_osw_cyc_t));
    memset(&mask_cyc, 0, sizeof(tdb_osw_cyc_t));
    cfg_cyc.type =  OSW;
    mask_cyc.type = 1;
    cyc_num = tmsdb_Select_osw_cyc_bak(&cfg_cyc, &mask_cyc, &cfg_cyc_buf);
    res_code = RET_UNEXIST;
    if(cyc_num > 0)
    {
        //需要进行周期测量的队列
        queCycleMeasur.objMutex.lock();
        queCycleMeasur.xqueue.clear();
        queCycleMeasur.xqueue.reserve(cyc_num);
        queCycleMeasur.objMutex.unlock();
        //需要进行周期性测量的端口，包含了next的测量时间
        vectorCycleTest.objMutex.lock();
        vectorCycleTest.xvector.clear();
        vectorCycleTest.xvector.reserve(cyc_num);

        for(i = 0; i < cyc_num; i++)
        {
            memcpy(&nextCycleTest, &cfg_cyc_buf[i].frame, sizeof(_tagNextCycleTest));
            res_code = check_dev_port_no(nextCycleTest.framNo,nextCycleTest.cardNo,nextCycleTest.portNo);
            if(res_code == RET_SUCCESS)
            {
                vectorCycleTest.xvector.append(nextCycleTest);
            }
            else
            {
                qDebug("read cyc time from db error frame %d card %d port%d", \
                       nextCycleTest.framNo,nextCycleTest.cardNo, nextCycleTest.portNo);
            }

        }
        vectorCycleTest.objMutex.unlock();
        free(cfg_cyc_buf);
    }

    qDebug("db read cycle time  cyc num %d ", cyc_num);
    return res_code;
}

/*
   **************************************************************************************
 *  函数名称：cancelCycleTest
 *  函数描述：网络建立之后，周期性测量由网管负责
 *  入口参数：
 *  返回参数：
 *  日期版本：
 *  作者       ：
 **************************************************************************************
*/
void MainWindow::cancelCycleTest()
{
    // 从数据库中清除周期性测量记录;
    tdb_osw_cyc_bak_t cyc_time, mask;
    int res_db;
    bzero(&mask, sizeof(tdb_osw_cyc_t));
    bzero(&cyc_time, sizeof(tdb_osw_cyc_t));

    res_db = tmsdb_Delete_osw_cyc_bak(&cyc_time, &mask);
    vectorCycleTest.objMutex.lock();
    vectorCycleTest.xvector.clear();
    vectorCycleTest.objMutex.unlock();
}
void MainWindow::sendRespondSignal(int cmd, int resCmd, int errorCode,int dlgType)
{
    char buf[8];
    memcpy(buf, &resCmd, sizeof(resCmd));
    memcpy(buf + 4, &errorCode, sizeof(errorCode));
    if(m_currenShowDlg > 0 && dlgType == curShowDlgAttr.devType)
        emit(sendMsgToDlg(cmd, 0,0));
}
//通过对话框段形式显示返回的码
void MainWindow::dealAbnormalResCod(int rescode)
{
    QString str;
    switch(rescode)
    {
    case RET_SUCCESS:
        str = tr("发送成功！");
        break;
    case RET_UNEXIST:
        str = tr("业务板卡不存在！");
        break;
    case RET_COMMU_ABORT:
        str = tr("与板卡通信异常！");
        break;
    case RET_UNMATCH_FRAME:
        str = tr("机框号不匹配！");
        break;
    case RET_UNMATCH_SLOT:
        str = tr("槽位号不匹配！");
        break;
    case RET_UNMATCH_TYPE:
        str = tr("设备类型不匹配！");
        break;
    case RET_PARAM_INVALID:
        str = tr("参数非法！");
        break;
    case RET_IGNORE_SAVE:
        str = tr("参数无法保存！");
        break;
    case RET_WAIT:
        str = tr("已经存在一条命令，请稍候！");
        break;
    case RET_OTDR_ILLEGAL:
        str = tr("otdr测试异常！");
        break;
    case RET_OTDR_TIMEOUT:
        str = tr("otdr测试超时！");
        break;
    case RET_UPDATE_ILLEGAL:
        str = tr("升级过程出现异常");
        break;
    case RET_OLP_CANT_SWITCH:
        str = tr("olp不能切换到指定线路！");
        break;
    case RET_OSW_SWITCH_ABORT:
        str = tr("光开关不能切换到指定端口！");
        break;
    case RET_SEND_CMMD_TIMEOUT:
        str = tr("发送命令回应超时！");
        break;
    case RET_UNEXIST_ROW:
        str = tr("数据库相关记录不存在");
        break;
    case RET_OLP_REFUSE:
        str = tr("不能对olp业务端口进行测试！");
        break;
    case RET_CONNECT_PORT_NOT_TEST:
        str = tr("属于级联端口，不能进行测试！");
        break;
    case RET_MODULE_ROUTE_NO_EXIST:
        str = tr("模块路由不存在！");
        break;
    default: /*RET_CMD_INVALID:*/
        str = tr("无效命令");
        break;
    }
    qDebug("dealAbnormal res_code %d", rescode);
    dealAbnormal(str);
}
/*
   **************************************************************************************
 *  函数名称：inputDataToRingBuf
 *  函数描述：提供给通信线程调用的接口函数，收到数据之后，放入缓冲区
 *  函数描述：本函数要防止重入
 *  入口参数：buf, dataLen,最长的等待时间，毫秒
 *  返回参数：
 *  日期版本：
 *  作者       ：
 **************************************************************************************
*/
int MainWindow:: inputDataToRingBuf(char buf[], int dataLen, int msec)
{

    int returnValue, count;
    returnValue = 0;
    count = 0;
    _tagDataHead *pdata_head;
    pdata_head = (_tagDataHead *)buf ;
    //等待直到满足条件，或者超时跳出
    while(RingBufManager.getFreesSpace() < dataLen)
    {
        usleep(100);
        count++;
        if((count * 10) == msec)
            break;
    }

    if(RingBufManager.getFreesSpace() >= dataLen)
    {
        returnValue = RingBufManager.inputRingBuf(buf, dataLen);
        if(returnValue == dataLen)
            semRingBuf.release();
    }
    else
        qDebug("inputDataToRingBuf getFreesSpace 超时");

    return returnValue;
}
//处理收到的回应码
void MainWindow:: process_cmd_ack(char buf[], void *ptr_option)
{
    _tagResCode *pres_code;
    pres_code = (_tagResCode *)buf;
    dispatch_cmd_ack(pres_code, ptr_option);
    return;
}
/*
   **************************************************************************************
 *  函数名称：dispatch_cmd_ack
 *  函数描述：将收到的回应进行分类处理
 *  入口参数：
 *  返回参数：
 *  作者       ：
 *  日期       ：
 **************************************************************************************
*/
#define OTDR_ACK_TYPE   1
#define OSW_ACK_TYPE    0
int MainWindow:: dispatch_cmd_ack(_tagResCode * pRes,void *ptr_option)
{
    tms_context *ptr_context;
    //tms_devbase base;
    unsigned short  pkid;
    ptr_context = (tms_context *)ptr_option;
    pkid = ptr_context->pgb->pkid; //命令唯一标示号


    switch(pRes->res_cmd)
    {
    case ID_GET_OTDR_TEST: //测量曲线
    case ID_GET_OTDR_TEST_CYC:
    case ID_GET_ALARM_TEST:
        rcv_odtr_curv_ack(pRes, pkid, OTDR_ACK_TYPE);
        break;
    case ID_CMD_OLP_START_OTDR: //olp osw 切换
    case ID_CMD_OLP_FINISH_OTDR:
    case ID_CMD_OSW_SWITCH:
    {
        rcv_odtr_curv_ack(pRes, pkid,OSW_ACK_TYPE);
        break;
    }
    case ID_CMD_SMS_TEXT: //短消息回应码
    {
        if(pHostCommu != NULL && pRes->res_cmd == pHostCommu->semSynch.resCmd)
        {
            pHostCommu->semSynch.resCode = pRes->res_code;
            pHostCommu->semSynch.objSem.release();
        }
        break;
    }
        /*
         *总的硬件告警
         *变化的硬件告警
         *总的光功告警
         *变化的光功告警
         *olp切换记录
         *断网时的告警曲线
        */
    case ID_ALARM_HW:
    case ID_RET_ALARM_HW_CHANGE:
    case ID_ALARM_OPM:
    case ID_ALARM_OPM_CHANGE:
    case ID_REPORT_OLP_ACTION:
    case ID_RET_OTDR_CYC:
    {
        //必须是主网管发送过来的呼应码方处理
        if(ptr_context->pgb->src == ADDR_HOST_VIP)
        {
            printf("%s(): Line : %d  rcv retry tsk ret cmd 0x %x res code %d pkid 0x%x \n", \
                   __FUNCTION__, __LINE__, pRes->res_cmd,pRes->res_code,ptr_context->pgb->pkid);
            pSockRetry->objSynSem.objMutex.lock();
            pSockRetry->objSynSem.resCmd = pRes->res_cmd;
            pSockRetry->objSynSem.resCode = pRes->res_code;
            pSockRetry->objSynSem.pkid = ptr_context->pgb->pkid;
            pSockRetry->objSynSem.objSem.release();
            pSockRetry->objSynSem.objMutex.unlock();
        }
        break;
    }
    default:
        if(pRes->res_cmd == objSyncSem[SEM_COMMN].sendCmd)
        {
            objSyncSem[SEM_COMMN].resCode = pRes->res_code;
            objSyncSem[SEM_COMMN].objSem.release();
        }
        break;
    }
    qDebug("dispatch ack  res_cmd cmd %x res_code %d pkid 0x%x",\
           pRes->res_cmd, pRes->res_code,pkid);
    return 0;
}
/*
   **************************************************************************************
 *  函数名称：rcv_odtr_curv_ack
 *  函数描述：收到otdr曲线操作的响应码
 *  入口参数：type, 指名是收到otdr模块回应还是osw/olp切换的回应
 *  返回参数：
 *  作者       ：
 *  日期       ：
 **************************************************************************************
*/
int MainWindow::rcv_odtr_curv_ack(_tagResCode * pRes,unsigned short pkid,int ack_type)
{
    int i, j,frame,card, type,port;
    int res_code;
    res_code = -1;
    tsk_OtdrManage *pOtdrManage;
    frame = (pRes->reserved[0] >> 16)&0xFFFF;
    card = pRes->reserved[0]&0xFFFF;
    type = (pRes->reserved[1] >> 16)&0xFFFF;
    port = pRes->reserved[1]&0xffff;
    qDebug("curv ack frame/card/type/port %d %d %d %d",frame, card,type,port);
    if(ack_type == OTDR_ACK_TYPE) //otdr模块回应
    {
        if(port < 0)
            goto usr_exit;
        for(i = 0; i < m_otdrTskQue.xlist.size();i++)
        {
            pOtdrManage = (tsk_OtdrManage *)m_otdrTskQue.xlist.at(i);
            if(frame == pOtdrManage->otdrAddr.frame_no && card == pOtdrManage->otdrAddr.card_no\
                    &&port < pOtdrManage->otdrAddr.port)
            {
                if(pOtdrManage->isAllocaResource  == TSK_INITIAL_YES&&pRes->res_cmd == pOtdrManage->pObjSem[port].resCmd)
                {
                    pOtdrManage->pObjSem[port].resCode = pRes->res_code;
                    pOtdrManage->pObjSem[port].objSem.release();
                    res_code = RET_SUCCESS;
                }
            }
        }
        //如果其他任务没有找到，那么检查以下pCtuAsk
        if(res_code != RET_SUCCESS)
        {
            if(pCtuAsk->pObjSem[port].pkid == pkid)
            {
                pCtuAsk->pObjSem[port].resCode = pRes->res_code;
                pCtuAsk->pObjSem[port].objSem.release();
            }

        }
    }
    else //osw olp回应
    {
        for(i = 0; i < m_otdrTskQue.xlist.size();i++)
        {
            pOtdrManage = (tsk_OtdrManage *)m_otdrTskQue.xlist.at(i);
            //遍历所有端口的状态,通过pkid来计算
            for(j = 0; j < pOtdrManage->otdrAddr.port;j++)
            {
                if(pOtdrManage->isAllocaResource  == TSK_INITIAL_YES&&pOtdrManage->pobjComm[j].pkid == pkid &&\
                        pRes->res_cmd == pOtdrManage->pobjComm[j].resCmd)
                {
                    pOtdrManage->pobjComm[j].resCode = pRes->res_code;
                    pOtdrManage->pobjComm[j].objSem.release();
                    res_code = RET_SUCCESS;
                }
            }

        }
    }
usr_exit:
    return res_code;
}
//收到曲线时调用同名函数
int MainWindow::rcv_odtr_curv_ack( _tagDevComm dev, unsigned short pkid, int res_code)
{
    int i, frame,card, type,port;
    int result;
    result = -1;
    tsk_OtdrManage *pOtdrManage;
    frame = dev.frame_no;
    card = dev.card_no;
    type = dev.type;
    port = dev.port;


    if(port < 0)
        goto usr_exit;
    for(i = 0; i < m_otdrTskQue.xlist.size();i++)
    {
        pOtdrManage = (tsk_OtdrManage *)m_otdrTskQue.xlist.at(i);
        if(pOtdrManage->isAllocaResource  == TSK_INITIAL_YES&&pkid == pOtdrManage->pObjSem[port].pkid&&frame == pOtdrManage->otdrAddr.frame_no && card == pOtdrManage->otdrAddr.card_no\
                &&port < pOtdrManage->otdrAddr.port)
        {
            qDebug("rcv otdr curv frame/card/type/port %d %d %d %d", frame, card, type, port);
            qDebug("rcv otdr curv rcv pkid %d  saved pkid %d ", pkid, pOtdrManage->pObjSem[port].pkid);
            pOtdrManage->pObjSem[port].resCode = res_code;
            pOtdrManage->pObjSem[port].objSem.release();
            result = RET_SUCCESS;

        }
    }
    //如果其他任务没有找到，那么检查以下pCtuAsk
    if(result != RET_SUCCESS)
    {
        if(pCtuAsk->pObjSem[port].pkid == pkid)
        {
            pCtuAsk->pObjSem[port].resCode = res_code;
            pCtuAsk->pObjSem[port].objSem.release();
        }

    }
usr_exit:
    return result;
}
//通过单元槽位获取otdr任务
void * MainWindow::get_otdr_tsk(_tagDevComm otdrDev)
{
    int i,frame,card, type,port;
    int result;
    result = -1;
    tsk_OtdrManage *pOtdrManage;
    frame = otdrDev.frame_no;
    card = otdrDev.card_no;
    type = otdrDev.type;
    port = otdrDev.port;
    pOtdrManage = NULL;

    if(port < 0)
        goto usr_exit;
    for(i = 0; i < m_otdrTskQue.xlist.size();i++)
    {
        pOtdrManage = (tsk_OtdrManage *)m_otdrTskQue.xlist.at(i);
        if(pOtdrManage->isAllocaResource  == TSK_INITIAL_YES&& frame == pOtdrManage->otdrAddr.frame_no && card == pOtdrManage->otdrAddr.card_no\
                &&port < pOtdrManage->otdrAddr.port)
        {
            break;
        }
        else
            pOtdrManage = NULL; //少了这一句，要出错
    }
usr_exit:
    return (void *)pOtdrManage;
}

/*
   **************************************************************************************
 *  函数名称：get_otdr_card_slot
 *  函数描述：获取otdr gsm等地址
 *  入口参数：默认是提示的
 *  返回参数：
 *  日期版本：
 *  作者       ：
 **************************************************************************************
*/
void MainWindow::get_otdr_card_slot(int is_prompt)
{
    int i, j, state_1,state_2;
    int otdr_num, gsm_num;
    _tagDevComm otdrDev;
    bool is_ok;
    QString str;
    otdr_num = 0;
    gsm_num = 0;
    m_otdrDevQue.xlist.clear();
    for(i = 0; i < NUM_SUBRACK; i++)
    {
        //从机号存在
        is_ok = false;
        state_1 = m_subrackInfo.onState;
        if(1 == ((state_1 >> i)&1))
        {
            for(j = 0; j < NUM_CARD;j++)
            {
                //查找otdr，如果找到多个或者没有一个，都要告警
                state_2 = m_subrackCard[i].onState;
                if(1 == ((state_2 >>j)&1))
                {
                    if(OTDR == m_subrackCard[i].type[j])
                    {
                        otdrDev.frame_no = i;
                        otdrDev.card_no = j;
                        otdrDev.type = m_subrackCard[i].type[j];
                        otdrDev.port = m_subrackCard[i].ports[j];
                        m_otdrDevQue.xlist.append(otdrDev);
                        otdr_num++;
                    }
                    else if(GSM == m_subrackCard[i].type[j])
                    {
                        m_addrGsm.frame = i;
                        m_addrGsm.card = j;
                        m_addrGsm.port = 0;
                        gsm_num++;
                    }
                }
            }
        }

    }
    if(is_prompt == 1)
    {
        /*
        if(gsm_num >1)
        {
            str = tr("系统存在多个gsm,请确认!");
            dealAbnormal(str);
        }
        else if(gsm_num <= 0)
        {
            str = tr("系统中不存在gsm无法工作！");
            dealAbnormal(str);
        }
        */
    }
    //查询到otdr之后，创建otdr_tsk
    creat_otdr_tsk();
    qDebug("otdr  num %d gsm num %d ", otdr_num, gsm_num);
}
/*
   **************************************************************************************
 *  函数名称：creat_otdr_tsk
 *  函数描述：系统中增加或者删除otdr设备，都要对otdr对应的任务进行重新设定
 *  属于耗时操作，考虑信号槽来实现
 *  入口参数：
 *  返回参数：
 *  作者       ：
 *  日期       ：
 **************************************************************************************
*/
int MainWindow::creat_otdr_tsk()
{
    int otdrDevNum, otdrTskNum, i, j;
    _tagDevComm otdrDev;
    tsk_OtdrManage * pOtdrTsk;
    if(!m_otdrDevQue.xlist.isEmpty()) //存在otdr
    {
        otdrDevNum = m_otdrDevQue.xlist.size();

        //otdrTsk中存在，但otdr不存在，要删除
        for(i = 0; i < m_otdrTskQue.xlist.size();i++)
        {
            pOtdrTsk = (tsk_OtdrManage *)m_otdrTskQue.xlist.at(i);
            for(j = 0; j < otdrDevNum; j++)
            {
                otdrDev = m_otdrDevQue.xlist.at(j);
                if(pOtdrTsk->otdrAddr.frame_no == otdrDev.frame_no&&pOtdrTsk->otdrAddr.card_no == otdrDev.card_no&&\
                        pOtdrTsk->otdrAddr.type == otdrDev.type&&pOtdrTsk->otdrAddr.port == otdrDev.port)
                {
                    //找到相同的otdr则跳出
                    //增加检查属性的步骤 2015-10-23
                    if(m_subrackCard[otdrDev.frame_no].opt[otdrDev.card_no][1] == OTDR_MODE_SERRI || \
                            m_subrackCard[otdrDev.frame_no].opt[otdrDev.card_no][1] == OTDR_MODE_PARAL)
                    {
                        if(pOtdrTsk->otdr_mode != m_subrackCard[otdrDev.frame_no].opt[otdrDev.card_no][1])
                        {
                            pOtdrTsk->otdr_mode = \
                                    m_subrackCard[otdrDev.frame_no].opt[otdrDev.card_no][0];
                        }
                    }

                    break;
                }
            }
            if(j == otdrDevNum)//最新的配置中，不存在该otdr，需要将该otdr删除掉
            {
                m_otdrTskQue.xlist.removeAt(i);
                pOtdrTsk->stop();
                pOtdrTsk->release_resource();
                pOtdrTsk->wait(500);
                pOtdrTsk->terminate();
                i--;
                delete pOtdrTsk;
            }
        }
        //otdr存在，但otdrTsk中不存在，要增加任务
        otdrTskNum = m_otdrTskQue.xlist.size(); //加入otdrTsk数目为0，也能正确执行
        for(j = 0; j < otdrDevNum; j++)
        {
            otdrDev = m_otdrDevQue.xlist.at(j);
            for(i = 0; i < otdrTskNum;i++)
            {
                pOtdrTsk = (tsk_OtdrManage *)m_otdrTskQue.xlist.at(i);
                if(pOtdrTsk->otdrAddr.frame_no == otdrDev.frame_no&&pOtdrTsk->otdrAddr.card_no == otdrDev.card_no&&\
                        pOtdrTsk->otdrAddr.type == otdrDev.type&&pOtdrTsk->otdrAddr.port == otdrDev.port)
                {
                    //找到相同的otdr则跳出
                    //增加检查属性的步骤 2015-10-23
                    if(m_subrackCard[otdrDev.frame_no].opt[otdrDev.card_no][0] == OTDR_MODE_SERRI || \
                            m_subrackCard[otdrDev.frame_no].opt[otdrDev.card_no][0] == OTDR_MODE_PARAL)
                    {
                        if(pOtdrTsk->otdr_mode != m_subrackCard[otdrDev.frame_no].opt[otdrDev.card_no][0])
                        {
                            pOtdrTsk->otdr_mode = \
                                    m_subrackCard[otdrDev.frame_no].opt[otdrDev.card_no][0];
                        }
                    }
                    break;
                }
            }
            if(i == otdrTskNum)//需要新增otdr_tsk
            {
                //新增的在最后，不用再次比较
                pOtdrTsk = new tsk_OtdrManage::tsk_OtdrManage(this);
                memcpy(&pOtdrTsk->otdrAddr, &otdrDev, sizeof(_tagDevComm));
                pOtdrTsk->alloca_resource(pOtdrTsk->otdrAddr.port);
                //增加检查属性的步骤 2015-10-23
                if(m_subrackCard[otdrDev.frame_no].opt[otdrDev.card_no][0] == OTDR_MODE_SERRI || \
                        m_subrackCard[otdrDev.frame_no].opt[otdrDev.card_no][0] == OTDR_MODE_PARAL)
                {
                    //使用配置中的OTDR属性
                    pOtdrTsk->otdr_mode = \
                            m_subrackCard[otdrDev.frame_no].opt[otdrDev.card_no][0];
                }
                else //默认是串行的OTDR
                {
                    pOtdrTsk->otdr_mode = OTDR_MODE_SERRI;
                }
                pOtdrTsk->start();
                m_otdrTskQue.xlist.append(pOtdrTsk);

            }
        }

    }
    else if(!m_otdrTskQue.xlist.isEmpty())
    {
        //全部删除otdr_tsk
        for(j = 0; j < m_otdrTskQue.xlist.size(); j++)
        {
            pOtdrTsk =  (tsk_OtdrManage *)m_otdrTskQue.xlist.at(j);
            pOtdrTsk->stop();
            pOtdrTsk->release_resource();
            pOtdrTsk->wait(500);
            pOtdrTsk->terminate();
            delete pOtdrTsk;
        }
        qDebug("clear all tsk");
        m_otdrTskQue.xlist.clear();
    }
    isGetOtdrPara = 1;
    return 0;
}

//更新板卡组成信息，弹出mcu对话框之后调用
void MainWindow::refresh_card_slot_config()
{
    QString str;
    InitialCardType();
    //在主界面上显示当前段机框编号
    str.setNum(m_subrackInfo.curShowNo + 1);
    str = tr("当前显示框号") + str;
    ui->labelCurFrameNo->setText(str);
}
/*
   **************************************************************************************
 *  函数名称：get_dev_type
 *  函数描述：从字符串设备类型转化成int类型
 *  入口参数：
 *  返回参数：
 *  作者       ：
 *  日期       ：
 **************************************************************************************
*/
int MainWindow::get_dev_type(QString strType)
{
    int dev_type;
    dev_type = m_strlstAllEqType.indexOf(strType);
    if(dev_type <= -1)
    {
        dev_type = 0;
        qDebug()<<"get dev_type error"<<strType.simplified();
    }
    return dev_type;
}
/*
   **************************************************************************************
 *  函数名称：update_dev_type
 *  函数描述：如果机框号有效，那么查找告警判断通信中断时间不累计，如果
 *                ：机框号-1，每30秒更新一次，通信中断时间累计
 *  入口参数：
 *  返回参数：
 *  作者       ：
 *  日期       ：
 *  修改日期：2015-11-17
 *  修改内容：
 *                ：
 **************************************************************************************
*/
int MainWindow::update_dev_type(int frame_no)
{
    /*
     *2016-02-02 等待连接稳定之后再检查硬件告警
    */
    if(countTimer.coutUpdateDevType < ALARM_DELAY_TIME_S) //前3分钟不产生告警
    {
        printf("%s(): Line : %d  not judge alarm change time %d\n", \
               __FUNCTION__, __LINE__,countTimer.coutUpdateDevType);
        goto usr_exit;
    }
    tms_devbase oneframe[NUM_CARD];

    int i,k;
    //首先将主机框主槽位设置成MCU
    if(frame_no  >-1&& frame_no < NUM_SUBRACK )
    {
        //随时更新
        objHwAlarm.lock();
        //更新0-13号板卡，MCU-PW1-PW2通过其他渠道更新
        tms_GetFrame(frame_no,&oneframe);
        for(i = 0; i < NUM_COMM_CARD;i++)
        {
            DevCommuState[frame_no][i].cur_type = oneframe[i].type;
            DevCommuState[frame_no][i].cur_port = oneframe[i].port;
            memcpy(&DevCommuState[frame_no][i].cur_opt, &oneframe[i].wave, sizeof(int)*CARD_OPT_NUM);
        }
        //MCU槽位必须自行更新 由于MCU模块会产生短信模块硬件告警，因此不能清空
        /*
        bzero(&DevCommuState[MCU_FRAME][MCU_CARD],\
              sizeof(DevCommuState[MCU_FRAME][MCU_CARD]));
        */
        DevCommuState[MCU_FRAME][MCU_CARD].cur_type = MCU;
        DevCommuState[MCU_FRAME][MCU_CARD].card_state = PULL_IN;
        objHwAlarm.unlock();
        check_card_commu_state();

    }
    else //30更新一次
    {
        objHwAlarm.lock();
        for(k = 0; k < NUM_SUBRACK; k++)
        {
            tms_GetFrame(k,&oneframe);
            for(i = 0; i < NUM_COMM_CARD;i++)
            {
                DevCommuState[k][i].cur_type = oneframe[i].type;
                DevCommuState[k][i].cur_port = oneframe[i].port;
                memcpy(&DevCommuState[k][i].cur_opt, &oneframe[i].wave, sizeof(int)*CARD_OPT_NUM);

            }
        }
        //MCU槽位必须自行更新 由于MCU模块会产生短信模块硬件告警，因此不能清空
        /*
        bzero(&DevCommuState[MCU_FRAME][MCU_CARD],\
              sizeof(DevCommuState[MCU_FRAME][MCU_CARD]));
        */
        DevCommuState[MCU_FRAME][MCU_CARD].cur_type = MCU;
        DevCommuState[MCU_FRAME][MCU_CARD].card_state = PULL_IN;
        objHwAlarm.unlock();
        check_card_commu_state(1);
    }
usr_exit:
    return 0;

}

/*
   **************************************************************************************
 *  函数名称：check_card_commu_state
 *  函数描述：检查设备通信状态，判断是否产生通信故障，如果不能获得权限，
 *                    立即返回。
 *  入口参数：0,随机检查，1，周期性检查
 *  返回参数：
 *  作者       ：
 *  日期       ：2015-10-14
 *  修改日期：2015-11-17
 *  修改内容：周期性检查，判断通信中断计时累计。修改bug:某位置的板卡
 *                ：拔出后，过一段时间再插入，会立刻报消失
 **************************************************************************************
*/
int MainWindow:: check_card_commu_state(int check_type)
{
    int i, j, fd, frame_state, card_state;
    int commu_error;
    int alarm_num;
    char alarm_time[20];
    char reason_buf[64*3]; //注意此长度
    char *changed_alarm_buf;
    int count, charLen, alarm_type;
    int is_add_new_card, offset;
    int gsm_count, nm_count;

    glink_addr dst_addr;
    tms_alarm_hw_change_val *hw_val; //硬件告警缓冲区
    tms_alarm_hw_change_val hw_buf[1 + NUM_CARD* NUM_SUBRACK];
    _tagDataHead *pDataHead;
    nm_count = 0;
    changed_alarm_buf = (char *)hw_buf;
    //前面20个字节用来填充cmd datalen 和其他信息
    hw_val = (tms_alarm_hw_change_val *)(changed_alarm_buf + 20);
    int alarmLev;
    QString str, strFrame, strSlot;
    QByteArray byteArry;
    charLen = 64;
    alarmLev = 0;
    memset(&hw_buf, 0, sizeof(hw_buf));
    alarm_num = 0;
    GetCurrentTime(alarm_time);
    commu_error = 0;
    //遍历所有机框设备
    is_add_new_card = 0;
    gsm_count = 0;
    objHwAlarm.lock();
    for(i = 0; i < NUM_SUBRACK; i++)
    {
        for(j = 0; j < NUM_CARD; j++)
        {
            /*
             *设置当前告警类型
            */
            //如果板卡拔出
            if(DevCommuState[i][j].card_state == PULL_OUT)
            {
                DevCommuState[i][j].cur_type = NONE;
                DevCommuState[i][j].opt = 0;
                //如果已经配置过，那么告警为板卡拔出
                if(m_subrackCard[i].type[j] != NONE)
                {
                    DevCommuState[i][j].cur_alarm = CARD_ALARM_PULL_OUT;
                    /*
                    qDebug("pull out frame %d card %d type %d stat %d cur alarm %d last alarm %d ",\
                           i,j, m_subrackCard[i].type[j],DevCommuState[i][j].card_state,DevCommuState[i][j].cur_alarm,\
                           DevCommuState[i][j].last_alarm);
                    */
                }
            }
            //设备类型不一致
            else if(DevCommuState[i][j].cur_type != m_subrackCard[i].type[j]) //设备类型不相同是有告警
            {

                if(DevCommuState[i][j].cur_type == NONE) //板卡丢失，找不到了，通信中断
                {
                    if(DevCommuState[i][j].opt < 0)
                    {
                        DevCommuState[i][j].opt = 0;
                    }
                    else if(DevCommuState[i][j].opt > 1)//等待两个周期之后判为通信故障
                    {
                        DevCommuState[i][j].cur_alarm = CARD_ALARM_LOST;
                    }
                    //只有周期性查询才加1
                    if(check_type == 1)
                    {

                        DevCommuState[i][j].opt ++;
                    }
                }
                else if(m_subrackCard[i].type[j] == NONE) //在没有配置过的槽位上新发现了设备
                {
                    DevCommuState[i][j].opt = 0;
                    DevCommuState[i][j].cur_alarm = CARD_ALARM_NEW;
                    //如果是电源,MCU初始化在本地
                    if(DevCommuState[i][j].cur_type == MCU || DevCommuState[i][j].cur_type == TU)
                        bzero(DevCommuState[i][j].cur_opt, sizeof(int)*CARD_OPT_NUM);
                }
                else //已经配置的槽位上设备类型不一致
                {
                    DevCommuState[i][j].opt = 0;
                    DevCommuState[i][j].cur_alarm = CARD_ALARM_TYPE_DIFFERENT;
                }
            }
            /*
             *2016-01-12
             *明确探测到板卡插入，但一直没有与MCU通信，上报告警
            */
            else if(DevCommuState[i][j].cur_type == NONE&&DevCommuState[i][j].card_state == PULL_IN&&\
                    DevCommuState[i][j].cur_type == m_subrackCard[i].type[j])
            {
                if(DevCommuState[i][j].opt < 0)
                {
                    DevCommuState[i][j].opt = 0;
                }
                else if(DevCommuState[i][j].opt > 1)//两个周期之后判断为通信故障
                {
                    DevCommuState[i][j].cur_alarm = CARD_ALARM_LOST;
                }
                //只有周期性查询才加1
                if(check_type == 1)
                {
                    DevCommuState[i][j].opt ++;
                }

            }
            /*DevCommuState[i][j].cur_type == MCU*/
            else if(i== MCU_FRAME && j == MCU_CARD)
            {
                DevCommuState[i][j].opt = 0;
                //短信猫故障
                if(pSmsSend->objSynSem.commu_stat == SMS_STAT_ERROR)
                {
                    DevCommuState[i][j].cur_alarm = CARD_ALARM_SMS_EQ_ERROR;
                }
                else
                {
                    DevCommuState[i][j].cur_alarm = CARD_ALARM_RECOVER;
                }
            }
            else if(DevCommuState[i][j].cur_type == PWU )
            {
                DevCommuState[i][j].opt = 0;
                //电源，判断电压是否正常
                if(DevCommuState[i][j].pw_state == PW_STATE_ERROR)

                {
                    DevCommuState[i][j].cur_alarm = CARD_ALARM_POWER_ABNORMAL;
                }
                else
                {
                    DevCommuState[i][j].cur_alarm = CARD_ALARM_POWER_NORMAL;
                }
                //初始化的时候是清零，可能回误判，导致电压正常报告
                if(DevCommuState[i][j].last_alarm != CARD_ALARM_POWER_ABNORMAL)
                {
                    DevCommuState[i][j].last_alarm = CARD_ALARM_POWER_NORMAL;
                }

            }
            //设备端口不一致
            else if(DevCommuState[i][j].cur_port != m_subrackCard[i].ports[j])
            {
                DevCommuState[i][j].opt = 0;
                DevCommuState[i][j].cur_alarm = CARD_ALARM_PORT_DIFFERENT;
            }
            //属性不一致，比如串行otdr变成并行otdr，电源48V变成220V
            else if(DevCommuState[i][j].cur_opt[0] != m_subrackCard[i].opt[j][0])
            {
                DevCommuState[i][j].opt = 0;
                DevCommuState[i][j].cur_alarm = CARD_ALARM_ATTRIBUT_DIFF;
            }
            else //告警消失，或者没有配置的槽位没有发现新增设备
            {
                DevCommuState[i][j].opt = 0;
                DevCommuState[i][j].cur_alarm = CARD_ALARM_RECOVER;
            }
            /*
             *判断告警有没有变化
             *2016-01-14 附加了告警条件：配置不为空，或者配置为空，但插入链设备
            */
            //告警恢复，新增板卡，电源正常，都不是告警
            if(
                    (m_subrackCard[i].type[j] != NONE || \
                     m_subrackCard[i].type[j] == NONE&&DevCommuState[i][j].card_state == PULL_IN)\
                    &&\
                    (DevCommuState[i][j].cur_alarm != CARD_ALARM_RECOVER || \
                     DevCommuState[i][j].cur_alarm  != CARD_ALARM_NEW ||\
                     DevCommuState[i][j].cur_alarm != CARD_ALARM_POWER_NORMAL)
                    )
                commu_error++;

            //如果是新增模块，自动保存数据库,不用上报
            if(DevCommuState[i][j].cur_alarm == CARD_ALARM_NEW)
            {
                is_add_new_card = 1;//从未插入板卡的地方插入了信板卡
                qDebug("new card  frame / card / type %d %d %d", i, j, DevCommuState[i][j].cur_type);
                qDebug("new card opt[0--3] %d %d %d %d", DevCommuState[i][j].cur_opt[0],\
                       DevCommuState[i][j].cur_opt[1],DevCommuState[i][j].cur_opt[2],DevCommuState[i][j].cur_opt[3]);
                objOperateFrameCard.lock();
                strcpy(m_subrackCard[i].oprateTime,alarm_time);
                m_subrackCard[i].type[j] = DevCommuState[i][j].cur_type;
                m_subrackCard[i].ports[j] = DevCommuState[i][j].cur_port;
                memcpy(&m_subrackCard[i].opt[j], &DevCommuState[i][j].cur_opt, sizeof(int) * CARD_OPT_NUM);
                card_state = m_subrackCard[i].onState;

                if( 1!= ((card_state >> j) & 1)) //该槽位之前没有启用,理论上应该是必须的
                {
                    m_subrackCard[i].numInUse++;
                    m_subrackCard[i].onState = (m_subrackCard[i].onState | 1 << j);
                }
                //如果主控没有添加，那么添加主控,二期之后，要屏蔽掉该代码
                if( 1!= ((card_state >> MCU_CARD) & 1))//MCU
                {
                    m_subrackCard[i].numInUse++;
                    /*
                     *2016-01-11 出现错误
                     *此处
                     *(m_subrackCard[i].onState | 1 << NUM_COMM_CARD
                    */
                    m_subrackCard[i].onState = (m_subrackCard[i].onState | (1 << MCU_CARD));
                    m_subrackCard[i].numInUse++;
                    if(i == 0)
                        m_subrackCard[i].type[MCU_CARD ] = MCU;
                    else
                        m_subrackCard[i].type[MCU_CARD ] = TU;
                    m_subrackCard[i].ports[MCU_CARD ] = 0;

                }

                frame_state = m_subrackInfo.onState;
                strcpy(m_subrackInfo.oprateTime[i],alarm_time);
                if( 1!= ((frame_state >> i) & 1)) //该机框没有之前没有启用
                {
                    m_subrackInfo.numInUse++;
                    m_subrackInfo.onState = (m_subrackInfo.onState | 1 << i);
                }
                objOperateFrameCard.unlock();
                //如果是新增模块，告警设置为恢复，否则下次一定会产生告警
                DevCommuState[i][j].last_alarm = CARD_ALARM_RECOVER;

            }
            else if(DevCommuState[i][j].cur_alarm != DevCommuState[i][j].last_alarm) //确定变化的告警
            {
                if(countTimer.coutUpdateDevType < ALARM_DELAY_TIME_S) //前3分钟不产生告警
                {
                    printf("%s(): Line : %d  not judge alarm change time %d\n", \
                           __FUNCTION__, __LINE__,countTimer.coutUpdateDevType);
                    goto usr_exit;
                }
                DevCommuState[i][j].last_alarm = DevCommuState[i][j].cur_alarm;
                strcpy((char *)(&DevCommuState[i][j].alarm_time), alarm_time);//更新告警时间
                hw_val[alarm_num].frame = i;
                hw_val[alarm_num].slot = j;
                hw_val[alarm_num].reason = DevCommuState[i][j].cur_alarm;
                //确定告警级别
                if(DevCommuState[i][j].cur_alarm == CARD_ALARM_RECOVER)
                    hw_val[alarm_num].level = 0;
                else
                {
                    hw_val[alarm_num].level = 1;
                }
                str.clear();
                strFrame.clear();
                strSlot.clear();
                switch(DevCommuState[i][j].cur_alarm)
                {
                case CARD_ALARM_RECOVER :
                {
                    str = tr("模块正常!");
                    break;
                }
                case CARD_ALARM_LOST :
                {
                    str = tr("模块消失! ");
                    break;
                }
                case CARD_ALARM_NEW :
                {
                    str = tr("新增模块! ");
                    break;
                }
                case CARD_ALARM_TYPE_DIFFERENT :
                {
                    str = tr("模块不一致! ");
                    break;
                }
                case CARD_ALARM_PORT_DIFFERENT:
                {
                    str = tr("端口不一致! ");
                    break;
                }
                case CARD_ALARM_ATTRIBUT_DIFF:
                {
                    str = tr("模块属性发生变化! ");
                    break;
                }
                case CARD_ALARM_POWER_ABNORMAL:
                {
                    str = tr("电压输出不正常! ");
                    break;
                }
                case CARD_ALARM_POWER_NORMAL:
                {
                    str = tr("电压输出正常! ");
                    break;
                }
                case CARD_ALARM_PULL_OUT:
                {
                    str = tr("板卡拔出! ");
                    break;
                }
                case CARD_ALARM_SMS_EQ_ERROR:
                {
                    str = tr("短信模块故障! ");
                    break;
                }
                default:
                {
                    qDebug("frame / card %d %d hw_arlarm %d ", i,j,DevCommuState[i][j].cur_alarm);
                    break;
                }
                }//switch end
                strFrame.setNum(i + 1);
                strSlot.setNum(j + 1);
                str = tr("机框/槽位 ") + strFrame + "/"+ strSlot + str;
                byteArry = str.toLocal8Bit();
                //清空存储空间
                bzero(reason_buf, sizeof(reason_buf));

                count = byteArry.length();
                if(count > charLen)
                    count = charLen;
                memcpy(reason_buf, byteArry, count);

                strcpy((char *)(&hw_val[alarm_num].time), alarm_time);//告警时间
                //如果断网，添加到短信队列中去
                if(m_ctrlStat.NMstat != NM_EXIST)
                {
                    input_gsm_queue(hw_val[alarm_num].level, ALARM_HARD_WARE,NULL, reason_buf);
                    gsm_count++;
                }
                alarm_num++;
            }
            else
            {
                DevCommuState[i][j].last_alarm = DevCommuState[i][j].cur_alarm;
            }
        }

    }
    nm_count = tms_ManageCount();
    //告警数目不为0，并且网管存在
    if(alarm_num > 0&& nm_count > 0)
    {
        //获取主网管fd
        dst_addr.dst = ADDR_HOST_VIP;
        fd = tms_SelectFdByAddr(&dst_addr.dst);
        dst_addr.src = ADDR_MCU;
        dst_addr.dst = ADDR_MASS_SEND;
        dst_addr.pkid = creat_pkid();
        //如果主网管存在
        if(fd > 0 )
        {
            /*
             *数据头：命令码+数据长度
             *告警类型-告警数目-变化的告警数目
             *级别-机框-槽位-原因-时间
            */
            pDataHead = (_tagDataHead *)(changed_alarm_buf);
            pDataHead->cmd = ID_RET_ALARM_HW_CHANGE;
            pDataHead->dataLen = sizeof(_tagDataHead) + 12 + alarm_num * sizeof(tms_alarm_hw_change_val);
            offset = sizeof(_tagDataHead);
            //告警类型
            alarm_type = ALARM_HARD_WARE;
            memcpy(changed_alarm_buf + offset, &alarm_type, sizeof(int));
            offset += sizeof(int);
            //总的告警数目
            memcpy(changed_alarm_buf + offset, &commu_error, sizeof(int));
            offset += sizeof(int);
            //变化的告警数目
            memcpy(changed_alarm_buf + offset, &alarm_num, sizeof(int));
            offset += sizeof(int);
            input_cpy_sock_retry_queue(changed_alarm_buf, dst_addr.pkid);
        }
        tms_RetAlarmHWChange(0,&dst_addr,commu_error,alarm_num,( tms_alarm_hw_change_val *)hw_val);

    }

usr_exit:
    objHwAlarm.unlock();
    /*
     *2016-02-04 挪到解锁的后面 ，否则保存硬件告警的时候会被锁死
    */
    if(is_add_new_card == 1)
    {
        objOperateFrameCard.lock();
        wrtie_db_card_comp(); //保存数据库
        objOperateFrameCard.unlock();
        InitialCardType();
    }
    return 0;
}

/*
   **************************************************************************************
 *  函数名称：input_changed_hw_gsm_queue
 *  函数描述：将变化的告警加入到发送队列,重发任务调用
 *  入口参数：
 *  返回参数：
 *  作者       ：
 *  日期       ：2015-10-14
 *  修改日期：2015-10-30
 *  修改内容：控制for循环的次数错误，i,j不是实际告警的机框槽位，逻辑错误
 **************************************************************************************
*/
int MainWindow::input_changed_hw_gsm_queue(char buf[])
{
    int j;
    char alarm_time[20];
    char reason_buf[64]; //注意此长度
    int count, charLen;
    int offset, alarm_type, alarm_num, total_alarm_num;
    int alarmLev;
    QString str, strFrame, strSlot;
    QByteArray byteArry;
    tms_alarm_hw_change_val *hw_val; //硬件告警缓冲区

    offset = sizeof(_tagDataHead);
    //告警类型
    memcpy(&alarm_type, buf + offset, sizeof(int));
    offset += sizeof(int);
    //总的告警数目
    memcpy(&total_alarm_num, buf + offset, sizeof(int));
    offset += sizeof(int);
    //变化的告警数目
    memcpy(&alarm_num, buf + offset, sizeof(int));
    offset += sizeof(int);
    if(alarm_type != ALARM_HARD_WARE || alarm_num < 0 \
            || alarm_num > NUM_CARD * NUM_SUBRACK)
    {
        goto usr_exit;
    }
    charLen = 64;
    //遍历所有变化的告警
    for(j = 0; j < alarm_num; j++)
    {

        hw_val = (tms_alarm_hw_change_val *)(buf + offset);
        offset += sizeof(tms_alarm_hw_change_val);
        //确定告警级别
        str.clear();
        strFrame.clear();
        strSlot.clear();
        if(hw_val->level == ALARM_LEVE_1)
            alarmLev = ALARM_LEVE_1; //硬件告警级别全部是1
        else
            alarmLev = ALARM_NONE; //硬件告警级别全部是1
        switch((int) (hw_val->reason))
        {
        case CARD_ALARM_RECOVER :
            alarmLev = ALARM_NONE;
            str = tr("模块恢复");
            break;
        case CARD_ALARM_LOST :
            str = tr("模块消失");
            break;
        case CARD_ALARM_NEW :
            str = tr("新增模块");
            break;
        case CARD_ALARM_TYPE_DIFFERENT :
            str = tr("模块不一致");
            break;
        case CARD_ALARM_PORT_DIFFERENT:
            str = tr("端口不一致");
            break;
        case CARD_ALARM_ATTRIBUT_DIFF:
            str = tr("模块属性发生变化");
            break;
        case CARD_ALARM_POWER_ABNORMAL:
            str = tr("电压输出不正常");
            break;
        case CARD_ALARM_POWER_NORMAL:
            str = tr("电压输出正常");
            break;
        default:
        {
            qDebug("frame / card %d / %d hw_arlarm %d ", hw_val->frame,hw_val->slot\
                   ,DevCommuState[hw_val->frame][hw_val->slot].cur_alarm);
            break;
        }
        }
        strFrame.setNum(hw_val->frame + 1);
        strSlot.setNum(hw_val->slot + 1);
        str = tr("机框/槽位") + strFrame + "/"+ strSlot + str;
        byteArry = str.toLocal8Bit();
        //清空存储空间
        bzero(reason_buf, sizeof(reason_buf));

        count = byteArry.length();
        if(count > charLen)
            count = charLen;
        memcpy(reason_buf, byteArry, count);
        memcpy(alarm_time, hw_val->time, TIME_STR_LEN);//告警时间
        input_gsm_queue(alarmLev, ALARM_HARD_WARE,NULL, reason_buf);

    }


usr_exit:
    return 0;

}

/*
   **************************************************************************************
 *  函数名称：send_total_hw_alarm
 *  函数描述：网管第一次连上主控，将所有的硬件告警发送上去
 *  入口参数：is_retry,是否重发？ pkid是否有效
 *  返回参数：
 *  作者       ：
 *  日期       ：
 *  修改日期：2015-11-01
 *  修改内容：总的硬件告警原因已经修改成数字表示
 **************************************************************************************
*/
int MainWindow::send_total_hw_alarm(int send_type, int pkid, void * ptr_opt)
{
    int i, j, fd, res_code;
    int alarm_num,isHwAlarm;
    glink_addr dst_addr;
    tms_alarm_hw_val hw_val[NUM_CARD* NUM_SUBRACK]; //硬件告警缓冲区
    int alarmLev;
    tms_context *pcontext;

    alarmLev = 0;
    memset(&hw_val, 0, sizeof(tms_alarm_hw_val)*NUM_COMM_CARD* NUM_SUBRACK);
    alarm_num = 0;
    //遍历所有机框设备
    objHwAlarm.lock();
    for(i = 0; i < NUM_SUBRACK; i++)
    {
        for(j = 0; j < NUM_CARD; j++)
        {
            isHwAlarm = check_is_hw_alarm(DevCommuState[i][j].cur_alarm);
            if(isHwAlarm == RET_SUCCESS)
            {
                /*
                 *2016-02-01此处失误
                */
                //DevCommuState[i][j].last_alarm = DevCommuState[i][j].cur_alarm;
                hw_val[alarm_num].frame = i;
                hw_val[alarm_num].slot = j;
                //确定告警级别
                hw_val[alarm_num].level = 1;
                hw_val[alarm_num].reason = DevCommuState[i][j].cur_alarm;
                strcpy((char *)(&hw_val[alarm_num].time), DevCommuState[i][j].alarm_time);//告警时间
                alarm_num++;
            }

        }

    }
    objHwAlarm.unlock();
    //重发
    if(send_type == SEND_TYPE_RETRY)
    {
        if(pkid == -1)
        {
            printf("retry total hw alarm pkid error pkid %d\n", pkid);
            res_code = UNDEFINED_ERROR;
            goto usr_exit;
        }
        /*
         *2016-01-29 修改成网管必须存在方可重发
        */
        if(m_ctrlStat.NMstat == NM_EXIST)
        {
            //重发，只发送主网管
            dst_addr.src = ADDR_MCU;
            dst_addr.dst = ADDR_HOST_VIP;
            fd = tms_SelectFdByAddr(&dst_addr.dst);
            dst_addr.pkid = pkid;
        }
        else
        {
            printf("%s(): Line : %d  retry send  hw total alarm host disconnect \n",  __FUNCTION__, __LINE__);
            res_code = UNDEFINED_ERROR;
            goto usr_exit;
        }
    }
    else if(send_type == SEND_TYPE_ASK)//查询
    {
        pcontext = (tms_context *)ptr_opt;
        dst_addr.src = ADDR_MCU;
        dst_addr.dst = pcontext->pgb->src;
        fd = pcontext->fd;
        dst_addr.pkid = pcontext->pgb->pkid;
    }
    else if(send_type == SEND_TYPE_INITIATIVE)//主动上报，群发
    {
        _tagDataHead *pDataHead;
        char buf[48];
        dst_addr.src = ADDR_MCU;
        dst_addr.dst = ADDR_HOST_VIP;
        fd = tms_SelectFdByAddr(&dst_addr.dst);
        dst_addr.pkid = creat_pkid();
        bzero(buf, sizeof(buf));
        pDataHead = (_tagDataHead *)(buf);
        pDataHead->cmd = ID_ALARM_HW;
        pDataHead->dataLen = sizeof(_tagDataHead);
        input_cpy_sock_retry_queue(buf, dst_addr.pkid);

    }
    else
    {
        printf("%s(): Line : %d  send  hw total alarm type error \n",  __FUNCTION__, __LINE__);
        goto usr_exit;
    }
    tms_AlarmHW(fd,&dst_addr,alarm_num,( tms_alarm_hw_val *)hw_val);
usr_exit:
    return 0;
}
/*
   **************************************************************************************
 *  函数名称：check_is_hw_alarm
 *  函数描述：输入告警原因检查是否是硬件告警。告警原因具有多样性，统一
 *                ：检查方便以后修改
 *  入口参数：告警原因
 *  返回参数：是否是告警 0 是告警，-1 不是告警
 *  作者       ：
 *  日期       ：2016-02-01
 *  修改日期：
 *  修改内容：
 *                ：
 **************************************************************************************
*/
int MainWindow::check_is_hw_alarm(int alarm_reason)
{
    int retv;
    retv = RET_SUCCESS;
    //告警恢复，电源正常均不认为是告警
    if(
            (alarm_reason == CARD_ALARM_RECOVER) ||\
            (alarm_reason == CARD_ALARM_POWER_NORMAL)
            )
    {
        retv = -1;
    }
usr_exit:
    return retv;
}

//槽函数
int MainWindow::s_update_dev_type()
{
    update_dev_type();
    return 0;
}
/*
   **************************************************************************************
 *  函数名称：get_dev_composition_fromdb
 *  函数描述：从数据库中获取板卡组成信息
 *  入口参数：
 *  返回参数：
 *  作者       ：
 *  日期       ：
 **************************************************************************************
*/
int MainWindow::get_dev_composition_fromdb()
{
    //板卡组成
    tdb_common dev_comp;
    tdb_common mask;
    int res_db, j;
    int res_code;
    res_code = RET_SUCCESS;
    memset(&m_subrackInfo, 0, sizeof(_tagSubrackInfo));
    memset(m_subrackCard, 0, sizeof(_tagSubrackCard)*NUM_SUBRACK);


    memset(&mask, 0, sizeof(tdb_common));
    memset(&dev_comp, 0, sizeof(tdb_common));
    mask.val1 = 1;
    dev_comp.val1 = DB_COMM_ID_CARD_COMMP;
    res_db = tmsdb_Select_common(&dev_comp, &mask,db_read_card_commp, m_subrackCard);
    if(res_db == RET_SUCCESS)
    {
        res_code = check_card_comp(m_subrackCard);
        if(res_code == RET_SUCCESS)
            get_frame_state_from_card_comp();
    }
    if(res_db != RET_SUCCESS || res_code != RET_SUCCESS || m_subrackInfo.numInUse == 0)
    {
        qDebug("get dev commp from db error res_db %d res_code %d frame numInuse %d" ,\
               res_db, res_code,m_subrackInfo.numInUse);

        m_subrackInfo.numTotal = NUM_SUBRACK;
        m_subrackInfo.numInUse = 1;
        m_subrackInfo.onState = 1;
        m_subrackCard[0].onState = (1 << MCU_CARD);
        m_subrackCard[0].numInUse = 1;
        m_subrackCard[0].numTotal = NUM_CARD;
        m_subrackCard[0].type[MCU_CARD] = MCU;
        m_subrackCard[0].type[MCU_CARD + 1] = NONE;
        m_subrackCard[0].type[MCU_CARD + 2] = NONE;

    }
    return 0;

}
//检查板卡组成信息
int MainWindow::check_card_comp(_tagSubrackCard *pSubRackCard)
{
    int i, j, return_val;
    int state, result;
    return_val = 0;
    /*
     *MCU必须存在  2016-01-11
    */
    state = pSubRackCard[0].onState;
    if(((state >>MCU_CARD) & 1) != 1)
    {
        pSubRackCard[0].onState  = (pSubRackCard[0].onState | 1 << MCU_CARD);
        pSubRackCard[0].numInUse++;
        pSubRackCard[0].type[MCU_CARD] = MCU;
    }

    for(i = 0; i < NUM_SUBRACK;i++)
    {
        state = pSubRackCard[i].onState;
        pSubRackCard[i].numTotal = NUM_CARD;
        pSubRackCard[i].numInUse = 0;
        for(j = 0; j < NUM_CARD;j++)
        {
            if(((state >>j) & 1) == 1)
            {
                if(pSubRackCard[i].type[j] < m_ctrlStat.minDevType ||\
                        pSubRackCard[i].type[j] > m_ctrlStat.maxDevType)
                {
                    return_val = -1;
                    break;
                }
                result = check_dev_type_port_num(pSubRackCard[i].type[j], pSubRackCard[i].ports[j]);
                if(result < 0)
                {
                    return_val = -1;
                    break;
                }
                else
                {
                    pSubRackCard[i].numInUse++;
                }
            }

        }
    }
    //if(return_val != RET_SUCCESS)
    //    memset(pSubRackCard, 0, sizeof(_tagSubrackCard)*NUM_CARD);
    return return_val;
}

//从板卡组成信息中得到机框组成信息
int MainWindow::get_frame_state_from_card_comp()
{
    int i, return_val;
    int  frameState;
    return_val = 0;
    frameState = 0;
    memset(&m_subrackInfo, 0,sizeof(m_subrackInfo));
    m_subrackInfo.numTotal = NUM_SUBRACK;
    for(i = 0; i < NUM_SUBRACK;i++)
    {
        if(m_subrackCard[i].onState > 0)
        {
            m_subrackInfo.numInUse++;
            memcpy(m_subrackInfo.oprateTime[i], m_subrackCard[i].oprateTime,\
                   sizeof(m_subrackCard[i].oprateTime));
            frameState = (frameState | 1<< i);
        }
    }
    m_subrackInfo.onState = frameState;
    return return_val;
}
/*
   **************************************************************************************
 *  函数名称：wrtie_db_card_comp
 *  函数描述：板卡组成改变之后需要调用该函数保存配置信息
 *  入口参数：
 *  返回参数：
 *  作者       ：
 *  日期       ：
 **************************************************************************************
*/

int MainWindow::wrtie_db_card_comp()
{
    int res_db, res_code;
    int return_val;
    tdb_common_t input, mask;
    char msg[SHORT_MSG_LEN];
    QString str;

    glink_addr dst_addr;
    dst_addr.src = ADDR_MCU;
    dst_addr.dst = ADDR_NET_MANAGER;
    return_val = RET_SUCCESS;

    memset(&input, 0, sizeof(tdb_common_t));
    memset(&mask, 0, sizeof(tdb_common_t));

    input.val1 = DB_COMM_ID_CARD_COMMP;
    mask.val1 = 1;
    input.lenpdata = NUM_SUBRACK * sizeof(_tagSubrackCard);
    input.pdata = m_subrackCard;
    res_code = check_card_comp(m_subrackCard);
    if(res_code == RET_SUCCESS)
    {
        res_db =  tmsdb_Delete_common(&input, &mask);
        if(res_db >= 0)
            res_db = tmsdb_Insert_common(&input, &mask, 1);
    }

    if(res_code != RET_SUCCESS)
    {
        memset(msg, 0, sizeof(msg));
        sprintf(msg, "check card commp error! ");
        qDebug("%s",msg);
        //tms_Trace(&dst_addr,msg, strlen(msg),1);
        return_val = res_code ;
        str = tr("配置数据检查失败！");
    }
    else if(res_db != RET_SUCCESS)
    {
        memset(msg, 0, sizeof(msg));
        sprintf(msg, "write db card commp error res_db %d !", res_db);
        qDebug("%s",msg);
        // tms_Trace(&dst_addr,msg, strlen(msg),1);
        return_val = RET_IGNORE_SAVE;
        str = tr("配置无法写入数据库！");
    }
    else
    {
        str = tr("配置数据成功!");
    }
    // dealAbnormal(str);
    get_otdr_card_slot(0);
    /*
     *2016-02-01在修改板卡组成配置之后回重新更新以下总的光功告警池子
     *和olp切换记录池子,同时将告警数据进行保存到数据库
    */

    refresh_opm_alarm();
    refresh_olp_action_log();
    save_data_befor_reboot();
    return return_val;
}
/*
   **************************************************************************************
 *  函数名称：refresh_opm_alarm
 *  函数描述：重新配置板卡组成信息后，检查当前总的光功告警池子，删除那些
 *                ：非olp或者OPM类型的element
 *  入口参数：
 *  返回参数：
 *  作者       ：
 *  日期       ：2016-02-01
 *  修改日期：
 *  修改内容：
 *                ：
 **************************************************************************************
*/
int MainWindow::refresh_opm_alarm()
{
    int i, retv;
    int frame, card, type;
    retv = 0;
    total_opm_alarm.mutexBoj.lock();
    printf("%s(): Line : %d  opm/olp size %d \n", \
           __FUNCTION__, __LINE__,total_opm_alarm.OpmList.size());
    for(i = 0; i < total_opm_alarm.OpmList.size();)
    {
        frame = total_opm_alarm.OpmList[i].frame;
        card = total_opm_alarm.OpmList[i].card;
        type = total_opm_alarm.OpmList[i].type;
        retv = check_dev_type(frame, card, type);
        if(retv != RET_SUCCESS)
        {
            total_opm_alarm.OpmList.removeAt(i);
        }
        else
        {
            i++;
        }

    }
    total_opm_alarm.mutexBoj.unlock();

    return retv;

}
/*
   **************************************************************************************
 *  函数名称：refresh_olp_action_log
 *  函数描述：重新配置板卡组成信息后，检查当前总的OLP切换记录，删除那些
 *                ：非olp element
 *  入口参数：
 *  返回参数：
 *  作者       ：
 *  日期       ：2016-02-01
 *  修改日期：
 *  修改内容：
 *                ：
 **************************************************************************************
*/
int MainWindow::refresh_olp_action_log()
{
    int i, retv;
    int frame ,card ,type;
    OlpActionRecordBuf.obj.lock();
    printf("%s(): Line : %d  log num %d \n", \
           __FUNCTION__, __LINE__,OlpActionRecordBuf.cur_total_record);
    for(i= 0; i < OlpActionRecordBuf.list.size();)
    {
        if(i >= OlpActionRecordBuf.cur_total_record)
            break;
        frame = OlpActionRecordBuf.list[i].frame;
        card = OlpActionRecordBuf.list[i].card;
        type = OlpActionRecordBuf.list[i].type;
        retv = check_dev_type(frame, card ,type);
        if(retv != RET_SUCCESS)
        {
            OlpActionRecordBuf.list.remove(i);
            OlpActionRecordBuf.cur_total_record--;
            if(i > 0 && OlpActionRecordBuf.cur_index >= i)
                OlpActionRecordBuf.cur_index--;
        }
        else
        {
            i++;
        }
    }
    OlpActionRecordBuf.obj.unlock();
    return retv;
}
/*
   **************************************************************************************
 *  函数名称：check_dev_type_port_num
 *  函数描述：检查端口数目(注意：不是序号，是总数)是否合法
 *  入口参数：
 *  返回参数：-1代表类型与总数不匹配，0代表成功
 *  作者       ：
 *  日期       ：
 *  修改       ：OPM端口可以是0～最大端口之间任何一个端口，OTDR端口也随时变化
 *  修改       ：所以，不在检查端口数目，该函数全部返回0 time :2015-10-26
 **************************************************************************************
*/
int MainWindow::check_dev_type_port_num(int dev_type, int port_num)
{
    int  result ;
    result = 0;
    /*
    QString str;
    str.setNum(port_num);
    switch(dev_type)
    {
    case OPM:
    case OLS:
    case OSW:
    {
        result = strlstDevPortNum.indexOf(str);
        break;
    }
    case OTDR:
    {
        result = strlstOtdrPortNum.indexOf(str);
        break;
    }
    case OLP:
    {
        result = strlstOlpPortNum.indexOf(str);
        break;
    }
    default:
    {
        if(port_num == 0)
            result = 0;
        break;
    }
    }
    if(result == -1)
        qDebug("type %d port_num %d", dev_type, port_num);
    */
    qDebug("type %d port_num %d", dev_type, port_num);
    return 0;
}
/*
   **************************************************************************************
 *  函数名称：check_frame_card_range
 *  函数描述：检查 机框编号，板卡编号是否越界
 *  入口参数：
 *  返回参数：-1代表越界，0代表成功
 *  作者       ：
 *  日期       ：
 **************************************************************************************
*/
int MainWindow::check_frame_card_range(int frame_no, int card_no)
{
    int result;
    result = 0;
    if(frame_no < 0 || frame_no > NUM_SUBRACK)
        result = -1;
    else if( card_no < 0 || card_no > NUM_CARD)
        result = -1;

    return result;
}
/*
   **************************************************************************************
 *  函数名称：local_process_cmd
 *  函数描述：mainwindow处理cmd的函数
 *  入口参数：
 *  返回参数：
 *  作者       ：
 *  日期       ：
 **************************************************************************************
*/
int MainWindow::local_process_cmd(char buf[], int dataLen,int msec, void *ptr_opt)
{
    int returnValue, count;
    _tagDataHead *pdata_head;
    tms_context *ptr_context;
    ptr_context = (tms_context *)ptr_opt;
    returnValue = 0;
    count = 0;
    pdata_head = (_tagDataHead *)(buf);
    qDebug("from_sock cmd %x, datalen %d src 0x%x dst 0x%x", \
           pdata_head->cmd,pdata_head->dataLen,ptr_context->pgb->src,ptr_context->pgb->dst);
    /*
     *2016-03-10 如果从底层收到曲线必须进行处理，否则otdr管理线程收不到回应回超时的
    */
    //如果曲线等大批量数据，不用环形缓冲区，有专门的缓存
    if(ID_RET_OTDR_TEST == pdata_head->cmd || ID_RET_ALARM_TEST == pdata_head->cmd||\
            ID_RET_OTDR_TEST_CYC == pdata_head->cmd)
    {
        dealRcvCardOtdrCrv(buf, ptr_opt); //板卡----->otdr曲线
    }
    /*
     *2016-01-19 如果目的地址不是本地，直接返回
     *2016-01-25 如果不处理ID_CMD_ACK，点名测量的命令会出错
    */
    if(ptr_context->pgb->dst != ADDR_MCU&& pdata_head->cmd != ID_CMD_ACK)
        return 0;
    if(ID_ALARM_OPM_CHANGE == pdata_head->cmd)
    {
        dealRcvCardOpmAlarm(buf, ptr_opt); //板卡--->变化光功率告警
    }
    else if(ID_ALARM_OPM == pdata_head->cmd )
    {
        dealRcvCardOpmAlarmTotal(buf, ptr_opt);//板卡--->总的光功率告警
    }
    else if(ID_CMD_SMS_ERROR == pdata_head->cmd)
    {
        dealRcvGsmRes( buf);//板卡---->短信操作返回码
    }
    else if(ID_RET_DEVTYPE == pdata_head->cmd)
    {
        dealRcvCardDevtype(buf, ptr_opt); //板卡---->设备类型
    }
    else if(ID_RET_OPM_OP == pdata_head->cmd)
    {
        rcv_opm_power(buf,ptr_opt);//收到光功率值,要在对话框上显示
    }
    else if(ID_REPORT_OLP_ACTION == pdata_head->cmd)
    {
        olp_switch(buf, ptr_opt);//板卡----->OLP 切换
    }
    else if(ID_RET_OTDR_PARAM == pdata_head->cmd)
    {
        rcv_otdr_modul_para(buf, ptr_opt);//板卡----->otdr模块参数 波长动态范围
    }
    else if(ID_RET_DEV_STATE_FROM_TU == pdata_head->cmd \
            || ID_RET_POWER_STATE_FROM_TU == pdata_head->cmd )
    {
        dealRcvTuReportInfor(buf, ptr_opt); //板卡--->收到的TU报告的设备插拔状态或者电源信息
    }
    else if(ID_DEV_RET_MCU_ALARM == pdata_head->cmd)
    {
        dealRcvCardHwAlarm(buf, ptr_opt); //板卡---->收到板卡上报的硬件告警
    }
    else if(ID_CMD_ACK == pdata_head->cmd)
    {
        process_cmd_ack(buf, ptr_opt);//板卡/网管----->收到回应码
    }
    else if(ID_OLP_REQUEST_OTDR == pdata_head->cmd) //OLP发生故障，请求对指定端口进行测试
    {
        olp_ask_test(buf, ptr_opt);
    }
    else if(ID_GET_COMPOSITION == pdata_head->cmd)
    {
        getCfgCardForm(ptr_opt);//网管----->查询已配置板卡组成
    }
    else if(ID_GET_COMPOSITION_RT == pdata_head->cmd)
    {
        getRealCardForm(ptr_opt);//网管----->查询实时板卡组成
    }
    else if(ID_ACK_COMPOSITION == pdata_head->cmd)
    {
        rcvConfirmCardForm(buf,ptr_opt);//网管------>网管确认板卡组成
    }
    else if(ID_CFG_MCU_OSW_PORT == pdata_head->cmd || \
            ID_CFG_MCU_OLP_PORT == pdata_head->cmd)
    {
        //网管----->配置osw端口关联光缆信息
        save_osw_port_cable_infor(buf,ptr_opt);
    }
    else if( ID_CFG_MCU_OSW_PORT_CLEAR == pdata_head->cmd ||\
             ID_CFG_MCU_OLP_PORT_CLEAR == pdata_head->cmd)
    {
        //网管----->清除端口关联光缆信息
        clear_osw_port_cable_infor(buf, ptr_opt);
    }
    else if(ID_CFG_MCU_U_OPM_OSW == pdata_head->cmd || \
            ID_CFG_MCU_U_OLP_OSW== pdata_head->cmd)
    {
        save_port_refer_osw(buf,ptr_opt);//网管----->配置opm/olp---osw
    }
    else if(ID_CFG_MCU_U_OPM_OSW_CLEAR == pdata_head->cmd || \
            ID_CFG_MCU_U_OLP_OSW_CLEAR== pdata_head->cmd)
    {
        clear_port_refer_osw(buf, ptr_opt);//网管----->清除opm/olp----osw
    }
    else if(ID_CFG_SMS == pdata_head->cmd)
    {
        save_sms_cfg(buf,ptr_opt);  //网管----->配置通过短信发送告警权限
    }
    else if(ID_CFG_SMS_CLEAR == pdata_head->cmd)
    {
        clear_sms_cfg(buf,ptr_opt);//网管----->清除通过短信发送告警权限
    }
    else if(ID_CFG_OTDR_REF == pdata_head->cmd)
    {
        dealRcvNMReferCrv(buf, ptr_opt);//网管---->收到参考曲线
    }
    else if(ID_DEL_TBOTDRREFDATA == pdata_head->cmd || ID_DELALL_TBOTDRREFDATA == pdata_head->cmd)
    {
        db_del_otdr_refer_curv(buf, ptr_opt);//网管---->逐条删除，全部删除参考曲线
    }
    else if( ID_INSERT_TBUNIT== pdata_head->cmd)
    {
        db_save_A_trigger_B(buf, ptr_opt);//网管---->收到告警触发表
    }
    else if(ID_DEL_TBUNIT == pdata_head->cmd || ID_DELALL_TBUNIT == pdata_head->cmd)
    {
        db_del_A_trigger_B(buf, ptr_opt);//网管---->逐条删除，全部删除告警触发表
    }
    else if(ID_INSERT_TBROUTE == pdata_head->cmd)
    {
        db_save_module_rout(buf, ptr_opt);//网管---->插入模块路由表
    }
    else if( ID_DELALL_TBROUTE == pdata_head->cmd)
    {
        db_del_module_rout(buf, ptr_opt);//网管---->全部删除模块路由表
    }
    else if(ID_CFG_MCU_OSW_CYCLE == pdata_head->cmd || ID_INSERT_TBCYCTEST ==  pdata_head->cmd)
    {
        db_save_cyc_cfg(buf, ptr_opt);//网管---->配置端口周期性测量
    }
    else if(ID_DELALL_TBCYCTEST== pdata_head->cmd || ID_DEL_TBCYCTEST ==  pdata_head->cmd)
    {
        db_del_cyc_cfg( buf, ptr_opt); //网管---->删除周期性测量端口
    }
    else if(ID_GET_OTDR_TEST == pdata_head->cmd || ID_GET_OTDR_TEST_CYC == pdata_head->cmd)
    {
        dealNMAppointTest(buf,ptr_opt);//网管----->点名测量或者周期性测量
    }
    else if(ID_CU_NOTE_MANAGE_CONNECT == pdata_head->cmd)
    {
        dealRcvHostNetManagStats(buf, ptr_opt);//网管---->网管建立连接
    }
    else if(ID_GET_SN == pdata_head->cmd)
    {
        dealRcvNMAskSerilNum(buf, ptr_opt);//网管---->网管查询序列号
    }
    else if(ID_ADJUST_TIME == pdata_head->cmd || ID_GET_MCU_TIME == pdata_head->cmd)
    {
        pc_adjust_mcu_time(buf, ptr_opt); //网管---->网管校时，查询时间
    }
    else if(ID_GET_VERSION == pdata_head->cmd || ID_GET_DEV_PRODUCE == pdata_head->cmd)
    {
        send_verson2PC(buf, ptr_opt); ////网管---->网管查询软件，硬件版本号
    }
    else if(ID_GET_TOTAL_HW_ALARM == pdata_head->cmd)
    {
        send_total_hw_alarm(SEND_TYPE_ASK,-1,ptr_opt);//查询总的硬件告警
    }
    else if(ID_GET_OLP_ACTION_LOG == pdata_head->cmd)
    {
        send_olp_action_record_to_host(SEND_TYPE_ASK,-1, ptr_opt);//查询olp切换记录
    }
    else if(ID_ALARM_SOUND_ON_OFF == pdata_head->cmd)
    {
        ctrl_mcu_alarm_sound(buf, ptr_opt);//开关mcu声音
    }
    else if(ID_GET_ALARM_SOUND_STATE == pdata_head->cmd)
    {
        nm_get_alarm_sound_state(buf,ptr_opt);//网管查询当前告警声音状态
    }
    else if(ID_CMD_SMS_TEXT == pdata_head->cmd)
    {
        RcvNMShorMsg(buf, ptr_opt); //网管发送短消息
    }
    else if(ID_GET_TOTAL_OP_ALARM == pdata_head->cmd)
    {
        send_totoal_opm_alarm(SEND_TYPE_ASK, -1, ptr_opt);//网管查询总的光功告警
    }
    else if(ID_GET_ALARM_POWER == pdata_head->cmd)
    {
        ret_sms_authority(buf,ptr_opt);//网管查询短信告警权限
    }
    else if(ID_GET_MCU_OSW_PORT == pdata_head->cmd)
    {
        ret_port_infor(buf,ptr_opt);//网管查询端口关联信息
    }
    else if(ID_GET_OTDR_REF == pdata_head->cmd)
    {
        ret_refer_curv(buf,ptr_opt);//网管查询短参考曲线
    }
    else if(ID_GET_TBROUTE == pdata_head->cmd)
    {
        //ret_sms_authority(buf,ptr_opt);//网管查询模块级联表
    }
    else if(ID_GET_TBUNIT == pdata_head->cmd)
    {
        // ret_sms_authority(buf,ptr_opt);//网管查询端口触发表
    }
    else if(ID_GET_TBCYCTEST == pdata_head->cmd)
    {
        ret_sms_authority(buf,ptr_opt);//网管查询周期性测量表
    }
    //长度小于2k
    else if(pdata_head->dataLen <= 2048)
    {
        inputDataToRingBuf( buf,  dataLen,  msec);
    }
    return  returnValue;
}
/*
   **************************************************************************************
 *  函数名称：ret_sms_authority
 *  函数描述：返回短信告警权限
 *                ：
 *  入口参数：
 *  返回参数：
 *  作者       ：
 *  日期       ：2016-03-11
 *  修改日期：
 *  修改内容：
 *                ：
 **************************************************************************************
*/
int MainWindow::ret_sms_authority(char buf[],void *ptr_opt)
{

    int i,ret,count, offset,row;
    tdb_sms_t cfg_sms, mask_sms ,*smsout;
    glink_addr dst;
    tms_context *pcontext;
    tms_cfg_sms_val *smslist;
    _tagDataHead *pDataHead;

    count = 0;
    pcontext = (tms_context *)ptr_opt;
    pDataHead = (_tagDataHead *)(buf);
    dst.src = ADDR_MCU;
    dst.dst = pcontext->pgb->src;
    dst.pkid = pcontext->pgb->pkid;

    ret = RET_SUCCESS;
    smsout = NULL;
    smslist = NULL;
    bzero(&cfg_sms, sizeof(tdb_sms_t));
    bzero(&mask_sms, sizeof(tdb_sms_t));
    row = tmsdb_Select_sms(&cfg_sms, &mask_sms, &smsout);
    if(row >  0 && smsout != NULL)
    {
        smslist = new tms_cfg_sms_val [row];
        if(smslist == NULL)
        {
            ret = RET_SYS_NEW_MEM_ERR;
            goto usr_exit;
        }
        offset = 0;
        for(i = 0; i < row;i++)
        {
            memcpy(smslist + offset, &smsout[i].time,sizeof(tms_cfg_sms_val));
        }
    }
    if(smsout != NULL)
        free(smsout);
usr_exit:
    if(ret != RET_SUCCESS)
    {
        mcu_Ack(ptr_opt,pDataHead->cmd,ret);
    }
    else
    {
        /*
         *2016-03-14 该函数本用来网管模拟配置，现在用来向上发送短信内容
        */
        tms_CfgSMSAuthorization(pcontext->fd,&dst,row,smslist);
    }
    if(smslist != NULL)
        delete []smslist;
    printf("%s(): Line : %d  row %d  ret %d \n",  __FUNCTION__, __LINE__,row, ret);
    return ret;
}
/*
   **************************************************************************************
 *  函数名称：ret_port_infor
 *  函数描述：网管查询端口的关联信息时的返回函数
 *                ：
 *  入口参数：
 *  返回参数：
 *  作者       ：
 *  日期       ：
 *  修改日期：2016-03-14
 *  修改内容：
 *                ：
 **************************************************************************************
*/
int MainWindow::ret_port_infor(char buf[], void *ptr_opt)
{
    int ret,row,i;
    int cmd;
    glink_addr dst;
    tms_context *pcontext;
    _tagDataHead *pDataHead;
    _tagDevComm *pDev;
    //光开关关联地理信息
    tdb_dev_map_t *map_buf = NULL;
    tdb_dev_map_t cfg_map, mask_map;

    pDataHead = (_tagDataHead *)buf;
    pcontext = (tms_context *)ptr_opt;
    pDev = (_tagDevComm *)(buf + sizeof(_tagDataHead));
    cmd = ID_CFG_MCU_OSW_PORT;
    //目的地址
    dst.pkid = pcontext->pgb->pkid;
    dst.src = ADDR_MCU;
    dst.dst = pcontext->pgb->src;

    ret = check_dev_type(pDev->frame_no, pDev->card_no, pDev->type);
    if(ret != RET_SUCCESS)
        goto usr_exit;
    ret = check_dev_port_no(pDev->frame_no,pDev->card_no,pDev->port);
    if(ret != RET_SUCCESS)
        goto usr_exit;

    bzero(&mask_map, sizeof(tdb_dev_map_t));
    bzero(&cfg_map, sizeof(tdb_dev_map_t));
    memcpy(&cfg_map.frame, pDev, sizeof(_tagDevComm));

    mask_map.frame = 1;
    mask_map.slot = 1;
    mask_map.port = 1;
    mask_map.type = 1;
    row = tmsdb_Select_dev_map(&cfg_map, &mask_map, &map_buf);
    if(row <= 0)
    {
        ret = RET_UNEXIST_ROW;
        goto usr_exit;
    }
    else
    {
        ret = RET_SUCCESS;
        for(i = 0; i < row;i++)
        {

            tms_CfgMCUAnyPort(pcontext->fd,&dst,cfg_map.frame,cfg_map.slot,cfg_map.type,cfg_map.port,\
                              cmd, &cfg_map.dev_name, &cfg_map.cable_name, &cfg_map.start_name,\
                              &cfg_map.end_name);
        }
    }
usr_exit:
    if(map_buf != NULL)
        free(map_buf);
    if(ret != RET_SUCCESS)
        mcu_Ack(ptr_opt,pDataHead->cmd,ret);
    printf("%s(): Line : %d  row %d ret %d \n",  __FUNCTION__, __LINE__,row,ret);
    return ret;
}
/*
   **************************************************************************************
 *  函数名称：ret_refer_curv
 *  函数描述：网管查询参考曲线时的返回结果
 *                ：
 *  入口参数：
 *  返回参数：
 *  作者       ：
 *  日期       ：2016-03-14
 *  修改日期：
 *  修改内容：
 *                ：
 **************************************************************************************
*/
int MainWindow::ret_refer_curv(char buf[], void *ptr_opt)
{
    int ret,row,i;
    int cmd;
    glink_addr dst;
    tms_context *pcontext;
    _tagDataHead *pDataHead;
    _tagDevComm *pDev;
    _tagReferCurv *pReCurv = NULL;


    struct tms_otdr_ref_hdr       *pref_hdr;
    struct tms_retotdr_test_param *ptest_param;
    struct tms_retotdr_data_hdr   *pdata_hdr;
    struct tms_retotdr_data_val   *pdata_val;
    struct tms_retotdr_event_hdr  *pevent_hdr;
    struct tms_retotdr_event_val  *pevent_val;
    struct tms_retotdr_chain      *pchain;
    struct tms_cfg_otdr_ref_val   *pref_date;

    pDataHead = (_tagDataHead *)buf;
    pcontext = (tms_context *)ptr_opt;
    pDev = (_tagDevComm *)(buf + sizeof(_tagDataHead));
    cmd = ID_CFG_MCU_OSW_PORT;
    //目的地址
    dst.pkid = pcontext->pgb->pkid;
    dst.src = ADDR_MCU;
    dst.dst = pcontext->pgb->src;

    ret = check_dev_type(pDev->frame_no, pDev->card_no, pDev->type);
    if(ret != RET_SUCCESS)
        goto usr_exit;
    ret = check_dev_port_no(pDev->frame_no,pDev->card_no,pDev->port);
    if(ret != RET_SUCCESS)
        goto usr_exit;

    pReCurv = new _tagReferCurv;
    if(pReCurv == NULL)
    {
        ret = RET_SYS_NEW_MEM_ERR;
        goto usr_exit;
    }

    ret = db_get_refr_otdr(pDev, pReCurv);


    if(ret != RET_SUCCESS)
    {
        goto usr_exit;
    }
    else
    {
        ret = RET_SUCCESS;
        pref_hdr = (tms_otdr_ref_hdr *)(&pReCurv->ref_head.osw_frame);
        ptest_param = (tms_retotdr_test_param *)(&pReCurv->measurPara.range_m);
        pdata_hdr = (tms_retotdr_data_hdr *)(&pReCurv->dpid);
        pdata_val = (tms_retotdr_data_val *)(pReCurv->dataPt);
        pevent_hdr = (tms_retotdr_event_hdr *)(pReCurv->eventID);
        pevent_val = (tms_retotdr_event_val *)(pReCurv->eventBuf);
        pchain = (tms_retotdr_chain *)(pReCurv->measurResultID);
        pref_date = (tms_cfg_otdr_ref_val *)(&pReCurv->alarmTh.seriousAlarmTh);
        tms_CfgOTDRRef(pcontext->fd,&dst,
                       pref_hdr,
                       ptest_param,
                       pdata_hdr,
                       pdata_val,
                       pevent_hdr,
                       pevent_val,
                       pchain,
                       pref_date);
    }
usr_exit:
    if(pReCurv != NULL)
        delete pReCurv;
    if(ret != RET_SUCCESS)
        mcu_Ack(ptr_opt,pDataHead->cmd,ret);
    printf("%s(): Line : %d  ret %d \n",  __FUNCTION__, __LINE__,ret);
    return ret;
}
/*
   **************************************************************************************
 *  函数名称：ret_module_cascade_table
 *  函数描述：命令码，0x80000088,返回模块级联表，具体内容返回0x80000054
 *                ：协议内容
 *  入口参数：ptr_opt:数据请求方的相关信息,比如fd，源地址，目的地址
 *  返回参数：处理结果
 *  作者       ：
 *  日期       ：2016-03-31
 *  修改日期：
 *  修改内容：
 *                ：
 **************************************************************************************
*/
int MainWindow::ret_module_cascade_table(char buf[],void *ptr_opt)
{
    int count, retv;
    tdb_route_t condition;
    tdb_route_t mask;
    tms_context *pcontext;
    tms_route  *rout_buf;
    _tagDataHead *pDataHead;
    _tagDBCallbackPara dbCallbackPara;
    glink_addr dst_addr;

    pDataHead = (_tagDataHead *)buf;    
    rout_buf = NULL;
    pcontext = (tms_context *)(ptr_opt);
    char sql[] = "select count() from tb_route where(1);";
    //获取该此表有多少记录
    count =  tmsdb_Select_count(sql, NULL, NULL);

    if(count > 0){
        rout_buf = new tms_route [count];
        if(rout_buf == NULL){
            retv = RET_MEM_ERR;
            goto usr_exit;
            bzero(&dbCallbackPara, sizeof(dbCallbackPara));
            bzero(rout_buf, sizeof(tms_route)*count);
            dbCallbackPara.list_num = count;
            dbCallbackPara.dst_buf = (char *)rout_buf;
            bzero(&condition, sizeof(condition));
            bzero(&mask, sizeof(mask));
            tmsdb_Select_Page_route(
                        &condition,
                        &mask,
                        0,
                        count,
                        db_read_all_rout,
                        (void *)&dbCallbackPara);


        }
    }
    else{
        count = 0;
    }
    dst_addr.src = ADDR_MCU;
    dst_addr.dst = pcontext->pgb->dst;
    dst_addr.pkid = pcontext->pgb->pkid;
    tms_TbRoute_Insert(pcontext->fd,&dst_addr,count,rout_buf);
usr_exit:
    if(retv != RET_SUCCESS)
        mcu_Ack(ptr_opt, pDataHead->cmd,retv,count);
    printf("%s(): Line : %d route total record %d  \n",  __FUNCTION__, __LINE__, count);
    return retv;
}

/*
   **************************************************************************************
 *  函数名称：send_totoal_opm_alarm
 *  函数描述：上报总的光功告警
 *                ：
 *  入口参数：
 *  返回参数：
 *  作者       ：
 *  日期       ：2016-01-18
 *  修改日期：
 *  修改内容：
 *                ：
 **************************************************************************************
*/
int MainWindow::send_totoal_opm_alarm(int send_type,short int pkid, void *ptr_opt)
{
    int byte_size, card_count, i,j;
    int alarm_num,port_count;
    int res_code, offset,fd;
    char  *SendBuf;
    bool isSend;
    _tagFSOpmAlarm frameCardOpm;
    tms_total_op_alarm_val *ptr_total_opm_alarm;
    glink_addr dst_addr;
    tms_context *ptr_context;
    isSend = false;
    ptr_total_opm_alarm = NULL;
    total_opm_alarm.mutexBoj.lock();
    card_count = total_opm_alarm.OpmList.size();
    //先遍历一遍，获取当前光功告警的数目
    byte_size = 0;
    SendBuf = NULL;
    alarm_num = 0;
    port_count = 0;
    for(i = 0; i < card_count; i++)
    {
        frameCardOpm.frame = -1;
        frameCardOpm = total_opm_alarm.OpmList[i];
        if(frameCardOpm.frame == -1) //读取出错
        {
            qDebug("total_opm_alarm value at %d error", i);
            continue;
        }
        res_code = check_dev_port_no(frameCardOpm.frame, frameCardOpm.card, frameCardOpm.port_num - 1);
        if(res_code != RET_SUCCESS)
        {
            qDebug("check total opm alarm error frame %d card %d port_num %d", \
                   frameCardOpm.frame, frameCardOpm.card, frameCardOpm.port_num);
            goto usr_exit;
        }
        /*
         *每个槽位的每个端口都遍历查看是否有告警
         */
        port_count = total_opm_alarm.OpmList[i].alarmArray.size();
        for(j = 0; j < port_count; j++)
        {
            if(total_opm_alarm.OpmList[i].alarmArray[j].lev > ALARM_NONE)
                alarm_num++;
        }
    }
    printf("%s(): %d  total alarm num %d \n",  __FUNCTION__, __LINE__,alarm_num);
    if(alarm_num > 0 )
    {
        byte_size = alarm_num * sizeof(tms_total_op_alarm_val);
        SendBuf = new char [byte_size];
    }

    if( SendBuf == NULL )
    {
        res_code = RET_SYS_NEW_MEM_ERR;
        printf("%s(): Line : %d  new mem erro ret %d \n",  __FUNCTION__, __LINE__,res_code);
        goto usr_exit;
    }
    if(alarm_num > 0)
    {
        //开始拷贝数据
        offset = 0;
        card_count = total_opm_alarm.OpmList.size();
        for(i = 0; i < card_count; i++)
        {
            frameCardOpm.frame = -1;
            frameCardOpm = total_opm_alarm.OpmList[i];
            if(frameCardOpm.frame == -1) //多心了，检查一下
            {
                printf("%s(): %d  total alarm error index %d \n",  __FUNCTION__, __LINE__,i);
                continue;
            }
            res_code = check_dev_port_no(frameCardOpm.frame, frameCardOpm.card, frameCardOpm.port_num - 1);
            if(res_code != RET_SUCCESS)
            {
                printf("%s(): Line : %d total opm alarm error frame %d card %d port_num %d\n", \
                       __FUNCTION__, __LINE__,frameCardOpm.frame, frameCardOpm.card, frameCardOpm.port_num);
                goto usr_exit;
            }
            /*
         *每个槽位的每个端口都遍历查看是否有告警
         */
            port_count = total_opm_alarm.OpmList[i].alarmArray.size();

            for(j = 0; j < port_count;j++)
            {
                ptr_total_opm_alarm = (tms_total_op_alarm_val *)(SendBuf + offset);
                if(total_opm_alarm.OpmList[i].alarmArray[j].lev > ALARM_NONE)
                {
                    offset += sizeof(tms_total_op_alarm_val);
                    ptr_total_opm_alarm->alarm_level = total_opm_alarm.OpmList[i].alarmArray[j].lev;
                    ptr_total_opm_alarm->cur_power = total_opm_alarm.OpmList[i].alarmArray[j].power;
                    strcpy((char *)(ptr_total_opm_alarm->time), total_opm_alarm.OpmList[i].alarmArray[j].come_time);
                    ptr_total_opm_alarm->frame = total_opm_alarm.OpmList[i].frame;
                    ptr_total_opm_alarm->slot = total_opm_alarm.OpmList[i].card;
                    ptr_total_opm_alarm->port = j;
                }
            }
        }
    }

usr_exit:
    total_opm_alarm.mutexBoj.unlock();
    fd = get_dst_host_addr_info((char *)(&dst_addr), send_type,-1, ptr_opt);
    if(fd <= INVALID_FD)
    {
        if(SendBuf != NULL) //释放分配的内存
            delete []SendBuf;
        return 0;
    }
    if(res_code ==  RET_SUCCESS)
        tms_RetTotalOPAlarm(fd,&dst_addr,ALARM_OPM,alarm_num,(tms_total_op_alarm_val*)SendBuf);
    else
    {
        if(alarm_num != 0)
            printf("%s(): Line : %d  nm ask opm alarm SendBuf == NULL alarm num %d \n", \
                   __FUNCTION__, __LINE__, alarm_num);
        alarm_num = 0;
        tms_RetTotalOPAlarm(fd,&dst_addr,ALARM_OPM,alarm_num,NULL);
    }

    if(SendBuf != NULL) //释放分配的内存
        delete []SendBuf;
    return res_code;
}
/*
   **************************************************************************************
 *  函数名称：get_dst_host_addr_info
 *  函数描述：根据发送类型，填充目的地址相关信息，并返回fd
 *                ：
 *  入口参数：
 *  返回参数：
 *  作者       ：
 *  日期       ：2016-01-29
 *  修改日期：
 *  修改内容：
 *                ：
 **************************************************************************************
*/
int MainWindow:: get_dst_host_addr_info(char * dst_addr,int send_type, int pkid , void *ptr_opt)
{
    tms_context *ptr_context;
    glink_addr *pDstAddr;
    int fd;
    fd = INVALID_FD;
    if(dst_addr == NULL)
        goto usr_exit;

    pDstAddr = (glink_addr *)dst_addr;
    if(send_type == SEND_TYPE_ASK)
    {
        ptr_context = (tms_context *)ptr_opt;
        pDstAddr->dst = ptr_context->pgb->src;
        pDstAddr->src = ADDR_MCU;
        pDstAddr->pkid = ptr_context->pgb->pkid;
        fd = ptr_context->fd;
    }
    else if(send_type == SEND_TYPE_INITIATIVE)
    {
        pDstAddr->dst = ADDR_HOST_VIP;
        pDstAddr->src = ADDR_MCU;
        pDstAddr->pkid = creat_pkid();
        fd = tms_SelectFdByAddr(&pDstAddr->dst);
    }
    else if(send_type == SEND_TYPE_RETRY)
    {
        pDstAddr->dst = ADDR_HOST_VIP;
        pDstAddr->src = ADDR_MCU;
        pDstAddr->pkid = pkid;
        fd = tms_SelectFdByAddr(&pDstAddr->dst);
    }
usr_exit:
    if(fd <= 0)
        fd = INVALID_FD;
    return fd;
}

/*
   **************************************************************************************
 *  函数名称：olp_ask_test
 *  函数描述：OLP因故切换时，请求进行otdr测试
 *                ：
 *  入口参数：
 *  返回参数：
 *  作者       ：
 *  日期       ：2015-12-04
 *  修改日期：
 *  修改内容：
 *                ：
 **************************************************************************************
*/
int MainWindow::olp_ask_test(char buf[], void *ptr_opt)
{
    _tagDataHead *pDataHead;
    _tagDevComm Dev;
    int retv, offset;
    retv = RET_SUCCESS;
    pDataHead = (_tagDataHead *)(buf);
    offset = sizeof(_tagDataHead);

    memcpy(&Dev, buf + offset, sizeof(_tagDevComm));
    if(Dev.type != OLP)
        retv = RET_PARAM_INVALID;
    retv = check_frame_card_range(Dev.frame_no,Dev.card_no);
    if(retv != RET_SUCCESS)
        goto usr_exit;
    retv = check_dev_port_no(Dev.frame_no, Dev.card_no, Dev.port);
    if(retv != RET_SUCCESS)
        goto usr_exit;
    retv = input_arlarm_gsm_test_queue(Dev,0,0,TEST_OLP_ACTION,NULL);

usr_exit:
    if(retv != RET_SUCCESS)
        qDebug("olp ask test error % d", retv);
    mcu_Ack(ptr_opt, pDataHead->cmd,retv,0);
    return 0;

}

/*
   **************************************************************************************
 *  函数名称：nm_get_alarm_sound_state
 *  函数描述：网管获取声音告警状态
 *                ：
 *  入口参数：
 *  返回参数：
 *  作者       ：
 *  日期       ：2015-12-01
 *  修改日期：
 *  修改内容：
 *                ：
 **************************************************************************************
*/
int MainWindow::nm_get_alarm_sound_state(char *buf, void *ptr_opt)
{
    int fd;
    glink_addr addr;
    tms_context *ptr_context;
    ptr_context = (tms_context *)ptr_opt;
    fd = ptr_context->fd;
    addr.src = ADDR_MCU;
    addr.dst = ptr_context->pgb->src;
    addr.pkid = ptr_context->pgb->pkid;
    tms_RetAlarmSoundState(fd,&addr,devCfg.gpioAlarm);
    return 0;
}

/*
   **************************************************************************************
 *  函数名称：RcvNMShorMsg
 *  函数描述：收到网管请求发送短信
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
int MainWindow::RcvNMShorMsg(char buf[], void *ptr_opt)
{
    _tagGsmContext text;
    _tagDataHead *pDataHead;
    tms_context *pContext;
    int retv, offset;
    offset = 0;
    retv = RET_SUCCESS;
    pDataHead = (_tagDataHead *)(buf + offset);
    offset += sizeof(_tagDataHead);
    pContext = (tms_context *)ptr_opt;

    bzero(&text, sizeof(text));
    memcpy(text.phone, buf + offset,GSM_PHOME_NUM);
    offset += GSM_PHOME_NUM;
    memcpy(&text.count, buf + offset, sizeof(int));
    offset += sizeof(int);
    //检查短信电话，长度
    if(strlen(text.phone ) > PHONE_ACTUAL_NUM)
    {
        retv = RET_SMS_PHONE_ILLEGALT;
        goto usr_exit;
    }
    if(text.count > GSM_TEXT_LEN)
    {
        retv = RET_SMS_TEXT_VOERFLOW;
        goto usr_exit;
    }
    if(pSmsSend->objSynSem.commu_stat == SMS_STAT_ERROR)
    {
        retv = RET_SMS_EQ_ERROR;
        goto usr_exit;
    }

    memcpy(text.context, buf + offset, text.count * 2);
    text.usr_addr.dst = pContext->pgb->dst;
    text.usr_addr.src = pContext->pgb->src;
    text.usr_addr.pkid = pContext->pgb->pkid;

    pSmsSend->GSMQueue.objMutex.lock();
    if(pSmsSend->GSMQueue.xqueue.size() < GSM_BUF_SIZE)
    {
        pSmsSend->GSMQueue.xqueue.enqueue(text);
    }
    else
    {
        //短消息队列已满，去掉时间最长的一个，然后添加一个
        pSmsSend->GSMQueue.xqueue.dequeue();
        pSmsSend->GSMQueue.xqueue.enqueue(text);
    }
    pSmsSend->GSMQueue.objMutex.unlock();
usr_exit:
    //如果错误码不成功，需要立刻返回处理结果
    if(retv != RET_SUCCESS)
        mcu_Ack(ptr_opt, pDataHead->cmd, retv);
    return 0;

}

/*
   **************************************************************************************
 *  函数名称：ctrl_mcu_alarm_sound
 *  函数描述：host操作mcu声音
 *  入口参数：
 *  返回参数：
 *  作者       ：
 *  日期       ：2015-11-01
 *  修改日期：2016-01-15
 *  修改内容：保存之前要判断设定的有效值是否正确
 **************************************************************************************
*/
int MainWindow::ctrl_mcu_alarm_sound(char buf[], void *ptr_opt)
{
    int val, offset;
    int res_code;
    _tagDataHead *pDataHead;

    pDataHead = (_tagDataHead *)(buf);
    offset = sizeof(_tagDataHead);
    memcpy(&val, buf + offset,sizeof(int));
    if(val == 0 || val == 1)
    {
        res_code = RET_SUCCESS;
    }
    else
    {
        res_code = RET_PARAM_INVALID;
    }
    if(res_code == RET_SUCCESS && devCfg.gpioAlarm != val)
    {
        devCfg.gpioAlarm = val;
        save_dev_cfg();
    }
    mcu_Ack(ptr_opt,pDataHead->cmd, res_code);

    return 0;
}

/*
   **************************************************************************************
 *  函数名称：dealRcvCardHwAlarm
 *  函数描述：板卡上报的硬件告警，TU连带上报电源告警信息
 *  入口参数：
 *  返回参数：
 *  作者       ：
 *  日期       ：2015-10-20
 **************************************************************************************
*/
int MainWindow::dealRcvCardHwAlarm(char buf[], void *ptr_opt)
{
    int retv, alarm_type, alarm_count;
    int offset, i;
    int frame, card;
    bool isDeal;
    _tagDevComm *pDevAlarm; //注意：port所在位置指的是告警数目
    _tagDataHead *pDataHead;
    tms_alarm_hw_change_val * pHwAlarm;

    isDeal = false;
    pDataHead = (_tagDataHead *)(buf);
    offset = sizeof(_tagDataHead);
    //回应板卡
    retv = check_rcv_hw_alarm(buf);
    mcu_Ack(ptr_opt,pDataHead->cmd,retv);
    if(retv != RET_SUCCESS)
        goto usr_exit;

    memcpy(&alarm_type, buf + offset, sizeof(int));
    offset += sizeof(int);
    pDevAlarm = (_tagDevComm *)(buf + offset);
    offset += sizeof(_tagDevComm);

    alarm_count = pDevAlarm->port;
    objHwAlarm.lock();
    frame = pDevAlarm->frame_no;
    card = pDevAlarm->card_no;
    //板卡自认为硬件告警消失
    if(alarm_count == 0)
    {
        //如果当前告警是由本地产生的，那么仍然保持
        if(DevCommuState[frame][card].cur_alarm > CARD_ALARM_LOCAL_MAX_VLAUE ||\
                m_subrackCard[frame].type[card] == TU)
        {
            isDeal = true;
            //如果MCU发现本板卡有硬件告警，则，即使板卡上报告警消失，也不做处理
            if(DevCommuState[frame][card].cur_alarm > CARD_ALARM_LOCAL_MAX_VLAUE)
                DevCommuState[frame][card].cur_alarm = CARD_ALARM_RECOVER;

            //如果是TU设备，硬件告警为0，意味着处于插入的电源正常
            if(m_subrackCard[frame].type[card] == TU)
            {
                for(i = NUM_CARD - 2; i < NUM_CARD;i++)
                    if(DevCommuState[frame][i].card_state == PULL_IN)
                    {
                        DevCommuState[frame][i].pw_state = PW_STATE_NORMAL;
                    }

            }
        }
        else
        {
            qDebug("Rcv hw alarm from card ,alarm_count 0,frame %d card %d cur alarm %d ",\
                   frame, card,DevCommuState[frame][card].cur_alarm );
        }

    }
    else
    {
        offset = sizeof(_tagDataHead) + sizeof(int) + sizeof(_tagDevComm);
        for(i = 0; i < alarm_count; i++)
        {
            pHwAlarm = (tms_alarm_hw_change_val * )(buf + offset);
            offset += sizeof(tms_alarm_hw_change_val);
            frame = pHwAlarm->frame;
            card = pHwAlarm->slot;
            //只有本地产生的硬件告警消失之后，才会处理从板卡收到的硬件告警
            if(DevCommuState[frame][card].cur_alarm > CARD_ALARM_LOCAL_MAX_VLAUE || \
                    m_subrackCard[frame].type[card] == PWU)
            {
                isDeal = true;
                if(m_subrackCard[frame].type[card] == PWU)
                {
                    DevCommuState[frame][card].pw_state = PW_STATE_ERROR;
                }
                else
                {
                    DevCommuState[frame][card].cur_alarm = pHwAlarm->reason;
                }
            }
            else
            {
                qDebug("Rcv hw alarm from card ,frame %d card %d  type %d lev %d reason %d",\
                       frame, card ,m_subrackCard[frame].type[card],pHwAlarm->level,pHwAlarm->reason);
            }

        }
    }

usr_exit:
    objHwAlarm.unlock();
    //准备产生告警
    if(isDeal)
        check_card_commu_state();
    return retv;
}
/*
   **************************************************************************************
 *  函数名称：check_rcv_hw_alarm
 *  函数描述：对收到的硬件告警进行参数是否合法进行检查
 *                ：
 *  入口参数：
 *  返回参数：
 *  作者       ：
 *  日期       ：2015-12-07
 *  修改日期：
 *  修改内容：
 *                ：
 **************************************************************************************
*/
int MainWindow ::check_rcv_hw_alarm(const char buf[])
{
    int retv, offset,i, alarm_count;
    tms_alarm_hw_change_val * pHwAlarm;
    int alarm_type,frame, card;
    _tagDevComm * pDevAlarm;
    //检查头信息
    offset = sizeof(_tagDataHead);
    memcpy(&alarm_type, buf + offset, sizeof(int));
    offset += sizeof(int);
    pDevAlarm = (_tagDevComm *)(buf + offset);
    offset += sizeof(_tagDevComm);

    retv = check_dev_type(pDevAlarm->frame_no, pDevAlarm->card_no, pDevAlarm->type);
    if(retv != RET_SUCCESS)
        goto usr_exit;

    alarm_count = pDevAlarm->port;
    if(alarm_type != ALARM_HARD_WARE || \
            alarm_count > 1024 || alarm_count < 0)
    {
        retv = RET_PARAM_INVALID;
        printf("deal rcv card Hw alarm error type %d  count %d \n", alarm_type, alarm_count);
        goto usr_exit;
    }
    offset = sizeof(_tagDataHead) + sizeof(int) + sizeof(_tagDevComm);
    for(i = 0; i < alarm_count; i++)
    {
        pHwAlarm = (tms_alarm_hw_change_val * )(buf + offset);
        offset += sizeof(tms_alarm_hw_change_val);
        frame = pHwAlarm->frame;
        card = pHwAlarm->slot;
        retv = check_frame_card_range(frame, card);
        if(retv != 0)
        {
            retv = RET_PARAM_INVALID;
            goto usr_exit;
        }

    }
usr_exit:
    return retv;
}

/*
   **************************************************************************************
 *  函数名称：dealRcvTuReportInfor
 *  函数描述：MCU收TU报告设备插拔状态或者电源信息；TU报告除了电源和它
 *  函数描述：本身之外的板卡插拔状态。电源信息报告本机框的电源数目以及其
 *  函数描述：电压要么45V，要么220V
 *  入口参数：
 *  返回参数：
 *  作者       ：
 *  日期       ：2015-10-14
 **************************************************************************************
*/
int MainWindow::dealRcvTuReportInfor(char buf[], void *ptropt)
{
    int ret, offset, i;
    int card_state;
    int frame, card;
    bool result;
    tms_context *pcontext;
    _tagDataHead *pDataHead;
    _tagDevComm *pDevTu;
    _tagDevInfo     *pDevInfo;


    result = false;
    ret = RET_SUCCESS;
    pDataHead = (_tagDataHead *)buf;
    pcontext = (tms_context *)ptropt;
    offset = sizeof(_tagDataHead);
    //先检查TU的机框槽位设备类型是否正确
    pDevTu = (_tagDevComm *)(buf + offset);
    offset += sizeof(_tagDevComm);
    ret = check_frame_card_range(pDevTu->frame_no, pDevTu->card_no);//是否越界
    if(ret != RET_SUCCESS)
        goto usr_exit;
    if( pDevTu->type != TU || pDevTu->card_no != NUM_CARD - 3)//类型，槽位是否正确
    {
        ret = RET_PARAM_INVALID;
        qDebug("Rcv TU error frame %d card %d type %d ",pDevTu->frame_no,\
               pDevTu->card_no, pDevTu->type);
        goto usr_exit;
    }
    //板卡插拔状态
    if(pDataHead->cmd == ID_RET_DEV_STATE_FROM_TU)
    {
        result = false;
        //业务板卡
        card_state = pDevTu->port; //port所在的偏移量正好表示的是板卡插拔状态
        objHwAlarm.lock();
        DevCommuState[pDevTu->frame_no][TU_CARD].card_state = PULL_IN;
        qDebug("card state %d " ,card_state);
        for(i = 0; i < (NUM_COMM_CARD - 1);i++) //要把TU板子排除在外
        {
            //如果板卡拔出
            if(((card_state >> i)&1) == 0 )
            {
                result = true;
                //清空当前拔出的设备
                if(DevCommuState[pDevTu->frame_no][i].card_state == PULL_IN)
                    clear_usr_link(pDevTu->frame_no,i);
                DevCommuState[pDevTu->frame_no][i].cur_type = NONE;
                DevCommuState[pDevTu->frame_no][i].card_state = PULL_OUT;
                qDebug("card %d pull out " ,i);
            }
            else
            {
                DevCommuState[pDevTu->frame_no][i].card_state = PULL_IN;
                qDebug("card %d pull in " ,i);
            }
        }
        objHwAlarm.unlock();

    }
    //收到电源组成
    else if(pDataHead->cmd == ID_RET_POWER_STATE_FROM_TU)//
    {
        result = false;
        //检查电源端口数目
        if(pDevTu->port < 0 || pDevTu->port > POWER_NUM )
        {
            ret = RET_PARAM_INVALID;
            qDebug("power num error %d ", pDevTu->port);
            goto usr_exit;
        }
        offset = sizeof(_tagDataHead) + sizeof(_tagDevComm);
        //检查参数是否合法
        for(i = 0; i < pDevTu->port;i++)
        {
            pDevInfo = (_tagDevInfo *)(buf + offset);
            offset += sizeof(_tagDevInfo);
            //设备类型必须是电源，电压只能是48V，220V,电源槽位也是固定的
            if(pDevInfo->type != PWU  || pDevInfo->frame != pDevTu->frame_no\
                    ||!( pDevInfo->reservd[0] == PWU_V48 ||  pDevInfo->reservd[0] == PWU_V220)
                    ||!( pDevInfo->card == (NUM_CARD - 1) ||  pDevInfo->card == (NUM_CARD - 2))
                    )
            {
                qDebug("mcu rcv power form error  frame %d card %d type %d, v %d ", pDevInfo->frame,\
                       pDevInfo->card,pDevInfo->type, pDevInfo->reservd[0] );
                ret = RET_PARAM_INVALID;
                goto usr_exit;
            }
        }
        //每收到电源组成信息之后，将原来的电源组成信息用新的代替
        objHwAlarm.lock();
        //首先清空电源的设备类型和选项，等待初始化
        frame = pDevTu->frame_no;
        for( i = NUM_CARD - 2; i < NUM_CARD;i ++)
        {
            DevCommuState[frame][i].cur_type = NONE;
            DevCommuState[frame][i].card_state = PULL_OUT;
            bzero(DevCommuState[frame][i].cur_opt, sizeof(int)*CARD_OPT_NUM);
        }

        //开始处理电源, 重新计算偏移量
        offset = sizeof(_tagDataHead) + sizeof(_tagDevComm);
        for(i = 0; i < pDevTu->port;i++)
        {
            pDevInfo = (_tagDevInfo *)(buf + offset);
            offset += sizeof(_tagDevInfo);
            frame = pDevInfo->frame;
            card = pDevInfo->card;
            result = true;
            qDebug("Tu frame %d card %d  v %d type %d",pDevInfo->frame,pDevInfo->card,\
                   pDevInfo->reservd[0],m_subrackCard[frame].type[card]);
            DevCommuState[frame][card].cur_type = PWU;
            DevCommuState[frame][card].card_state = PULL_IN;
            //更新当前的告警
            memcpy(DevCommuState[frame][card].cur_opt, \
                   pDevInfo->reservd, sizeof(DevCommuState[frame][card].cur_opt));
            ret = 0;

        }
        objHwAlarm.unlock();

    }
    //如果数据发生变化，那么就需要处理
    if(result)
        check_card_commu_state();
usr_exit:
    mcu_Ack(ptropt,pDataHead->cmd, ret);
    return ret;
}



/*
   **************************************************************************************
 *  函数名称：send_verson2PC
 *  函数描述：发送软件，硬件版本号到网管
 *  入口参数：
 *  返回参数：
 *  作者       ：
 *  日期       ：
 **************************************************************************************
*/
int MainWindow::send_verson2PC(char buf[], void * ptr_opt)
{
    _tagDataHead *pDataHead;
    glink_addr sendAddr;
    tms_context *popt;
    _tagDevComm *pDev;
    int fd,offset;

    popt = (tms_context *)ptr_opt;
    sendAddr.src = ADDR_MCU;
    sendAddr.dst = popt->pgb->src;
    sendAddr.pkid = popt->pgb->pkid;
    pDataHead = (_tagDataHead *)buf;
    offset = sizeof(_tagDataHead);
    pDev = (_tagDevComm*)(buf + offset);
    if(!(pDev->frame_no == MCU_FRAME && pDev->card_no == MCU_CARD))
    {
        printf("%s(): Line : %d  nm ask hard verson \n",  __FUNCTION__, __LINE__,\
               pDev->frame_no,pDev->card_no);
        goto usr_exit;
    }


    fd = tms_SelectFdByAddr(&sendAddr.dst);
    if(pDataHead->cmd == ID_GET_VERSION) //软件版本号
    {
        char softV[48];
        QByteArray byteArry;
        byteArry = strSoftVerson.toLocal8Bit();
        bzero(softV, sizeof(softV));
        //snprintf(softV, sizeof(softV), "%s\0", byteArry);
        strcpy(softV, byteArry);
        tms_RetVersion(fd, &sendAddr, MCU_FRAME, MCU_CARD, MCU,(uint8_t *)softV);
        qDebug("mcu softV %s", softV);
    }
    else //硬件版本号
    {
        tms_dev_produce hwSofgV;
        bzero(&hwSofgV, sizeof(hwSofgV));
        hwSofgV.frame = MCU_FRAME;
        hwSofgV.slot = MCU_CARD;
        hwSofgV.type = MCU;
        strcpy((char*)(&hwSofgV.hw_ver), "V15.08.28");
        strcpy((char*)(&hwSofgV.date), "2015-08-28 10:37");
        strcpy((char*)(&hwSofgV.sn), "1508281037225523");
        tms_RetDevProduce(fd, &sendAddr, &hwSofgV);
        qDebug("mcu hwSoftV date %s", hwSofgV.date);
    }

usr_exit:
    return 0;

}
/*
   **************************************************************************************
 *  函数名称：rcv_otdr_modul_para
 *  函数描述：获取otdr模块参数，波长，动态范围等
 *  入口参数：
 *  返回参数：
 *  作者       ：
 *  日期       ：
 **************************************************************************************
*/
int MainWindow::rcv_otdr_modul_para(char buf[], void * ptr_opt)
{
    tsk_OtdrManage *pOtdrTsk;
    _tagDevComm *pOtdrDev;
    _tagDevComm OtdrDev;
    QString strLamda;
    QStringList listLamda;
    int i, offset;
    short int lamda, dyscop, count;
    int res_code;
    offset = sizeof(_tagDataHead);
    pOtdrDev = (_tagDevComm *)(buf + offset);
    offset += 12;//板卡，槽位，类型
    memcpy(&count, buf + offset, sizeof(count));
    offset += sizeof(count);
    count = pOtdrDev->port;//实际上是波长的个数
    res_code = -1;
    if(count < 0 || count > 256) //检查波长是否非法
        goto usr_exit;

    OtdrDev.frame_no = pOtdrDev->frame_no;
    OtdrDev.card_no = pOtdrDev->card_no;
    OtdrDev.type = OTDR;
    OtdrDev.port = 0;
    pOtdrTsk = (tsk_OtdrManage *)get_otdr_tsk(OtdrDev);
    if(pOtdrTsk == NULL)
        goto usr_exit;

    res_code = 0;
    listLamda.clear();
    for(i = 0; i < count; i++)
    {
        memcpy(&lamda, buf + offset, sizeof(lamda));
        offset += sizeof(lamda);
        memcpy(&dyscop, buf + offset, sizeof(dyscop));
        offset += sizeof(dyscop);//跳过动态范围
        strLamda.setNum(lamda);
        listLamda.append(strLamda);
        /*
        //调试代码
        qDebug("dyscop %d lamda %d", dyscop, lamda);
        qDebug()<<"strLamda"<<strLamda.simplified();
        strLamda = listLamda.at(0);
        qDebug()<<"strLamda"<<strLamda.simplified();
        */
    }
    //设置波长
    pOtdrTsk->setModulPara(listLamda);
    /*
    //调试代码
    strLamda = pOtdrTsk->modulPara.lamdaList.at(0);
    qDebug()<<"strLamda"<<strLamda.simplified();
    */
usr_exit:
    qDebug("count %d lamda %d res_code %d", count, lamda, res_code);
    return res_code;

}

/*
   **************************************************************************************
 *  函数名称：pc_adjust_mcu_time
 *  函数描述：网管校时
 *  入口参数：
 *  返回参数：
 *  作者       ：
 *  日期       ：
 **************************************************************************************
*/
int MainWindow::pc_adjust_mcu_time(char buf[], void *ptr_opt)
{
    _tagDataHead *pDataHead;
    char ctime[20];
    int fd;
    glink_addr dst_addr;
    tms_context *popt;
    popt = (tms_context *)ptr_opt;
    dst_addr.dst = popt->pgb->src;
    dst_addr.src = ADDR_MCU;
    dst_addr.pkid = popt->pgb->pkid;
    fd = tms_SelectFdByAddr(&dst_addr.dst);
    pDataHead  = (_tagDataHead *)buf;
    if(pDataHead->cmd == ID_GET_MCU_TIME)
    {
        bzero(ctime, sizeof(ctime));
        GetCurrentTime(ctime);
        tms_RetMCUTime(fd, &dst_addr, (uint8_t *)ctime);
    }
    else
    {
        //调用脚本
        memcpy(ctime, buf + sizeof(_tagDataHead), 19);
        ctime[19] = '\0';
        char strout[64];
        snprintf(strout, 64, "/bin/settime.sh \"%s\"", ctime);
        system(strout);
        qDebug("pc adjust time %s" , (buf + sizeof(_tagDataHead)));
        mcu_Ack(ptr_opt, pDataHead->cmd, RET_SUCCESS);
    }
    return 0;
}

/*
   **************************************************************************************
 *  函数名称：olp_switch
 *  函数描述：olp发生了切换，如果断网要通知到人
 *  入口参数：
 *  返回参数：
 *  作者       ：
 *  日期       ：
 **************************************************************************************
*/
int MainWindow::olp_switch(char buf[], void *ptr_opt)
{
    tms_context *popt;
    _tagDlgAttribute *pdev;
    QString str, str1;
    char msg[48];
    char cTime[TIME_STR_LEN];
    QByteArray pbuf;
    _tagDataHead *pDataHead;
    glink_addr dst_addr;
    _tagOlpActionRecordCell record;
    int offset, switch_type, switch_port;
    int res_code, nm_count;
    int fd, gsm_send;
    res_code = RET_SUCCESS;
    popt = (tms_context *)ptr_opt;
    //命令头
    pDataHead = (_tagDataHead *)buf;
    offset = sizeof(_tagDataHead);
    //机框，槽位信息
    pdev = (_tagDlgAttribute *)(buf + offset);
    offset += sizeof(_tagDlgAttribute) ; //缺少端口
    if(pdev->devType != OLP)
    {
        res_code = RET_PARAM_INVALID;
        goto usr_exit;
    }
    //切换类型
    memcpy(&switch_type, buf + offset, sizeof(switch_type));
    offset += sizeof(int);
    //切换端口
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
    {
        res_code = RET_PARAM_INVALID;
        //      goto usr_exit;
    }
    if(switch_port == 1 || switch_port == 2)
    {
        str1.setNum(switch_port);
    }
    else
    {
        res_code = RET_PARAM_INVALID;
        goto usr_exit;
    }
    gsm_send = 0;
    record.frame =  pdev->frameNo;
    record.card =  pdev->cardNo;
    record.type =  pdev->devType;
    record.action_type = switch_type;
    record.to_port = switch_port;
    GetCurrentTime(cTime);
    strcpy(record.time, cTime);
    input_new_olp_switch_record(record);
    if(m_ctrlStat.NMstat != NM_EXIST)
    {
        gsm_send = 1;
        bzero(msg, sizeof(msg));
        str  = str + str1;
        pbuf = str.toLocal8Bit();
        strcpy(msg, pbuf);
        input_gsm_queue(ALARM_LEVE_1, ALARM_OLP_SWITCH,NULL,msg);
    }
    nm_count = tms_ManageCount();
    if(nm_count > 0)
    {
        fd = 0;
        dst_addr.src = ADDR_MCU;
        dst_addr.pkid = creat_pkid();
        dst_addr.dst = ADDR_HOST_VIP;
        fd = tms_SelectFdByAddr(&dst_addr.dst);
        if(fd > 0) //说明前面没有发送短信
        {
            input_cpy_sock_retry_queue(buf, dst_addr.pkid);
        }
        dst_addr.dst = ADDR_MASS_SEND;
        tms_ReportOLPActionEx(0,&dst_addr,(tms_report_olp_action*)(&record));
    }
usr_exit:
    mcu_Ack(ptr_opt, pDataHead->cmd, res_code);
    return 0;
}


/*
   **************************************************************************************
 *  函数名称：input_new_olp_switch_record
 *  函数描述：收到olp动作，将其保存到缓冲区中,如果超出最大长度，就从头
 *  函数描述：开始覆盖
 *  入口参数：
 *  返回参数：
 *  作者       ：
 *  日期       ：
 **************************************************************************************
*/
int MainWindow::input_new_olp_switch_record(_tagOlpActionRecordCell record)
{
    int index;
    int total_alarm;
    //将记录赋值到index指示的位置
    OlpActionRecordBuf.obj.lock();
    index = OlpActionRecordBuf.cur_index;
    index = index % OLP_ACTION_MAX_NUM; //为了安全起见
    OlpActionRecordBuf.list.replace(index,record);
    index++;
    index = index % OLP_ACTION_MAX_NUM;
    OlpActionRecordBuf.cur_index = index;
    //语句太长，因此用一个变量代替
    total_alarm = OlpActionRecordBuf.cur_total_record;
    total_alarm ++;
    total_alarm = total_alarm <  OLP_ACTION_MAX_NUM ? total_alarm : OLP_ACTION_MAX_NUM;
    OlpActionRecordBuf.cur_total_record = total_alarm;


    OlpActionRecordBuf.obj.unlock();

    return index;

}
/*
   **************************************************************************************
 *  函数名称：initial_olp_switch_record_buf
 *  函数描述：从数据库中读取记录，初始化当前的切换记录缓存
 *  入口参数：
 *  返回参数：
 *  作者       ：
 *  日期       ：
 **************************************************************************************
*/
int MainWindow::initial_olp_switch_record_buf()
{
    OlpActionRecordBuf.obj.lock();
    OlpActionRecordBuf.buf_size = OLP_ACTION_MAX_NUM;
    OlpActionRecordBuf.cur_index = 0;
    OlpActionRecordBuf.cur_total_record = 0;
    //一开始分配记录的最大空间
    OlpActionRecordBuf.list.resize(OLP_ACTION_MAX_NUM);
    //读取OLP记录
    db_read_comm_record(DB_COMM_ID_OLP_ACTION);
    OlpActionRecordBuf.obj.unlock();
    return 0;

}
/*
   **************************************************************************************
 *  函数名称：send_olp_action_record_to_host
 *  函数描述：host查询olp切换记录,留下了重发的接口，根据协议是查询
 *  入口参数：tms_context,opt;如果需要重发，opt可以填pkid
 *  返回参数：
 *  作者       ：
 *  日期       ：
 *  修改日期：
 *  修改内容：
 **************************************************************************************
*/
int MainWindow::send_olp_action_record_to_host(int send_type,  int pkid, void *popt)
{
    glink_addr dst_addr;
    tms_context *pcontext;
    int ret, i, bytes, fd;
    int count, offset;
    bool  isLock;
    char *buf;

    isLock = false;
    ret = -1;
    dst_addr.src =  ADDR_MCU;
    if(send_type == SEND_TYPE_ASK) //查询
    {
        if(popt == NULL)
        {
            printf(" host ask olp switch record error popt = NULL \n");
            goto usr_exit;
        }
        pcontext = (tms_context *)popt;
        dst_addr.dst = pcontext->pgb->src;
        dst_addr.pkid = pcontext->pgb->pkid;
        fd = pcontext->fd;
    }
    else if(send_type == SEND_TYPE_RETRY)
    {
        if(pkid < 0)
        {
            printf("%s(): Line : %d retry olp log pkid error 0x%x \n",  __FUNCTION__, __LINE__,pkid);
            goto usr_exit;
        }
        dst_addr.dst = ADDR_HOST_VIP;
        dst_addr.pkid = pkid;
        fd = tms_SelectFdByAddr(&dst_addr.dst);
        printf("%s(): Line : %d retry olp log pkid 0x%x \n",  __FUNCTION__, __LINE__,pkid);
    }
    else if(send_type == SEND_TYPE_INITIATIVE)
    {
        /*
         *2016-01-28 增加主动发送代码
        */
        dst_addr.dst = ADDR_HOST_VIP;
        dst_addr.pkid = creat_pkid();
        fd = tms_SelectFdByAddr(&dst_addr.dst);
        printf("%s(): Line : %d active send olp log pkid 0x%x \n",  __FUNCTION__, __LINE__,dst_addr.pkid);
    }
    else
    {
        printf("%s(): Line : %d retry olp log send type error 0x%x \n",\
               __FUNCTION__, __LINE__,send_type);
        goto usr_exit;
    }

    OlpActionRecordBuf.obj.lock();
    isLock = true;
    count = OlpActionRecordBuf.cur_total_record ;
    if(count >= 0 && count < OLP_ACTION_MAX_NUM)
    {
        bytes = 16 + count * sizeof(_tagOlpActionRecordCell);
        buf = new char [bytes];
        if(buf == NULL)
            goto usr_exit;
        bzero(buf, sizeof(buf));
        offset = 0;
        for(i = 0; i < count;i++)
        {
            memcpy(buf + offset, &OlpActionRecordBuf.list[i], sizeof(_tagOlpActionRecordCell));
            offset += sizeof(_tagOlpActionRecordCell);
        }
        //调用发送函数吧，骚年
        tms_RetOLPActionLog(fd,&dst_addr, count,(tms_olp_action_log_val *)(buf));
        //释放资源
        delete []buf;
    }
usr_exit:
    if(isLock)
        OlpActionRecordBuf.obj.unlock(); //解锁
    /*
     *2016-01-28 如果主动发送，将相关信息增加到重发队列中
    */
    if(fd > 0 && send_type == SEND_TYPE_INITIATIVE)
    {
        _tagDataHead DataHead;

        DataHead.cmd = ID_RET_OLP_ACTION_LOG;
        DataHead.dataLen = sizeof(_tagDataHead);
        input_cpy_sock_retry_queue((const char *)(&DataHead),dst_addr.pkid);
    }
    return ret;
}

/*
   **************************************************************************************
 *  函数名称：initial_hw_alarm_buf
 *  函数描述：给硬件告警分配记录，采用reserve的方式分配，用append添加
 *  函数描述：从数据库中读取记录，初始化当前的切换记录缓存
 *  入口参数：
 *  返回参数：2015-10-12
 *  作者       ：
 *  日期       ：
 **************************************************************************************
*/
int MainWindow::initial_hw_alarm_buf()
{
    int retv;
    objHwAlarm.lock();
    //读取OLP记录
    bzero(DevCommuState, sizeof(DevCommuState));
    retv = db_read_comm_record(DB_COMM_ID_HW_ALARM);
    objHwAlarm.unlock();

    return 0;

}

/*
   **************************************************************************************
 *  函数名称：rcv_opm_power
 *  函数描述：主控收到光功率值，在对应的对话框上显示
 *  入口参数：
 *  返回参数：
 *  作者       ：
 *  日期       ：
 **************************************************************************************
*/
int MainWindow:: rcv_opm_power(char buf[], void *ptr_opt)
{
    tms_context *popt;
    dlgOpm *pOpmDlg;
    _tagDevComm devOP;
    //_tagDataHead *pDataHead;
    memcpy(&devOP, buf + sizeof(_tagDataHead),  sizeof(_tagDevComm));
    popt = (tms_context *)ptr_opt;
    pOpmDlg = NULL;
    qDebug("rcv opm_power");
    if(m_currenShowDlg > 0&&m_ptrCurentDlg != NULL&& \
            curShowDlgAttr.frameNo == devOP.frame_no &&curShowDlgAttr.cardNo == devOP.card_no)
    {
        if(curShowDlgAttr.devType == OPM)
        {
            pOpmDlg = (dlgOpm *)m_ptrCurentDlg;
            pOpmDlg->initial_cur_pw(buf);
        }
    }
    //mcu_Ack(ptr_opt, pDataHead->cmd, RET_SUCCESS);
    return 0;
}

/*
   **************************************************************************************
 *  函数名称：db_save_module_rout db_del_module_rout db_check_module_rout
 *  函数描述：保存，删除，检查模块级联表
 *  入口参数：
 *  返回参数：
 *  作者       ：
 *  日期       ：
 **************************************************************************************
*/
int MainWindow::db_save_module_rout(char buf[], void *ptr_opt)
{
    tdb_route_t cfg_record, mask;
    tms_route *ptr_rout;
    glink_addr dst_addr;
    _tagDataHead *pDataHead;
    tms_context *popt;

    int res_code, count, i;
    int offset, res_db;
    pDataHead = (_tagDataHead *)(buf);
    popt = (tms_context *)ptr_opt;
    dst_addr.src = ADDR_MCU;
    dst_addr.dst = popt->pgb->src;
    dst_addr.pkid = popt->pgb->pkid;
    res_code = db_check_module_rout(buf); //检查参数是否正确

    if(res_code != RET_SUCCESS)
        goto usr_exit;


    bzero(&cfg_record, sizeof(tdb_route_t));
    bzero(&mask, sizeof(tdb_route_t));
    mask.frame_a = 1;
    mask.frame_b = 1;
    mask.slot_a = 1;
    mask.slot_b = 1;
    mask.type_a = 1;
    mask.type_b = 1;
    mask.port_a = 1;
    mask.port_b = 1;
    offset = sizeof(_tagDataHead);
    memcpy(&count, buf + offset, sizeof(int));
    offset += sizeof(int);
    for(i = 0; i < count; i++)
    {
        ptr_rout = (tms_route *)(buf + offset);
        offset += sizeof(tms_route);
        memcpy(&cfg_record.ip_src, ptr_rout, sizeof(tms_route));
        //tmsdb_Delete_route(&cfg_record, &mask,1);

        res_db = tmsdb_Insert_route(&cfg_record, &mask,1);
        if(res_db < 0)
        {
            res_code = RET_IGNORE_SAVE;
            break;
        }
    }
usr_exit:
    dst_addr.pkid = popt->pgb->pkid;
    mcu_Ack(ptr_opt,pDataHead->cmd, res_code);
    return res_code;
}
//删除模块级联表
int MainWindow::db_del_module_rout(char buf[], void *ptr_opt)
{
    tdb_route_t cfg_record, mask;
    glink_addr dst_addr;
    _tagDataHead *pDataHead;
    tms_context *popt;
    int res_code;
    int offset, res_db;
    res_code = RET_SUCCESS;

    popt = (tms_context *)ptr_opt;
    dst_addr.src = ADDR_MCU;
    dst_addr.dst = popt->pgb->src;

    pDataHead = (_tagDataHead *)(buf);
    offset = sizeof(_tagDataHead);

    bzero(&cfg_record, sizeof(tdb_route_t));
    bzero(&mask, sizeof(tdb_route_t));
    res_db = tmsdb_Delete_route(&cfg_record, &mask,1);
    if(res_db < 0)
        res_code = RET_UNEXIST_ROW;
    dst_addr.pkid = popt->pgb->pkid;
    mcu_Ack(ptr_opt,pDataHead->cmd, res_code);
    return res_code;
}
//检查模块级联表是否正确
int MainWindow::db_check_module_rout(char buf[])
{
    tms_route *ptr_rout;
    int res_code, offset;
    int i, count;
    offset = sizeof(_tagDataHead);
    memcpy(&count, buf + offset, sizeof(int));
    offset += sizeof(int);
    res_code = RET_SUCCESS;
    if(count < 1 || count > 0x0fffffff)
    {
        res_code = RET_PARAM_INVALID;
        goto usr_exit;
    }
    /*
     *2016-01-19 修改为判断条件，允许多条记录一起发送
    */
    for(i = 0; i < count; i++)
    {
        ptr_rout = (tms_route *)(buf + offset);
        offset += sizeof(tms_route);
        /*
         *2016-01-19 一条完整路由以全0结束
        */
        if(ptr_rout->frame_b == 0&&ptr_rout->slot_b== 0 &&ptr_rout->type_b== 0)
            continue;
        res_code = check_dev_type(ptr_rout->frame_b, ptr_rout->slot_b, ptr_rout->type_b);
        if(res_code != RET_SUCCESS)
            goto usr_exit;

        res_code = check_dev_port_no(ptr_rout->frame_b, ptr_rout->slot_b, ptr_rout->port_b);
        if(res_code != RET_SUCCESS)
            goto usr_exit;
    }
usr_exit:
    return res_code;
}

/*
   **************************************************************************************
 *  函数名称：db_save_A_trigger_B db_del_A_trigger_B db_check_A_trigger_B
 *  函数描述：保存/删除/检查告警触发表
 *  入口参数：
 *  返回参数：
 *  作者       ：
 *  日期       ：
 **************************************************************************************
*/
//收到告警触发表，首先对其检查，无误之后保存
int MainWindow::db_save_A_trigger_B(char buf[], void *ptr_opt)
{
    tdb_a_trigger_b_t  cfg_record, mask;
    tms_unit  *ptr_record;
    glink_addr dst_addr;
    _tagDataHead *pDataHead;
    tms_context *popt;
    int res_code, count, i;
    int offset, res_db;
    popt = (tms_context *)ptr_opt;
    dst_addr.src =  ADDR_MCU;
    dst_addr.dst = popt->pgb->src;
    res_code = db_check_A_trigger_B(buf);
    pDataHead = (_tagDataHead *)(buf);
    if(res_code != RET_SUCCESS)
        goto usr_exit;

    offset = sizeof(_tagDataHead);
    bzero(&mask,sizeof(tdb_a_trigger_b_t));
    bzero(&cfg_record,sizeof(tdb_a_trigger_b_t));
    //严格按照机框，槽位，类型，端口数目添加，删除
    mask.frame_a = 1;
    mask.frame_b = 1;
    mask.port_a = 1;
    mask.port_b = 1;
    mask.type_a = 1;
    mask.type_b = 1;
    mask.slot_a = 1;
    mask.slot_b = 1;
    memcpy(&count, buf + offset, sizeof(int));
    offset += sizeof(int);
    for(i = 0; i < count;i++)
    {
        ptr_record = (tms_unit  *)(buf + offset);
        offset += sizeof(tms_unit);
        memcpy(&cfg_record.frame_a, ptr_record, sizeof(tms_unit));
        tmsdb_Delete_a_trigger_b(&cfg_record, &mask);
        res_db = tmsdb_Insert_a_trigger_b(&cfg_record, &mask, 1);
        if(res_db < 0)
        {
            res_code = RET_IGNORE_SAVE;
            break;
        }
    }
usr_exit:
    dst_addr.pkid = popt->pgb->pkid;
    mcu_Ack(ptr_opt, pDataHead->cmd, res_code);
    return 0;
}
//删除告警触发表
int MainWindow::db_del_A_trigger_B(char buf[], void *ptr_opt)
{
    tdb_a_trigger_b_t  cfg_record, mask;
    tms_unit  *ptr_record;
    glink_addr dst_addr;
    _tagDataHead *pDataHead;
    tms_context *popt;
    int res_code, count, i;
    int offset, res_db;
    res_db = -1;
    pDataHead = (_tagDataHead *)(buf);
    offset = sizeof(_tagDataHead);
    popt = (tms_context *)ptr_opt;
    dst_addr.src =  ADDR_MCU;
    dst_addr.dst = popt->pgb->src;
    res_code = RET_SUCCESS;
    if(pDataHead->cmd == ID_DEL_TBUNIT)
        res_code = db_check_A_trigger_B(buf);
    if(res_code != RET_SUCCESS)
        goto usr_exit;

    bzero(&mask,sizeof(tdb_a_trigger_b_t));
    bzero(&cfg_record,sizeof(tdb_a_trigger_b_t));
    if(pDataHead->cmd == ID_DELALL_TBUNIT)
    {
        res_db = tmsdb_Delete_a_trigger_b(&cfg_record, &mask);
        if(res_db < 0)
        {
            res_code = RET_UNEXIST_ROW;
        }
    }
    else if(pDataHead->cmd == ID_DEL_TBUNIT)
    {
        //严格按照机框，槽位，类型，端口数目添加，删除
        mask.frame_a = 1;
        mask.frame_b = 1;
        mask.port_a = 1;
        mask.port_b = 1;
        mask.type_a = 1;
        mask.type_b = 1;
        mask.slot_a = 1;
        mask.slot_b = 1;
        memcpy(&count, buf + offset, sizeof(int));
        offset += sizeof(int);
        for(i = 0; i < count;i++)
        {
            ptr_record = (tms_unit  *)(buf + offset);
            offset += sizeof(tms_unit);
            memcpy(&cfg_record.frame_a, ptr_record, sizeof(tms_unit));
            res_db = tmsdb_Delete_a_trigger_b(&cfg_record, &mask);
            if(res_db < 0)
            {
                res_code = RET_UNEXIST_ROW;
            }
        }
    }
usr_exit:
    dst_addr.pkid = popt->pgb->pkid;
    mcu_Ack(ptr_opt, pDataHead->cmd, res_code);
    return 0;
}
//检查告警触发表是否正确
int MainWindow::db_check_A_trigger_B(char buf[])
{
    tms_unit  *ptr_record;
    int res_code, count, i;
    int offset;
    offset = sizeof(_tagDataHead);
    res_code = RET_SUCCESS;
    memcpy(&count, buf + offset, sizeof(int));
    offset += sizeof(int);
    if(count < 0 || count > 0x0fffffff)
    {
        res_code = RET_PARAM_INVALID;
        goto usr_exit;
    }
    for(i = 0; i < count;i++)
    {
        ptr_record = (tms_unit  *)(buf + offset);
        offset += sizeof(tms_unit);
        res_code = check_dev_type(ptr_record->frame_a, ptr_record->slot_a, ptr_record->type_a);
        if(res_code != RET_SUCCESS)
            goto usr_exit;

        res_code = check_dev_port_no(ptr_record->frame_a, ptr_record->slot_a, ptr_record->port_a);
        if(res_code != RET_SUCCESS)
            goto usr_exit;

        res_code = check_dev_type(ptr_record->frame_b, ptr_record->slot_b, ptr_record->type_b);
        if(res_code != RET_SUCCESS)
            goto usr_exit;

        res_code = check_dev_port_no(ptr_record->frame_b, ptr_record->slot_b, ptr_record->port_b);
        if(res_code != RET_SUCCESS)
            goto usr_exit;
    }
usr_exit:
    return res_code;
}

/*
   **************************************************************************************
 *  函数名称：dealRcvGsmRes
 *  函数描述：收到短信回应码
 *  入口参数：
 *  返回参数：
 *  作者       ：
 *  日期       ：
 **************************************************************************************
*/
int MainWindow::dealRcvGsmRes(char buf[])
{
    char phone[16];
    _tagDataHead *pDataHead;
    int res_code;
    int offset;
    pDataHead = (_tagDataHead *)(buf);
    offset = (sizeof(_tagDataHead));
    memcpy(phone, buf + offset, 16);
    offset += 16;
    memcpy(&res_code, buf + offset, sizeof(int));
    qDebug("%s", phone);
    qDebug("gsm send status %d", res_code);
    if(pHostCommu != NULL && pHostCommu->semSynch.resCmd == pDataHead->cmd)
    {
        pHostCommu->semSynch.resCode = res_code;
        pHostCommu->semSynch.objSem.release();
    }
    return res_code;

}

/*
   **************************************************************************************
 *  函数名称：dealNMAppointTes
 *  函数描述：mcu收到网管的点名测量命令
 *  入口参数：
 *  返回参数：
 *  作者       ：
 *  日期       ：
 **************************************************************************************
*/
int MainWindow::dealNMAppointTest(char buf[], void *ptr_opt)
{
    _tagDevComm *posw_port;
    _tagDataHead *pDataHead;
    _tagCtrlAppointTest curAppointTest;
    tms_context *ptr_context;
    int offset, res_code,waitTime_ms;
    ptr_context = (tms_context *)ptr_opt;
    offset = 0;
    res_code = 0;


    pDataHead = (_tagDataHead *)(buf);
    offset = sizeof(_tagDataHead);
    posw_port = (_tagDevComm *)(buf + offset);
    offset += (sizeof(_tagDevComm) + 4); //还有一个保留项


    res_code = check_dev_type(posw_port->frame_no, posw_port->card_no, posw_port->type);
    if(res_code != RET_SUCCESS) //设备类型不是光开关，或者对应槽位没有配置
        goto usr_exit;
    res_code = check_dev_port_no(posw_port->frame_no, posw_port->card_no, posw_port->port);
    if(res_code != RET_SUCCESS)//端口号错误
        goto usr_exit;

    memcpy(&curAppointTest.test_port, posw_port, sizeof(_tagDevComm));
    curAppointTest.ack_to = ACK_TO_NM;
    curAppointTest.cmd = pDataHead->cmd;
    //点名测量的目的地址源地址 将目的地址修改成板卡，源地址保持不变化
    curAppointTest.opt.dst = ADDR_CARD;
    curAppointTest.opt.src= ptr_context->pgb->src;
    curAppointTest.opt.pkid = ptr_context->pgb->pkid;
    //qDebug("recv pc appoint src %x dst %x", curAppointTest.opt.src, curAppointTest.opt.dst);
    memcpy(&curAppointTest.para, buf + offset, sizeof(_tagMeasurPara));
    //    qDebug("nm appoint  para range %d lamda %d end %f non %f n %f",curAppointTest.para.range_m, curAppointTest.para.lamda_ns,\
    //           curAppointTest.para.endThreshold,curAppointTest.para.NonRelectThreshold,curAppointTest.para.n);
    //    qDebug("nm appoint  para pw %d time %d reser[1] %d",curAppointTest.para.pulseWidth_ns, curAppointTest.para.measureTime_s,\
    //           curAppointTest.para.rserv[1]);
    waitTime_ms = 0;
    res_code = dispatch_test_port(&curAppointTest, OTDR_MOD_APPOINT);
    if(res_code < 0)
        res_code = abs(res_code);
    else
    {
        waitTime_ms = res_code;
        res_code = RET_SUCCESS;
    }


usr_exit:
    if(res_code != RET_SUCCESS)
    {
        mcu_Ack(ptr_opt, pDataHead->cmd, res_code,waitTime_ms);
        qDebug("nm appoint test error %d", res_code);
    }

    return res_code;
}
/*
   **************************************************************************************
 *  函数名称：db_get_osw_connect_dev
 *  函数描述：获取光开关端口关联的设备类型，是opm,还是olp
 *  入口参数：
 *  返回参数：
 *  作者       ：
 *  日期       ：2015-08-20由于功能变更 该函数废弃
 **************************************************************************************
*/
int MainWindow::db_get_osw_connect_dev(_tagDevComm *posw, _tagDevComm *pconnect_dev)
{
    return 0;
    int res_code, row, i;
    char msg[SHORT_MSG_LEN];
    glink_addr nm_addr;
    tdb_any_unit_osw_t *ppout = NULL;
    tdb_any_unit_osw_t cfg_osw, mask;

    nm_addr.src = ADDR_MCU;
    nm_addr.dst = ADDR_NET_MANAGER;
    memset(msg, 0, sizeof(msg));
    res_code = -1;

    memset(&cfg_osw, 0, sizeof(tdb_any_unit_osw_t));
    memset(&mask, 0, sizeof(tdb_any_unit_osw_t));
    memcpy(&cfg_osw.osw_frame, posw, sizeof(_tagDevComm));
    mask.osw_frame = 1;
    mask.osw_slot = 1;
    mask.osw_port = 1;

    row = tmsdb_Select_any_unit_osw(&cfg_osw, &mask, &ppout);
    if(row > 0)
    {
        for(i = 0; i < row; i++)
        {
            memcpy(pconnect_dev, &ppout[i].any_frame, sizeof(_tagDevComm)); //只拷贝opm或者olp部分
        }
        free(ppout);
        //检查读到的数据是否合法
        res_code = check_dev_type(pconnect_dev->frame_no, pconnect_dev->card_no, pconnect_dev->type);
        if(res_code == RET_SUCCESS)
            res_code = check_dev_port_no(pconnect_dev->frame_no, pconnect_dev->card_no, pconnect_dev->port);
        if(res_code != RET_SUCCESS)
        {
            sprintf(msg, "osw connected dev error: frame %d card %d port %d type %d", \
                    pconnect_dev->frame_no, pconnect_dev->card_no, pconnect_dev->port, pconnect_dev->type) ;

        }
        if(row > 1)
            qDebug(" osw port connect more port num %d", row);
    }
    else
    {
        sprintf(msg, " read osw connected dev db  error row %d ",row );
    }

    if(res_code != RET_SUCCESS)
    {
        //tms_Trace(&nm_addr,msg, strlen(msg), 1);
        qDebug("%s", msg);
    }
    return RET_SUCCESS;
}

/*
   **************************************************************************************
 *  函数名称：save_osw_port_cable_infor
 *  函数描述：mcu保存osw的关联光缆信息
 *  入口参数：
 *  返回参数：0,成功，其他为错误码
 *  作者       ：
 *  日期       ：
 **************************************************************************************
*/
int MainWindow::save_osw_port_cable_infor(char buf[], void * ptr_opt)
{
    tms_cfg_mcu_osw_port *posw_port_cfg;
    tdb_dev_map_t mask;
    tdb_dev_map_t db_record;

    _tagDataHead *pDataHead;
    glink_addr dst_addr;
    tms_context *popt;
    int res_code, dev_type;
    int  offset, res_oprate_db;
    res_oprate_db = 0;
    popt = (tms_context *)ptr_opt;
    dst_addr.src =  ADDR_MCU;
    dst_addr.dst = popt->pgb->src;
    pDataHead = (_tagDataHead *)(buf);
    if(pDataHead->cmd == ID_CFG_MCU_OSW_PORT)
        dev_type = OSW;
    else
        dev_type = OLP;
    offset = sizeof(_tagDataHead);
    posw_port_cfg = (tms_cfg_mcu_osw_port *)(buf + offset);
    //检查设备类型/端口
    res_code = check_dev_type(posw_port_cfg->frame, posw_port_cfg->slot,posw_port_cfg->type);
    if(res_code == RET_SUCCESS)
        res_code = check_dev_port_no(posw_port_cfg->frame, posw_port_cfg->slot,posw_port_cfg->port);
    if(res_code == RET_SUCCESS)
    {

        wchar_t wmsg_1[64];
        char msg[256];
        unsigned short *ptr_uint16;
        if(NULL == setlocale (LC_ALL, "zh_CN.UTF-8"))
        {
            qDebug("setlocale zh_CN.UTF-8 fail");
        }
        ptr_uint16 = (unsigned short *) posw_port_cfg->start_name;
        bzero(wmsg_1, sizeof(wmsg_1));
        utf162wcs(ptr_uint16, wmsg_1, 0);
        wcstombs(msg, wmsg_1,sizeof(msg));
        qDebug("start name %s",msg);
        PrintfMemory(posw_port_cfg->start_name, 64);

        ptr_uint16 = (unsigned short *) posw_port_cfg->end_name;
        bzero(wmsg_1, sizeof(wmsg_1));
        utf162wcs(ptr_uint16, wmsg_1, 0);
        wcstombs(msg, wmsg_1,sizeof(msg));
        qDebug("end name %s",msg);
        PrintfMemory(posw_port_cfg->end_name, 64);

        ptr_uint16 = (unsigned short *) posw_port_cfg->dev_name;
        bzero(wmsg_1, sizeof(wmsg_1));
        utf162wcs(ptr_uint16, wmsg_1, 0);
        wcstombs(msg, wmsg_1,sizeof(msg));
        qDebug("dev name %s",msg);
        PrintfMemory(posw_port_cfg->dev_name, 64);

        ptr_uint16 = (unsigned short *) posw_port_cfg->cable_name;
        bzero(wmsg_1, sizeof(wmsg_1));
        utf162wcs(ptr_uint16, wmsg_1, 0);
        wcstombs(msg, wmsg_1,sizeof(msg));
        qDebug("cable name %s",msg);
        PrintfMemory(posw_port_cfg->cable_name, 64);


        memset(&mask, 0, sizeof(tms_cfg_mcu_osw_port));
        mask.frame = 1;
        mask.slot = 1;
        mask.port = 1;
        mask.type = 1;
        //数据库操作
        memcpy(&db_record.frame,posw_port_cfg,sizeof(tms_cfg_mcu_osw_port));
        db_record.id = 0;
        res_oprate_db =  tmsdb_Delete_dev_map(&db_record, &mask);
        if(res_oprate_db > -1)
            res_oprate_db =  tmsdb_Insert_dev_map(&db_record, &mask, 1);
    }
    //参数不能保存
    if(res_oprate_db < -1)
        res_code = RET_IGNORE_SAVE;
    dst_addr.pkid = popt->pgb->pkid;
    mcu_Ack(ptr_opt, pDataHead->cmd, res_code);
    qDebug("config osw fiber cable result %d", res_code);
    return res_code;
}
/*
   **************************************************************************************
 *  函数名称：save_port_refer_osw
 *  函数描述：mcu保存olp/opm osw关联信息.为了统一性，必须整个设备一起设置
 *  入口参数：
 *  返回参数：
 *  作者       ：
 *  日期       ：
 **************************************************************************************
*/
int MainWindow::save_port_refer_osw(char buf[],void *ptr_opt)
{
    tdb_any_unit_osw db_mask;
    tdb_any_unit_osw db_record;
    _tagDevComm *pFrameCommn;
    _tagDataHead *pDataHead;
    _tagAnyOsw *pConnectdOsw;
    glink_addr dst_addr;
    tms_context *popt;

    int res_code, dev_type;
    int i, offset, res_oprate_db,count;
    popt = (tms_context *)ptr_opt;
    dst_addr.src =  ADDR_MCU;
    dst_addr.dst = popt->pgb->src;
    //初始化msk
    res_oprate_db = 0;
    pDataHead = (_tagDataHead *)buf;
    res_code = RET_SUCCESS;

    //判断设备类型
    if(pDataHead->cmd == ID_CFG_MCU_U_OPM_OSW)
        dev_type = OPM;
    else
        dev_type = OLP;

    //检查参数是否正确
    res_code = check_any_port_osw( buf);
    //如果检查正确，开始写入数据库
    if(res_code == RET_SUCCESS)
    {
        offset = sizeof(_tagDataHead);
        pFrameCommn = (_tagDevComm *)(buf + offset);
        offset += sizeof(_tagDevComm);
        i = 0;
        //先借用一下port，该位刚好表示的是关系数目
        count = pFrameCommn->port;

        //初始化mask
        memset(&db_mask, 0, sizeof(db_mask));
        memset(&db_record, 0, sizeof(db_mask));
        //收到本条命令之后，清空所有该设备的关联信息
        db_record.any_frame = pFrameCommn->frame_no;
        db_record.any_slot =  pFrameCommn->card_no;
        db_mask.any_frame = 1;
        db_mask.any_slot = 1;

        //清空数据库
        res_oprate_db = tmsdb_Delete_any_unit_osw(&db_record,&db_mask);
        if(res_oprate_db > -1)
        {

            db_mask.osw_frame = 1;
            db_mask.osw_port = 1;
            db_mask.osw_slot = 1;
            db_mask.osw_type = 1;

            // 拷贝的内容包含链 frame  card dev_type
            memcpy(&db_record.any_frame, pFrameCommn, sizeof(_tagDevComm) - 4);
            for(i = 0; i < count; i++)
            {
                pConnectdOsw = (_tagAnyOsw *) (buf + offset);
                offset += sizeof(_tagAnyOsw);
                memcpy(&db_record.any_port, pConnectdOsw, sizeof(_tagAnyOsw));
                //res_oprate_db = tmsdb_Delete_any_unit_osw(&db_record,&db_mask);
                res_oprate_db = tmsdb_Insert_any_unit_osw(&db_record,&db_mask,1);
                if(res_oprate_db < 0) //写数据库失败
                {
                    res_code = RET_IGNORE_SAVE; //无法保存
                    break;
                }
            }
        }
        else
        {
            res_code = RET_IGNORE_SAVE; //无法保存
        }
    }
    dst_addr.pkid = popt->pgb->pkid;
    mcu_Ack(ptr_opt, pDataHead->cmd, res_code);
    qDebug("config opm/olp---osw result %d", res_code);
    return res_code;
}
/*
   **************************************************************************************
 *  函数名称：clear_port_refer_osw
 *  函数描述：清除opm/olp --- osw端口对应关系
 *  入口参数：
 *  返回参数：
 *  作者       ：
 *  日期       ：
 **************************************************************************************
*/
int  MainWindow:: clear_port_refer_osw(char buf[], void *ptr_opt)
{
    tdb_any_unit_osw db_record; // db input
    tdb_any_unit_osw db_mask; // db input
    _tagDevComm *pFrameComm; //帧头 公共部分 frame ,card devtype port
    _tagDataHead *pDataHead;
    tms_context *popt;
    glink_addr dst_addr;
    int res_code, dev_type;
    int i, offset, res_oprate_db,count;
    res_oprate_db = 0;
    //初始化mask
    memset(&db_mask, 0, sizeof(db_mask));
    db_mask.any_frame = 1;
    db_mask.any_port = 1;
    db_mask.any_type = 1;
    db_mask.any_slot = 1;
    //获取网管的fd
    popt = (tms_context *)ptr_opt;
    dst_addr.src =  ADDR_MCU;
    dst_addr.dst = popt->pgb->src;
    pDataHead = (_tagDataHead *)(buf);
    if(pDataHead->cmd == ID_CFG_MCU_OSW_PORT)
        dev_type = OPM;
    else
        dev_type = OLP;
    offset = sizeof(_tagDataHead);
    pFrameComm = (_tagDevComm *)(buf + offset);
    offset += sizeof(_tagDevComm);
    //检查设备类型
    res_code = check_dev_type(pFrameComm->frame_no, pFrameComm->card_no,pFrameComm->type);
    if(res_code == RET_SUCCESS)
    {
        //检查端口
        count = pFrameComm->port;//根据协议借用
        for(i = 0; i < count;i++)
        {
            memcpy(&pFrameComm->port, buf + offset, sizeof(int));
            offset += sizeof(int);
            res_code = check_dev_port_no(pFrameComm->frame_no, pFrameComm->card_no,pFrameComm->port);
            if(res_code != RET_SUCCESS)
                break;
        }
    }
    //开始删除
    if(res_code == RET_SUCCESS)
    {
        //复制机框编号，槽位号，设备类型
        memcpy(&db_record.any_frame, pFrameComm, sizeof(_tagDevComm) - 4);
        //重新定位offset
        offset = sizeof(_tagDataHead);
        offset += sizeof(_tagDevComm);
        //数据库操作
        for(i = 0; i < count;i++)
        {
            memcpy(&db_record.any_port, buf + offset, sizeof(int));
            offset += sizeof(int);
            //mask.port = osw_port_cfg.port;
            res_oprate_db =  tmsdb_Delete_any_unit_osw(&db_record, &db_mask);
            if(res_oprate_db <= -1)
            {
                res_code = RET_IGNORE_SAVE;
                break;
            }
        }
    }
    //参数不能保存
    if(res_oprate_db < -1)
        res_code = RET_IGNORE_SAVE;
    dst_addr.pkid = popt->pgb->pkid;
    mcu_Ack(ptr_opt,pDataHead->cmd , res_code);
    qDebug("clear osw fiber cable result %d", res_code);
    return res_code;
}
/*
   **************************************************************************************
 *  函数名称：clear_osw_port_cable_infor
 *  函数描述：mcu清除olp/opm osw关联信息
 *  入口参数：
 *  返回参数：
 *  作者       ：
 *  日期       ：
 **************************************************************************************
*/
int MainWindow::clear_osw_port_cable_infor(char buf[], void *ptr_opt)
{
    tdb_dev_map_t mask;
    tdb_dev_map_t osw_port_cfg;
    _tagDevComm *posw_port;
    _tagDataHead *pDataHead;
    tms_context *popt;
    glink_addr dst_addr;
    int res_code, dev_type;
    int i, offset, res_oprate_db,count;
    res_oprate_db = 0;
    //获取网管的fd
    popt = (tms_context *)ptr_opt;
    dst_addr.src =  ADDR_MCU;
    dst_addr.dst = popt->pgb->src;
    pDataHead = (_tagDataHead *)(buf);
    if(pDataHead->cmd == ID_CFG_MCU_OSW_PORT)
        dev_type = OPM;
    else
        dev_type = OLP;
    offset = sizeof(_tagDataHead);
    posw_port = (_tagDevComm *)(buf + offset);
    offset += sizeof(_tagDevComm);
    //检查设备类型
    res_code = check_dev_type(posw_port->frame_no, posw_port->card_no,posw_port->type);
    //检查端口
    count = posw_port->port;//根据协议借用
    for(i = 0; i < count;i++)
    {
        if(res_code != RET_SUCCESS)
            break;
        memcpy(&posw_port->port, buf + offset, sizeof(int));
        offset += sizeof(int);
        res_code = check_dev_port_no(posw_port->frame_no, posw_port->card_no,posw_port->port);
    }
    //开始删除
    if(res_code == RET_SUCCESS)
    {

        //复制机框编号，槽位号，设备类型
        memcpy(&osw_port_cfg.frame, posw_port, sizeof(_tagDevComm) - 4);
        memset(&mask, 0, sizeof(mask));
        mask.frame = 1;
        mask.slot = 1;
        mask.port = 1;
        mask.type = 1;
        //重新定位offset
        offset = sizeof(_tagDataHead);
        offset += sizeof(_tagDevComm);
        //数据库操作
        for(i = 0; i < count;i++)
        {

            memcpy(&posw_port->port, buf + offset, sizeof(int));
            offset += sizeof(int);
            osw_port_cfg.port = posw_port->port;
            res_oprate_db =  tmsdb_Delete_dev_map(&osw_port_cfg, &mask);
            if(res_oprate_db <= -1)
            {
                res_code = RET_IGNORE_SAVE;
                break;
            }
        }
    }
    //参数不能删除
    dst_addr.pkid = popt->pgb->pkid;
    mcu_Ack(ptr_opt,pDataHead->cmd , res_code);
    qDebug("clear osw fiber cable result %d", res_code);
    return res_code;
}

/*
   **************************************************************************************
 *  函数名称：check_dev_type
 *  函数描述：检查设备类型与配置的设备类型是否一致，如果一致，返回0，
 *  函数描述：否则返回相应的错误码
 *  入口参数：
 *  返回参数：
 *  作者       ：
 *  日期       ：
 **************************************************************************************
*/
int MainWindow::check_dev_type(int frame_no, int card_no, int dev_type)
{
    int frame_state, card_state;
    int res_code;
    //输出消息
    char msg_short[SHORT_MSG_LEN];
    glink_addr nm_addr;
    nm_addr.dst = ADDR_NET_MANAGER;
    nm_addr.src = ADDR_MCU;
    res_code = RET_PARAM_INVALID;
    checkFrameCardNo(frame_no, card_no,frame_state,card_state);
    if((frame_state + card_state) == 0)
    {
        if(m_subrackCard[frame_no].type[card_no] == dev_type)
        {
            res_code = RET_SUCCESS;
        }
        else
        {
            memset(msg_short, 0, sizeof(msg_short));
            sprintf(msg_short,"check dev_type error frame %d card %d  saved / input  %d /%d",\
                    frame_no,card_no, m_subrackCard[frame_no].type[card_no],dev_type);
        }
    }
    else
    {
        memset(msg_short, 0, sizeof(msg_short));
        sprintf(msg_short,"check dev_type error frame %d card %d  not config or over range",\
                frame_no,card_no);
    }

    if(res_code !=  RET_SUCCESS)
    {
        //tms_Trace(&nm_addr,msg_short,strlen(msg_short),1);
        qDebug("%s",msg_short);
    }
    return res_code;
}
/*
  **************************************************************************************
*  函数名称：check_dev_port_no
*  函数描述：检查设备端口号是否超过该设备类型的最大端口数目，如果一致，返回0，
*  函数描述：否则返回相应的错误码
*  入口参数：
*  返回参数：
*  作者       ：
*  日期       ：
**************************************************************************************
*/
int MainWindow::check_dev_port_no(int frame_no, int card_no, int port_no)
{
    int frame_state, card_state;
    int res_code;
    //输出消息
    char msg_short[SHORT_MSG_LEN];
    glink_addr nm_addr;
    nm_addr.dst = ADDR_NET_MANAGER;
    nm_addr.src = ADDR_MCU;
    res_code = RET_PARAM_INVALID;
    checkFrameCardNo(frame_no, card_no,frame_state,card_state);
    if((frame_state + card_state) == 0)
    {
        if((port_no > -1&&port_no < m_subrackCard[frame_no].ports[card_no])|| port_no == COMN_COM )
        {
            res_code = RET_SUCCESS;
        }
        else
        {
            memset(msg_short, 0, sizeof(msg_short));
            sprintf(msg_short,"check dev_port error frame %d card %d  port saved max num / input NO. %d /%d",\
                    frame_no,card_no, m_subrackCard[frame_no].ports[card_no],port_no);
        }
    }
    else
    {
        memset(msg_short, 0, sizeof(msg_short));
        sprintf(msg_short,"check dev_port error frame %d card %d  not config or over range",\
                frame_no,card_no);
    }
    if(res_code !=  RET_SUCCESS)
        //tms_Trace(&nm_addr,msg_short,strlen(msg_short),1);
        qDebug("%s",msg_short);
    return res_code;
}
/*
   **************************************************************************************
 *  函数名称：check_any_port_osw
 *  函数描述：检查olp/osw与光开关段关联关系，主要检查端口合法否，设备类型合法否
 *  入口参数：
 *  返回参数：
 *  作者       ：
 *  日期       ：
 **************************************************************************************
*/
int MainWindow::check_any_port_osw(char buf[])
{
    _tagDevComm *pFrameComm;       //协议的frame card type count
    _tagDataHead *pDataHead; //cmd len
    _tagAnyOsw *pConnecedOsw;    // any_port, osw:frame card type port
    int res_code, dev_type;
    int i, offset,count;
    pDataHead = (_tagDataHead *)(buf);
    offset = sizeof(_tagDataHead);
    pFrameComm = (_tagDevComm *)(buf + offset);
    offset += sizeof(_tagDevComm);
    //判断设备类型
    if(pDataHead->cmd == ID_CFG_MCU_U_OPM_OSW)
        dev_type = OPM;
    else
        dev_type = OLP;
    //检查设备类型
    res_code = check_dev_type(pFrameComm->frame_no, pFrameComm->card_no, pFrameComm->type);
    //    qDebug("check_any_osw frame comm frame_no %d card _no %d type %d port %d ", pFrameComm->frame_no, \
    //           pFrameComm->card_no, pFrameComm->type, pFrameComm->port);
    if(res_code == RET_SUCCESS)
    {
        i = 0;
        //先借用一下port，该位刚好表示的是关系数目
        count = pFrameComm->port;
        //首先检查数据是否正确
        while(i < count)
        {
            pConnecedOsw = (_tagAnyOsw *) (buf + offset);
            offset += sizeof(_tagAnyOsw);
            i++;
            //检查被关联的设备类型是不是光开关
            res_code = check_dev_type(pConnecedOsw->frame_no, pConnecedOsw->card_no, pConnecedOsw->type);
            //            qDebug("check_any_osw osw frame_no %d card _no %d type %d port %d frame port %d", pConnecedOsw->frame_no, \
            //                   pConnecedOsw->card_no, pConnecedOsw->type, pConnecedOsw->osw_port, pConnecedOsw->any_port);

            if(res_code == RET_SUCCESS)
            {
                //检查opm的端口号
                res_code = check_dev_port_no(pFrameComm->frame_no, pFrameComm->card_no, pConnecedOsw->any_port);
            }
            if(res_code == RET_SUCCESS)
            {
                res_code = check_dev_port_no(pConnecedOsw->frame_no, pConnecedOsw->card_no, pConnecedOsw->osw_port);
                if(res_code != RET_SUCCESS)
                {
                    break;
                }
            }
            else
                break;
        }
    }
    return res_code;
}
/*
   **************************************************************************************
 *  函数名称：save_sms_cfg
 *  函数描述：保存短信权限
 *  入口参数：
 *  返回参数：
 *  作者       ：
 *  日期       ：
 **************************************************************************************
*/
int MainWindow::save_sms_cfg(char buf[],void * ptr_opt)
{
    int i, offset, count;
    int res_code, db_res;
    _tagDataHead *pDataHead;
    tms_cfg_sms_val *psms_cfg;
    tdb_sms_t db_record;
    tdb_sms_t mask;
    glink_addr dst_addr;
    tms_context *popt;
    popt = (tms_context *)ptr_opt;
    dst_addr.src =  ADDR_MCU;
    dst_addr.dst = popt->pgb->src;
    pDataHead = (_tagDataHead *)buf;
    offset = sizeof(_tagDataHead);

    res_code = check_sms_cfg(buf);
    if(res_code == RET_SUCCESS)
    {
        memcpy(&count, buf + offset, sizeof(count));
        offset += sizeof(int);
        memset(&mask, 0, sizeof(tdb_sms_t));
        memset(&db_record, 0, sizeof(tdb_sms_t));
        mask.time = 1;
        for(i = 0; i < count;i++)
        {
            psms_cfg = ( tms_cfg_sms_val *)(buf + offset);
            offset += sizeof(tms_cfg_sms_val);
            memcpy(&db_record.time, psms_cfg, sizeof(tms_cfg_sms_val));
            //db_res = tmsdb_Delete_sms(&db_record,&mask);
            if(/*db_res < -1*/0)
            {
                res_code = RET_IGNORE_SAVE;
                break;
            }
            db_res = tmsdb_Insert_sms(&db_record,&mask,1);
            if(db_res < -1)
            {
                res_code = RET_IGNORE_SAVE;
                break;
            }

        }
    }
    dst_addr.pkid = popt->pgb->pkid;
    mcu_Ack(ptr_opt,pDataHead->cmd, res_code);
    if(res_code != RET_SUCCESS)
        qDebug("db save sms error %d", res_code);

usr_exit:
    return res_code;
}
/*
**************************************************************************************
*  函数名称：check_sms_cfg
*  函数描述：检查该配置是否合法
*  入口参数：
*  返回参数：
*  作者       ：
*  日期       ：
**************************************************************************************
*/
int MainWindow::check_sms_cfg(char buf[])
{
    int i, offset, count;
    int res_code;
    tms_cfg_sms_val *psms_cfg;
    char msg_short[SHORT_MSG_LEN];
    offset = sizeof(_tagDataHead);
    memcpy(&count, buf + offset, sizeof(count));
    offset += sizeof(int);
    res_code = RET_SUCCESS;
    if(count < 0 || count > 200000)
    {
        memset(msg_short, 0, sizeof(msg_short));
        sprintf(msg_short, "sms_cfg count %d", count);
        res_code = RET_PARAM_INVALID;
        goto usr_exit;
    }

    for(i = 0; i < count;i++)
    {
        psms_cfg = ( tms_cfg_sms_val *)(buf + offset);
        offset += sizeof(tms_cfg_sms_val);
        if(psms_cfg->time < 1 || psms_cfg->time > 7)
        {
            memset(msg_short, 0, sizeof(msg_short));
            sprintf(msg_short, "sms_cfg  error time %d", psms_cfg->time);
            res_code =RET_PARAM_INVALID;
            break;
        }
        else if(psms_cfg->type < MSG_ALARM_TYPE_MIN || psms_cfg->type > MSG_ALARM_TYPE_MAX)
        {
            memset(msg_short, 0, sizeof(msg_short));
            sprintf(msg_short, "sms_cfg  error authorization type %d", psms_cfg->type);
            res_code =RET_PARAM_INVALID;
            break;
        }
        else if(psms_cfg->type == MSG_ALARM_OLP_SWITCH&&\
                (psms_cfg->level < MSG_ALARM_LEVE_MIN || psms_cfg->level > MSG_ALARM_LEVE_MAX))
        {
            memset(msg_short, 0, sizeof(msg_short));
            sprintf(msg_short, "sms_cfg  error authorization level type %d level %d", psms_cfg->type, psms_cfg->level);
            res_code =RET_PARAM_INVALID;
            break;
        }
        else if(psms_cfg->type != MSG_ALARM_OLP_SWITCH&&\
                (psms_cfg->level < MSG_ALARM_LEVE_MIN || (psms_cfg->level> (MSG_ALARM_LEVE_MAX - 1))))
        {
            memset(msg_short, 0, sizeof(msg_short));
            sprintf(msg_short, "sms_cfg  error authorization level type %d level %d", psms_cfg->type, psms_cfg->level);
            res_code =RET_PARAM_INVALID;
            break;
        }
    }
    if(res_code != RET_SUCCESS)
        qDebug("%s",msg_short);
usr_exit: return res_code;
}
/*
   **************************************************************************************
 *  函数名称：clear_sms_cfg
 *  函数描述：清除通过短信发送告警的参数配置
 *  入口参数：
 *  返回参数：
 *  作者       ：
 *  日期       ：
 **************************************************************************************
*/
int MainWindow::clear_sms_cfg(char buf[], void *ptr_opt)
{
    int  res_code, db_res;
    glink_addr dst_addr;
    _tagDataHead *pDataHead;
    tdb_sms_t db_record;
    tdb_sms_t mask;
    tms_context *popt;
    popt = (tms_context *)ptr_opt;
    dst_addr.src =  ADDR_MCU;
    dst_addr.dst = popt->pgb->src;
    res_code = RET_SUCCESS;
    pDataHead = (_tagDataHead*)buf;
    bzero(&db_record, sizeof(tdb_sms_t));
    bzero(&mask, sizeof(tdb_sms_t));
    db_res = tmsdb_Delete_sms(&db_record,&mask);
    if(db_res != RET_SUCCESS)
        res_code = RET_UNEXIST_ROW;
    dst_addr.pkid = popt->pgb->pkid;
    mcu_Ack(ptr_opt,pDataHead->cmd, res_code);
    qDebug("usr clear sms res_db %d", db_res);
    return res_code;
}
/*
   **************************************************************************************
 *  函数名称：db_check_otdr_ref
 *  函数描述：检查从数据库中读取的参考曲线,事件点，数据点，其他一些标志位
 *  入口参数：
 *  返回参数：
 *  作者       ：
 *  日期       ：
 **************************************************************************************
*/
int MainWindow::  db_check_otdr_ref(char buf[])
{
    tdb_otdr_ref_t *output;
    int return_val;
    char msg[SHORT_MSG_LEN];
    output = ( tdb_otdr_ref_t *)(buf);
    return_val = RET_SUCCESS;
    if(output->pevent_hdr->count < 0 || output->pevent_hdr->count > MAX_EVENT_NUM ||
            output->pdata_hdr->count < 0 || output->pdata_hdr->count > MAX_PT_NUM)
    {
        sprintf(msg,"ref curv count error  event num %d data num %d",\
                output->pevent_hdr->count,output->pdata_hdr->count );
        return_val = RET_PARAM_INVALID;
    }
    //    else if(strcmp((char *)(output->pevent_hdr->eventid), "KeyEvents") != 0 || strcmp((char * )output->pchain->inf, "OTDRTestResultInfo") != 0)
    //    {
    //        sprintf(msg,"ref curv event/ result  id error  event id %s result id %s",\
    //                output->pevent_hdr->eventid,output->pchain->inf);
    //        return_val = RET_PARAM_INVALID;
    //    }
    else if(!(output->pref_data->leve2 > 0 && output->pref_data->leve1>output->pref_data->leve2 &&\
              output->pref_data->leve0 > output->pref_data->leve1))
    {
        sprintf(msg,"ref curv alarm lev error  lev0 %d lev1 %d lev2 %d",\
                output->pref_data->leve0,output->pref_data->leve1,output->pref_data->leve2);
        return_val = RET_PARAM_INVALID;
    }
    if(return_val != RET_SUCCESS)
        qDebug("%s", msg);

    return return_val;
}
/*
   **************************************************************************************
 *  函数名称：check_cyc_cfg
 *  函数描述：检查周期性测量是否满足要求
 *  入口参数：
 *  返回参数：0 成功 其他，为错误码
 *  作者       ：
 *  日期       ：
 **************************************************************************************
*/
int MainWindow::db_check_cyc_cfg(char buf[])
{
    _tagOswCycCfg *posw_cyc;
    int res_code, count, i;
    int offset;
    offset = sizeof(_tagDataHead);
    memcpy(&count, buf + offset, sizeof(int));
    offset += sizeof(int);
    res_code = RET_SUCCESS;
    for(i = 0; i < count; i++)
    {
        posw_cyc = (_tagOswCycCfg *)(buf + offset);
        offset += sizeof(_tagOswCycCfg);
        res_code = check_dev_type(posw_cyc->frame, posw_cyc->card,posw_cyc->type);
        if (res_code != RET_SUCCESS)
            break;

        res_code = check_dev_port_no(posw_cyc->frame, posw_cyc->card, posw_cyc->port);
        if (res_code != RET_SUCCESS)
            break;
    }

    return res_code;
}
/*
   **************************************************************************************
 *  函数名称：db_save_cyc_cfg
 *  函数描述：将周期性测量配置保存到数据库
 *  入口参数：
 *  返回参数：0 成功 其他，为错误码
 *  作者       ：
 *  日期       ：
 **************************************************************************************
*/
int MainWindow::db_save_cyc_cfg(char buf[], void *ptr_opt)
{
    _tagOswCycCfg *posw_cyc;
    tdb_osw_cyc cyc_cfg;
    tdb_osw_cyc mask;
    _tagDataHead *pDataHead;


    int res_code, count, i;
    int offset, res_db;

    pDataHead = (_tagDataHead *)(buf);
    offset = sizeof(_tagDataHead);

    res_code = db_check_cyc_cfg(buf);//检查参数正确与否
    if(res_code == RET_SUCCESS)
    {
        memset(&mask, 0, sizeof(tdb_osw_cyc));
        memset(&cyc_cfg, 0, sizeof(tdb_osw_cyc));
        mask.frame = 1;
        mask.slot = 1;
        mask.port = 1;

        memcpy(&count, buf + offset, sizeof(int));
        offset += sizeof(int);
        for(i = 0; i < count; i++)
        {
            posw_cyc = (_tagOswCycCfg *)(buf + offset);
            offset += sizeof(_tagOswCycCfg);
            memcpy(&cyc_cfg.frame, posw_cyc, sizeof(_tagOswCycCfg));
            res_db = tmsdb_Delete_osw_cyc(&cyc_cfg, &mask);//检查是否有相同的条目在里面
            if (res_db < 0)
            {
                res_code = RET_IGNORE_SAVE;
                break;
            }
            res_db = tmsdb_Insert_osw_cyc(&cyc_cfg, &mask, 1);
            if (res_db < 0)
            {
                res_code = RET_IGNORE_SAVE;
                break;
            }
        }
    }
    mcu_Ack(ptr_opt, pDataHead->cmd, res_code);
    if(res_code != RET_SUCCESS)
    {
        qDebug("save cyc cfg error %d", res_code);
    }
    return res_code;
}
/*
   **************************************************************************************
 *  函数名称：db_del_cyc_cfg
 *  函数描述：删除周期性测量条目，全部删除或者逐条删除
 *  入口参数：
 *  返回参数：
 *  作者       ：
 *  日期       ：
 **************************************************************************************
*/
int MainWindow::db_del_cyc_cfg(char buf[], void *ptr_opt)
{
    tdb_osw_cyc cyc_cfg;
    tdb_osw_cyc mask;
    _tagDataHead *pDataHead;
    _tagDevComm *pDelDev;

    int res_code, count, i;
    int offset, res_db;
    pDataHead = (_tagDataHead *)(buf);

    bzero(&cyc_cfg, sizeof(cyc_cfg));
    bzero(&mask, sizeof(mask));
    res_code = RET_PARAM_INVALID;//现设定号非法参数
    if(pDataHead->cmd == ID_DELALL_TBCYCTEST)
    {
        res_code = RET_SUCCESS;
        res_db = tmsdb_Delete_osw_cyc(&cyc_cfg, &mask);//检查是否有相同的条目在里面;
        if(res_db < 0)
        {
            res_code = RET_UNEXIST_ROW;
        }
    }
    else if(pDataHead->cmd == ID_DEL_TBCYCTEST)
    {
        //按机框号，槽位，设备类型，端口删除
        mask.frame = 1;
        mask.slot = 1;
        mask.type = 1;
        mask.port = 1;
        offset = sizeof(_tagDataHead);
        memcpy(&count, buf + offset, sizeof(int));
        offset += sizeof(int);
        res_code = RET_SUCCESS;
        for(i = 0; i < count;i++)
        {
            pDelDev = (_tagDevComm *)(buf + offset);
            offset += sizeof(_tagDevComm);
            memcpy(&cyc_cfg, pDelDev, sizeof(_tagDevComm));
            res_db = tmsdb_Delete_osw_cyc(&cyc_cfg, &mask);//检查是否有相同的条目在里面;
            if(res_db < 0)
            {
                res_code = RET_UNEXIST_ROW;
            }
        }
    }

    mcu_Ack(ptr_opt, pDataHead->cmd, res_code);
    return res_code;
}

/*
   **************************************************************************************
 *  函数名称：find_changed_dev
 *  函数描述：frame is delete 则删除该机框所有的记录，如果板卡类型发生变化或者
 *  函数描述：端口总数发生变化，那么也会删掉该设备关联的信息
 *  入口参数：
 *  返回参数：
 *  作者       ：
 *  日期       ：
 **************************************************************************************
*/
int MainWindow::find_changed_dev(_tagSubrackInfo *pdstSubrackInfor, _tagSubrackCard dstCardCommp[])
{
    int i, j, res_code;
    int dst_state, src_state;
    int dst_type, src_type;
    int dst_port, src_port;
    bool is_delete;
    is_delete = false;
    res_code = RET_SUCCESS;
    for(i = 0; i < NUM_SUBRACK;i++)
    {
        dst_state = (pdstSubrackInfor->onState >> i) & 1;
        src_state = (m_subrackInfo.onState >> i) & 1;
        if(src_state == 1&& dst_state == 1)
        {
            for(j = 0; j < NUM_CARD;j++)
            {
                is_delete = false;
                dst_type = dstCardCommp[i].type[j];
                src_type = m_subrackCard[i].type[j] ;
                dst_port = dstCardCommp[i].ports[j];
                src_port = m_subrackCard[i].ports[j] ;
                if(src_type == OSW || src_type == OPM || src_type == OLP)
                {
                    if(src_type != dst_type)
                        is_delete = true;
                    else if(dst_port != src_port)
                        is_delete = true;
                }
                if(is_delete)
                    db_delete_record(i,j);
            }
        }
        else if(src_state == 1&& dst_state == 0)
        {
            db_delete_record(i); //按机框号删除相关记录
        }
    }
    return 0;
}
/*
   **************************************************************************************
 *  函数名称：db_delete_record，
 *  函数描述：按照机框或者机框，槽位删除数据库对应的记录
 *  函数描述：周期性测量，osw地理信息，opm/olp--osw，参考曲线
 *  入口参数：
 *  返回参数：
 *  作者       ：
 *  日期       ：
 **************************************************************************************
*/
int MainWindow::db_delete_record(int frame, int card)
{
    //不主动清空
    return 0;
    //删除光开关地理信息
    db_delete_osw_map(frame,card);
    //删除olp/opm---osw
    db_delete_osw_connect( frame,  card);
    //删除 osw参考曲线
    db_delete_osw_ref_curv( frame , card);
    //删除osw周期性测量
    db_delete_osw_cyc( frame, card);
    //删除周期性测量时间
    db_delete_cyc_time(frame, card);

    return 0;
}
/*
   **************************************************************************************
 *  函数名称：db_delete_osw_cyc
 *  函数描述：删除光开关关联地理信息测量表， 按机框或者按机框槽位删除
 *  入口参数：
 *  返回参数：
 *  作者       ：
 *  日期       ：
 **************************************************************************************
*/
int MainWindow::db_delete_osw_map(int frame ,int card)
{
    tdb_dev_map_t   cfg_map, mask_map;

    //光开关的端口关联信息
    memset(&cfg_map, 0, sizeof(tdb_dev_map_t));
    memset(&mask_map, 0, sizeof(tdb_dev_map_t));

    if(frame >= 0 && frame < NUM_SUBRACK)
    {
        if(card >= 0 && card <= NUM_CARD)
        {
            cfg_map.frame = frame;
            cfg_map.slot = card;
            mask_map.frame = 1;
            mask_map.slot = 1;
            tmsdb_Delete_dev_map(&cfg_map, &mask_map);
        }
        else
        {
            cfg_map.frame = frame;
            mask_map.frame = 1;
            tmsdb_Delete_dev_map(&cfg_map, &mask_map);

        }
    }

    return 0;
}
/*
   **************************************************************************************
 *  函数名称：db_delete_osw_cyc
 *  函数描述：删除olp/opm---osw测量表， 按机框或者按机框槽位删除
 *  入口参数：
 *  返回参数：
 *  作者       ：
 *  日期       ：
 **************************************************************************************
*/
int MainWindow::db_delete_osw_connect(int frame, int card)
{
    int dev_type;
    tdb_any_unit_osw_t cfg_connect, mask_connect;
    //opm/olp-- OSW关系
    memset(&cfg_connect, 0, sizeof(tdb_any_unit_osw_t));
    memset(&mask_connect, 0, sizeof(tdb_any_unit_osw_t));

    if(frame >= 0 && frame < NUM_SUBRACK)
    {
        if(card >= 0 && card <= NUM_CARD)//按槽位号
        {
            dev_type = m_subrackCard[frame].type[card];
            if(dev_type == OPM || dev_type == OLP)
            {
                cfg_connect.any_frame = frame;
                cfg_connect.any_slot = card;
                mask_connect.any_frame = 1;
                mask_connect.any_slot = 1;
                tmsdb_Delete_any_unit_osw (&cfg_connect, &mask_connect);
            }
            else if(dev_type == OSW)
            {
                cfg_connect.osw_frame = frame;
                cfg_connect.osw_slot = card;
                mask_connect.osw_frame = 1;
                mask_connect.osw_slot = 1;
                tmsdb_Delete_any_unit_osw (&cfg_connect, &mask_connect);
            }
        }
        else // 删两遍第一次删olp opm 第二次删osw
        {
            cfg_connect.any_frame = frame;
            mask_connect.any_frame = 1;
            tmsdb_Delete_any_unit_osw (&cfg_connect, &mask_connect);
            cfg_connect.osw_frame = frame;
            mask_connect.osw_frame = 1;
            tmsdb_Delete_any_unit_osw (&cfg_connect, &mask_connect);

        }
    }
    return 0;
}
/*
   **************************************************************************************
 *  函数名称：db_delete_osw_ref_curv
 *  函数描述：删除参考曲线表， 按机框或者按机框槽位删除
 *  入口参数：
 *  返回参数：
 *  作者       ：
 *  日期       ：
 **************************************************************************************
*/
int MainWindow::db_delete_osw_ref_curv(int frame ,int card)
{
    tdb_otdr_ref_t cfg_refotdr, mask_refotdr;
    tms_otdr_ref_hdr cfg_hdr, mask_hdr;

    //参考曲线
    memset(&cfg_refotdr, 0, sizeof(tdb_otdr_ref_t));
    memset(&mask_refotdr, 0, sizeof(tdb_otdr_ref_t));

    memset(&cfg_hdr, 0, sizeof(tms_otdr_ref_hdr));
    memset(&mask_hdr, 0, sizeof(tms_otdr_ref_hdr));
    cfg_refotdr.pref_hdr = & cfg_hdr;
    mask_refotdr.pref_hdr = &mask_hdr;
    if(frame >= 0 && frame < NUM_SUBRACK)
    {
        if(card >= 0 && card <= NUM_CARD)//按槽位号
        {
            cfg_hdr.osw_frame = frame;
            cfg_hdr.osw_slot = card;
            mask_hdr.osw_frame = 1;
            mask_hdr.osw_slot = 1;
            tmsdb_Delete_otdr_ref(&cfg_refotdr,&mask_refotdr);
        }
        else //机框被删除
        {
            cfg_hdr.osw_frame = frame;
            mask_hdr.osw_frame = 1;
            tmsdb_Delete_otdr_ref(&cfg_refotdr,&mask_refotdr);
        }
    }
    return 0;
}
/*
   **************************************************************************************
 *  函数名称：db_delete_osw_cyc
 *  函数描述：删除周期性测量表， 按机框或者按机框槽位删除
 *  入口参数：
 *  返回参数：
 *  作者       ：
 *  日期       ：
 **************************************************************************************
*/
int MainWindow::db_delete_osw_cyc(int frame ,int card)
{
    tdb_osw_cyc_t cfg_cyc, mask_cyc;
    //周期性测量
    memset(&cfg_cyc, 0, sizeof(tdb_osw_cyc_t));
    memset(&mask_cyc, 0, sizeof(tdb_osw_cyc_t));

    //判断是按照槽位是否有效，无效则随按机框删除
    if(frame >= 0 && frame < NUM_SUBRACK)
    {
        if(card >= 0 && card <= NUM_CARD)
        {
            cfg_cyc.frame = frame;
            cfg_cyc.slot = card;
            mask_cyc.frame = 1;
            mask_cyc.slot = 1;
            tmsdb_Delete_osw_cyc(&cfg_cyc, &mask_cyc);
        }
        else //a整个机框删除
        {
            cfg_cyc.frame = frame;
            mask_cyc.frame = 1;
            tmsdb_Delete_osw_cyc(&cfg_cyc, &mask_cyc);
        }
    }
    return 0;
}
/*
   **************************************************************************************
 *  函数名称：db_delete_osw_cyc_time
 *  函数描述：删除周期性测量时间， 按机框或者按机框槽位删除
 *  入口参数：
 *  返回参数：
 *  作者       ：
 *  日期       ：
 **************************************************************************************
*/
int MainWindow::db_delete_cyc_time(int frame ,int card)
{
    tdb_osw_cyc_bak_t cfg_cyc, mask_cyc;
    //周期性测量
    memset(&cfg_cyc, 0, sizeof(tdb_osw_cyc_t));
    memset(&mask_cyc, 0, sizeof(tdb_osw_cyc_t));

    //判断是按照槽位是否有效，无效则随按机框删除
    if(frame >= 0 && frame < NUM_SUBRACK)
    {
        if(card >= 0 && card <= NUM_CARD)
        {
            cfg_cyc.frame = frame;
            cfg_cyc.slot = card;
            mask_cyc.frame = 1;
            mask_cyc.slot = 1;
            tmsdb_Delete_osw_cyc_bak(&cfg_cyc, &mask_cyc);
        }
        else //a整个机框删除
        {
            cfg_cyc.frame = frame;
            mask_cyc.frame = 1;
            tmsdb_Delete_osw_cyc_bak(&cfg_cyc, &mask_cyc);
        }
    }
    return 0;
}
/*
   **************************************************************************************
 *  函数名称：db_test
 *  函数描述：测试数据库读取代码
 *  入口参数：
 *  返回参数：
 *  作者       ：
 *  日期       ：
 **************************************************************************************
*/
int MainWindow::db_test()
{
    tdb_otdr_ref_t input, mask;
    struct tms_otdr_ref_hdr dev_osw,mtest_hdr;

    bzero(&mask,      sizeof(tdb_otdr_ref_t));
    bzero(&mtest_hdr, sizeof(struct tms_otdr_ref_hdr));
    input.pref_hdr = &dev_osw;
    mask.pref_hdr  = &mtest_hdr;

    mask.id = 0;
    dev_osw.osw_frame =15;
    dev_osw.osw_slot = 0;
    dev_osw.osw_port = 0;
    dev_osw.osw_type = OSW;

    mtest_hdr.osw_frame = 1;
    mtest_hdr.osw_slot = 1;
    mtest_hdr.osw_port = 1;
    mtest_hdr.osw_type = 1;

    tmsdb_Select_otdr_ref(&input, &mask, db_read_refotdr, &ReferCrv);
    qDebug(" refer curv data num %d event num %d alarm leve 0 %d",\
           ReferCrv.ptNum, ReferCrv.eventNum, ReferCrv.alarmTh.seriousAlarmTh);
    return 0;
}

/*
   **************************************************************************************
 *  函数名称：deal_db_alarm_curv
 *  函数描述：主网管连接上mcu,上报告警曲线，收到主网管回应时，删除对应
 *                ：的曲线.暂时不支持查询告警曲线
 *  入口参数：要删除的曲线id，或者上次发送成功的曲线id
 *  返回参数：删除成功 0;select成功,曲线的id; 曲线空, 返回-2;其他-1
 *  日期版本：
 *  作者       ：
 *  日期       ：
 *  修改日期：2015-11-03
 *  修改内容：既可以发送曲线，又可以删除曲线，群发，暂不支持查询
 *                ：
 **************************************************************************************
*/
int MainWindow::deal_db_alarm_curv(int db_id, int type, int pkid)
{
    int nm_fd;
    int res_db;
    res_db = -1;
    glink_addr dst_addr;
    tdb_otdr_alarm_data  cfg_cyc, cyc_curv, mask_cyc;
    tms_retotdr_test_hdr itest_hdr,mtest_hdr;

    tms_retotdr_test_hdr   test_hdr;
    tms_retotdr_test_param test_param;
    tms_retotdr_data_hdr   data_hdr;
    tms_retotdr_event_hdr  event_hdr;
    tms_retotdr_chain      chain;
    tms_alarm_line_hdr alarmLine;

    //初始化读取数据库的参数
    bzero(&mask_cyc, sizeof(tdb_otdr_his_data_t));//mask
    bzero(&cyc_curv, sizeof( tdb_otdr_his_data_t));//输出内容
    bzero(&cfg_cyc, sizeof( tdb_otdr_his_data_t));//查询选项

    bzero(&mtest_hdr,  sizeof(tms_retotdr_test_hdr));
    bzero(&itest_hdr,  sizeof(tms_retotdr_test_hdr));

    cfg_cyc.ptest_hdr = &itest_hdr;
    mask_cyc.ptest_hdr = &mtest_hdr;
    cyc_curv.ptest_hdr = &test_hdr;
    cyc_curv.ptest_param = &test_param;
    cyc_curv.pdata_hdr = &data_hdr;
    cyc_curv.pdata_val = ( tms_retotdr_data_val *)cyc_buf;
    cyc_curv.pevent_hdr = &event_hdr;
    cyc_curv.pevent_val = (tms_retotdr_event_val *)cyc_event_buf;
    cyc_curv.pchain = &chain;
    cyc_curv.palarm = &alarmLine;

    bzero(&test_hdr, sizeof(test_hdr));
    bzero(&test_param, sizeof(test_param));
    bzero(&event_hdr, sizeof(event_hdr));
    bzero(&chain, sizeof(chain));
    //收到回应码，删除对应ID的曲线
    objDealCycCurv.lock();
    if(type == ALARM_CURV_DEL)
    {
        if(db_id < 0)
        {
            res_db = DB_RET_OTHSER_ERROR;
            //printf("del alarm curv id error id %d \n", db_id);
            printf("%s(): Line : %d del curv  error id %d  \n",\
                   __FUNCTION__, __LINE__,db_id);
            goto usr_exit;
        }
        mask_cyc.id = 1;
        cfg_cyc.id = db_id;
        res_db = tmsdb_Delete_otdr_alarm_data(&cfg_cyc, &mask_cyc);
        //printf("del alarm curv id %d res_db %d,\n",db_id, res_db);
        printf("%s(): Line : %d del curv id %d res-db %d \n",\
               __FUNCTION__, __LINE__,db_id,res_db);
        res_db = db_id;
    }
    else
    {
        /*
         *2016-01-28 初始化发送曲线标志
        */
        m_ctrlUpData.IsPermitRet32 = UNPERMIT_SEND;
        _tagDbReadCurv dbReadCurv;
        bzero(&dbReadCurv, sizeof(dbReadCurv));
        //获取数据库中的周期性测量曲线
        dbReadCurv.curv_type = CURV_TYPE_ALARM;
        dbReadCurv.ptr = (void *)(&cyc_curv);
        /*
        *主网管连接上MCU，所有的网管全部发送一遍,
        *然后将曲线的id加入重发队列中去
        */
        if(type == SEND_TYPE_INITIATIVE)
        {
            char buf[48];
            _tagDataHead *pDataHead;
            bzero(buf, sizeof(buf));
            pDataHead = (_tagDataHead *)buf;
            mask_cyc.id = DB_MORE;
            cfg_cyc.id = db_id;
            res_db = tmsdb_Select_otdr_alarm_data(&cfg_cyc, &mask_cyc, \
                                                  db_read_curv_alarm ,&dbReadCurv);
            if(res_db != RET_SUCCESS)
                goto usr_exit;
            nm_fd = 0;
            dst_addr.src = ADDR_MCU;

            dst_addr.pkid = creat_pkid();
            dst_addr.dst = ADDR_HOST_VIP;
            nm_fd = tms_SelectFdByAddr(&dst_addr.dst);
            /*
             *2016-01-28 曲线的id是是在回掉函数中自行赋值的
            */
            res_db = cyc_curv.id;
            pDataHead->cmd = ID_RET_OTDR_CYC;
            pDataHead->dataLen = sizeof(_tagDataHead) + sizeof(int);
            memcpy(buf + sizeof(_tagDataHead),&res_db, sizeof(res_db));
            if(nm_fd > 0)
                input_cpy_sock_retry_queue(buf, dst_addr.pkid);
            //dst_addr.dst = ADDR_MASS_SEND;
            tms_RetOTDRCycle_V2(nm_fd,&dst_addr,cyc_curv.palarm, cyc_curv.ptest_hdr, cyc_curv.ptest_param,cyc_curv.pdata_hdr,\
                                cyc_curv.pdata_val, cyc_curv.pevent_hdr,cyc_curv.pevent_val,cyc_curv.pchain);
            printf("%s(): Line : %d activ send curv id %d  \n",  __FUNCTION__, __LINE__,res_db);

        }
        //重发
        else if(type == SEND_TYPE_RETRY)
        {
            mask_cyc.id = DB_EQ;
            cfg_cyc.id = db_id;
            res_db = tmsdb_Select_otdr_alarm_data(&cfg_cyc, &mask_cyc, \
                                                  db_read_curv_alarm ,&dbReadCurv);
            if(res_db != RET_SUCCESS)
                goto usr_exit;
            dst_addr.src = ADDR_MCU;
            dst_addr.dst = ADDR_HOST_VIP;
            if(pkid == -1)
            {
                printf("retry alarm curv pkid error %d \n", pkid);
                res_db = DB_RET_OTHSER_ERROR;
                goto usr_exit;
            }
            tms_RetOTDRCycle_V2(nm_fd,&dst_addr,cyc_curv.palarm, cyc_curv.ptest_hdr, cyc_curv.ptest_param,cyc_curv.pdata_hdr,\
                                cyc_curv.pdata_val, cyc_curv.pevent_hdr,cyc_curv.pevent_val,cyc_curv.pchain);
            res_db = cyc_curv.id;
            printf("%s(): Line : %d  retry send  curv_id %d ask id %d\n",  __FUNCTION__, __LINE__,res_db,db_id);

        }

    }

usr_exit:
    objDealCycCurv.unlock();

    return res_db;
}
int MainWindow::  db_check_cyc_curv(char buf[])
{
    tdb_otdr_his_data_t *output;
    int return_val;
    char msg[SHORT_MSG_LEN];
    output = ( tdb_otdr_his_data_t *)(buf);
    return_val = RET_SUCCESS;
    if(output->pevent_hdr->count < 0 || output->pevent_hdr->count > MAX_EVENT_NUM ||
            output->pdata_hdr->count < 0 || output->pdata_hdr->count > MAX_PT_NUM)
    {
        sprintf(msg,"ref curv count error  event num %d data num %d",\
                output->pevent_hdr->count,output->pdata_hdr->count );
        return_val = RET_PARAM_INVALID;
    }
    //    else if(strcmp((char *)(output->pevent_hdr->eventid), "KeyEvents") != 0 || strcmp((char * )output->pchain->inf, "OTDRTestResultInfo") != 0)
    //    {
    //        sprintf(msg,"ref curv event/ result  id error  event id %s result id %s",\
    //                output->pevent_hdr->eventid,output->pchain->inf);
    //        return_val = RET_PARAM_INVALID;
    //    }
    if(return_val != RET_SUCCESS)
        qDebug("%s", msg);

    return return_val;
}
/*
   **************************************************************************************
 *  函数名称：get_optical_powr_alarm
 *  函数描述：mcu olp opm 板卡发送获取功率告警，板卡直接返回给网管
 *  入口参数：
 *  返回参数：
 *  作者       ：
 *  日期       ：
 **************************************************************************************
*/
int MainWindow::get_optical_powr_alarm()
{
    int i, j;
    int state_1, state_2;
    glink_addr dst_addr;
    dst_addr.src = ADDR_MCU;
    dst_addr.dst = ADDR_CARD;
    dst_addr.pkid = creat_pkid();
    tms_devbase card_addr;
    //遍历所有板卡，给所有的opm，olp 发送查询命令，由板卡主动上报
    for(i = 0; i < NUM_SUBRACK; i++)
    {
        state_1 = m_subrackInfo.onState;
        if(1 == ((state_1 >> i)&1))
        {
            for(j = 0; j < NUM_CARD;j++)
            {
                //查找otdr，如果找到多个或者没有一个，都要告警
                state_2 = m_subrackCard[i].onState;
                if(1 == ((state_2 >>j)&1))
                {
                    tms_GetDevBaseByLocation(i, j,&card_addr);
                    if(OPM == m_subrackCard[i].type[j])
                    {
                        tms_MCU_GetOPMAlarm(card_addr.fd,&dst_addr, i,j );
                    }
                    else if(OLP == m_subrackCard[i].type[j])
                    {
                        tms_MCU_GetOLPAlarm(card_addr.fd, &dst_addr,i,j );
                    }
                }
            }
        }
    }
    qDebug("get opm total alarm !");
    return 0;
}
/*
   **************************************************************************************
 *  函数名称：get_optical_powr_alarm
 *  函数描述：mcu olp opm 板卡发送获取功率告警，板卡直接返回给网管
 *  入口参数：
 *  返回参数：
 *  作者       ：
 *  日期       ：
 **************************************************************************************
*/
unsigned short MainWindow::creat_pkid()
{
    objPkid.lock();
    pkid++;
    objPkid.unlock();
    return pkid;
}
/*
   **************************************************************************************
 *  函数名称：db_get_test_port_rout
 *  函数描述：获取整条模块级联表。调用者可根据该条路由逐级切换测试
 *                ：
 *  入口参数：
 *  返回参数：
 *  作者       ：
 *  日期       ：
 *  修改日期：2015-11-04
 *  修改内容：支持CTU模式路由其特点:倒数第二个设备ip非本机,类型为otdr osw
 **************************************************************************************
*/
int MainWindow::db_get_test_port_rout(_tagDevComm testDev, _tagOswRout *ptrRou)
{
    int ret_db, record_num, i;
    int res_code, index;
    tdb_route_t record, msk;
    tdb_route_t record_buf[ROUT_BUF_LEN];

    res_code =  RET_UNEXIST_ROW;
    bzero(&msk, sizeof(tdb_route_t));
    bzero(&record, sizeof(tdb_route_t));
    record.frame_b = testDev.frame_no;
    record.slot_b = testDev.card_no;
    record.type_b = testDev.type;

    //先用被测端口进行查询，如果查询成功，说明该端口属于级联端口
    record_num = ROUT_BUF_LEN;
    record.port_b = testDev.port;
    ret_db = tmsdb_Select_oneline(&record,record_buf,&record_num);
    if(ret_db >= 0&&record_num > 0) //级联端口不允许测试
    {
        res_code =  RET_CONNECT_PORT_NOT_TEST;
        goto usr_exit;
    }
    //如果是otdr端口，查找不到端口，可以进行测试
    if(testDev.type == OTDR) //如果是otdr设备,可以进行测量了,不再向下查找
    {
        ptrRou->depth = 1;
        memcpy(&ptrRou->table_buf[0].Dev, &testDev, sizeof(_tagDevComm));
        ptrRou->table_buf[0].dstIP = 0;
        ptrRou->table_buf[0].srcIP = 0;
        res_code = RET_SUCCESS;
        goto usr_exit;
    }
    //用被测端口所在的设备的公共端口查询
    record_num = ROUT_BUF_LEN;
    record.port_b = COMN_COM;
    ret_db = tmsdb_Select_oneline(&record,record_buf,&record_num);
    /*
     * 模块级联表中最简单的也必须存在3条记录或者
     *从公共端口查询到结束也必须存在3条记录
    */
    if(ret_db < 0)
    {
        //不存在
        res_code = RET_MODULE_ROUTE_NO_EXIST;
        goto usr_exit;
    }
    else if(record_num < 3 || record_num > ROUT_BUF_LEN)
    {
        //非法
        res_code = RET_MODULE_ROUTE_ILLEGALT;
        goto usr_exit;
    }
    else/* if(ret_db >= 0&&record_num >= 3 && record_num <= ROUT_BUF_LEN)*/
    {
        /*
         *最后一个设备必须是otdr,或者最后一个设备的ip不是本地的
        */
        if((record_buf[record_num - 2].type_b != OTDR) && record_buf[record_num - 2].ip_dst == 0)
        {
            res_code = RET_MODULE_ROUTE_ILLEGALT;
            goto usr_exit;
        }
        res_code = RET_SUCCESS;
        index = 0;
        //第0个设备就是我们查询的入口设备,所以从1开始记录
        for(i = 1; i < record_num - 1;i++)
        {
            //关联的设备应该是otdr,osw,并且，不能超过两个osw,光缆路由上不会超过3个设备
            if((record_buf[i].type_b != OTDR && record_buf[i].type_b != OSW &&record_buf[i].type_b != OLP ) \
                    || index > OSW_CONNECT_DEPTH)
            {
                qDebug("search rout exit dev type %d depth %d ", record_buf[i].type_a, index);
                res_code = RET_MODULE_ROUTE_ILLEGALT;
                break;
            }
            /*
             *级联表中有公共端口
            */
            if(record_buf[i].port_b != COMN_COM)
            {
                ptrRou->table_buf[index].Dev.frame_no = record_buf[i].frame_b;
                ptrRou->table_buf[index].Dev.card_no = record_buf[i].slot_b;
                ptrRou->table_buf[index].Dev.type = record_buf[i].type_b;
                ptrRou->table_buf[index].Dev.port = record_buf[i].port_b;
                ptrRou->table_buf[index].srcIP = record_buf[i].ip_src;
                ptrRou->table_buf[index].dstIP = record_buf[i].ip_dst;
                index++;
                /*
                 *2016-01-19 将查询到的路由打印出来
                */
                printf("%s(): Line : %d  index %d frame %d card %d type %d port %d \n", \
                       __FUNCTION__, __LINE__,index, record_buf[i].frame_b,record_buf[i].slot_b,\
                       record_buf[i].type_b,record_buf[i].port_b);
                /*
                 *如果该条记录是CTU模式，那么倒数第二个的模块的ip一定
                 *不是本机IP
                */
                if(record_buf[i].ip_dst != 0 && i != (record_num - 2))
                {
                    res_code = RET_MODULE_ROUTE_ILLEGALT;
                    goto usr_exit;
                }
            }
        }
        if(index > 0)
        {
            ptrRou->depth = index;
        }
        else
        {
            res_code = RET_MODULE_ROUTE_ILLEGALT;
            goto usr_exit;
        }
    }

    if(res_code != RET_SUCCESS)
        qDebug("search rout error dev ret_db %d record_num %d  depth %d ",\
               ret_db, record_num, index);
usr_exit:
    return res_code;
}
/*
   **************************************************************************************
 *  函数名称：insert_tablewidget_item
 *  函数描述：插入tablewidget item的公共函数
 *  入口参数：
 *  返回参数：
 *  作者       ：
 *  日期       ：
 **************************************************************************************
*/
void MainWindow::insert_tablewidget_item(QString str, QTableWidget *ptr_widget, int row, int column)
{
    QTableWidgetItem *ptr_newItem;
    if(ptr_widget == NULL)
        goto usr_exit;
    ptr_newItem = new QTableWidgetItem(str);
    ptr_newItem->setTextAlignment(Qt::AlignCenter);
    ptr_widget->setItem(row,column, ptr_newItem);
usr_exit:
    return;
}
/*
   **************************************************************************************
 *  函数名称：
 *  函数描述：第10个槽位的响应函数，创建dlg
 *  入口参数：
 *  返回参数：
 *  作者       ：
 *  日期       ：
 **************************************************************************************
*/
void MainWindow::on_pushBtnCard_10_clicked()
{
    creatDlg(m_subrackInfo.curShowNo, 9);
}
/*
   **************************************************************************************
 *  函数名称：get_otdr_modul_para
 *  函数描述：创建otdr任务后，需要从模块获取到otdr波长等信息
 *  入口参数：
 *  返回参数：
 *  作者       ：
 *  日期       ：
 **************************************************************************************
*/
int MainWindow::get_otdr_modul_para()
{
    int res_code, i;
    int is_uninitial;
    bool isLock;
    _tagDlgAttribute otdrDev;
    //得到的设备软件版本号
    tms_devbase devbase;
    tsk_OtdrManage *pOtdrTsk;
    res_code = 0;
    isLock = refreshOtdrDev.tryLock();
    if(!isLock) //正在更新otdr模块信息
        goto usr_exit;
    is_uninitial = 0;
    for(i = 0; i <m_otdrTskQue.xlist.size(); i++ )
    {
        pOtdrTsk = (tsk_OtdrManage *)m_otdrTskQue.xlist.at(i);
        if(pOtdrTsk->modulPara.initial == 0)
        {
            is_uninitial = 1;
            otdrDev.frameNo = pOtdrTsk->otdrAddr.frame_no;
            otdrDev.cardNo = pOtdrTsk->otdrAddr.card_no;
            otdrDev.devType = pOtdrTsk->otdrAddr.type;
            tms_GetDevBaseByLocation(otdrDev.frameNo, otdrDev.cardNo,&devbase);
            if( DevCommuState[otdrDev.frameNo][otdrDev.cardNo].cur_type == OTDR)
                res_code = send_cmd(devbase.fd, (char *)&otdrDev,ID_GET_OTDR_PARAM);
        }
    }
    //全部已经初始化，不再更新
    if(is_uninitial == 0)
        isGetOtdrPara = 0;
usr_exit:
    if(isLock)
        refreshOtdrDev.unlock();
    return res_code;

}

void MainWindow::on_pushBtnCard_11_clicked()
{
    creatDlg(m_subrackInfo.curShowNo, 10);
}

void MainWindow::on_pushBtnCard_12_clicked()
{
    creatDlg(m_subrackInfo.curShowNo, 11);
}

void MainWindow::on_pushBtnCard_13_clicked()
{
    creatDlg(m_subrackInfo.curShowNo, 12);
}

void MainWindow::on_pushBtnCard_14_clicked()
{
    creatDlg(m_subrackInfo.curShowNo, 13);
}

void MainWindow::on_pushBtnCard_15_clicked()
{
    creatDlg(m_subrackInfo.curShowNo, 14);
}

void MainWindow::on_pushBtnCard_16_clicked()
{
    creatDlg(m_subrackInfo.curShowNo, 15);
}
/*
   **************************************************************************************
 *  函数名称：gpio_init
 *  函数描述：初始化io口，目前是将所有io口初始化成输出状态
 *  入口参数：
 *  返回参数：
 *  作者       ：
 *  日期       ：2015-10-29
 **************************************************************************************
*/
int MainWindow::gpio_init()
{
    struct gpio_dir iodir;
    int io_count, retv,i;
    //打开初始化函数
    retv = UNDEFINED_ERROR;
    m_ctrlStat.io_fd = 0;
    m_ctrlStat.io_fd = open_io(NULL);
    bzero(mcu_stat, sizeof(mcu_stat));
    mcu_stat[26] = 1;
    mcu_stat[PIN_ALARM_SILENCE] = 1;
    if(m_ctrlStat.io_fd <= 0)
    {
        m_ctrlStat.io_fd = 0;
        qDebug("open io error fd %d",m_ctrlStat.io_fd);
        goto usr_exit;
    }
    //获取io口数目
    get_count(&io_count);
    if(io_count != GPIO_PIN_NUM)
    {
        //出错，关闭fd
        //close_io();
        m_ctrlStat.io_fd = NULL;
        printf("get io count error count %d \n", io_count);
        goto usr_exit;
    }
    //将io口初始化成输出状态

    for(i = 21;i< 26;i++)
    {
        iodir.index = i;
        iodir.dir = IO_OUTPUT;
        set_dir(&iodir);
    }

    retv = RET_SUCCESS;
usr_exit:
    return retv;
}
/*
   **************************************************************************************
 *  函数名称：read_commn_io_state
 *  函数描述：读取普通的io口的状态
 *  入口参数：存放io口的值
 *  返回参数：
 *  作者       ：
 *  日期       ：2015-10-29
 **************************************************************************************
*/
int MainWindow::read_all_io_val(int val[])
{
    int retv;
    int i;
    struct gpio_data iodata;
    retv = UNDEFINED_ERROR;
    if(m_ctrlStat.io_fd == 0)
    {
        printf("read commn io state fd is NULL !\n");
        goto usr_exit;
    }
    for(i = 0; i < GPIO_PIN_NUM;i++)
    {
        iodata.index = i;
        get_io(&iodata);
        val[i] = iodata.data;
    }

    retv = RET_SUCCESS;
usr_exit:
    return retv;
}
/*
   **************************************************************************************
 *  函数名称：set_io_val
 *  函数描述：设置io口值
 *  入口参数：
 *  返回参数：
 *  作者       ：
 *  日期       ：2015-10-29
 **************************************************************************************
*/
int MainWindow::set_io_val(int index, int val)
{
    int retv;
    struct gpio_data iodata;

    retv = UNDEFINED_ERROR;
    if(index < 0 || index > GPIO_PIN_NUM || !(val == 0 || val == 1))
    {
        printf("set io para error index %d, val %d", index, val);
        goto usr_exit;
    }
    iodata.index = index;
    iodata.data = val;
    set_io(&iodata);
    retv = RET_SUCCESS;
usr_exit:
    return retv;
}
/*
   **************************************************************************************
 *  函数名称：get_io_val
 *  函数描述：读取io口值
 *  入口参数：
 *  返回参数：
 *  作者       ：
 *  日期       ：2015-10-29
 **************************************************************************************
*/
int MainWindow::get_io_val(int index, int &val)
{
    int retv;
    struct gpio_data iodata;

    retv = UNDEFINED_ERROR;
    if(index < 0 || index > GPIO_PIN_NUM)
    {
        printf("set io para error index %d, val %d", index, val);
        goto usr_exit;
    }
    iodata.index = index;
    get_io(&iodata);
    val = iodata.data;
    retv = RET_SUCCESS;
usr_exit:
    return retv;
}
/*
   **************************************************************************************
 *  函数名称：refresh_mcu_card_state
 *  函数描述：更新主控板卡状态
 *  入口参数：
 *  返回参数：
 *  作者       ：
 *  日期       ：2015-10-29
 **************************************************************************************
*/
int MainWindow::refresh_mcu_card_state()
{
    int retv, index, card;
    int pw_v, pw_stat;
    retv = UNDEFINED_ERROR;
    objHwAlarm.lock();
    //检查电源电压，以及是否正常工作
    card = 15;

    for(index = 1; index < 3; index++)
    {
        DevCommuState[MCU_FRAME][card ].card_state = PULL_IN;
        DevCommuState[MCU_FRAME][card].cur_type = PWU;
        pw_stat = PW_STATE_NORMAL;
        if(mcu_stat[index] == 0)//220 pin
        {
            pw_v = PWU_V220;
            if(mcu_stat[index + 4] == 1) //12v 电压是否正常
                pw_stat = PW_STATE_ERROR;
        }
        else if(mcu_stat[index + 2] == 0)//48V PIN
        {
            pw_v = PWU_V48;
            if(mcu_stat[index + 4] == 1)//12V 电压是否正常
                pw_stat = PW_STATE_ERROR;
        }
        else
        {
            pw_v = PWU_V0;
            DevCommuState[MCU_FRAME][card ].card_state = PULL_OUT;
            DevCommuState[MCU_FRAME][card].cur_type = NONE;
            pw_stat = PW_STATE_NORMAL;
        }
        DevCommuState[MCU_FRAME][card].cur_opt[0] = pw_v;
        DevCommuState[MCU_FRAME][card].pw_state = pw_stat;
        card--;
    }

    //MCU槽位必须自行更新 由于MCU模块会产生短信模块硬件告警，因此不能清空
    /*
    bzero(&DevCommuState[MCU_FRAME][MCU_CARD],\
          sizeof(DevCommuState[MCU_FRAME][MCU_CARD]));
    */
    DevCommuState[MCU_FRAME][MCU_CARD].cur_type = MCU;
    DevCommuState[MCU_FRAME][MCU_CARD].card_state = PULL_IN;
    //IO口标示的是12----0槽位的插拔状态
    card = 12;
    for(index = 7;index < 20;index++,card--)
    {
        if(mcu_stat[index] == 1)
        {
            DevCommuState[0][card].cur_type = NONE;
            if(DevCommuState[0][card].card_state == PULL_IN)
                clear_usr_link(0,card);
            DevCommuState[0][card].card_state = PULL_OUT;
        }
        else
        {
            DevCommuState[0][card].card_state = PULL_IN;
        }
    }

    objHwAlarm.unlock();
    check_card_commu_state();
    return retv;
}
/*
   **************************************************************************************
 *  函数名称：mcu_get_fd_by_ip
 *  函数描述：通过给定的IP获得对应的fd.RTU收到曲线时检查SRC ip如果IP有效
 *                ：需要找到对应的fd,将曲线发送出去
 *  入口参数：IP
 *  返回参数：fd
 *  作者       ：
 *  日期       ：2015-11-05
 *  修改日期：
 *  修改内容：
 *                ：
 **************************************************************************************
*/
#ifdef __cplusplus
extern "C" {
#endif
int get_fd(struct ep_con_t *pconNode, void *ptr)
{
    int fd, res_code;
    uint32_t IP;
    struct sockaddr_in remoteAddr;
    socklen_t 		 len;
    res_code = 0;
    len = sizeof(struct sockaddr_in);
    IP = *((uint32_t *)ptr);
    fd = pconNode->sockfd;
    getpeername(fd, (struct sockaddr*)&remoteAddr , &len);
    if(IP == remoteAddr.sin_addr.s_addr)
    {
        *((int *)ptr) = fd;
        res_code = -1;
    }
    return res_code;
}
#ifdef __cplusplus
}
#endif
/*
   **************************************************************************************
 *  函数名称：mcu_get_fd_by_ip
 *  函数描述：通过ip获取fd.ctu的点名测量中带有src-ip
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
int MainWindow::mcu_get_fd_by_ip(int IP)
{
    int fd;
    fd = -1;
    fd = IP;
    ep_Ergodic(&ep, get_fd, &fd);
    //如果fd的值没有改变，那么获取失败
    if(fd == IP)
        fd = -1;
    return fd;
}
/*
   **************************************************************************************
 *  函数名称：save_dev_cfg
 *  函数描述：保存设备的配置信息,sn 告警输出口
 *                ：
 *  入口参数：
 *  返回参数：
 *  作者       ：
 *  日期       ：2015-11-10
 *  修改日期：
 *  修改内容：
 *                ：
 **************************************************************************************
*/
int MainWindow::save_dev_cfg()
{
    int retv;
    FILE *fp;
    retv = RET_SUCCESS;
    fp = fopen((char *)cfg_file_path,"wb");
    if(fp != NULL)
    {
        fwrite(&devCfg, sizeof(devCfg),1,fp);
        fclose(fp);
    }
    else
    {
        retv = -1;
    }
    printf("save dev cfg  alarmSatae %d  buzzing time %s retv %d sn %s \n", \
           devCfg.gpioAlarm,devCfg.buzzing_time, retv,devCfg.sn);

    return retv;
}
/*
   **************************************************************************************
 *  函数名称：read_dev_cfg
 *  函数描述：读取设备的配置信息
 *                ：
 *  入口参数：
 *  返回参数：
 *  作者       ：
 *  日期       ：2015-11-10
 *  修改日期：
 *  修改内容：
 *                ：
 **************************************************************************************
*/
int MainWindow::read_dev_cfg()
{
    int retv,read_bytes;
    FILE *fp;
    retv = RET_SUCCESS;
    bzero(&devCfg, sizeof(devCfg));
    fp = fopen((char *)cfg_file_path,"rb");
    read_bytes = 0;
    if(fp != NULL)
    {
        read_bytes = fread(&devCfg, sizeof(devCfg),1,fp);
        fclose(fp);
    }
    else
    {
        retv = -1;
    }
    //如果读取的sn是空的
    if(strlen(devCfg.sn) <= 0)
    {
        create_sn(devCfg.sn);
        save_dev_cfg();
    }
    if(read_bytes <= 0 || devCfg.gpioAlarm != GPIO_ALARM_OPEN \
            && devCfg.gpioAlarm != GPIO_ALARM_CLOSE)
    {
        devCfg.gpioAlarm = GPIO_ALARM_OPEN;
    }
    printf("read cfg  read_bytes %d alarm stat %d sn %s\n", read_bytes,devCfg.gpioAlarm, devCfg.sn);
    return retv;
}
/*
   **************************************************************************************
 *  函数名称：create_sn
 *  函数描述：创建序列号
 *                ：
 *  入口参数：
 *  返回参数：
 *  作者       ：
 *  日期       ：2015-11-10
 *  修改日期：
 *  修改内容：
 *                ：
 **************************************************************************************
*/
int MainWindow::create_sn(char sn[])
{
    QUuid id = QUuid::createUuid();
    QString strId;
    QByteArray byte_arry ;
    char *id_buf;


    strId = id.toString();
    byte_arry = strId.toLatin1();
    id_buf = byte_arry.data();
    memcpy(sn, id_buf, 128);
    printf("create sn %s \n", sn);

    return 0;
}
/*
   **************************************************************************************
 *  函数名称：detect_net_card
 *  函数描述：检测网卡，如果网卡数目不正确，系统出现严重问题，需要重启
 *                ：
 *  入口参数：
 *  返回参数：返回网卡数目
 *  作者       ：
 *  日期       ：
 *  修改日期：
 *  修改内容：
 *                ：
 **************************************************************************************
*/
int MainWindow::detect_net_card(unsigned int *wlan_IP, unsigned int *lan_IP)
{
    struct sockaddr_in *sin = NULL;
    struct ifaddrs *ifa = NULL, *ifList;
    int eth_num;
    const char wlan_name[] = "wlan";
    const char lan_name[] = "lan";
    eth_num = 0;

    if (getifaddrs(&ifList) < 0)
    {
        eth_num = -1;
        printf("getifaddrs error! \n");
        goto usr_exit;
    }

    for (ifa = ifList; ifa != NULL; ifa = ifa->ifa_next)
    {
        if(ifa->ifa_addr->sa_family == AF_INET)
        {
            eth_num++;
            printf("n>>> interfaceName: %s\n", ifa->ifa_name);

            sin = (struct sockaddr_in *)ifa->ifa_addr;
            printf(">>> ipAddress: %s\n", inet_ntoa(sin->sin_addr));

            sin = (struct sockaddr_in *)ifa->ifa_dstaddr;
            printf(">>> broadcast: %s\n", inet_ntoa(sin->sin_addr));

            sin = (struct sockaddr_in *)ifa->ifa_netmask;
            printf(">>> subnetMask: %s\n", inet_ntoa(sin->sin_addr));
            if(strstr((const char *)(ifa->ifa_name),wlan_name) && wlan_IP != NULL)
                *wlan_IP = (unsigned int)(sin->sin_addr.s_addr);
            else if(strstr((const char *)ifa->ifa_name,lan_name) && lan_IP != NULL)
                *lan_IP = (unsigned int)(sin->sin_addr.s_addr);

        }
    }

    freeifaddrs(ifList);

usr_exit:    
    printf("%s: Line:%d eth num %d \n",__FUNCTION__,__LINE__,eth_num);
    return eth_num;
}
/*
   **************************************************************************************
 *  函数名称：clear_usr_link
 *  函数描述：清除掉已经建立的连接
 *                ：
 *  入口参数：
 *  返回参数：
 *  作者       ：
 *  日期       ：2015-11-19
 *  修改日期：
 *  修改内容：
 *                ：
 **************************************************************************************
*/
int MainWindow::clear_usr_link(int frame, int card)
{
    int fd,retv;
    tms_devbase devbase;
    retv = check_frame_card_range(frame, card);
    if(retv != RET_SUCCESS)
    {
        printf("usr clear link  frame or card error frame %d card %d \n", frame, card);
        goto usr_exit;
    }
    fd = tms_GetDevBaseByLocation(frame, card, &devbase);
    if(fd <=0)
    {
        printf("usr clear link fd  error fd %d\n", fd);
        goto usr_exit;
    }
    retv = ep_Close(&ep, NULL, fd);
    if(/*retv != RET_SUCCESS*/true)
        printf("usr clear link ep_close result %d \n", retv);
usr_exit:
    return retv;
}
/*
   **************************************************************************************
 *  函数名称：show_total_hw_alarm
 *  函数描述：输出全部硬件告警，调试使用
 *                ：
 *  入口参数：
 *  返回参数：
 *  作者       ：
 *  日期       ：2016-02-02
 *  修改日期：
 *  修改内容：
 *                ：
 **************************************************************************************
*/
void MainWindow::show_total_hw_alarm()
{
    int i,j, type,port_num;
    int IsAlarm;
    printf("%s(): Line : %d  \n", __FUNCTION__, __LINE__);
    for(i = 0;i < NUM_SUBRACK;i++)
    {
        for(j = 0; j < NUM_CARD;j++)
        {
            IsAlarm = check_is_hw_alarm(DevCommuState[i][j].cur_alarm);
            if(IsAlarm == RET_SUCCESS)
            {
                type = DevCommuState[i][j].cur_type;
                port_num = DevCommuState[i][j].cur_port;
                printf("%s(): Line : %d  frame %d card %d type %d port %d cur_alarm %d time %s \n",\
                       __FUNCTION__, __LINE__,i,j,type,port_num,DevCommuState[i][j].cur_alarm,DevCommuState[i][j].alarm_time);
            }
        }
    }
    return ;
}
/*
   **************************************************************************************
 *  函数名称：show_total_opm_alarm
 *  函数描述：输出全部光功告警，调试使用
 *                ：
 *  入口参数：
 *  返回参数：
 *  作者       ：
 *  日期       ：2016-02-02
 *  修改日期：
 *  修改内容：
 *                ：
 **************************************************************************************
*/
void  MainWindow::show_total_opm_alarm()
{
    int i,j,cur_alarm_num;
    int frame, card, type, port_num;
    printf("%s(): Line : %d  list size %d \n",  __FUNCTION__, __LINE__,total_opm_alarm.OpmList.size());
    for( i = 0; i< total_opm_alarm.OpmList.size();i++)
    {
        cur_alarm_num = total_opm_alarm.OpmList[i].cur_alarm_num;
        frame = total_opm_alarm.OpmList[i].frame;
        card = total_opm_alarm.OpmList[i].card;
        type = total_opm_alarm.OpmList[i].type;
        port_num = total_opm_alarm.OpmList[i].port_num;
        printf("%s(): Line : %d  frame %d card %d type %d port_num %d \n",\
               __FUNCTION__, __LINE__,frame, card ,type,port_num);
        for(j = 0;j < total_opm_alarm.OpmList[i].alarmArray.size();j++)
        {
            if(total_opm_alarm.OpmList[i].alarmArray[j].lev != ALARM_NONE)
                printf(" come time %s \n",total_opm_alarm.OpmList[i].alarmArray[j].come_time);

        }
    }
    return ;
}
/*
   **************************************************************************************
 *  函数名称：show_olp_swtich_log
 *  函数描述：输出全部光功告警，调试使用
 *                ：
 *  入口参数：
 *  返回参数：
 *  作者       ：
 *  日期       ：2016-02-02
 *  修改日期：
 *  修改内容：
 *                ：
 **************************************************************************************
*/
void  MainWindow::show_olp_swtich_log()
{
    int i, frame, card ,type,port;
    printf("%s(): Line : %d  size %d log num %d \n",  \
           __FUNCTION__, __LINE__,OlpActionRecordBuf.list.size(),OlpActionRecordBuf.cur_total_record);
    for(i = 0; i < OlpActionRecordBuf.list.size();i++)
    {
        if(i < OlpActionRecordBuf.cur_total_record)
        {
            frame = OlpActionRecordBuf.list[i].frame;
            card = OlpActionRecordBuf.list[i].card;
            type = OlpActionRecordBuf.list[i].type;
            port = OlpActionRecordBuf.list[i].to_port;
            printf(" frame %d card %d type %d to port %d time %s \n", \
                   frame, card ,type,port,OlpActionRecordBuf.list[i].time);
        }
        else
        {
            break;
        }
    }
    return ;
}
/*
   **************************************************************************************
 *  函数名称：show_db_saved_alarm
 *  函数描述：将数据库保存的告警数据展现出来
 *                ：
 *  入口参数：
 *  返回参数：
 *  作者       ：
 *  日期       ：
 *  修改日期：2016-02-03
 *  修改内容：
 *                ：
 **************************************************************************************
*/
void MainWindow::show_db_saved_alarm()
{
    show_total_hw_alarm();
    show_total_opm_alarm();
    show_olp_swtich_log();
}
