#ifndef SMS_PROTOCOL_H
#define SMS_PROTOCOL_H
#include "struct.h"


//定义一些常量，协议相关
#define GSM_UCS2    8
#define SMS_SIZE_ST       70   // 70字节以上为长短信，拆分发送
#define SMS_TYPE_SHORT          0   //短信内容少于70个字符
#define SMS_TYPE_LONG            1  //短信内容大于70个字符
#define SMS_HEAD_LEN               29 //短信帧长度
#define SMS_SPLIT_LEN               67//长短信分割字符
struct _SMParam
{
    char SCA[16];			// 短消息服务中心号码(SMSC地址),一般使用SIM自带的
    char TPA[16];			// 目标号码或回复号码(TP-DA或TP-RA)
    char TP_PID;			// 用户信息协议标识(TP-PID)
    char TP_DCS;			// 用户信息编码方式(TP-DCS)
    char TP_SCTS[16];		// 服务时间戳字符串(TP_SCTS), 接收时用到
    char TP_UD[GSM_TEXT_LEN];		// 原始用户信息(编码前或解码后的TP-UD)
    short index;			// 短消息序号，在读取时用到
};
//使用SIM卡保存的短消息中心号码，本人手机号码,手机号码长度为11位
//短消息头长度为29






enum {IndexCentrAddr = 0, IndexSmsType, IndexDstPhoneLen, IndexSrcPhoneAddr\
     ,IndexDstPhoenType};

int Bytes2String(const unsigned char* pSrc, char* pDst, int nSrcLength);
int String2Bytes(const char* pSrc, unsigned char* pDst, int nSrcLength);
int InvertNumbers(const char* pSrc, char* pDst, int nSrcLength);
int EncodePdu(const _SMParam* pSrc, char* pDst,bool bIsLongSMS,\
              int iTotalPartitionSMSNum, int iSMSSequenceNumber,unsigned char cSMSSeiral);
//设置pdu packet 帧头，编码格式，协议有效期,电话
int set_pdu_packet_head(const _SMParam* pSmsPara,char pdu_packet[],  int sms_type = SMS_TYPE_SHORT);
int set_pdu_packet_len(char pdu_packet[], int packet_len,int offset);//设置pdu packet 包的长度
//设置长短信的拆分段数，当前段，长短信的条目
int set_long_msg_info(char pdu_packet[],unsigned char total_sec_num, \
                      unsigned char cur_num,unsigned char cSMSSeiral,int offset);
int filled_pdu_packet_context(unsigned short int text_buf[], char pdu_packet[], int offset , int text_count);
int get_pdu_packet_cmd_len(char pdu_packet[]);


#endif // SMS_PROTOCOL_H
