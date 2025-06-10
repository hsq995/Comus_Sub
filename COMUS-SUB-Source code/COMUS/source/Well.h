#pragma once
#include "ClassDef.h"

//////////////////////////////////////////////////////////
//开采井网格单元类声明
class CWellCell : public CBndCell
{
public:
	CWellCell();
	virtual ~CWellCell();

public:
	double m_PumpRate;                            //对应的开采强度(L3/T)
	double m_SatThr;                              //仅模拟网格单元干-湿转化，井流量为负(开采)，网格单元所属含水层类型为1或3时有效, 
	                                              //为开采量线性减少时的含水层饱和厚度阈值(L)
};

//////////////////////////////////////////////////////////
//井流处理类声明
class CWell : public CBndHandler
{
public:
	CWell();
	virtual ~CWell();

public:
	virtual CBndCell* ReadPerCellData(_RecordsetPtr            //读取应力期网格单元源汇项数据(数据库)
		pCellDataRec);
	virtual CBndCell* ReadPerCellData(CString strData);        //读取应力期网格单元源汇项数据(TXT)
	virtual void FormMatrix(long IBOUND[], double HNEW[],      //根据开采井边界条件形成/处理矩阵方程
		double HCOF[], double RHS[], double HOLD[],
		double HPRE[]);
	virtual void Budget(long IBOUND[], double HNEW[], 
		double HOLD[]);                                        //计算开采井边界处水量通量
};