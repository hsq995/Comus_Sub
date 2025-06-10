#pragma once
#include "ClassDef.h"
#include "GridCell.h"
#include "GridLyr.h"
#include "PressPeriod.h"

//////////////////////////////////////////////////////////
//地下水主模型类
class CGDWater
{
public:
	CGDWater(void);
	virtual ~CGDWater(void);

protected:  
	//模拟控制参数表数据
	long m_NUMLYR;                             //含水层的分层数
	long m_NUMROW;                             //含水层的行数
	long m_NUMCOL;                             //含水层的列数
	long m_NUMRC;                              //单层含水层中的网格单元数量
	CString m_DIMUNIT;                         //空间单位
	CString m_TIMEUNIT;                        //时间单位
	double m_XSTCORD;                          //网格左上角的X坐标
	double m_YSTCORD;                          //网格左上角的Y坐标
	long m_SIMMTHD;                            //模拟方法选项. 1: 全有效单元法; 2: 原MODFLOW方法
	long m_SIMTYPE;                            //模拟类型. 1:稳定流；2:非稳定流
	double m_LAMBDA;                           //仅SIMMTHD=1且SIMTYPE=1时有效, 为网格单元差分方程右端附加项中的阻滞系数
	long m_INTBLKM;                            //含水层类型与网格单元数据的输入格式选项. 1: BCF格式; 2: LPF格式
	long m_ISOLVE;                             //矩阵方程的求解方式选项. 1: 强隐式法(Sip); 2: 预处理共轭梯度法(PCG)
	long m_MAXIT;                              //矩阵求解时最大迭代次数, 至少设为200(有效取值区间: 200~100000)
	double m_DAMP;                             //迭代计算延缓因子，一般设为1.0(有效取值区间：0.0001~1.0)
	double m_HCLOSE;                           //水位精度控制阈值指标(L), 一般设为0.0001m(有效取值区间：1e-8~1e-1)
	double m_RCLOSE;                           //仅ISOLVE = 2时有效, 为矩阵方程残差控制阈值指标(L3/T)
	long m_IRELAX;                             //是否启用逐单元松弛迭代算法选项. 0: 不启用; 1: 启用
	double m_THETA;                            //仅IRELAX=1时有效. 为迭代计算过程出现震荡时动态松弛因子的调减系数
	double m_GAMMA;                            //仅IRELAX=1时有效. 为连续NITER次迭代计算不出现数值震荡时动态松弛因子的调增系数
	double m_AKAPPA;                           //仅IRELAX=1时有效. 为连续NITER次迭代计算不出现数值震荡时动态松弛因子的单位调增量
	long m_NITER;                              //仅IRELAX=1时有效. 为模型调增动态松弛因子时所需的连续无数值震荡次数
	double m_HNOFLO;                           //无效或不透水计算单元的水头值(一般为一个负的极大值)
	long m_ICHFLG;                             //是否计算相邻两个水头单元间流量选项。0：不计算；1：计算
	long m_IWDFLG;                             //仅SIMMTD=2时有效, 为是否模拟单元的干湿转化选项. 0: 不模拟; 1: 采用试算法模拟
	double m_WETFCT;                           //单元从疏干状态向湿润状态转化计算时试算水头的相乘因子(有效值0.0-1.0, 需大于0.0可等于1.0)
	long m_NWETIT;                             //迭代过程中每隔m_NWETIT个迭代尝试湿润疏干的单元(至少设置为间隔1次)
	long m_IHDWET;                             //单元从疏干向湿润转换时算法选项. 1：不使用指定的WETDRY数据；2：使用指定的WETDRY数据
	long m_IREGSTA;                            //是否启用分区水量平衡统计功能。0：不启用; 1：启用.需要在<水量平衡统计分区表>中输入数据.
	long m_IOBSOUT;                            //是否输出网格单元观测点模拟结果选项. 0: 不输出; 1: 输出.若选择输出, 用户需要在<网格单元观测点表>中准备数据
	//源汇项选项表数据
	long m_SIMSHB;                             //是否模拟时变水头选项. 0:不模拟; 1: 模拟
	long m_SIMRCH;                             //是否模拟面上补给选项. 0:不模拟; 1: 模拟
	long m_SIMWEL;                             //是否模拟井流选项. 0:不模拟; 1: 模拟
	long m_SIMSUB;                             //仅模拟非稳定流时有效. 为是否模拟地面沉降选项. 0:不模拟; 1: 模拟
	//模拟输出选项表数据
	long m_IGDWBDPRN;                          //地下水全系统水量平衡输出控制：0、不输出；1、逐应力期内模拟时段输出；2、逐应力期输出
	long m_ILYRBDPRN;                          //含水层水量平衡输出控制：0、不输出；1、逐应力期内模拟时段输出；2、逐应力期输出
	long m_ICELLBDPRN;                         //网格单元水量平衡输出控制：0、不输出；1、逐应力期内模拟时段输出；2、逐应力期输出
	long m_ICELLHHPRN;                         //网格单元水位输出控制：0、不输出；1、逐应力期内模拟时段输出；2、逐应力期输出
	long m_ICELLDDPRN;                         //网格单元降深输出控制：0、不输出；1、逐应力期内模拟时段输出；2、逐应力期输出
	long m_ICELLFLPRN;                         //网格单元流量输出控制：0、不输出；1、逐应力期内模拟时段输出；2、逐应力期输出
	long m_LAKBDPRN;                           //仅模拟湖泊时有效.为湖泊水量平衡输出控制选项. 0: 不输出; 1: 逐应力期内模拟时段输出; 2: 逐应力期输出
	long m_SEGMBDPRN;                          //仅模拟季节性河流时有效. 为季节性河流河道水量平衡输出控制选项. 0: 不输出; 1: 逐应力期内模拟时段输出; 2: 逐应力期输出
	long m_RECHBDPRN;                          //仅模拟季节性河流时有效. 为季节性河流河段水量平衡输出控制选项. 0: 不输出; 1: 逐应力期内模拟时段输出; 2: 逐应力期输出
	long m_IBSPRN;                             //仅模拟夹层体贮水时有效. 为夹层体模拟结果输出控制选项. 0: 不输出; 1: 逐应力期内模拟时段输出; 2: 逐应力期输出
	long m_SUBPRN;                             //仅模拟地面沉降时有效, 为网格单元地面沉降模拟结果输出控制选项. 0: 不输出; 1: 逐应力期内模拟时段输出; 2: 逐应力期输出
	long m_REGBDPRN;                           //仅启用分区水量平衡统计功能时有效. 为统计分区水量平衡输出控制选项. 0: 不输出; 1: 逐应力期内模拟时段输出; 2: 逐应力期输出

protected: //运行变量
	_ConnectionPtr m_pConnection;              //数据库连接接口指针
	_CommandPtr	m_pCommand;                    //数据库命令接口指针
	CString m_strCurrentDir;                   //当前工作目录
	CString m_strDataInDir;                    //模拟输入数据目录(TXT工作环境)
	CString m_strDataOutDir;                   //模拟结果输出目录(TXT工作环境)
	long m_DataContext;                        //模拟工作的数据环境(1表示TXT数据, 2表示sql-Server数据库)
	gcroot<String^> m_strSqlCon;               //数据库连接语句
	long m_NumNodes;                           //总的单元个数
	long m_SimIBS;                             //是否模拟夹层体贮水
	CSolver* m_pSolver;                        //矩阵方程求解对象
    CGridLyr** m_pGDLayers;                    //含水层指针数组
	long m_iPer;                               //运行期间的应力期,从1开始计数
	long m_iStep;                              //应力期内模拟时段编号,从1开始计数
	long m_iIter;                              //时段内迭代次数编号,从1开始计数
	double m_DeltT;                            //计算时间步长(T)
	double m_CumTLen;                          //从模拟开始到当前的累计计算时长(T)
	double m_PerCumDeltT;                      //从当前应力期开始到当前的累积计算时长(T)
	long* m_AryIBOUND;                         //网格单元有效性指示数组
	long* m_AryWetFlag;                        //在迭代过程中标识网格单元是否被重新湿润的标识符数组
	double m_MinHSat;                          //仅SIMMTHD=1时有效, 为不可疏干单元允许的最小饱和厚度值(L)
	double* m_AryDelR;                         //沿行方向的离散间距数组
	double* m_AryDelC;                         //沿列方向的离散间距数组
	double* m_AryTR;                           //沿行方向上的导水系数T数组
	double* m_AryCR;                           //沿行方向上的水力传导水系数数组
	double* m_AryCC;                           //沿列方向上的水力传导水系数数组
	double* m_AryCV;                           //沿垂向上的水力传导水系数数组
	double* m_AryHCOF;                         //矩阵对角线元素数组
	double* m_AryRHS;                          //矩阵右端项向量数组
	double* m_AryHNEW;                         //存放新求解水头值的数组
	double* m_AryHOLD;                         //存放旧水头值的数组
	double* m_AryHPRE;                         //存放矩阵方程上次迭代求解水头的数组
	double* m_AryCVWD;                         //仅BCF. m_AryCV的备份数组,供单元干湿转化时用
	double* m_ArySC1;                          //存放第一类贮水量的数组
	double* m_ArySC2;                          //存放第二类贮水量的数组
	long m_CnhBndID;                           //定常水头边界对应的边界/源汇项编号

protected: //内部数组
	vector<CPressPeriod*> m_pPPeriodAry;       //应力期数据数组
	vector<CCHBCell*> m_pCHBCellAry;           //定常水头网格单元数组
	vector<CBndHandler*> m_pSimBndObjAry;      //边界/源汇项对象数组(不包括定常水头边界)
	vector<CGridCell*> m_pCantDryCellAry;      //存放不可疏干单元指针的数组
	vector<CGridCell*> m_pObsCellAry;          //存放观测点网格单元指针的数组
	CResultVal* m_ResultAry;                   //模拟输出结果存储数组
	static vector<CString> m_ItemNamAry;       //输出项名称数组
	static vector<int> m_ItemAtriAry;          //输出项性质数组

public:  //多线程运算相关参数
 	long m_IMULTD;                             //是否启用多线程并行计算选项. 0: 不启用; 1: 启用
	long m_NUMTD ;                             //仅IMULTD=1时有效. 为并行计算时采用的线程数
	omp_lock_t m_GDWOmpLock;                   //并行互斥锁

protected: //运行变量
	static CMemTable* m_pMemDataTable;         //数据库输出数据缓存表
	static CMemTable* m_pMemObsCellTable;      //网格单元观测点输出数据缓存表
	CRegBudget* m_pRegBudget;                  //分区水量平衡统计对象指针
	CStdioFile m_GDWOutPutFile;                //地下水系统模拟结果输出文件(TXT)
	CStdioFile m_LyrOutPutFile;                //含水层模拟结果输出文件(TXT)
	CStdioFile m_OBSCellOutFile;               //网格单元观测点模拟结果输出文件(TXT)
	CStdioFile m_HeadOutFile;                  //水头模拟结果输出文件(TXT)
	CStdioFile m_HDownOutFile;                 //降深模拟结果输出文件(TXT)
	CStdioFile m_FLowOutFile;                  //流量模拟结果输出文件(TXT)
	CStdioFile m_CellBDOutFile;                //网格单元水量平衡分项模拟结果输出文件(TXT)

protected: //运行变量(相关平衡项)
	double m_StrgIn;                           //当前计算时段内地下水系统内单元蓄量减少(水头下降)释放的水量(L3)
	double m_StrgOut;                          //当前计算时段内地下水系统内单元蓄量增加(水头上升)蓄积的水量(L3)
	double m_StepABEr;                         //当前计算时段内的水量平衡误差(L3)
	double m_SumFlowIn;                        //从模拟开始到累计总进入地下水系统的水量(L3)
	double m_SumFlowOut;                       //从模拟开始到累计总离开地下水系统的水量(L3)
	double* m_AryGDWSinkIn;                    //当前计算时段内进入地下水系统的源汇项数组(L3)
	double* m_AryGDWSinkOut;                   //当前计算时段内离开地下水系统的源汇项数组(L3)

protected: //内部功能函数
	void BaseInit();                                        //进行基础性数据处理
	void ReadCtrPar(_RecordsetPtr& pTable);                 //读入地下水数值计算控制参数(数据库)
	void ReadCtrPar(CStdioFile& FileTxtIn,                  //读入地下水数值计算控制参数(TXT文件)
		CString FileCode);
	void CheckCtrlParData();                                //检查地下水数值计算控制参数
	void ReadPerCtrl(_RecordsetPtr& pTable);                //读入应力期控制参数(数据库)
	void ReadPerCtrl(CStdioFile& FileTxtIn);                //读入应力期控制参数(TXT文件)
	void ReadBndOpt(_RecordsetPtr& pTable);                 //读入边界条件模拟选项(数据库)
	void ReadBndOpt(CStdioFile& FileTxtIn);                 //读入边界条件模拟选项(TXT文件)
	void CheckBndOptData();                                 //检查并处理边界模拟选项数据
	void ReadPrnOpt(_RecordsetPtr& pTable);                 //读入模拟结果输出控制参数
	void ReadPrnOpt(CStdioFile& FileTxtIn);                 //读入模拟结果输出控制参数(TXT文件)
	void CheckPrnOptData();                                 //检查模拟结果输出控制参数
	void ReadDimRC(_RecordsetPtr& pTable);                  //读入沿行和沿列方向的离散间距数据(数据库)		
	void ReadDimRC(CStdioFile& FileTxtIn);                  //读入沿行或沿列方向的离散间距数据(TXT文件)	
	void ReadLyrData(_RecordsetPtr& pTable);                //读入含水层属性数据(数据库)
	void ReadLyrData(CStdioFile& FileTxtIn);                //读入含水层属性数据(TXT文件)
	void ReadCellData(_RecordsetPtr& pTable);               //读入网格单元属性数据(数据库)
	void ReadCellData(CStdioFile& FileTxtIn);               //读入网格单元属性数据(TXT文件)
	void ReadObsCellData(_RecordsetPtr& pTable);            //读入网格单元观测点数据(数据库)
	void ReadObsCellData(CStdioFile& FileTxtIn);            //读入网格单元管沉淀数据(TXT文件)
	void AddObsCell(long iLyr, long iRow, long iCol);       //添加网格单元观测点数据
	void InitSimulation();                                  //模拟之前进行初始化工作
	void SetDelt(long iStep);                               //设置计算时间步长
	void SimOneStep();                                      //进行1个时间步长的地下水数值模拟
	void CalBalError();                                     //计算水量平衡
	void SaveResult();                                      //保存计算结果
	void OutPutFlowDB();                                    //输出网格单元流量模拟结果(数据库)
	void OutPutFlowTXT();                                   //输出网格单元流量模拟结果(TXT)
	void OutPutCellBDDB();                                  //输出网格单元水量平衡模拟结果(数据库)
	void OutPutCellBDTXT();                                 //输出网格单元水量平衡模拟结果(TXT)
	void OutPutAllTXT();                                    //输出全体模拟计算结果(TXT）
	void OutPutAllDB();                                     //输出全体模拟计算结果(数据库)
	void GDWOutPutDB();                                     //输出地下水系统水量平衡结果(数据库)
	void GDWOutPutTXT();                                    //输出地下水系统水量平衡结果(TXT)
	void OutPutObsCellDB();                                 //输出网格单元观测点模拟结果(数据库)
	void OutPutObsCellTXT();                                //输出网格单元观测点模拟结果(TXT)
	void DealWithHOLD();                                    //处理HOLD值
	void InitAndCheck();                                    //进行部分数组元素初始化,并进行数据检查
	void CalCnstHoriCond(long lyrID);                       //对导水系数不变含水层计算水平向水力传导系数
	void CalVaryHoriCond(long lyrID);                       //对导水系数可变含水层计算水平向水力传导系数
	void CalCnstVertCond(long lyrID);                       //仅LPF, 对编号为lyrID的含水层计算固定的垂向水力传导系数CV
	void CalVaryVertCond(long lyrID);                       //仅LPF, 对编号为lyrID的含水层计算变化的垂向水力传导系数CV
	void DealWithSC(long lyrID);                            //若模拟非稳定流, 对编号为lyrID的含水层处理与时间有关的对角项和右端项
	void ModiRHS(long lyrID);                               //根据含水层水平向和垂向渗流情况修正矩阵方程的右端项
	void BASICFM(long KITER);                               //进行矩阵方程的基础构建(之后处理其他源汇和边界问题)
	void AddRight();                                        //全有效单元法且模拟稳定流时给网格单元方程右端增加附加项
	void UpHoriTRANS(long LyrID, long KITER);               //更新水力传导系数可变含水层(1,3)沿行方向的导水系数, 非全有效单元法时调用
	void CellBudget();                                      //计算各网格单元间渗流量、网格单元源汇项和蓄量变化情况
	void PreBudget();                                       //保存水位计算结果到网格单元, 统计变量清零
	void CellStrgBudget();                                  //计算各网格单元的蓄量变化率
	void CellFlowBudget();                                  //计算各网格单元之间的地下水渗流量
	void CHBBudget();                                       //定常水头边界渗漏/补给计算
	CString GetDbFromServer(_ConnectionPtr& pCon,           //从SQL Server中查找对应的数据库
		CString LocalDB);
	int EnableDirAccess(PCTSTR pszPath, PCTSTR pszAccount); //设置文件夹访问权限
	static void                                             
		MakeDataTable(_RecordsetPtr pTable,CString TbName); //构建缓存表              
	static void
		MakeObsCellTable(_RecordsetPtr pTable,
			CString TbName);                                //构建网格单元观测点缓存表 
	static void WriteDatabase();                            //写数据库
	static void SetOutputItem();                            //设置数据库输出项名称

public:  //外部接口函数
	void LinkDataSource(CString StrPath);                   //连接模拟数据系统
	String^ GetstrSqlCon();                                 //返回写入数据库时的连接语句
	_ConnectionPtr GetAdoDatabaseCon();                     //返回数据库连接
	void SetModOutPutItem();                                //设置模型输出项
	void ReadModDataTXT();                                  //读入模型数据(TXT)
	void ReadModDataDB();                                   //读入模型数据(数据库)
	void Simulate(CString StrPath);                         //进行地下水数值模拟
	void CheckCellLimit(long iLyr, long iRow, long iCol);   //测试单元编号是否超出范围
	void CheckCellLimit(long iRow, long iCol);              //测试单元编号是否超出范围
	CBndHandler* GetSimBndObj(CString BndObjNam);           //返回对应边界/源汇项名称的对象指针
	double GetDimCol(long iCol);                            //返回第iCol列的宽度(m)
	double GetDimRow(long iRow);                            //返回第iRow行的宽度(m)
	double GetXStCord();                                    //返回网格系统左上角X坐标
	double GetYStCord();                                    //返回网格系统左上角Y坐标
	double GetHNOFLO();                                     //返回无效或不透水计算单元的水头值
	long GetNumLyr();                                       //返回网格的层数
	long GetNumRow();                                       //返回网格的行数
	long GetNumCol();                                       //返回网格的列数
	long GetNumRC();                                        //返回单层含水层的网格单元数量
	long GetSIMMTHD();                                      //返回模拟方法选项. 1: 全有效单元法; 2: 原MODFLOW方法
	long GetIWDFLG();                                       //返回MODFLOW原算法下是否模拟单元的干湿转化选项. 0: 不模拟; 1: 试算法模拟
	long GetSimType();                                      //返回模拟类型。1:稳定流；2:非稳定流。
	long GetINTBLKM();                                      //返回单元间渗流计算的方式。1：BCF方式; 2：LPF方式
	double GetHClose();                                     //返回迭代计算收敛精度控制参数
	double GetRClose();                                     //返回迭代计算收敛残差控制参数
	double GetMinHSat();                                    //返回不可疏干单元允许的最小饱和厚度阈值(L)
	double GetDeltT();                                      //返回当前的计算时间步长
	long GetSimIBS();                                       //返回是否模拟夹层体贮水
	CString GetOutPutPath();                                //返回模拟结果输出目录
	double GetPerCumDeltT();                                //返回从当前应力期开始到当前的累积计算时长
	double GetCumTLen();                                    //返回从模拟开始到当前的累计计算时长(T)
	double GetPerLen();                                     //返回当前应力期的时长
	long GetPerID();                                        //返回当前的应力期编号
	long GetStepID();                                       //返回当前的计算时段编号
	long GetIterID();                                       //返回当前的迭代次数编号
	long GetLyrCon(long iLyr);                              //返回iLyr层的属性
	long GetDataContex();                                   //返回模拟的数据系统环境
	double GetCellArea(long iRow, long iCol);               //返回第iRow行第ICol列的单元面积
	CGridLyr* GetGridLyr(long iLyr);                        //返回第iLyr层含水层的指针
	long GetCellNode(long iLyr, long iRow,                  //返回第iLyr层第iRow行第ICol列的网格单元节点号
		long iCol);
	CGridCell* GetGridCell(long iLyr, long iRow,            //返回第iLyr层第iRow行第ICol列的网格单元指针
		long iCol); 
	long GetCellIniIBOUND(long iLyr, long iRow,             //返回第iLyr层第iRow行第ICol列的网格单元的INIIBOUND值
		long iCol);
	void SetCellIniIBound(long iLyr, long iRow,             //设置第iLyr层第iRow行第ICol列的网格单元的INIIBOUND值
		long iCol, long IBNDVal);
	long GetCellIBOUND(long iLyr, long iRow,                //返回第iLyr层第iRow行第ICol列的网格单元当前的IBOUND值
		long iCol);
	double GetCellTop(long iLyr, long iRow,                 //返回第iLyr层第iRow行第ICol列的网格单元的顶板高程值
		long iCol);
	double GetCellBot(long iLyr, long iRow,                 //返回第iLyr层第iRow行第ICol列的网格单元的底板高程值
		long iCol);
	double GetCellHNew(long iLyr, long iRow,                //返回第iLyr层第iRow行第ICol列的网格单元当前的水头值
		long iCol);
	double GetCellHOld(long iLyr, long iRow,                //返回第iLyr层第iRow行第ICol列的网格单元时段初的水头值
		long iCol);
	double GetCellHK(long iLyr, long iRow,                  //返回第iLyr层第iRow行第ICol列的网格单元沿行方向上的渗透系数值
		long iCol);
	long* GetAryIBOUND();                                   //返回IBOUND数组
	double* GetAryHPRE();                                   //返回HPRE数组
	long GetSolver();                                       //返回矩阵求解方法. 1: 强隐式法(Sip); 2: 预处理共轭梯度法(PCG)
	vector<CGridCell*>& GetCantDryCellAry();                //返回不可疏干单元指针数组
};

