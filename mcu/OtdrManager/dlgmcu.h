#ifndef DLGMCU_H
#define DLGMCU_H


#include "struct.h"
#include "mainwindow.h"
#include <QDialog>
#include <QModelIndex>
#include <QComboBox>

namespace Ui {
class QDlgMcu;
}

class QDlgMcu : public QDialog
{
    Q_OBJECT
public:
    void LayoutOnDlg();
    void CreatSubrackTable();
    void UpdateSubrackState();
    void InitialSubrackList(int sel_frame = -1);
    void InitialSubrackAndCadInfo();
    void InitialCardTypeCombox();
    void InitialSubrackCombox();
    void InitialSelectSubrackCardType(int subrackNo = 0);
    void GetCurrentTime(char timeBuf[]);
    //处理异常
    void dealAbnormal(QString str);
    //检查设备类型
    int check_dev_type(int fram_no, int dev_type[], int port[]);
    void save_config();
    void initial_dev_port_num(int slot_index, int dev_type,int port_num = -1);
public:
    //机框信息
    _tagSubrackInfo m_subrackInfo;
    //板卡信息
    _tagSubrackCard m_subrackCard[NUM_SUBRACK];
    //comb列表框中设备类型部分
    QStringList m_strlstFuEqType;
    //comb设备列表框中全部的
    QStringList m_strlstAllEqType;
    QComboBox *m_pComBoxSlot[NUM_CARD];
    QComboBox *m_pComBoxSlotPort[NUM_CARD];
    //指向父窗口的类
    MainWindow * m_pParent;
    //从机号,槽位号，设备类型
    _tagDlgAttribute dlgAttribute;
    //是否有修改配置
    bool is_config_modfy;

private slots:

    
public:
    explicit QDlgMcu(QWidget *parent = 0, int frameNo = 0, \
                     int cardNo = 9, int deviceType = MCU);
    ~QDlgMcu();
    
private slots:
    void on_pushBtnExit_clicked();

    void on_listFrame_clicked(const QModelIndex &index);

    void on_pushBtnAddFrame_clicked();

    void on_pushBtnDelFrame_clicked();

    void on_pushBtnShowFram_clicked();

    void on_pushBtnModifyFram_clicked();

    void on_pushBtnSaveFram_clicked();

    void on_pushBtnViewCurCfg_clicked();

    void on_comboxSlot_1_currentIndexChanged(const QString &arg1);

    void on_comboxSlot_2_currentIndexChanged(const QString &arg1);

    void on_comboxSlot_3_currentIndexChanged(const QString &arg1);

    void on_comboxSlot_4_currentIndexChanged(const QString &arg1);

    void on_comboxSlot_5_currentIndexChanged(const QString &arg1);

    void on_comboxSlot_6_currentIndexChanged(const QString &arg1);

    void on_comboxSlot_7_currentIndexChanged(const QString &arg1);

    void on_comboxSlot_8_currentIndexChanged(const QString &arg1);

    void on_comboxSlot_9_currentIndexChanged(const QString &arg1);

    void on_comboxSlot_10_currentIndexChanged(const QString &arg1);

    void on_comboxSlot_11_currentIndexChanged(const QString &arg1);

    void on_comboxSlot_12_currentIndexChanged(const QString &arg1);

    void on_comboxSlot_13_currentIndexChanged(const QString &arg1);

    void on_comboxSlot_14_currentIndexChanged(const QString &arg1);

    void on_comboxSlot_15_currentIndexChanged(const QString &arg1);

    void on_comboxSlot_16_currentIndexChanged(const QString &arg1);

private:
    Ui::QDlgMcu *ui;
};

#endif // DLGMCU_H
