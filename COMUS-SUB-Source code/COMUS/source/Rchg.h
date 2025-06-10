#pragma once
#include "ClassDef.h"

//////////////////////////////////////////////////////////
//面状补给网格单元类声明
class CRchgCell : public CBndCell
{
public:
	CRchgCell();
	virtual ~CRchgCell();

public:
	long m_IRECHRG;                                //面上补给计算选项. 0: 按指定层位单元补给；1: 按最高的有效层位单元补给
	double m_RechRate;                             //对应的面上开采强度(L/T)
};

//////////////////////////////////////////////////////////
//面状补给处理类声明
class CRchg : public CBndHandler
{
public:
	CRchg();
	virtual ~CRchg();

public:
	virtual CBndCell* ReadPerCellData(_RecordsetPtr            //读取应力期网格单元源汇项数据(数据库)
		pCellDataRec);
	virtual CBndCell* ReadPerCellData(CString strData);        //读取应力期网格单元源汇项数据(TXT)
	virtual void FormMatrix(long IBOUND[], double HNEW[],      //根据面状补给边界条件形成/处理矩阵方程
		double HCOF[], double RHS[], double HOLD[], 
		double HPRE[]);
	virtual void Budget(long IBOUND[], double HNEW[], 
		double HOLD[]);                                        //计算面状补给边界处水量通量
};