//////////////////////////////////////////////////////////
//注: 以下这些公用函数都设置为内联函数以加快调用速度
inline double CGDWater::GetCellArea(long iRow, long iCol)
{
	//返回第iRow行第ICol列的单元面积
	//注: iRow和iCol必须都从1开始编号

	return m_pGDLayers[0]->m_pGridCellAry[iRow - 1][iCol - 1]->m_CellArea;
}

inline double CGDWater::GetCellBot(long iLyr, long iRow, long iCol)
{
	//返回第iLyr层第iRow行第ICol列的网格单元的底板高程值
	//注: iLyr, iRow和iCol必须都从1开始编号

	return m_pGDLayers[iLyr - 1]->m_pGridCellAry[iRow - 1][iCol - 1]->m_BOT;
}

inline double CGDWater::GetCellHK(long iLyr, long iRow, long iCol)
{
	//返回第iLyr层第iRow行第ICol列的网格单元沿行方向上的渗透系数值
	//注: iLyr, iRow和iCol必须都从1开始编号

	return m_pGDLayers[iLyr - 1]->m_pGridCellAry[iRow - 1][iCol - 1]->m_HK;
}

inline long CGDWater::GetCellIniIBOUND(long iLyr, long iRow, long iCol)
{
	//返回第iLyr层第iRow行第ICol列的网格单元的初始属性值
	//注: iLyr, iRow和iCol必须都从1开始编号

	return m_pGDLayers[iLyr - 1]->m_pGridCellAry[iRow - 1][iCol - 1]->m_INIIBOUND;
}

