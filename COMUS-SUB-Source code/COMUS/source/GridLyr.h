#pragma once
#include "ClassDef.h"

//////////////////////////////////////////////////////////
//地下水含水层类类声明
class CGridLyr
{
	friend class CGDWater;
	friend class CRegion;

public:
	CGridLyr(void);
	virtual ~CGridLyr(void);

protected:  //模型参数(含水层属性表中数据)
	long m_LYRID;           //含水层编号（从1开始，1表示顶层）
	long m_LYRCON;          //含水层类型标示码。每个输入数字表示一个含水层。等于0：承压含水层；等于1：非承压含水层，仅用于最上层；
	                        //等于2：导水系数为常数的承压/非承压含水层；等于3：导水系数随厚度变化的承压/非承压含水层.
	                        //对于BCF，含水层类型标识可以为0-3中的任意一个；对于LPF，只能为0和3.
	double m_LYRTRPY;       //等于-1：含水层沿列方向的饱和渗透系数异性值随每个单元变化.
	                        //大于0：含水层沿列方向的饱和渗透系数异性值全含水层为同一值，大小为输入值.
	                        //对于BCF，只能为大于0的值；对于LPF，可以为-1或大于0的值.
	long m_LYRCBD;          //仅LPF。含水层是否处理拟三维问题。0：不处理；1：处理.
	long m_LYRVKA;          //仅LPF。等于0：表示VKA参数(见<LPF网格单元属性表>)为单元的垂向渗透系数值Kz；
                            //等于1：表示VKA输入值为Kx的比例。
	long m_LYRIBS;          //含水层是否模拟夹层体贮水. 0: 不模拟；1: 模拟. 仅当SIMTYPE=1且SIMIBS=1时有效.

protected:  //运行变量
	CGridCell*** m_pGridCellAry;                  //网格单元指针数组(二维)
	double ** m_AryKav_R;                         //沿行方向两个相邻单元之间的平均渗透系数数组(L/T), 仅全有效单元法时用到(二维)
	double ** m_AryKav_C;                         //沿列方向两个相邻单元之间的平均渗透系数数组(L/T), 仅全有效单元法时用到(二维)
	long m_NumRow;                                //网格单元的行数
	long m_NumCol;                                //网格单元的列数
	long m_NumRC;                                 //含水层中网格单元的数量
	double m_StrgIn;                              //当前计算时段内含水层内单元蓄量减少(水头下降)释放的水量(L3)
	double m_StrgOut;                             //当前计算时段内含水层内单元蓄量增加(水头上升)蓄积的水量(L3)
	double m_TopIn;                               //当前计算时段内上部含水层向本含水层越流的水量(L3)
	double m_TopOut;                              //当前计算时段内本含水层向上部含水层越流的水量(L3)
	double m_BotIn;                               //当前计算时段内下部含水层向本含水层越流的水量(L3)
	double m_BotOut;                              //当前计算时段内本含水层向下部含水层越流的水量(L3)
	double m_LyrBalErr;                           //当前计算时段内的水量平衡误差(L3)
	double* m_AryLyrSinkIn;                       //当前计算时段内进入含水层的源汇项数组(L3)
	double* m_AryLyrSinkOut;                      //当前计算时段内离开含水层的源汇项数组(L3)
	CResultVal* m_ResultAry;                      //模拟输出结果存储数组
	static vector<CString> m_ItemNamAry;          //输出项名称数组
	static vector<int> m_ItemAtriAry;             //输出项性质数组
	static CMemTable* m_pMemDataTable;            //数据库水量平衡输出数据缓存表

protected: //内部功能函数
	void CalBalError();                           //计算时段内地下水平衡误差
  
public:  //功能函数
	void UpdateLyrCellStatus(double HNEW[],       //更新含水层内网格单元的水头和IBOUND值, 清零统计变量
		long IBOUND[]);           
	void CalLyrCellStgR(long IBOUND[],            //计算含水层内网格单元的蓄量变化
		double HNEW[], double HOLD[],
		double SC1[], double SC2[]);           
	void CalLyrCellFlowR(long IBOUND[],           //计算含水层内网格单元的单元间流量
		double HNEW[], double CR[], double CC[], 
		 double CV[], long iCHFlg);
	void LyrOutPut(long PrnID, long iPer,         //输出含水层水量平衡计算结果(数据库)
		long iStep, double CumTLen);                      
	void LyrOutPut(long PrnID, long iPer,         //输出含水层水量平衡计算结果(TXT)
		long iStep, double CumTLen,
		CStdioFile& OutFile);
	void OutPutLyrCellHH(long PrnID, long iPer,   //输出含水层内网格单元的水位(二进制)
		long iStep, double PerTim, double CumTLen,
		CStdioFile& OutFile);
	void OutPutLyrCellHH(long PrnID, long iPer,   //输出含水层内网格单元的水位(数据库)
		long iStep, double CumTLen);
	void OutPutLyrCellDD(long PrnID, long iPer,   //输出含水层内网格单元的水位降深(数据库)
		long iStep, double CumTLen);
	void OutPutLyrCellDD(long PrnID, long iPer,   //输出含水层内网格单元的水位降深(二进制)
		long iStep, double PerTim, double CumTLen,
		CStdioFile& OutFile);
	void OutPutLyrCellFlow(long PrnID, long iPer, //输出含水层内网格单元的流量(数据库)
		long iStep, double PerLen, double DeltT);
	void OutPutLyrCellBD(long PrnID, long iPer,   //输出含水层内网格单元的水量平衡(数据库)
		long iStep, double PerLen, double DeltT);		
	void CheckData();                             //检查和处理含水层输入数据
	long GetLyrID();                              //返回含水层的层号
	long GetLyrCon();                             //返回含水层的性质
	long GetLyrIBS();                             //返回含水层是否模拟夹层体贮水
	double GetLyrTRPY();                          //返回含水层沿列方向渗透系数与沿行方向渗流系数的比值
	long GetLyrVKA();                             //仅LPF.返回含水层所含网格单元的VKA参数含义		
	long GetLyrCBD();                             //仅LPF.返回含水层是否模拟拟三维问题
	double GetKav_R(long iRow, long iCol);        //返回第iRow行第iCol列单元与右侧单元之间的平均渗透系数(仅全有效单元法)
	double GetKav_C(long iRow, long iCol);        //返回第iRow行第iCol列单元与前侧单元之间的平均渗透系数(仅全有效单元法)
	void SaveResult(long iLyrPrint);              //保存含水层计算结果
	void SaveLyrCellResult(long CellBDPrn,        //保存含水层内网格单元的计算结果
		long CellFlowPrn);
	void InitSimulation();                        //模拟之前进行初始化工作
	static void MakeDataTable(_RecordsetPtr       //构建缓存表
		pTable, CString TbName);                         
	static void WriteDatabase();                  //写数据库
	static void SetOutputItem();                  //设置数据库输出项名称
};

