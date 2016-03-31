
/*
 **************************************************************************************
 *  文件描述：发送短信的接口
 *
 *  文件名字：sms_protocol.cpp
 *  创建者　：wjc
 *  创建日期：2015-09-28
 *  当前版本：v1.0
 *
  ***** 修改记录 *****
 *  修改者　：
 *  修改日期：
 *  备注       ：
 **************************************************************************************
*/

#include "sms_protocol.h"
#include <netinet/in.h>

/*
   **************************************************************************************
 *  函数名称：Bytes2String
 *  函数描述：{0xC8, 0x32, 0x9B, 0xFD, 0x0E, 0x01} --> "C8329BFD0E01"
 *  入口参数：pSrc - 源数据指针,目标地址指针，源字符传长度
 *  返回参数：目标字符串字节数
 *  作者       ：
 *  日期       ：
 **************************************************************************************
*/
int Bytes2String(const unsigned char* pSrc, char* pDst, int nSrcLength)
{
    const char tab[]="0123456789ABCDEF";	// 0x0-0xf的字符查找表

    for (int i = 0; i < nSrcLength; i++)
    {
        *pDst++ = tab[*pSrc >> 4];		// 输出高4位
        *pDst++ = tab[*pSrc & 0x0f];	// 输出低4位
        pSrc++;
    }

    // 输出字符串加个结束符
    *pDst = '\0';

    // 返回目标字符串长度
    return (nSrcLength * 2);
}
/*
   **************************************************************************************
 *  函数名称：String2Bytes
 *  函数描述：字符串转换为字节数据
 *  函数描述："C8329BFD0E01" --> {0xC8, 0x32, 0x9B, 0xFD, 0x0E, 0x01}
 *  入口参数：源，目的字符指针，源字符串长度
 *  返回参数：目标数据长度
 *  作者       ：
 *  日期       ：
 **************************************************************************************
*/
int String2Bytes(const char* pSrc, unsigned char* pDst, int nSrcLength)
{
    for (int i = 0; i < nSrcLength; i += 2)
    {
        // 输出高4位
        if ((*pSrc >= '0') && (*pSrc <= '9'))
        {
            *pDst = (*pSrc - '0') << 4;
        }
        else
        {
            *pDst = (*pSrc - 'A' + 10) << 4;
        }

        pSrc++;

        // 输出低4位
        if ((*pSrc>='0') && (*pSrc<='9'))
        {
            *pDst |= *pSrc - '0';
        }
        else
        {
            *pDst |= *pSrc - 'A' + 10;
        }

        pSrc++;
        pDst++;
    }

    // 返回目标数据长度
    return (nSrcLength / 2);

}
/*
   **************************************************************************************
 *  函数名称：InvertNumbers
 *  函数描述："8613851872468" --> "683158812764F8"
 *  函数描述：正常顺序的字符串转换为两两颠倒的字符串，若长度为奇数，
 *  函数描述：补'F'凑成偶数  pDst - 目标字符串指针
 *  入口参数：pSrc - 源字符串指针，目标字符传指针
 *  返回参数：目标字符串长度
 *  作者       ：wjc
 *  日期       ：2015-09-25
 **************************************************************************************
*/
int InvertNumbers(const char* pSrc, char* pDst, int nSrcLength)
{
    int nDstLength;		// 目标字符串长度
    char ch;			// 用于保存一个字符

    // 复制串长度
    nDstLength = nSrcLength;

    // 两两颠倒
    for(int i=0; i<nSrcLength;i+=2)
    {
        ch = *pSrc++;		// 保存先出现的字符
        *pDst++ = *pSrc++;	// 复制后出现的字符
        *pDst++ = ch;		// 复制先出现的字符
    }

    // 源串长度是奇数吗？
    if(nSrcLength & 1)
    {
        *(pDst-2) = 'F';	// 补'F'
        nDstLength++;		// 目标串长度加1
    }

    // 输出字符串加个结束符
    *pDst = '\0';

    // 返回目标字符串长度
    return nDstLength;

}
/*
   **************************************************************************************
 *  函数名称：EncodePdu
 *  函数描述：将短消息编码
 *  入口参数：
 *  返回参数：
 *  作者       ：wjc
 *  日期       ：2015-09-25
 **************************************************************************************
*/
int EncodePdu(const _SMParam* pSrc, char* pDst,bool bIsLongSMS,\
              int iTotalPartitionSMSNum, int iSMSSequenceNumber,unsigned char cSMSSeiral)
{
    int iTemp = 0;
    int nLength = 0;			// 内部用的串长度
    int nDstLength = 0;			// 目标PDU串长度
    unsigned char buf[512];	// 内部用的缓冲区
    memset(buf,0,512);
    // SMSC地址信息段
    buf[0] = 0;  //在这里为0，表示使用存储在SIM 卡中的短信息中心地址；
    nDstLength = Bytes2String(buf, pDst, 1);		// 转换1个字节到目标PDU串

    // TPA段基本参数、目标地址等
    nLength = strlen(pSrc->TPA);	// TP-DA地址字符串的长度
    if (bIsLongSMS)
    {
        buf[0] = 0x51;  //长短信
    }
    else
    {
        buf[0] = 0x11;	// 是发送短信(TP-MTI=01)，TP-VP用相对格式(TP-VPF=10)
    }
    buf[1] = 0;	// TP-MR=0
    buf[2] = (unsigned char)nLength;			// 目标地址数字个数(TP-DA地址字符串真实长度)
    buf[3] = 0x91;					// 固定: 用国际格式号码
    nDstLength += Bytes2String(buf, &pDst[nDstLength], 4);		// 转换4个字节到目标PDU串
    nDstLength += InvertNumbers(pSrc->TPA, &pDst[nDstLength], nLength);	// 转换TP-DA到目标PDU串

    // TPDU段协议标识、编码方式、用户信息等
    nLength = strlen(pSrc->TP_UD);	// 用户信息字符串的长度
    buf[0] = pSrc->TP_PID;			// 协议标识(TP-PID)
    buf[1] = pSrc->TP_DCS;			// 用户信息编码方式(TP-DCS)
    buf[2] = 0;						// 有效期(TP-VP)为5分钟
    if(pSrc->TP_DCS == GSM_UCS2)
    {
        // UCS2编码方式
        if (bIsLongSMS)
        {
            buf[4] = 0x05;
            buf[5] = 0x00;
            buf[6] = 0x03;
            buf[7] = cSMSSeiral; //0x39
            buf[8] = (unsigned char)iTotalPartitionSMSNum;
            buf[9] = (unsigned char)iSMSSequenceNumber;
            //iTemp = EncodeUcs2(pSrc->TP_UD, &buf[10], nLength);	// 转换TP-DA到目标PDU串
            nLength = iTemp + 10;		// nLength等于该段数据长度
            buf[3] =  (unsigned char)(iTemp + 6);
            nDstLength += Bytes2String(buf, &pDst[nDstLength], nLength);		// 转换该段数据到目标PDU串
        }
        else
        {
            //buf[3] = EncodeUcs2(pSrc->TP_UD, &buf[4], nLength);	// 转换TP-DA到目标PDU串
            nLength = buf[3] + 4;		// nLength等于该段数据长度
            nDstLength += Bytes2String(buf, &pDst[nDstLength], nLength);		// 转换该段数据到目标PDU串
        }
    }

    // 返回目标字符串长度
    return nDstLength;
}
/*
   **************************************************************************************
 *  函数名称：set_pdu_packet_head
 *  函数描述：设置pdu packet 帧的前面9个参数，适用环境：使用SIM卡短消息中心号
 *  函数描述：发送方号码为SIM卡本机号码，接收短信手机号前不加86
 *  入口参数：pdu_packet
 *  返回参数：头帧长度
 *  作者       ：
 *  日期       ：
 **************************************************************************************
*/
//1 短消息中心地址
//2 SMS_SUBMIT标示长消息，还是短消息；3 发送方地址消息；
//4接收号码长度；5接收方号码类型 0x91；6接收方号码(13B)；
//7 消息协议标示；8消息编码类型；9 用户数据长度
//头长度一共 29
int set_pdu_packet_head(const _SMParam *pSmsPara, char pdu_packet[], int sms_type )
{
    int nLength = 0;			// 内部用的串长度
    int nDstLength = 0;			// 目标PDU串长度
    unsigned char buf[8];	// 内部用的缓冲区
    char * pDst;
    const _SMParam* pSrc;
    pSrc = pSmsPara;
    pDst = pdu_packet;

    bzero(buf, sizeof(buf));

    //1 短消息中心地址0，表示采用SIM卡中保存的短消息中心号码
    buf[0] = 0;
    nDstLength = Bytes2String(buf, pDst, 1);		//转换00个字节到目标PDU串,pdu_packet:1


    if (sms_type == SMS_TYPE_LONG)//pdu_packet:2
    {
        buf[0] = 0x51;  //长短信
    }
    else
    {
        buf[0] = 0x11;	// 是发送短信(TP-MTI=01)，TP-VP用相对格式(TP-VPF=10)
    }
    buf[1] = 0;	// TP-MR=0 //pdu_packet:3
    // TPA段基本参数、目标地址等
    nLength = strlen(pSrc->TPA);	// TP-DA地址字符串的长度
    buf[2] = (unsigned char)nLength;// 目标地址数字个数(TP-DA地址字符串真实长度)
    buf[3] = 0x91;					// 固定: 用国际格式号码
    nDstLength += Bytes2String(buf, &pDst[nDstLength], 4);		// 转换4个字节到目标PDU串
    nDstLength += InvertNumbers(pSrc->TPA, &pDst[nDstLength], nLength);	// 转换TP-DA到目标PDU串

    // TPDU段协议标识、编码方式、用户信息等
    buf[0] = pSrc->TP_PID;			// 协议标识(TP-PID)
    buf[1] = pSrc->TP_DCS;			// 用户信息编码方式(TP-DCS)
    buf[2] = 0;						// 有效期(TP-VP)为5分钟
    nLength = 3;
    nDstLength += Bytes2String(buf, &pDst[nDstLength], nLength);
    qDebug("data head len %d ",nDstLength);
    return nDstLength;
}
/*
   **************************************************************************************
 *  函数名称：set_pdu_packet_len
 *  函数描述：在pdu包中设置短信的长度
 *  入口参数：pdu packet  pdu len  偏移量
 *  返回参数：
 *  作者       ：
 *  日期       ：
 **************************************************************************************
*/
int set_pdu_packet_len(char pdu_packet[], int packet_len, int offset)
{   
    unsigned char buf[2];
    int count;
    buf[0] = (unsigned char )(packet_len);
    count = Bytes2String(buf, &pdu_packet[offset], 1);
    return 2;
}
/*
   **************************************************************************************
 *  函数名称：set_long_msg_info
 *  函数描述：长短信的时候需要设置长短信的分段数，当前段号，序列号
 *  入口参数：
 *  返回参数：
 *  作者       ：
 *  日期       ：
 **************************************************************************************
*/
int set_long_msg_info(char pdu_packet[], unsigned char total_sec_num, \
                      unsigned char cur_num, unsigned char cSMSSeiral, int offset)
{
    char buf[16];
    int i, bytes;
    i = 0;
    buf[i++] = 0x05;
    buf[i++]  = 0x00;
    buf[i++]  = 0x03;
    buf[i++]  = cSMSSeiral; //0x39
    buf[i++]  = (unsigned char)total_sec_num;
    buf[i++]  = (unsigned char)cur_num;
    bytes = Bytes2String((const unsigned char *)buf,pdu_packet + offset, i);
    return bytes;

}
/*
   **************************************************************************************
 *  函数名称：filled_pdu_packet_context
 *  函数描述：短信内容，要转换成大端，然后在转换成转换成字符，每条短信最大字符为70
 *  函数描述：0x4F60(你)-----0x604F------"604F'
 *  入口参数：unicode, pdu_packet ,context 的偏移量， 短信文字个数
 *  返回参数：返回ASIIC码个数
 *  作者       ：
 *  日期       ：
 **************************************************************************************
*/
int filled_pdu_packet_context(unsigned short int text_buf[], char pdu_packet[],int offset ,int text_count)
{
    unsigned short int  text[SMS_SIZE_ST]; //每个短信条数不会超过70个字符
    int i, count;
    int bytes;
    if(text_count > SMS_SIZE_ST)
        count = SMS_SIZE_ST; //如果短信字符超过70，只转换70个
    else
        count = text_count;

    for(i = 0; i < count;i++)
    {
        text[i] = htons(text_buf[i]);
    }

    bytes = Bytes2String((unsigned const char *) text, pdu_packet + offset, count*2);
    return bytes;

}
/*
   **************************************************************************************
 *  函数名称：get_pdu_packet_cmd_len
 *  函数描述：短信发送之前，需要用AT+CMGS=%d\r 设置短信长度
 *  入口参数：pdu_packet
 *  返回参数：返回AT+CMGS=%d\r 长度值
 *  作者       ：
 *  日期       ：
 **************************************************************************************
*/
int get_pdu_packet_cmd_len(char pdu_packet[])
{
    unsigned char  smsc_len;
    int cmd_len;
    String2Bytes(pdu_packet, &smsc_len, 2);
    cmd_len =  strlen(pdu_packet) / 2 - smsc_len - 1;
    return cmd_len;
}
