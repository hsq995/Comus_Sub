#pragma once
#include "ClassDef.h"

//////////////////////////////////////////////////////////
//网格单元统计分区类声明
class CRegion
{
	class CRegCell
	{
	public:
		CRegCell(void);
		virtual ~CRegCell(void);
	public:
		long m_iLyr;                                           //统计分区网格单元的层号
		long m_iRow;                                           //统计分区网格单元的行号
		long m_iCol;                                           //统计分区网格单元的列号
	};

	friend class CGDWater;
	friend class CRegBudget;

public:
	CRegion(void);
	virtual ~CRegion(void);

public:
	long m_IREG;                                               //分区编号
	CString m_REGNAM;                                          //分区名称
	CRegSche* m_pBudgetSch;                                    //统计方案指针

protected:  //运行变量
	vector<CGridCell*> m_pRegGridCellAry;                      //统计分区的网格单元指针数组
	vector<CRegCell> m_RegBudgetCellAry;                       //统计分区的网格单元数组
	double m_StrgIn;                                           //当前计算时段内分区内单元蓄量减少(水头下降)释放的水量(L3)
	double m_StrgOut;                                          //当前计算时段内分区内单元蓄量增加(水头上升)蓄积的水量(L3)
	double m_ExcIn;                                            //当前计算时段内其他分区向本分区流入的水量(L3)
	double m_ExcOut;                                           //当前计算时段内本分区向其他分区流出水量(L3)
	double m_RegBalErr;                                        //当前计算时段内的水量平衡误差(L3)
	vector<double> m_AryRegSinkIn;                             //当前计算时段内进入分区的边界/源汇项数组(L3)
	vector<double> m_AryRegSinkOut;                            //当前计算时段内离开分区的边界/源汇项数组(L3)
	CResultVal* m_ResultAry;                                   //模拟输出结果存储数组
	static vector<CString> m_ItemNamAry;                       //输出项名称数组
	static vector<int> m_ItemAtriAry;                          //输出项性质数组
	map<long, CResultVal*> m_MapNearbyReg;                     //当前时段内本分区流出到邻近分区的水量查询表
	vector<CResultVal*>                                        //当前时段内本分区流出到邻近分区的水量数组(L3)
		m_AryFlowToNearbyReg;
	static CMemTable* m_pBDMemDataTable;                       //数据库分区水量平衡输出数据缓存表
	static CMemTable* m_pExcMemDataTable;                      //数据库分区水量交换量输出数据缓存表

protected: //内部功能函数
	void CalBalError();                                        //计算时段内地下水平衡误差

public:  //功能函数
	void AddRegCell(long iLyr, long iRow, long iCol);          //为统计分区添加统计网格单元
	void RegOutPut(long PrnID, long iPer,long iStep,           //输出统计分区统计结果(数据库)
		 double CumTLen);     
	void RegOutPut(long PrnID, long iPer, long iStep,          //输出统计分区统计结果(TXT)
		double CumTLen, CStdioFile& BDOutFile, CStdioFile& FLOutFile);		
	void SaveResult();                                         //保存计算结果
	void InitSimulation();                                     //模拟之前进行初始化工作
	void AddNearbyReg(long iRegID);                            //添加邻近分区到水量交换量数据数组
	static void MakeBDDataTable(_RecordsetPtr pTable,          //构建分区水量平衡输出缓存表
			CString TbName);
	static void MakeExcDataTable(_RecordsetPtr pTable,         //构建分区水量交换量输出缓存表
			CString TbName);
	static void WriteDatabase();                               //写数据库
	static void SetOutputItem();                               //设置数据库输出项名称
};

//////////////////////////////////////////////////////////
//分区水量平衡统计方案类声明
class CRegSche
{
	//定义统计方案网格单元
	class CRegScheCell
	{
	public:
		CRegScheCell();
		virtual ~CRegScheCell();
	public:
		long m_IREG;                //统计分区的编号
		CString m_RegNam;           //统计分区的名称
		long m_iLyr;                //统计分区网格单元的层号
		long m_iRow;                //统计分区网格单元的行号
		long m_iCol;                //统计分区网格单元的列号
	};

	friend class CRegBudget;

public:
	CRegSche(void);
	virtual ~CRegSche(void);

	friend class CGDWater;

protected:
	long m_SCHEID;                                             //统计方案编号
	CString m_SCHENAM;                                         //统计方案名称
	long m_NumLyr;                                             //地下水系统对象的网格层数
	long m_NumRow;                                             //地下水系统对象的网格行数
	long m_NumCol;                                             //地下水系统对象的网格列数
	vector<CRegScheCell> m_RegScheCellAry;                     //统计方案的网格单元
	long*** m_pCellRegIDAry;                                   //统计网格单元指针数组(三维)
	vector<CRegion*> m_pRegAry;                                //统计方案下的统计分区数组
	map<long, CString> m_RegNamMap;                            //水量平衡统计分区名称字典

public:
	void AddRegScheCell(long RegID, CString RegNam,            //添加统计方案的网格单元
		long iLyr, long iRow, long iCol);
	void InitRegScheData();                                    //初始化统计方案数据
	void SaveResult();                                         //保存统计方案各统计分区的计算结果
	void ScheOutPut(long PrnID, long iPer, long iStep,         //输出统计方案各统计分区的统计结果(数据库)
		double CumTLen);
	void ScheOutPut(long PrnID, long iPer, long iStep,         //输出统计方案各统计分区的统计结果(TXT)
		double CumTLen, CStdioFile& BDOutFile, CStdioFile& FLOutFile);		
	CString& GetRegScheNam();                                  //返回统计方案的名称
	long GetRegScheID();                                       //返回统计方案的编号
	long GetCellRegID(long iLyr, long iRow, long iCol);        //返回统计方案的网格单元的统计分区编号
	CString GetRegNam(long RegID);                             //返回对应统计分区编号的统计分区名称
};

//////////////////////////////////////////////////////////
//分区水量平衡统计处理类声明
class CRegBudget
{
public:
	CRegBudget();
	virtual ~CRegBudget();

protected:
	long m_iPrnOpt;                                      //输出控制. 0:不输出；1:逐应力期内模拟时段输出；2:逐应力期输出
	vector<CRegSche*> m_BudgetSchAry;                    //水量平衡统计方案数组
	CStdioFile m_BDOutPutFile;                                //统计分区水量平衡输出文件(TXT)
	CStdioFile m_FlowOutPutFile;                              //统计分区流量关系输出文件(TXT)

public:
	void ReadRBData(_ConnectionPtr pConnection);         //读取水量平衡统计方案数据(数据库)
	void ReadRBData(const CString& strFilePath);         //读取水量平衡统计方案数据(TXT文件)
	void SetPrnOpt(long iPrnOpt);                        //设置水量平衡统计结果输出控制
	void InitSimulation();                               //对水量平衡统计方案进行初始化
	void SaveResult();                                   //保存水量平衡分析结果
	void BudgetOutPutDB(BOOL bEndPer, long iPer,         //输出统计分区水量平衡统计结果(数据库)
		long iStep, double CumTLen);                     
	void BudgetOutPutTXT(BOOL bEndPer, long iPer,        //输出统计分区水量平衡统计结果(TXT)
		long iStep, double CumTLen);
};