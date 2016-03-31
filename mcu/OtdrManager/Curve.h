#pragma once
// #include "atltypes.h"
class CCurve
{
public:
	CCurve(void);
	~CCurve(void);
//	void DrawCurve(void);
	void DrawCurve(QPainter * pPainter, QPoint * pCurvePoint, 
		int iStartPos, int iEndPos, QColor clrCurve,int iCurvSpaceV = 0 ,int iPenStyle = PS_SOLID);
	// 画AB指示线，A线底端的位置，B线的底端位置，标准线高，默认A线处于选中状态
	void DrawABLine(QPainter * pPainter, QPoint ptA, QPoint ptB, QRect rectDraw,
		int iLineHeight, CString strInforA, CString strInforB, QColor clrLine, int iPenStyle = PS_DASH , bool bASel = true);
	// 在迹线窗口中画出一条横线或者竖线；入口参数：坐标，画图区域，竖线或者横线，默认的横线。
	void DrawIndictLine(QPainter * pPainter, int iFix, QRect rectDraw, QColor clrLine, int iPenStyle = PS_DASHDOT, bool bHori = true);
	//横向放大
	void HoriZoom(int iTimes, int iSelLinePos, int MaxPointNum, int &iStartPos, int &iEndPos);
	//纵向放大,确定纵轴坐标的最小值，最大值
	void VertiZoom(int iTimes, float fCenterLoss, float &fStartValue, float &fMaxValue);
	void DrawGridLine(QPainter *pPainter, QRect RectCurve,int iGridNumOnX, int iGridNumOnY,QColor clrLine = RGB(50,50,50));
	void DrawSignsOnXLocation(QPainter *pPainter, QRect RectCurve, int iEventType, QPoint PtEvnt);
	bool IsLineABSeleced(QPoint ptPositon, bool &bSelA, QPoint ptA, QPoint ptB);
};

