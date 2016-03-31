#include "qmanagelinkedbuf.h"
#include "string.h"

QManageLinkedBuf::QManageLinkedBuf()
{
    head = NULL;
}
/*
   **************************************************************************************
 *  函数名称：allcoBuf
 *  函数描述：申请分配缓冲区
 *  入口参数：buf,返回可用缓冲区首地址
 *  返回参数：
 *  作者       ：
 *  日期       ：
 **************************************************************************************
*/
int QManageLinkedBuf::allcoBuf(_tagBufList **buf, int bufNum )
{
    int returnValue, i;
    returnValue = 0;
    i = 0;
    mutexObj.lock();
    *buf = NULL;
    if((freeBufNum > 0)&&(freeBufNum >= bufNum))
    {
        *buf = head;
        for( i = 0; i < bufNum;i++)
        {
            head = head->next;
        }       
        //被分配走的最后一个buf的next指向NULL
        if(NULL != head)
        {
            head->pre->next = NULL;
            head->pre = NULL;
        }
        freeBufNum -= bufNum;        
        returnValue = bufNum;
    }
    else
    {
        returnValue = CODE_LINK_BUF_SPACE_LACK;
    }
    mutexObj.unlock();
    return returnValue;
}
/*
   **************************************************************************************
 *  函数名称：freeBuf
 *  函数描述：回收缓冲区
 *  入口参数：buf
 *  返回参数：返回释放缓冲区段书目，失败，则返回-1
 *  作者       ：
 *  日期       ：
 **************************************************************************************
*/
int QManageLinkedBuf::freeBuf(_tagBufList *buf )
{
    int returnValue, count;
    _tagBufList *pBufIndex;
    mutexObj.lock();
    returnValue = 0;
    count = 0;
    pBufIndex = buf;
    while( NULL != pBufIndex && NULL != pBufIndex->next)
    {
        if(BUF_LINST_ID != pBufIndex->bufListID)
        {
            returnValue = CODE_LINK_BUF_ID_ERROR;
            count = 0;
            break;
        }
        else
        {
            memset(pBufIndex->buf, 0, sizeof(pBufIndex->buf));
            pBufIndex = pBufIndex->next;
            count++;
        }
    }

    //将要释放的链表加入
    if(CODE_LINK_BUF_ID_ERROR != returnValue )
    {
        pBufIndex->next = head;
        head->pre = pBufIndex;
        head = buf;
        freeBufNum += (count + 1);
        returnValue = (count + 1);

    }
    mutexObj.unlock();
    return returnValue;
}
/*
   **************************************************************************************
 *  函数名称：attachBuf
 *  函数描述：将已存在段buflist挂载到管理类上
 *  入口参数：buf[],存放_tagBufList的数组，数组大小，以及每个_tagBufList buf
 *                      大小
 *  返回参数：返回buf,失败，就返回-1
 *  作者       ：
 *  日期       ：
 **************************************************************************************
*/
int QManageLinkedBuf::attachBuf(_tagBufList buf[], int bufNum,int bufSize)
{
    int returnValue, i;
    returnValue = 0;
    mutexObj.lock();
    if(NULL == head)
    {
        head = &buf[0];
        i = 0;
        buf[i].pre = NULL;
        buf[i].next = &buf[i + 1];
        buf[i].bufSize = bufSize;
        buf[i].bufListID = BUF_LINST_ID;
        memset(buf[i].buf, 0, sizeof(buf[i].buf));
        for( i = 1; i < (bufNum - 1);i++)
        {
            buf[i].pre = &buf[i - 1];
            buf[i].next = &buf[i + 1];
            buf[i].bufSize = bufSize;
            buf[i].bufListID = BUF_LINST_ID;
            memset(buf[i].buf, 0, sizeof(buf[i].buf));
        }
        buf[i].pre = &buf[i - 1];
        buf[i].next = NULL;
        buf[i].bufSize = bufSize;
        buf[i].bufListID = BUF_LINST_ID;
        memset(buf[i].buf, 0, sizeof(buf[i].buf));
        freeBufNum = bufNum;
        returnValue = bufNum;
    }
    else
    {
        returnValue = -1;
    }
    mutexObj.unlock();
    return returnValue;
}