inline long CGDWater::GetCellNode(long iLyr, long iRow, long iCol)
{
	//返回第iLyr层第iRow行第ICol列的网格单元节点号
	//注: iLyr, iRow和iCol必须都从1开始编号

	return 	(iLyr - 1) * m_NUMRC + (iRow - 1) * m_NUMCOL + (iCol - 1);
}

inline long CGDWater::GetCellIBOUND(long iLyr, long iRow, long iCol)
{
	//返回第iLyr层第iRow行第ICol列的网格单元当前的IBOUND值
	//注: iLyr, iRow和iCol必须都从1开始编号

	long node = GetCellNode(iLyr, iRow, iCol);
	return m_AryIBOUND[node];
}

inline double CGDWater::GetCellTop(long iLyr, long iRow, long iCol)
{
	//返回第iLyr层第iRow行第ICol列的网格单元的顶板高程值
	//注: iLyr, iRow和iCol必须都从1开始编号

	return m_pGDLayers[iLyr - 1]->m_pGridCellAry[iRow - 1][iCol - 1]->m_TOP;
}

inline double CGDWater::GetCellHNew(long iLyr, long iRow, long iCol)
{
	//返回第iLyr层第iRow行第ICol列的网格单元当前的水头值
	//注: iLyr, iRow和iCol必须都从1开始编号

	long node = GetCellNode(iLyr, iRow, iCol);
	return m_AryHNEW[node];
}

