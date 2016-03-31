#include "StdAfx.h"
#include "Curve.h"
#include  <math.h>

CCurve::CCurve(void)
	
{
	
}


CCurve::~CCurve(void)
{
}


/************************************************************************/
/*��������DrawCurve
**���룺pDC����ʼ���׵�ַ����ʼλ�ã�����λ�ã����ͣ���ɫ
**�������
**����������������
**ȫ�ֱ�����
**����ģ�飺
** ���ߣ��Ľ�
**���ڣ�2011.7.11
**�޸ģ�
**���ڣ�
**�汾��                                                              */
/************************************************************************/


void CCurve::DrawCurve(QPainter * pPainter, QPoint * pCurvePoint, 
	int iStartPos, int iEndPos, QColor clrCurve,int iCurvSpaceV,  int iPenStyle)
{

	int iSaveDC = pPainter->save();	
	QPen penCurve;	
	penCurve.CreatePen (iPenStyle, 1, clrCurve);
	//������
	pPainter->SelectObject (&penCurve);
	for(int j = iStartPos; j < iEndPos; j++)
	{
		if (j  == MAX_SAMPLE_NUM)
		{
			break;
		}
		if(j > iStartPos)
		{
			pPainter->MoveTo(pCurvePoint[j-1].x,pCurvePoint[j-1].y - iCurvSpaceV);
			pPainter->LineTo(pCurvePoint[j].x,pCurvePoint[j].y - iCurvSpaceV);

		}

	}
	pPainter->restore(iSaveDC);

}


/************************************************************************/
/*��������DrawLineAB(QPainter *pPainter)
**���룺pDC,A,Bָʾ�ߵ�λ�ã���ͼ�����߸ߣ����ʣ�A��B��Ϣ���ߵ���ɫ��
       ѡ��״̬
**�������
**����������  ����ͼ���򻭳�ABָʾ��
**ȫ�ֱ�����
**����ģ�飺
** ���ߣ��Ľ�
**���ڣ�2011.7.11
**�޸ģ�
**���ڣ�
**�汾��                                                              */
/************************************************************************/
void CCurve::DrawABLine(QPainter * pPainter, QPoint ptA, QPoint ptB, QRect rectDraw,
	int iLineHeight, QString strInforA, QString strInforB, QColor clrLine, int iPenStyle, bool bASel)
{

	int iSaveDC = pPainter->save();	
	QPen penLine;	
	penLine.CreatePen (iPenStyle, 2, clrLine);
	pPainter->SelectObject(&penLine);
	//ָʾ��
	pPainter->MoveTo(ptA.x, rectDraw.top + 20);
	pPainter->LineTo(ptA.x, rectDraw.bottom);	
	pPainter->MoveTo(ptB.x,  rectDraw.top + 40);
	pPainter->LineTo(ptB.x,  rectDraw.bottom);

	//��AB��˵�ָʾ����
	QRect rtA,rtB;
	//��A,B�߳�������ʱ������ʱ������ο�����ߣ��������ұ�	
	if(ptA.x < rectDraw.right/2)
	{
		rtA.left = ptA.x;
		rtA.top = rectDraw.top;
		rtA.right = ptA.x+70;
		rtA.bottom = rectDraw.top + 14;
	}
	else
	{
		rtA.left = ptA.x - 70;
		rtA.top = rectDraw.top;
		rtA.right = ptA.x;
		rtA.bottom = rectDraw.top + 14;
	}
	if(ptB.x< rectDraw.right/2)
	{
		rtB.left = ptB.x;
		rtB.top = rectDraw.top+20;
		rtB.right = ptB.x + 70;
		rtB.bottom = rectDraw.top + 34;
	}
	else
	{
		rtB.left = ptB.x-70;
		rtB.top = rectDraw.top + 20;
		rtB.right = ptB.x;
		rtB.bottom = rectDraw.top + 34;
	}

	//ѡ�еı����ʾ��Ϣ�ı�����ɫ�ó�����ɫ
	if( bASel )	
	{
		pPainter->SetTextColor (RGB(0,0,0));
		pPainter->ExtTextOut(rtB.left + 2, rectDraw.top + 20, ETO_CLIPPED, rtB, strInforB, NULL);
		pPainter->SetTextColor (RGB(255,255,255));
		pPainter->SetBkColor(RGB(0,0,255));
		pPainter->ExtTextOut(rtA.left + 2, rectDraw.top, ETO_CLIPPED, rtA,strInforA, NULL);
	}
	else
	{
		pPainter->SetTextColor (RGB(0,0,0));
		pPainter->ExtTextOut(rtA.left + 2, rectDraw.top, ETO_CLIPPED, rtA, strInforA, NULL);
		pPainter->SetTextColor (RGB(255,255,255));
		pPainter->SetBkColor(RGB(0,0,255));
		pPainter->ExtTextOut(rtB.left + 2, rectDraw.top + 20, ETO_CLIPPED, rtB, strInforB, NULL);
	}

	pPainter->restore(iSaveDC);
}


