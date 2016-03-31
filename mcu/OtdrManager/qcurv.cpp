#include "qcurv.h"
#include "constant.h"
#include <QtCore/qmath.h>


QCurv::QCurv()
{
    //光速 km/s
    C = 299792.458;
}
QCurv::~QCurv(void)
{
}


/************************************************************************/
/*函数名：DrawCurve
**输入：pDC，起始点首地址，开始位置，结束位置，线型，颜色
**输出：无
**功能描述：画曲线
**全局变量：
**调用模块：
** 作者：文金朝
**日期：2011.7.11
**修改：
**日期：
**版本：                                                              */
/************************************************************************/


void QCurv::DrawCurve(QPainter * pPainter, QPoint * pCurvePoint,
                      int iStartPos, int iEndPos, QColor clrCurve,int iCurvSpaceV,  int iPenStyle)
{

   pPainter->save();
    QPen penCurve;
    penCurve.setColor (clrCurve);
    penCurve.setStyle(Qt::PenStyle(iPenStyle));
    penCurve.setWidth(1);
   // penCurve.CreatePen (iPenStyle, 1, clrCurve);
    //画曲线
    pPainter->setPen(penCurve);
    for(int j = iStartPos; j < iEndPos; j++)
    {
        if (j  == MAX_PT_NUM)
        {
            break;
        }
        if(j > iStartPos)
        {
            pPainter->drawLine(pCurvePoint[j-1].x(),pCurvePoint[j-1].y() - iCurvSpaceV,
                               pCurvePoint[j].x(), pCurvePoint[j].y() - iCurvSpaceV);
//            pPainter->drawLine(pCurvePoint[j].x,pCurvePoint[j].y - iCurvSpaceV);

        }

    }
    pPainter->restore();

}


/*
   **************************************************************************************
 *  函数名称：DrawMarkLine
 *  函数描述：绘制标杆，一次绘制一根
 *  入口参数：
 *  返回参数：
 *  作者       ：
 *  日期       ：
 **************************************************************************************
*/
void QCurv::DrawMarkLine(QPainter * pPainter, int xPos, QRect rectDraw,
                        QString strInfor,  QColor clrLine, int iPenStyle, bool bSel)
{

    pPainter->save();
    QPen penLine;    
    //   penLine.CreatePen (iPenStyle, 2, clrLine);
    penLine.setColor (clrLine);
    penLine.setStyle(Qt::PenStyle(iPenStyle));
    penLine.setWidth(1);
    pPainter->setPen(penLine);
    //指示线
    pPainter->drawLine(xPos, rectDraw.top() + 20,xPos, rectDraw.bottom());
    //画AB标杆的指示距离
    QRect rtA;
    //当A,B线超过中线时，向右时距离矩形框在左边，否则在右边
    if(xPos < rectDraw.right() / 2)
    {
        rtA = QRect(xPos,rectDraw.top(), 150, 35);
    }
    else
    {
        rtA = QRect( xPos - 150, rectDraw.top(), 150, 35);
    }



    //选中的标杆显示信息的背景颜色置成深蓝色
    if( bSel )
    {
        pPainter->drawRect(rtA);
        pPainter->fillRect(rtA, QColor(128, 128, 128) );
        pPainter->drawText(rtA,strInfor);
    }
    else
    {
        pPainter->drawRect(rtA);
        pPainter->drawText(rtA,strInfor);
    }

    pPainter->restore();
}


