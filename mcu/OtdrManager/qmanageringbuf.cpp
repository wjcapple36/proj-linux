/*
 **************************************************************************************
 *  文件描述：char 环形缓冲区及其操作,qt类，如果在win32应用，须重写同步
 *  文件描述：用户可以直接将要管理段缓冲区挂载到本管理类，然后调用本管理类的管理函数
 *  文件描述：即可
 *  文件名字：qringbuf.c
 *  创建者　：wjc
 *  创建日期：2015-04-16
 *  当前版本：
 *
  ***** 修改记录 *****
 *  修改者　：
 *  修改日期：
 *  备注       ：
 **************************************************************************************
*/
#include "qmanageringbuf.h"
#include <string.h>

QManageRingBuf::QManageRingBuf()
{
    head = 0;
    tail = 0;
    bufSize = 0;
    inventory = 0;
    buf = NULL;
}
/*
   **************************************************************************************
 *  函数名称：attachBufManaged
 *  函数描述：挂载要管理段缓冲区
 *  入口参数：
 *  返回参数：如果已经挂载了buff，那么返回错误
 *  作者       ：
 *  日期       ：
 **************************************************************************************
*/
int QManageRingBuf::attachBufManaged(char *bufManaged, int bytes)
{
    if(NULL != buf)
        return -1;
    //访问前要上锁
    mutexObj.lock();
    buf = bufManaged;
    bufSize = bytes;
    head = 0;
    tail = 0;
    inventory = 0;
    freeSpace = bufSize;
    //访问结束解锁
    mutexObj.unlock();

    return bytes;
}

/*
   **************************************************************************************
 *  函数名称：inputRingBuf
 *  函数描述：如果缓冲区可写入空间大于或者等于要写入bytes,那么返回写入
 *                     数据的bytes,否则，返回可写入空间容量
 *  入口参数：
 *  返回参数：
 *  作者       ：
 *  日期       ：
 **************************************************************************************
*/
int QManageRingBuf::inputRingBuf(char * srcBuf, int bytes)
{
    int  returnValue;
    int firstCpy, secCpy;
    mutexObj.lock();   
    returnValue =freeSpace;


    if(bytes  <= freeSpace)
    {
        //tail < head if 肯定满足，tail > head段情况下才会出现if为真情况
        //首先存储空间够用，尾巴到数组最后空间变小
        if((bytes  + tail) > bufSize )
        {
            firstCpy = bufSize - tail;
            memcpy(&buf[tail], srcBuf, firstCpy);
            secCpy = bytes - firstCpy;
            memcpy(buf, &srcBuf[firstCpy],secCpy);
        }
        else
        {
            memcpy(&buf[tail], srcBuf, bytes);
        }
        tail = (tail + bytes) % bufSize;
        inventory += bytes;
        //本不应出错
        inventory = inventory % bufSize;
        freeSpace = bufSize - inventory;
        returnValue = bytes;
    }
   mutexObj.unlock();
    return returnValue;
}
/*
   **************************************************************************************
 *  函数名称：outputRingBuf
 *  函数描述：读环形缓冲区，如果要读段字节数大于已存在已存在字节数，返回错误
 *
 *  入口参数：
 *  返回参数：
 *  作者       ：
 *  日期       ：
 **************************************************************************************
*/
int QManageRingBuf::outputRingBuf(char * dstBuf, int bytes)
{
    int returnValue;
    int firstCpy, secCpy;
    mutexObj.lock();
    returnValue = inventory;


    if(bytes  <= inventory)
    {
        if((head + bytes) > bufSize)
        {
            firstCpy = bufSize - head;
            memcpy(dstBuf, &buf[head], firstCpy);
            secCpy = bytes - firstCpy;
            memcpy(&dstBuf[firstCpy], buf, secCpy);

        }
        else
        {
            memcpy(dstBuf, &buf[head],  bytes);
        }
        head = (head + bytes) % bufSize;
        inventory -= bytes;
        //本不应出错
        inventory = inventory % bufSize;
        freeSpace = bufSize - inventory;
        returnValue = bytes;
    }
    mutexObj.unlock();
    return returnValue;
}
//获取缓冲区容量
int QManageRingBuf::getBufSize()
{
    return bufSize;
}
//获取缓冲区空闲空间
int QManageRingBuf::getFreesSpace()
{
    return freeSpace;
}
//剥离缓冲区
int QManageRingBuf::disattachBufManaged()
{
    buf = NULL;
    head = 0;
    tail = 0;
    freeSpace = 0;
    inventory = 0;
    bufSize = 0;
    return bufSize;
}
//获取缓冲区内的数据量
int QManageRingBuf::getInverntory()
{
    return inventory;
}
//重置空间
int QManageRingBuf::resetBuf()
{
    return freeBuf(inventory);

}
//释放存储空间
int QManageRingBuf::freeBuf(int bytes)
{
    int returnValue;
    int firstClear, secClear;
    mutexObj.lock();
    if(bytes > inventory)
        bytes = inventory;
    returnValue = bytes;

    if(bytes  <= inventory)
    {
        if((head + bytes) > bufSize)
        {
            firstClear = bufSize - head;
            memset(&buf[head], 0, firstClear);
            secClear = bytes - firstClear;
            memset( buf, 0, secClear);
        }
        else
        {
            memset(&buf[head], 0,  bytes);
        }
        head = (head + bytes) % bufSize;
        inventory -= bytes;
        //本不应出错
        inventory = inventory % bufSize;
        freeSpace = bufSize - inventory;
        returnValue = bytes;
    }
    mutexObj.unlock();
    return returnValue;
}