/************************************************************************/
/*��������DrawIndictLine
**���룺pDC,A,Bָʾ�ߵ�λ�ã����꣬��ͼ�����ߵ���ɫ��ˮƽ��/��ֱ��
**�������
**���������� ������������
**ȫ�ֱ�����
**����ģ�飺
** ���ߣ��Ľ�
**���ڣ�2011.7.11
**�޸ģ�
**���ڣ�
**�汾��                                                              */
/************************************************************************/
void CCurve::DrawIndictLine(QPainter * pPainter, int iFix, QRect rectDraw, QColor clrLine, int iPenStyle, bool bHori)
{
	int iSaveDC = pPainter->save();	
	QPen penLine;	
	penLine.CreatePen (iPenStyle, 1, clrLine);
	pPainter->SelectObject(&penLine);
	if (bHori)
	{
		//����
		pPainter->MoveTo( rectDraw.left, iFix );
		pPainter->LineTo( rectDraw.right, iFix );
	}
	else
	{
		//����
		pPainter->MoveTo(iFix,  rectDraw.bottom);
		pPainter->LineTo(iFix,  rectDraw.top);
	}
	pPainter->restore(iSaveDC);
}
/************************************************************************/
/*��������HoriZoom
**���룺�Ŵ�������ѡ��ָʾ�ߵ�λ�ã��ܵĵ�������ʼλ�ã�����λ�ã�
**�������
**����������  ����Ŵ󣬸��ݷŴ�����ѡ�е�ָʾ��λ�ã��Ŵ����ģ�
ȷ���������ֹ��
**ȫ�ֱ�����
**����ģ�飺
** ���ߣ��Ľ�
**���ڣ�2011.7.11
**�޸ģ�
**���ڣ�
**�汾��                                                              */
/************************************************************************/
void CCurve::HoriZoom(int iTimes,int iSelLinePos, int MaxPointNum, int &iStartPos, int &iEndPos)
{	
 	int iZoomCoeff =  (int)pow(2.0,iTimes);
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
/*��������VertiZoom
**���룺�Ŵ�������ѡ��ָʾ�ߵ�λ�ã��ܵĵ�������ʼλ�ã�����λ�ã�
**�������
**��������:����Ŵ󣬸��ݷŴ�����ȷ��������������ֵ����Сֵ
**ȫ�ֱ�����
**����ģ�飺
** ���ߣ��Ľ�
**���ڣ�2011.7.11
**�޸ģ�
**���ڣ�
**�汾��                                                              */
/************************************************************************/
void CCurve::VertiZoom(int iTimes, float fCenterLoss, float &fStartValue, float &fMaxValue)
{
	int iZoomCoeff =  (int)pow(2.0,iTimes);
	float fStart = 0;
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
/*��������DrawGridLine
**���룺pDC����ͼ����
**�������
**��������:��դ����
**ȫ�ֱ�����
**����ģ�飺
** ���ߣ��Ľ�
**���ڣ�2011.7.11
**�޸ģ�wq
**���ڣ�2012.2.13
**�汾��                                                              */
/************************************************************************/
void CCurve::DrawGridLine(QPainter *pPainter, QRect RectCurve,int iGridNumOnX, int iGridNumOnY,QColor clrLine)
{
	int nSaveDC=pPainter->save();
	QPen penGridLine;
	penGridLine.CreatePen (PS_DOT/*��*/,1,clrLine);
	int ndx = RectCurve.Width ()/iGridNumOnX;
	int ndy = RectCurve.Height ()/iGridNumOnY;
	pPainter->SelectObject (&penGridLine);
	for ( int i = 1; i< iGridNumOnX; i++)
	{			
		pPainter->MoveTo (RectCurve.left + ndx * i,RectCurve.bottom );
		pPainter->LineTo (RectCurve.left + ndx * i,RectCurve.top );			
	}
	for (int i = 1; i < iGridNumOnY; i++)
	{
		pPainter->MoveTo (RectCurve.left , RectCurve.bottom - ndy * i);
		pPainter->LineTo (RectCurve.right, RectCurve.bottom - ndy * i);
	}
	//�ָ�
	pPainter->restore(nSaveDC);
}
/************************************************************************/
/*��������DrawSignsOnXLocation
**���룺pDC����ͼ�����¼����־
**�������
**��������:���¼����־
**ȫ�ֱ�����
**����ģ�飺
** ���ߣ��Ľ�
**���ڣ�2011.7.11
**�޸ģ�
**���ڣ�
**�汾��                                                              */
/************************************************************************/
void CCurve::DrawSignsOnXLocation(QPainter *pPainter, QRect RectCurve, int iEventType, QPoint PtEvnt)
{
	int iYEvent;
	iYEvent = RectCurve.bottom-15;
	int iSaveDC=pPainter->save();
	QPen penEvent;
	penEvent.CreatePen(PS_SOLID,2,RGB(250,0,0));	
	pPainter->SelectObject(&penEvent);

	if(0 == iEventType)//��ʼ�¼�
	{
		pPainter->MoveTo(PtEvnt.x,iYEvent-6);
		pPainter->LineTo(PtEvnt.x,iYEvent+6);
		pPainter->MoveTo(PtEvnt.x,iYEvent);
		pPainter->LineTo(PtEvnt.x+6,iYEvent);
	}
	if(1 == iEventType)//�����¼�
	{
		pPainter->MoveTo(PtEvnt.x-6,iYEvent+6);
		pPainter->LineTo(PtEvnt.x-2,iYEvent+6);
		pPainter->MoveTo(PtEvnt.x-2,iYEvent+6);
		pPainter->LineTo(PtEvnt.x-2,iYEvent-6);
		pPainter->MoveTo(PtEvnt.x-2,iYEvent-6);
		pPainter->LineTo(PtEvnt.x+2,iYEvent-6);
		pPainter->MoveTo(PtEvnt.x+2,iYEvent-6);
		pPainter->LineTo(PtEvnt.x+2,iYEvent+6);
		pPainter->MoveTo(PtEvnt.x+2,iYEvent+6);
		pPainter->LineTo(PtEvnt.x+6,iYEvent+6);
	}
	if(2 == iEventType)//�Ƿ����¼�
	{
		pPainter->MoveTo(PtEvnt.x-6,iYEvent-3);
		pPainter->LineTo(PtEvnt.x,iYEvent-3);
		pPainter->MoveTo(PtEvnt.x,iYEvent-3);
		pPainter->LineTo(PtEvnt.x,iYEvent+3);
		pPainter->MoveTo(PtEvnt.x,iYEvent+3);
		pPainter->LineTo(PtEvnt.x+6,iYEvent+3);
	}
	if(3 == iEventType)  //�����¼���
	{
		pPainter->MoveTo(PtEvnt.x,iYEvent-6);
		pPainter->LineTo(PtEvnt.x,iYEvent+6);
		pPainter->MoveTo(PtEvnt.x,iYEvent);
		pPainter->LineTo(PtEvnt.x-6,iYEvent);
	}
	pPainter->restore(iSaveDC);
}
/************************************************************************/
/*��������IsLineABSeleced
**���룺
**�������
**��������: �ж�����Ƿ����A,B���
**ȫ�ֱ�����
**����ģ�飺
** ���ߣ��Ľ�
**���ڣ�2011.7.11
**�޸ģ�
**���ڣ�
**�汾��                                                              */
/************************************************************************/
bool CCurve::IsLineABSeleced(QPoint ptPositon, bool &bSelA, QPoint ptA, QPoint ptB)
{
	int iSpaceA = abs(ptPositon.x - ptA.x);
	int iSpaceB = abs(ptPositon.x - ptB.x);
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



