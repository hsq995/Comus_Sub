#pragma once
#include "ClassDef.h"

//////////////////////////////////////////////////////////
//模拟中的边界或源汇项信息类声明
class CActSimBnd
{
	friend class CGridCell;
	friend class CGridLyr;
	friend class CGDWater;
	friend class CRegion;
	friend class CRegBudget;

public:
	CActSimBnd();
	virtual ~CActSimBnd();

protected:
	long m_BndID;                         //被模拟的边界或源汇项的编号
	CString m_BndNamAbb;                  //被模拟的边界或源汇项的简称(3个英文字符), 用于统计源汇项
	CString m_BndMean;                    //被模拟的边界或源汇项的释义(16个英文字符), 用于模拟输出信息
};

//////////////////////////////////////////////////////////
//地下水计算单元类
class CGridCell
{
	friend class CGridLyr;
	friend class CGDWater;
	friend class CRegion;
	friend class CRegBudget;
	friend class CSHB;

public:
	CGridCell();
	virtual ~CGridCell();

protected:
    //模型参数(地下水单元格属性表数据)
	long m_ILYR;                   //网格的层号（从1开始编号）
	long m_IROW;                   //网格的行号（从1开始编号）
	long m_ICOL;                   //网格的列号（从1开始编号）
	long m_INIIBOUND;              //网格单元的输入(初始)属性（0为无效单元，<0为水头单元，>0为变水头单元）, 初始属性为0时不可干湿转化
	long m_IBOUND;                 //模拟过程中网格单元的属性（0为无效单元，<0为水头单元，>0为变水头单元）, 该值在每个时段计算收敛后才更新
	double m_TOP;                  //网格单元的顶边界高程(L). BCF时仅当LYRCON=2或3时有效,LPF时都有效
	double m_BOT;                  //网格单元的底边界高程(L). BCF时仅当LYRCON=1或3时有效,LPF时都有效
	double m_TRAN;                 //网格单元沿行方向的导水系数(L2/T)，为沿行的渗透系数乘以单元厚度，仅当LYRCON=0,2时才有效
	double m_HK;                   //网格单元沿行方向的渗透系数(L/T)，LYRCON=1,3时才有效
	double m_VCONT;                //仅BCF.垂向水力传导率(1/T)，其值为垂向渗透系数与含水层厚度之比,最底层的含水层的数据无用
	double m_HANI;                 //仅LPF.网格单元格沿列方向上饱和渗透系数与沿行方向上饱和渗透系数之间的比值
	                               //仅含水层的CHANI值(见<LPF含水层属性表>)等于0时有效。
	double m_VKA;                  //仅LPF.网格单元的垂向渗透系数
	double m_VKCB;                 //仅LPF.网格单元底部低渗透性介质的垂向渗透系数(L/T),LAYCBD=1时有效.
	double m_TKCB;                 //仅LPF.网格单元底部低渗透性介质的厚度(L),LAYCBD=1时有效.
	double m_SC1;                  //网格单元第1类贮水系数(-)，当LYRCON=1时为给水度，2或3时为处于承压条件下的贮水系数，
	                               //0时为承压水的贮水系数(-)
	double m_SC2;                  //网格单元第2类贮水系数(-)，仅当LYRCON=2或3时有效，为非承压条件下的给水度
	double m_WETDRY;               //该值的绝对值为单元干湿转化时的阈值，仅当LYRCON=1,3时才有效。当某邻近单元的
	                               //水头超过本单元的abs(WETDRY)+BOT值时,单元从疏干状态转湿润。
	                               //等于0: 表示单元铁定不可重新湿润(不管单元从疏干向湿润转换时算法如何选择)
                                   //小于0: 表示仅下方的单元驱动该单元湿润;
                                   //大于0: 表示下方和同层的四个邻近单元中的任何一个都可驱动该单元湿润
	double m_SHEAD;                //网格单元的初始水头(L)

	//运行变量
	long m_DryHint;                //单元是否不可疏干单元标识(仅全有效单元法时用到). 0表示可疏干单元, 1表示不可疏干单元
	double m_XCord;                //单元中心位置的X坐标
	double m_YCord;                //单元中心位置的Y坐标
	double m_CellArea;             //单元的面积(L2)
	double m_CellWETFCT;           //单元干湿转化计算时的水头相乘因子
	double m_hNew;                 //当前的水头(L), 该值在每个时段计算收敛后才更新
	double m_FlowLeft;             //时段内单元左侧的通量(L3/T),正表示流出
	double m_FlowRight;            //时段内单元右侧的通量(L3/T),正表示流出
	double m_FlowFront;            //时段内单元前侧的通量(L3/T),正表示流出
	double m_FlowBack;             //时段内单元后侧的通量(L3/T),正表示流出
	double m_FlowUp;               //时段内单元上侧的通量(L3/T),正表示流出
	double m_FlowDown;             //时段内单元下侧的通量(L3/T),正表示流出