inline double CGDWater::GetCellHOld(long iLyr, long iRow, long iCol)
{
	//返回第iLyr层第iRow行第ICol列的网格单元时段初的水头值
	//注: iLyr, iRow和iCol必须都从1开始编号

	long node = GetCellNode(iLyr, iRow, iCol);
	return m_AryHOLD[node];
}


inline double CGDWater::GetDeltT()
{
	//返回当前的计算时间步长

	assert(m_DeltT > 1e-30);
	return m_DeltT;
}

inline double CGDWater::GetDimCol(long iCol)
{
	//返回第iCol列的宽度(m)
	//注: iCol必须从1开始编号

	assert(iCol > 0 && iCol <= m_NUMCOL);
	return m_AryDelR[iCol - 1];
}

inline double CGDWater::GetDimRow(long iRow)
{
	//返回第iRow行的宽度(m)
	//注: iRow必须从1开始编号

	assert(iRow > 0 && iRow <= m_NUMROW);
	return m_AryDelC[iRow - 1];
}

inline CGridCell* CGDWater::GetGridCell(long iLyr, long iRow, long iCol)
{
	//返回第iLyr层第iRow行第ICol列的网格单元指针
	//注: iLyr, iRow和iCol必须都从1开始编号

	return m_pGDLayers[iLyr - 1]->m_pGridCellAry[iRow - 1][iCol - 1];
}

