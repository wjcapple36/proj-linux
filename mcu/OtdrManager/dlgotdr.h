#ifndef DLGOTDR_H
#define DLGOTDR_H

#include <QDialog>
#include <QModelIndex>
#include "qcurv.h"
#include "struct.h"
#include "mainwindow.h"

namespace Ui {
class QDlgOtdr;
}

//量程数目
#define RANGE_NUM       7

class QDlgOtdr : public QDialog
{
    Q_OBJECT
public:
    //初始化一些参数
    void Initial_osw_combox();
    //布局部件
    void LayoutOnDlg();
    //加载曲线数据，调试使用
    void LoadData();
    //创建事件列表
    void CreatEventListTable();
    //初始化事件列表
    void InitialEventList();
    //获取事件类型
    QString getEventType(int eventType);
    //创建测量结果列表
    void CreatMeasurResultList();
    //初始化测量结果列表
    void InitialResultList();
    //处理异常，用户决定是否退出
    void dealAbnormal(QString str);
    //显示otdr进度条
    int showOtdrProgressbar(int msec, unsigned int cmd, int sucess_notify = true);
    //获取机框 板卡编号
    void get_frame_card_info();
    //初始化板卡组合框
    int  initial_card_combBox(int frame_no);
    int  initial_port_comBox(int frame_no, int slot_no);
    //初始化量程列表QStringList
    void initial_rang_pulse_strlist();
    void initial_otdr_test_para();
    //创建模块关联关系列表
    void CreatConctTable();
    void CreatModulParaTable();//创建模块参数列表
    void initial_attribute(); //初始化属性对话框
    void initial_module_conect_list();//初始化关联关系表
    void initial_local_device_test_port();//初始化本地可测试端口
    void usr_timer_initial();
private slots:
    //处理从mainwindow来的数据
    void rcvDataFromParent(int cmd, int dataLen, _tagBufList * pListHead);
    void rcvParentMsg(int cmd, int reservd1, int reservd2);
    void usr_time_out();
    void set_measur_lamda();

public:
    //绘图区域
    QRect m_rectFigure;
    //相框，里面随绘图区域
    QRect m_rectFrame;
    //绘图类
    QCurv m_qCurv;
    //绘图控制类
    _tagDrawCtrl m_drawCtrl;
    //存放损耗段数组
    int m_sampl_num;
    unsigned short m_db_1000[MAX_PT_NUM];
    //像素点数组
    QPoint m_ptBuf[MAX_PT_NUM];
    //参考曲线
    _tagOtdrCurv m_referCurv;
    //测量参数
    _tagMeasurPara m_measurPara;
    //测量结果
    _tagMeasurResult m_measurResult;
    //事件点书目
    int m_eventNum;
    //事件点数组
    _tagEvent m_eventBuf[MAX_EVENT_NUM];
    //定义指向父窗口的指针
    MainWindow *m_pParent;
    //从机号,槽位号，设备类型
    _tagDlgAttribute dlgAttribute;

    //该信息如果MCU修改了，需要从MainWindow及时更新
    //板卡信息
    _tagSubrackCard m_subrackCard[NUM_SUBRACK];
    //机框组成信息
    _tagSubrackInfo m_subrackInfo;

    //测量参数
    //脉宽，每个量程对应的脉宽不相同
    QStringList strlist_pulse[RANGE_NUM];
    //量程
    QStringList strlist_range;
    //模块路由
    _tagRoutBuf rout_buf;
    _tagDevComm referOtdr; //关联的otdr
    _tagOtdrModulPara referOtdrPara; //关联模块
    tsk_OtdrManage *pReferOtdrTsk; //关联Otdr任务
    _tagUsrTimer usr_timer;
public:
    explicit QDlgOtdr(QWidget *parent = 0,int subrackNo = 0, \
                      int cardNo = 0,int devType = OTDR);
    ~QDlgOtdr();
protected:
    //绘图事件
    void paintEvent(QPaintEvent *event);
    //鼠标按下事件
    void mousePressEvent(QMouseEvent *event) ;
    //窗口发生变化
    void resizeEvent(QResizeEvent *);
private slots:
    //退出按钮
    void on_pushBtnExit_clicked();
    //横向放大
    void on_pushBtnHZin_clicked();
    //横向缩小
    void on_pushBtnHZout_clicked();
    //纵向放大
    void on_pushBtnVZin_clicked();
    //纵向缩小
    void on_pushBtnVZout_clicked();
    //恢复原迹线
    void on_pushBtnResotre_clicked();
    //锁定标杆
    void on_pushBtnLockA_clicked();
    //移动标杆
    void on_pushBtnLockB_clicked();
    //开始测量
    void on_pushBtnMeasur_clicked();
    //事件列表上点击某一行
    void on_listEvent_pressed(const QModelIndex &index);


    void on_cobBoxFrameNo_currentIndexChanged(const QString &arg1);

    void on_cobBoxSlotNo_currentIndexChanged(const QString &arg1);

    void on_cobBoxRange_currentIndexChanged(const QString &arg1);

    void on_pushBtnGetSInfo_clicked();

    void on_pushBtnGetHInfo_clicked();

    void on_pushBtnGetModulPara_clicked();

    void on_pushBtnGetConct_clicked();

    void on_cobBoxPortNo_currentIndexChanged(const QString &arg1);

private:
    Ui::QDlgOtdr *ui;
};

#endif // DLGOTDR_H
