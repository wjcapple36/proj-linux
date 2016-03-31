#ifndef QMANAGELINKEDBUF_H
#define QMANAGELINKEDBUF_H
#include "struct.h"
#include <QMutex>
//定义链接缓冲区返回失败码
//buf
#define       CODE_LINK_BUF_ID_ERROR   -1
#define       CODE_LINK_BUF_SPACE_LACK -2
class QManageLinkedBuf
{
public:
    QManageLinkedBuf();
    int allcoBuf(_tagBufList **buf, int bufNum);
    int freeBuf(_tagBufList *buf);
    int attachBuf(_tagBufList buf[], int bufNum,int bufSize);
    int disattchBuf();
public:
    int freeBufNum;


private:
    _tagBufList *head;
    QMutex mutexObj;


};

#endif // QMANAGELINKEDBUF_H