inline CGridLyr*  CGDWater::GetGridLyr(long iLyr)
{
	//返回第iLyr层含水层的指针
	//注: iLyr必须从1开始编号

	assert(iLyr >= 1 && iLyr <= m_NUMLYR);
	return m_pGDLayers[iLyr - 1];
}

inline double CGDWater::GetHClose()
{
	//返回迭代计算收敛精度控制参数

	return m_HCLOSE;
}

inline double CGDWater::GetRClose()
{
	//返回迭代计算收敛残差控制参数

	return m_RCLOSE;
}

inline long* CGDWater::GetAryIBOUND()
{
	//返回IBOUND数组

	return m_AryIBOUND;
}

inline double* CGDWater::GetAryHPRE()
{
	//返回HPRE数组

	return m_AryHPRE;
}

inline long CGDWater::GetINTBLKM()
{
	//返回单元间渗流计算的方式。1：BCF方式; 2：LPF方式

	return m_INTBLKM;
}

inline long CGDWater::GetIWDFLG()
{
	//返回MODFLOW原算法下是否模拟单元的干湿转化选项. 0: 不模拟; 1: 试算法模拟

	return m_IWDFLG;
}

inline long CGDWater::GetSIMMTHD()
{
	//返回模拟方法选项. 1: 全有效单元法; 2: 原MODFLOW方法

	return m_SIMMTHD;
}

