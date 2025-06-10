#pragma once
#include "ClassDef.h"

//////////////////////////////////////////////////////////
//时变定水头网格单元类声明
class CSHBCell : public CBndCell
{
public:
	CSHBCell();
	virtual ~CSHBCell();

public:
	double m_StartHead;                            //应力期初始时刻单元的水头(L)
	double m_EndHead;                              //应力期结束时刻单元的水头(L)
};

//////////////////////////////////////////////////////////
//时变定水头处理类声明
class CSHB : public CBndHandler
{
public:
	CSHB();
	virtual ~CSHB();

public:
	virtual void PrePerSim(_ConnectionPtr pConnection,         //应力期模拟之前数据准备(数据库)
		long iPer);        
	virtual void PrePerSim(long iPer);                         //应力期模拟之前数据准备(TXT)		
	virtual void PreStepSim(long IBOUND[], double HNEW[],      //时段模拟之前数据准备
		double HOLD[]);

public:
	virtual CBndCell* ReadPerCellData(_RecordsetPtr            //读取应力期网格单元源汇项数据(数据库)
		pCellDataRec);
	virtual CBndCell* ReadPerCellData(CString strData);        //读取应力期网格单元源汇项数据(TXT)
	virtual void FormMatrix(long IBOUND[], double HNEW[],      //根据时变定水头条件形成/处理矩阵方程
		double HCOF[], double RHS[], double HOLD[], 
		double HPRE[]);
	virtual void Budget(long IBOUND[], double HNEW[], 
		double HOLD[]);                                        //计算时变定水头处水量通量
};