	//有关平衡量
	double m_BCFInOutR;                                 //时段内单元间的渗流通量(L3/T), 正表示从其他单元流入, 负表示本单元流出
	double m_STRGR;                                     //时段内单元的蓄量变化率(L3/T)(正表示存储水量,负表示释放水量)
	double m_CellBalError;                              //时段内的水量平衡误差(L3)
	static vector<CActSimBnd> m_ActSimBndAry;           //要模拟的边界/源汇项信息数组
	static vector<CString> m_ItemNamAry;                //输出项名称数组(简称, 用于数据库输出)
	static vector<CString> m_ItemMeanAry;               //输出项名称数组(释义, 16字符, 用于TXT输出)
	static vector<int> m_ItemAtriAry;                   //输出项性质数组
	double* m_ArySinkR;                                 //时段内单元上源汇项通量强度存储数组(正为进入单元, 负为离开单元)(L3/T)
	CResultVal* m_ResultAry;                            //模拟输出结果存储数组
	CResultVal* m_FlowRsltAry;                          //网格单元间流量模拟输出结果存储数组

	//数据库输出表缓存
	static CMemTable* m_pMemCellBDTable;                //网格单元水量平衡输出数据缓存表
	static CMemTable* m_pMemHHDataTable;                //网格单元水头输出数据缓存表
	static CMemTable* m_pMemDDDataTable;                //网格单元降深输出数据缓存表
	static CMemTable* m_pMemFlowDataTable;              //网格单元流量输出数据缓存表

protected:
	void CalBalError();                                 //计算时段内地下水平衡误差

public:    //外部接口
	void CheckCellData();                               //检查并处理网格单元的输入数据
	void ZeroStepBDItems();                             //清零网格单元的水平衡项
	void OutPutBD(long PrnID, long iPer, long iStep,    //输出水量平衡计算结果(数据库)
		double PerLen, double DeltT);
	void OutPutHH(long PrnID, long iPer, long iStep,    //输出水头计算结果(数据库)
		double CumTLen);             
	void OutPutDD(long PrnID, long iPer, long iStep,    //输出降深计算结果(数据库)
		double CumTLen);
	void OutPutFlow(long PrnID, long iPer, long iStep,  //输出流量计算结果(数据库)
		double PerLen, double DeltT);
	void SaveResult(long PrnBD, long PrnFlow);          //保存计算结果
	void InitSimulation();                              //模拟之前进行初始化工作
	double GetCellArea();                               //返回网格单元的面积(L2)
	double GetCellTop();                                //返回网格单元的顶板高程(L)
	double GetCellBot();                                //返回网格单元的底板高程(L)
	double GetStarHead();                               //返回网格单元的初始水头值(L)
	double GetCellSC1();                                //返回网格单元承压状态下的贮水率或贮水系数(-)或(1/L)
	double GetCellSC2();                                //返回网格单元非承压状态下的给水度(-)
	long GetCellLyr();                                  //返回网格单元的层号
	long GetCellRow();                                  //返回网格单元的行号
	long GetCellCol();                                  //返回网格单元的列号
	long GetDryHint();                                  //返回网格单元是否不可疏干单元
	void SetSTRGR(double STRGR);                        //设置蓄量变化/仅模拟湖泊时由湖泊单元调用)
	void AddSinkR(long BndID, double SinkR);            //累加单元的源汇项通量值. 源汇项正为进入单元，负为离开单元
	static void AddBndObjItem(long& BndID,              //添加边界/源汇项标识和名称到网格单元源汇项统计信息数组
		CString BndNamAbb, CString BndNamMean);
	static void MakeBDDataTable(_RecordsetPtr pTable,   //构建水量平衡输出缓存表
		CString TbName);         
	static void MakeHHDataTable(_RecordsetPtr pTable,   //构建水位输出缓存表
		 CString TbName);
	static void MakeDDDataTable(_RecordsetPtr pTable,   //构建水位降深输出缓存表
		 CString TbName);
	static void MakeFlowDataTable(_RecordsetPtr pTable, //构建单元流量输出缓存表
		 CString TbName);
	static void WriteDatabase();                        //写数据库
	static void SetOutputItem();                        //设置数据库输出项名称
};

//////////////////////////////////////////////////////////
//注: 以下这些公用函数都设置为内联函数以加快调用速度
inline double CGridCell::GetCellArea()
{
	//返回网格单元的面积(m2)

	return m_CellArea;
}

inline double CGridCell::GetCellTop()
{
	//返回网格单元的顶板高程(L)

	return m_TOP;
}

inline double CGridCell::GetCellBot()
{
	//返回网格单元的底板高程(L)

	return m_BOT;
}

inline double CGridCell::GetStarHead()
{
	//返回网格单元的初始水头值(L)

	return m_SHEAD;
}

inline long CGridCell::GetCellLyr()
{
	//返回网格单元的层号

	return m_ILYR;
}

inline long CGridCell::GetCellRow()
{
	//返回网格单元的行号

	return m_IROW;
}

inline long CGridCell::GetCellCol()
{
	//返回网格单元的列号

	return m_ICOL;
}

inline long CGridCell::GetDryHint()
{
	//返回网格单元的列号

	return m_DryHint;
}

inline void CGridCell::AddSinkR(long BndID, double SinkR)
{
	//累加单元的各种源汇项通量值(降水入渗、潜水蒸发、开采井等)
	//BndID为源汇项的编号, SinkR为通量值
	//源汇项正为进入单元，负为离开单元

	//能够累加源汇项的单元必须是有效单元
	assert(m_IBOUND != 0);
	m_ArySinkR[BndID] = m_ArySinkR[BndID] + SinkR;
}

inline double CGridCell::GetCellSC1()
{
	//返回网格单元承压状态下的贮水率或贮水系数(-)或(1/L)

	return m_SC1;
}

inline double CGridCell::GetCellSC2()
{
	//返回网格单元非承压状态下的给水度(-)

	return m_SC2;
}