inline long CGDWater::GetLyrCon(long iLyr)
{
	//返回iLyr层的属性
	//注: iLyr必须从1开始编号

	assert(iLyr >= 1 && iLyr <= m_NUMLYR);
	return m_pGDLayers[iLyr - 1]->GetLyrCon();
}

inline long CGDWater::GetNumCol()
{
	//返回网格的列数

	return m_NUMCOL;
}

inline long CGDWater::GetNumLyr()
{
	//返回网格的层数

	return m_NUMLYR;
}

inline long CGDWater::GetNumRow()
{
	//返回网格的行数

	return m_NUMROW;
}

inline long CGDWater::GetNumRC()
{
	//返回单层含水层的网格单元数量

	return m_NUMRC;
}

inline double CGDWater::GetPerCumDeltT()
{
	//返回从当前应力期开始到当前的累积计算时长(T)

	return m_PerCumDeltT;
}

inline double CGDWater::GetCumTLen()
{
	//返回从模拟开始到当前的累计计算时长(T)

	return m_CumTLen;
}

inline long CGDWater::GetPerID()
{
	//返回当前的应力期编号

	return m_iPer;
}

inline double CGDWater::GetPerLen()
{
	//返回当前应力期的时长

	return m_pPPeriodAry[m_iPer - 1]->m_PeridLen;
}

