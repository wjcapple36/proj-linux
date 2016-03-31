#ifndef QMANAGERINGBUF_H
#define QMANAGERINGBUF_H
#include <QMutex>
class QManageRingBuf
{
public:
    QManageRingBuf();
    //入栈
    int inputRingBuf(char * srcBuf, int bytes);
    //出栈
    int outputRingBuf(char * dstBuf, int bytes);
    //获取缓冲区大小
    int getBufSize();
    //获取缓冲区内数据量
    int getInverntory();
     //获取空闲缓冲区
    int getFreesSpace();
    //添加须管理段缓冲区
    int attachBufManaged(char *, int bytes);
    //卸载管理的缓冲区
    int disattachBufManaged();
    //清空缓冲区
    int resetBuf();
    int freeBuf(int bytes);
public:
    int head;           //头
    int tail;              //尾
    int bufSize;       //缓冲区大小
    int inventory;      //存货量，缓冲区内数据量
    int freeSpace;
    char *buf;
    QMutex mutexObj;
};
#endif // QMANAGERINGBUF_H
