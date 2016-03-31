#ifndef QCURV_H
#define QCURV_H
#include <QPainter>

class QCurv
{
public:
    float   C;
public:
     explicit QCurv();
     ~QCurv(void);
public:
    //	void DrawCurve(void);
    void DrawCurve(QPainter * pPainter, QPoint * pCurvePoint,
                   int iStartPos, int iEndPos, QColor clrCurve,int iCurvSpaceV = 0 ,int iPenStyle = (Qt::SolidLine));
    // 画AB指示线，A线底端的位置，B线的底端位置，标准线高，默认A线处于选中状态
    void DrawMarkLine(QPainter * pPainter, int xPos,  QRect rectDraw,
                    QString strInfor,QColor clrLine, int iPenStyle = Qt::SolidLine , bool bSel = true);
    // 在迹线窗口中画出一条横线或者竖线；入口参数：坐标，画图区域，竖线或者横线，默认的横线。
    void DrawIndictLine(QPainter * pPainter, int iFix, QRect rectDraw, QColor clrLine, int iPenStyle = Qt::DashDotDotLine, bool bHori = true);
    //横向放大
    void HoriZoom(int iTimes, int iSelLinePos, int MaxPointNum, int &iStartPos, int &iEndPos);
    //纵向放大,确定纵轴坐标的最小值，最大值
    void VertiZoom(int iTimes, float fCenterLoss, float &fStartValue, float &fMaxValue);
   //绘制网格
    void DrawGridLine(QPainter *pPainter, QRect RectCurve,int iGridNumOnX, int iGridNumOnY,QColor clrLine = QColor(50,50,50));
    //事件点标示
    void DrawSignsOnXLocation(QPainter *pPainter, QRect RectCurve, int iEventType, QPoint PtEvnt);
    //判断迹线是否被选中
    bool IsLineABSeleced(QPoint ptPositon, bool &bSelA, QPoint ptA, QPoint ptB);
    //绘制坐标轴
    void DrawCoordinate(QPainter *pPainter, QRect RectCurve,\
                        float xStart, float xEnd, float yStart, float yEnd);
    //将dB,距离转换成像素
    void GetPixel(QRect curvRect,QPoint ptBuf[], float dB[], \
                  float yStart, float yEnd, int xStart, int xEnd);
    void GetPixel(QRect curvRect,QPoint ptBuf[], unsigned short dB_1000[],\
                         float yStart,float yEnd, int xStart, int xEnd);
    //从点到米
    float Get_xCoord(int samplePt, int frequency, float n);
    //从像素转换到采样点
    int GetSampPt(QRect curvRect, int pixel_x  ,int xStart, int xEnd);
};


#endif // QCURV_H