inline long CGDWater::GetSimIBS()
{
	//返回是否模拟夹层体贮水

	return m_SimIBS;
}

inline long CGDWater::GetSimType()
{
	//返回模拟类型。1: 稳定流；2: 非稳定流

	return m_SIMTYPE;
}

inline long CGDWater::GetStepID()
{
	//返回当前的计算时段编号

	return m_iStep;
}

inline long CGDWater::GetIterID()
{
	//返回当前的迭代次数编号

	return m_iIter;
}

inline long CGDWater::GetDataContex()
{
	//返回模拟的数据系统环境

	return m_DataContext;
}

inline CString CGDWater::GetOutPutPath()
{
	//返回模拟的数据系统环境

	return m_strDataOutDir;
}

inline double CGDWater::GetXStCord()
{
	//返回网格系统左上角X坐标

	return m_XSTCORD;
}

inline double CGDWater::GetYStCord()
{
	//返回网格系统左上角Y坐标

	return m_YSTCORD;
}

inline double CGDWater::GetHNOFLO()
{
	//返回无效或不透水计算单元的水头值

	return m_HNOFLO;
}

inline double CGDWater::GetMinHSat()
{
	//返回无效或不透水计算单元的水头值

	return m_MinHSat;
}

inline vector<CGridCell*>& CGDWater::GetCantDryCellAry()
{
	return m_pCantDryCellAry;
}

