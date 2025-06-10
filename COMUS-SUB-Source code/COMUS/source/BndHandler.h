#pragma once
#include "ClassDef.h"

//////////////////////////////////////////////////////////
//源汇项网格单元类声明
class CBndCell
{
public:
	CBndCell();
	virtual ~CBndCell();

public:
	long m_ILYR;                                  //边界单元对应的网格的层号（从1开始编号）
	long m_IROW;                                  //边界单元对应的网格的行号（从1开始编号）
	long m_ICOL;                                  //边界单元对应的网格的列号（从1开始编号）
};

//////////////////////////////////////////////////////////
//边界/源汇项应力期数据类声明
class CPerBndData
{
public:
	CPerBndData(void);
	virtual ~CPerBndData(void);

public:
	long m_iPeriod;                                               //应力期编号
	vector<CBndCell*> m_pBndCellAry;                              //对应应力期下的源汇项单元指针数组
	map<long, CBndCell*> m_NodeMap;                               //单元的网格系统编号查询表

public:
	void AddBndCell(CBndCell* pBndCell, long iPer);               //添加对应应力期的源汇项网格单元数据
	void DeleteBndData();                                         //删除现有的边界/源汇项数据
};

//////////////////////////////////////////////////////////
//源汇项处理类模板声明
class CBndHandler
{
	friend class CGDWater;

public:
	CBndHandler();
	virtual ~CBndHandler();

protected:
	CString m_BndObjNam;                                          //边界/源汇项的名称(中文), 数据有问题时用于输出信息
	CString m_BndNamAbb;                                          //边界/源汇项的英文简称(只能3个字符), 用于源汇项的统计
	CString m_BndMean;                                            //边界/源汇项的英文解释(只能16个字符), 用于模拟结果的输出
	CString m_PerDataTbNam;                                       //Sql数据库应力期数据表名称
	long m_BndID;                                                 //边界/源汇项统计输出时的编号
	long m_iPrnOpt;                                               //边界/源汇项的模拟结果输出控制. 0:不输出；1:逐应力期内模拟时段输出；2:逐应力期输出
	_RecordsetPtr m_pPerDataTable;                                //应力期数据记录集对象
	CStdioFile m_PerDataFile;                                     //应力期数据记录文件(TXT接口)
	CPerBndData* m_pCurPerData;                                   //当前应力期数据指针

public:
	void SetBndBaseInfo(CString StrObjName, CString StrBndNamAbb, //设置边界/源汇项对象相关的名称信息
		CString StrBndMean, CString StrPerDataTB);
	void SetPrnOpt(long iPrnOpt);                                 //设置边界/源汇项模拟结果输出控制
	 
public:
	virtual void LinkDataInOut(_ConnectionPtr pConnection,        //关联边界/源汇项的输入数据和输出项并检查(数据库)
		long& BndID);            
	virtual void LinkDataInOut(const CString& strFilePath,        //关联边界/源汇项的输入数据和输出项并检查(TXT)
		long& BndID);
	virtual void AddBndSinkInfo(long& BndID);                     //添加边界/源汇项统计输出编号和名称
	virtual void InitSimulation();                                //模拟之前初始化
	virtual void SetPerData(_ConnectionPtr pConnection);          //设置Sql数据库应力期数据表(数据库)
	virtual void SetPerData(const CString& strFilePath);          //设置应力期数据表(TXT)
	virtual void PrePerSim(_ConnectionPtr pConnection,            //应力期模拟之前数据准备(数据库)
		long iPer);   
	virtual void PrePerSim(long iPer);                            //应力期模拟之前数据准备(TXT)		
	virtual void PreStepSim(long IBOUND[], double HNEW[],         //时段模拟之前数据准备
		double HOLD[]); 
	virtual void SaveResult();                                    //保存边界/源汇项的计算结果
	virtual void BndOutPutDB(BOOL bEndPer, long iPer,             //输出边界/源汇项的模拟计算结果(数据库)
		long iStep, double CumTLen, double PerLen,
		double DeltT);
	virtual void BndOutPutTXT(BOOL bEndPer, long iPer,            //输出边界/源汇项的模拟计算结果(TXT)
		long iStep, double CumTLen, double PerLen,
		double DeltT);


public:
	virtual CBndCell* ReadPerCellData(_RecordsetPtr               //读取应力期网格单元源汇项数据(数据库)
		pCellDataRec) = 0;                     
	virtual CBndCell* ReadPerCellData(CString strData) = 0;       //读取应力期网格单元源汇项数据(TXT)
	virtual void FormMatrix(long IBOUND[], double HNEW[],         //形成/处理矩阵方程
		double HCOF[], double RHS[], double HOLD[],
		double HPRE[]) = 0;
	virtual void Budget(long IBOUND[], double HNEW[],             //计算边界处水量通量
		double HOLD[]) = 0;
};