//////////////////////////////////////////////////////////
//注: 以下这些公用函数都设置为内联函数以加快调用速度
inline long CGridLyr::GetLyrID()
{
	//返回含水层的层号

	return m_LYRID;
}

inline long CGridLyr::GetLyrCon()
{
	//返回含水层的性质

	return m_LYRCON;
}

inline long CGridLyr::GetLyrIBS()
{
	//返回含水层是否模拟夹层体贮水

	return m_LYRIBS;
}

inline double CGridLyr::GetLyrTRPY()
{
	//返回含水层沿列方向渗透系数与沿行方向渗流系数的比值

	return m_LYRTRPY;
}

inline long CGridLyr::GetLyrVKA()
{
	//仅LPF.返回含水层所含网格单元的VKA参数含义

	return m_LYRVKA;
}

inline long CGridLyr::GetLyrCBD()
{
	//仅LPF.返回含水层是否模拟拟三维问题

	return m_LYRCBD;
}

inline double CGridLyr::GetKav_R(long iRow, long iCol)
{
	//返回第iRow行第iCol列单元与右侧单元之间的平均渗透系数(仅全有效单元法)
	//iRow和iCol必须从1开始编号

	assert(m_AryKav_R != NULL);
	assert(iRow >= 1 && iRow <= m_NumRow);
	assert(iCol >= 1 && iCol <= m_NumCol);
	return m_AryKav_R[iRow - 1][iCol - 1];
}

inline double CGridLyr::GetKav_C(long iRow, long iCol)
{
	//返回第iRow行第iCol列单元与前侧单元之间的平均渗透系数(仅全有效单元法)
	//iRow和iCol必须从1开始编号

	assert(m_AryKav_C != NULL);
	assert(iRow >= 1 && iRow <= m_NumRow);
	assert(iCol >= 1 && iCol <= m_NumCol);
	return m_AryKav_C[iRow - 1][iCol - 1];
}