inline long CGDWater::GetSolver()
{
	//返回矩阵求解方法. 1: 强隐式法(Sip); 2: 预处理共轭梯度法(PCG)

	return m_ISOLVE;
}

inline void CGDWater::CheckCellLimit(long iLyr, long iRow, long iCol)
{
	//测试单元编号是否超出范围
	//注: iRow和iCol必须都从1开始编号

	if (iLyr < 1 || iLyr > m_NUMLYR || iRow < 1 || iRow > m_NUMROW ||
		iCol < 1 || iCol > m_NUMCOL)
	{
		printf(_T("\n"));
		printf(_T("层号为%d, 行号为%d, 列号为%d的网格单元超出地下水系统范围, 请检查!\n"),
			iLyr, iRow, iCol);
		PauAndQuit();
	}
}

inline void CGDWater::CheckCellLimit(long iRow, long iCol)
{
	//测试单元编号是否超出范围
	//注: iRow和iCol必须都从1开始编号

	if (iRow < 1 || iRow > m_NUMROW || iCol < 1 || iCol > m_NUMCOL)
	{
		printf(_T("\n"));
		printf(_T("行号为%d, 列号为%d的网格单元超出地下水系统范围, 请检查!\n"),
			iRow, iCol);
		PauAndQuit();
	}
}