/************************************************************************/
/*函数名：DrawIndictLine
**输入：pDC,A,B指示线的位置，坐标，画图区域，线的颜色，水平线/垂直线
**输出：无
**功能描述： 画结束门限线
**全局变量：
**调用模块：
** 作者：文金朝
**日期：2011.7.11
**修改：
**日期：
**版本：                                                              */
/************************************************************************/
void QCurv::DrawIndictLine(QPainter * pPainter, int iFix, QRect rectDraw, QColor clrLine, int iPenStyle, bool bHori)
{
    pPainter->save();
    QPen penLine;
 //   penLine.CreatePen (iPenStyle, 1, clrLine);
 //   pPainter->SelectObject(&penLine);
    penLine.setColor (clrLine);
    penLine.setStyle(Qt::PenStyle(iPenStyle));
    penLine.setWidth(1);
    pPainter->setPen(penLine);
    if (bHori)
    {
        //横线
        pPainter->drawLine(rectDraw.left(), iFix, rectDraw.right(), iFix);
//        pPainter->MoveTo( rectDraw.left, iFix );
//        pPainter->LineTo( rectDraw.right, iFix );
    }
    else
    {
        //竖线
        pPainter->drawLine(iFix,  rectDraw.bottom(), iFix,  rectDraw.top());
//        pPainter->MoveTo(iFix,  rectDraw.bottom);
//        pPainter->LineTo(iFix,  rectDraw.top);
    }
    pPainter->restore();
}
/************************************************************************/
/*函数名：HoriZoom
**输入：放大倍数，被选中指示线的位置，总的点数，开始位置，结束位置，
**输出：无
**功能描述：  横向放大，根据放大倍数和选中的指示线位置（放大中心）
确定横轴的起止点
**全局变量：
**调用模块：
** 作者：文金朝
**日期：2011.7.11
**修改：
**日期：
**版本：                                                              */
/************************************************************************/
void QCurv::HoriZoom(int iTimes,int iSelLinePos, int MaxPointNum, int &iStartPos, int &iEndPos)
{
    int iZoomCoeff =  (int)qPow(2.0,iTimes);
    int PointNum = (int)(MaxPointNum /(iZoomCoeff/*iTimes + 1*/));
    switch(iTimes)
    {
    case 0: {
        iStartPos = 0;
        iEndPos = MaxPointNum - 1;
    }
        break;
    default:
    {
        if (iSelLinePos - (int)(PointNum/2) >= 0 && iSelLinePos + (int)(PointNum/2) < MaxPointNum - 1)
        {
            iStartPos = iSelLinePos - (int)(PointNum/2)+1;
            iEndPos = iSelLinePos + (int)(PointNum/2)-1;
        }
        else if( iSelLinePos +(int)(PointNum/2) > MaxPointNum - 1)
        {
            iStartPos = MaxPointNum - 1- PointNum;
            iEndPos = MaxPointNum - 1;
        }
        else/* if( iSelLinePos - (int)(PointNum/2) < 0)*/
        {
            iStartPos = 0;
            iEndPos = PointNum;
        }
    }
    }
    iStartPos = iStartPos > 0 ? iStartPos : 1;
    iStartPos = iStartPos < MaxPointNum ? iStartPos : MaxPointNum - 1;
    iEndPos = iEndPos > 0 ? iEndPos : 1;
    iEndPos = iEndPos < MaxPointNum ? iEndPos : MaxPointNum - 1;
}
/************************************************************************/
/*函数名：VertiZoom
**输入：放大倍数，被选中指示线的位置，总的点数，开始位置，结束位置，
**输出：无
**功能描述:纵向放大，根据放大倍数，确定纵轴坐标的最大值和最小值
**全局变量：
**调用模块：
** 作者：文金朝
**日期：2011.7.11
**修改：
**日期：
**版本：                                                              */
/************************************************************************/
void QCurv::VertiZoom(int iTimes, float fCenterLoss, float &fStartValue, float &fMaxValue)
{
    int iZoomCoeff =  (int)pow(2.0,iTimes);
//    float fStart = 0;
    float fMax = MAX_F_DB;
    float fDiv = fMax / (iZoomCoeff/*iTimes + 1*/);

    switch(iTimes)
    {
    case 0: {
        fStartValue = 0;
        fMaxValue = MAX_F_DB;
    }
        break;
    default:
        if ((fCenterLoss  >= fDiv /2) && (fCenterLoss  + fDiv /2 <= fMax) )
        {
            fStartValue = fCenterLoss  - fDiv /2;
            fMaxValue = fCenterLoss  + fDiv /2;

        }
        else if(fCenterLoss  + fDiv /2 > fMax)
        {
            fStartValue = fMax - fDiv;
            fMaxValue = fMax;
        }
        else
        {
            fStartValue = 0;
            fMaxValue =  fDiv;
        }
    }
}
/************************************************************************/
/*函数名：DrawGridLine
**输入：pDC，绘图区域
**输出：无
**功能描述:画栅格线
**全局变量：
**调用模块：
** 作者：文金朝
**日期：2011.7.11
**修改：wq
**日期：2012.2.13
**版本：                                                              */
/************************************************************************/
void QCurv::DrawGridLine(QPainter *pPainter, QRect RectCurve,int iGridNumOnX, int iGridNumOnY,QColor clrLine)
{
    pPainter->save();
    QPen penGridLine;
//    penGridLine.CreatePen (PS_DOT/*点*/,1,clrLine);
    penGridLine.setStyle(Qt::DotLine);
    penGridLine.setWidth(1);
    penGridLine.setColor(clrLine);
    int ndx = RectCurve.width() / iGridNumOnX;
    int ndy = RectCurve.height()/ iGridNumOnY;
    pPainter->setPen(penGridLine);
    for ( int i = 1; i< iGridNumOnX; i++)
    {
        pPainter->drawLine( RectCurve.left() + ndx * i,RectCurve.bottom(),
                            RectCurve.left() + ndx * i,RectCurve.top() );
//        pPainter->LineTo (RectCurve.left + ndx * i,RectCurve.top );
    }
    for (int i = 1; i < iGridNumOnY; i++)
    {
        pPainter->drawLine( RectCurve.left() , RectCurve.bottom() - ndy * i,
                            RectCurve.right(), RectCurve.bottom() - ndy * i);
//        pPainter->LineTo (RectCurve.right, RectCurve.bottom - ndy * i);
    }
    //恢复
    pPainter->restore();
}
/************************************************************************/
/*函数名：DrawSignsOnXLocation
**输入：pDC，画图区域，事件点标志
**输出：无
**功能描述:画事件点标志
**全局变量：
**调用模块：
** 作者：文金朝
**日期：2011.7.11
**修改：
**日期：
**版本：                                                              */
/************************************************************************/
void QCurv::DrawSignsOnXLocation(QPainter *pPainter, QRect RectCurve, int iEventType, QPoint PtEvnt)
{
    int iYEvent;
    iYEvent = RectCurve.bottom() -15;
    pPainter->save();
    QPen penEvent;

    penEvent.setStyle(Qt::SolidLine);
    penEvent.setWidth(1);
    penEvent.setColor(QColor(250,0,0));
    pPainter->setPen(penEvent);

    if(0 == iEventType)//开始事件
    {
        pPainter->drawLine(PtEvnt.x(),  iYEvent-6, PtEvnt.x(), iYEvent+6);
        pPainter->drawLine(PtEvnt.x(), iYEvent,PtEvnt.x() +6,iYEvent);

    }
    if(1 == iEventType)//反射事件
    {

        pPainter->drawLine(PtEvnt.x() - 6, iYEvent + 6, PtEvnt.x() - 2, iYEvent + 6);
        pPainter->drawLine(PtEvnt.x() - 2, iYEvent + 6, PtEvnt.x() - 2, iYEvent - 6);
        pPainter->drawLine(PtEvnt.x() - 2, iYEvent - 6, PtEvnt.x() + 2,iYEvent - 6);
        pPainter->drawLine(PtEvnt.x() + 2,iYEvent - 6, PtEvnt.x() + 2, iYEvent + 6);
        pPainter->drawLine(PtEvnt.x() + 2,iYEvent + 6 ,PtEvnt.x() + 6, iYEvent + 6);

    }
    if(2 == iEventType)//非反射事件
    {

        pPainter->drawLine(PtEvnt.x() - 6, iYEvent - 3, PtEvnt.x(), iYEvent - 3);
        pPainter->drawLine(PtEvnt.x() , iYEvent - 3, PtEvnt.x(), iYEvent+3);
        pPainter->drawLine(PtEvnt.x(), iYEvent+3 , PtEvnt.x() + 6, iYEvent + 3);
    }
    if(3 == iEventType)  //结束事件点
    {
        pPainter->drawLine(PtEvnt.x(), iYEvent - 6 , PtEvnt.x(),iYEvent + 6);
        pPainter->drawLine(PtEvnt.x(),iYEvent, PtEvnt.x() - 6, iYEvent);
    }
    pPainter->restore();
}
/************************************************************************/
/*函数名：IsLineABSeleced
**输入：
**输出：无
**功能描述: 判断鼠标是否点中A,B标杆
**全局变量：
**调用模块：
** 作者：文金朝
**日期：2011.7.11
**修改：
**日期：
**版本：                                                              */
/************************************************************************/
bool QCurv::IsLineABSeleced(QPoint ptPositon, bool &bSelA, QPoint ptA, QPoint ptB)
{
    int iSpaceA = abs(ptPositon.x() - ptA.x());
    int iSpaceB = abs(ptPositon.x() - ptB.x());
    if (iSpaceA < MAX_SELSPAC && iSpaceB > MAX_SELSPAC)
    {
        bSelA = true;
        return true;
    }
    else if (iSpaceA > MAX_SELSPAC && iSpaceB < MAX_SELSPAC)
    {
        bSelA = false;
        return true;
    }
    else if (iSpaceA < MAX_SELSPAC && iSpaceB < MAX_SELSPAC)
    {
        return true;
    }
    else
    {
        return false;
    }

}
/*
   **************************************************************************************
 *  函数名称：DrawCoordinate
 *  函数描述：在曲线左边和下边做出坐标轴
 *  入口参数：
 *  返回参数：
 *  作者       ：
 *  日期       ：
 **************************************************************************************
*/
void QCurv::DrawCoordinate(QPainter *pPainter, QRect RectCurve,float xStart, float xEnd, float yStart, float yEnd)
{
    int i, left, bottom, xSpace, ySpace;
    float xScare, yScare;
    QPen pen;
    QString str;
    //设在画笔
    pen.setStyle(Qt::SolidLine);
    pen.setWidth(1);
    pen.setColor(QColor(0, 0, 0));
    pPainter->setPen(pen);
    //坐标轴的起始点
    left = RectCurve.left();
    bottom = RectCurve.bottom();
    //x,y的间隔，刻度
    xSpace = RectCurve.width() /10 ;
    ySpace = RectCurve.height() / 10;
    xScare = (xEnd - xStart) / 10;
    yScare = (yEnd - yStart) / 10;
    pPainter->drawLine(RectCurve.left(), RectCurve.bottom(),RectCurve.right(),RectCurve.bottom());
    pPainter->drawLine(RectCurve.left(), RectCurve.bottom(),RectCurve.left(),RectCurve.top());
    for(i = 1; i < 11; i++)
    {
        //横轴坐标/刻度
        pPainter->drawLine(left + i*xSpace, bottom, left + i*xSpace, bottom + 5);
        str.setNum(xStart + xScare * i, 'f', 1);
        pPainter->drawText(left + i*xSpace - 20, bottom + 20,  str);
        //纵轴坐标刻度
        pPainter->drawLine(left, bottom - i*ySpace, left - 5, bottom - i*ySpace);
        str.setNum(yStart + yScare * i, 'f', 1);
        pPainter->drawText(left - 35, bottom - i*ySpace + 20, str);

    }
    str = "dB";
    pPainter->drawText(RectCurve.left(), RectCurve.top() - 5,str);
    str = "km";
    pPainter->drawText(RectCurve.right() + 5, RectCurve.bottom(),str);
}
/*
   **************************************************************************************
 *  函数名称：GetPixel
 *  函数描述：将光纤损耗转换成像素
 *  入口参数：
 *  返回参数：
 *  作者       ：
 *  日期       ：
 **************************************************************************************
*/
void QCurv::GetPixel(QRect curvRect,QPoint ptBuf[], float dB[],\
                     float yStart,float yEnd, int xStart, int xEnd)
{
    int i, ptX, ptY, ptCount;
    float ydB;
    ydB = yEnd - yStart;
    ptCount = xEnd - xStart;
    for(i = xStart; i < xEnd; i++)
    {
        ptX = curvRect.left() + curvRect.width()*(i - xStart) / ptCount;
        ptY =curvRect.bottom() - curvRect.height() * (dB[i] - yStart) / ydB;
        //保证不能超出编界
        ptX = ptX > curvRect.right() ? curvRect.right() : ptX ;
        ptX = ptX < curvRect.left() ? curvRect.left() : ptX;
        ptY = ptY > curvRect.bottom() ? curvRect.bottom() : ptY;
        ptY = ptY < curvRect.top() ? curvRect.top() : ptY;
        ptBuf[i].setX(ptX);
        ptBuf[i].setY(ptY) ;
    }
}
void QCurv::GetPixel(QRect curvRect,QPoint ptBuf[], unsigned short dB_1000[],\
                     float yStart,float yEnd, int xStart, int xEnd)
{
    int i, ptX, ptY, ptCount;
    int ydB_1000, y_start_1000, y_end_1000;
    y_start_1000 = yStart*1000;
    y_end_1000 = yEnd * 1000;
    ydB_1000 = (y_end_1000 - y_start_1000);
    ptCount = xEnd - xStart;
    for(i = xStart; i < xEnd; i++)
    {
        ptX = curvRect.left() + curvRect.width()*(i - xStart) / ptCount;
        ptY =curvRect.bottom() - curvRect.height() * (dB_1000[i] - y_start_1000) / ydB_1000;
        //保证不能超出编界
        ptX = ptX > curvRect.right() ? curvRect.right() : ptX ;
        ptX = ptX < curvRect.left() ? curvRect.left() : ptX;
        ptY = ptY > curvRect.bottom() ? curvRect.bottom() : ptY;
        ptY = ptY < curvRect.top() ? curvRect.top() : ptY;
        ptBuf[i].setX(ptX);
        ptBuf[i].setY(ptY) ;
    }
}
//将采样点转换成米
float QCurv::Get_xCoord(int samplePt, int frequency, float n)
{
    float x;
    x = ( C / ( 2 * frequency * n) * (samplePt) );
    return x;
}
//将像素点转换成采样点
int QCurv:: GetSampPt(QRect curvRect, int pixel_x ,int xStart, int xEnd)
{
    int sampPt;
    sampPt = xStart + (xEnd - xStart)*(pixel_x - curvRect.left()) /curvRect.width();
    return sampPt;

}

