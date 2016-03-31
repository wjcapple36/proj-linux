#include <QDebug>

#include "tsk_datadispatch.h"
#include "math.h"
#include "protocol/tmsxx.h"

tsk_dataDispatch::tsk_dataDispatch(QObject *parent) :
    QThread(parent)
{
    m_pParent = (MainWindow *)parent;
    stopped = false;
}
/*
   **************************************************************************************
 *  函数名称：
 *  函数描述：
 *  入口参数：
 *  返回参数：
 *  作者       ：
 *  日期       ：
 **************************************************************************************
*/
void tsk_dataDispatch::allocaLinkedBuf(_tagBufList **pBufHead, int bufNum, int maxWait_ms)
{  
    int count;
    count = maxWait_ms / 100;
    *pBufHead = NULL;
    while(bufNum > m_pParent->LinkedBufManager.allcoBuf(pBufHead, bufNum))
    {
        count++;
        usleep(100);
        //等带最长2s
        if(count > 10)
            break;
    }
    return;
}
/*
   **************************************************************************************
 *  函数名称：filledLinkedBuf
 *  函数描述：
 *  入口参数：
 *  返回参数：
 *  作者       ：
 *  日期       ：
 **************************************************************************************
*/
int  tsk_dataDispatch:: filledLinkedBuf(_tagBufList *pBufHead, char bufHead[])
{
    _tagBufList  *pBufIndex;
    _tagDataHead *pDataHead;
    bool filled;
    int cpyedBytes;
    pDataHead = (_tagDataHead *)bufHead;
    cpyedBytes = 0;
    pBufIndex = pBufHead;
    //第一个缓冲区拷贝单独操作
    //先拷贝数据头
    filled = false;
    memcpy(pBufIndex->buf,  bufHead, sizeof(_tagDataHead));
    cpyedBytes += sizeof(_tagDataHead);
    //可以一次性拷贝完
    if(pBufIndex->bufSize > pDataHead->dataLen)
    {

        m_pParent->RingBufManager.outputRingBuf(pBufIndex->buf +\
                                                sizeof(_tagDataHead),pDataHead->dataLen - cpyedBytes);
        cpyedBytes = pDataHead->dataLen;
        filled = true;
    }
    else //需要拷贝第二次
    {
        //仍然需要先拷贝头
        m_pParent->RingBufManager.outputRingBuf(pBufIndex->buf +\
                                                sizeof(_tagDataHead),pBufIndex->bufSize - cpyedBytes);
        cpyedBytes += ( pBufIndex->bufSize - cpyedBytes);
        pBufIndex = pBufIndex->next;
    }
    //拷贝剩下的数据
    while(cpyedBytes < pDataHead->dataLen)
    {
        //当前缓冲区够用
        if(pBufIndex->bufSize > (pDataHead->dataLen - cpyedBytes))
        {
            m_pParent->RingBufManager.outputRingBuf(pBufIndex->buf,\
                                                    pDataHead->dataLen - cpyedBytes);
            cpyedBytes = pDataHead->dataLen;
            filled = true;
            break;
        }
        else //
        {
            m_pParent->RingBufManager.outputRingBuf(pBufIndex->buf,\
                                                    pBufIndex->bufSize );
            cpyedBytes += pBufIndex->bufSize;
            pBufIndex = pBufIndex->next;
        }
        if(NULL == pBufIndex)
        {
            //一般不会出错，但以防万一
            m_pParent->LinkedBufManager.freeBuf(pBufHead);
            break;
        }
    }
    return pDataHead->dataLen;
}

/*
   **************************************************************************************
 *  函数名称：run
 *  函数描述：数据处理段核心线程
 *  入口参数：
 *  返回参数：
 *  作者       ：
 *  日期       ：
 **************************************************************************************
*/
void tsk_dataDispatch::run()
{
    _tagDataHead *pDataHead;
    _tagBufList *pBufHead;
    char buf[8];
    char msg_short[SHORT_MSG_LEN];
    int inverntory;
    int bufNum;
    bool rcv_data;
    rcv_data = false;
    glink_addr send_addr;
    send_addr.dst  = ADDR_NET_MANAGER;
    send_addr.src = ADDR_MCU;
    while (!stopped)
    {
        //阻塞，直到获取信号
        rcv_data = false;
        while(!rcv_data && !stopped)
        {
            //如果读到错误的命令，可以将数据全部读出
            inverntory = m_pParent->RingBufManager.getInverntory();
            if(inverntory > 0)
            {
                m_pParent->semRingBuf.tryAcquire(1);
                rcv_data = true;
            }
            //否则的话等待信号阻塞
            else
            {
                rcv_data = m_pParent->semRingBuf.tryAcquire(1,500);
            }
        }
        //如果被外部函数stop，就直接跳出
        if(stopped)
            break;

        //先读取一个数据头部
        //inverntory = m_pParent->RingBufManager.getInverntory();
        inverntory = m_pParent->RingBufManager.getInverntory();
        m_pParent->RingBufManager.outputRingBuf(buf, sizeof(_tagDataHead));
        pDataHead = (_tagDataHead *)buf;
        //检查数据命令码是否正确
        //qDebug("data dispatch datalen %d  Inverntory %d",\
        //     pDataHead->dataLen ,inverntory);
        if(pDataHead->dataLen <= inverntory && inverntory > 0)
        {
            //根据数据头部计算需要段申请段缓冲区
            bufNum = ceil(pDataHead->dataLen / ((float)BUF_LIST_SIZE));
            allocaLinkedBuf(&pBufHead,  bufNum,  1000);
            //分配成功
            if( NULL != pBufHead)
            {
                filledLinkedBuf(pBufHead, buf);
                qDebug("申请到linked buf %d 个", bufNum);
                m_pParent->processRcvData(pDataHead->cmd, pDataHead->dataLen, pBufHead);
            }
            else
            {
                m_pParent->RingBufManager.freeBuf(pDataHead->dataLen - sizeof(_tagDataHead));
                memset(msg_short, 0, sizeof(msg_short));
                sprintf(msg_short, "tsk_dataDispatch alloca buf timeout cmd %x len %d",pDataHead->cmd,
                        pDataHead->dataLen );
                //tms_Trace(&send_addr, msg_short, strlen(msg_short),1);
                qDebug("%s",msg_short);

            }
        }
        else
        {
            //如果读错，清空所有的数据
            m_pParent->RingBufManager.resetBuf();
            memset(msg_short, 0, sizeof(msg_short));
            sprintf(msg_short, "data dispatch output erro cmd %x len %d",pDataHead->cmd,
                    pDataHead->dataLen );
            //tms_Trace(&send_addr, msg_short, strlen(msg_short),1);
            qDebug("%s",msg_short);
        }
    }

    stopped = false;
}

void tsk_dataDispatch::stop()
{
    stopped = true;
}
