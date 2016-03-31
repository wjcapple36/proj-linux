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
	// ��ABָʾ�ߣ�A�ߵ׶˵�λ�ã�B�ߵĵ׶�λ�ã���׼�߸ߣ�Ĭ��A�ߴ���ѡ��״̬
	void DrawABLine(QPainter * pPainter, QPoint ptA, QPoint ptB, QRect rectDraw,
		int iLineHeight, CString strInforA, CString strInforB, QColor clrLine, int iPenStyle = PS_DASH , bool bASel = true);
	// �ڼ��ߴ����л���һ�����߻������ߣ���ڲ��������꣬��ͼ�������߻��ߺ��ߣ�Ĭ�ϵĺ��ߡ�
	void DrawIndictLine(QPainter * pPainter, int iFix, QRect rectDraw, QColor clrLine, int iPenStyle = PS_DASHDOT, bool bHori = true);
	//����Ŵ�
	void HoriZoom(int iTimes, int iSelLinePos, int MaxPointNum, int &iStartPos, int &iEndPos);
	//����Ŵ�,ȷ�������������Сֵ�����ֵ
	void VertiZoom(int iTimes, float fCenterLoss, float &fStartValue, float &fMaxValue);
	void DrawGridLine(QPainter *pPainter, QRect RectCurve,int iGridNumOnX, int iGridNumOnY,QColor clrLine = RGB(50,50,50));
	void DrawSignsOnXLocation(QPainter *pPainter, QRect RectCurve, int iEventType, QPoint PtEvnt);
	bool IsLineABSeleced(QPoint ptPositon, bool &bSelA, QPoint ptA, QPoint ptB);
};

