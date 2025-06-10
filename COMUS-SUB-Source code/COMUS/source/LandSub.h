#pragma once
#include "ClassDef.h"

///////////////////////////////////
//可压缩性沉积物网格单元类声明
class CCompSedmCell
{
public:
	CCompSedmCell();
	virtual ~CCompSedmCell();

protected:
	long m_IROW;                                             //网格的行号（从1开始编号）
	long m_ICOL;                                             //网格的列号（从1开始编号）
	double m_SEDMCR;                                         //网格单元处计算可压缩性沉积物弹性贮水率时所用的系数(-)
	double m_COM;                                            //网格单元处可压缩性沉积物的前期压实量(L). 用来统计总地面沉降量.
};

///////////////////////////////////
//粗颗粒沉积物网格单元类声明
class CCoGrainCell : public CCompSedmCell
{
	friend class CCGSediment;
	friend class CLandSub;

public:
	CCoGrainCell();
	virtual ~CCoGrainCell();

protected:
	double m_CGTHK;                                         //网格单元处粗颗粒沉积物当前的总厚度(L)
	double m_CGTHKOLD;                                      //网格单元处粗颗粒沉积物上个时段末的总厚度(L)
	double m_CGTHETA;                                       //网格单元处粗颗粒沉积物当前的孔隙率(即孔隙体积与总体积之比)(-)
	double m_CGTHETAOLD;                                    //网格单元处粗颗粒沉积物上个时段的孔隙率(即孔隙体积与总体积之比)(-)
	double m_CGSKE;                                         //网格单元处粗颗粒沉积物当前的弹性贮水系数(-)
public:
	void UpdatePar(long iLyr, double CGCOM);                //更新粗颗粒沉积物孔隙率和厚度
};

///////////////////////////////////
//无延迟夹层体类网格单元类声明
class CNDIntBedCell : public CCompSedmCell
{
	friend class CNDelayIntBed;
	friend class CLandSub;

public:
	CNDIntBedCell();
	virtual ~CNDIntBedCell();

protected:
	double m_IBTHK;         //网格单元处无延迟夹层体当前的总厚度(L)
	double m_IBTHKOLD;      //网格单元处无延迟夹层体上个时段末的总厚度(L)
	double m_IBTHETA;       //网格单元处无延迟夹层体当前的孔隙率(即孔隙体积与总体积之比)(-)
	double m_IBTHETAOLD;    //网格单元处无延迟夹层体上个时段末的孔隙率(即孔隙体积与总体积之比)(-)
	double m_IBCC;          //计算非弹性贮水率时所用的系数(-)
	double m_PCS;           //网格单元处无延迟夹层体的前期压缩固结应力(L), 注: 底板处
	double m_SKE;           //网格单元处无延迟夹层体当前的弹性贮水率(1/L)
	double m_SKV;           //网格单元处无延迟夹层体当前的非弹性贮率(1/L)

public:
	void UpdatePar(long iLyr, long DBedID, double IBTCOM);                  //更新夹层体各节点单元孔隙率和厚度
};

///////////////////////
//粗颗粒沉积物类声明
class CCGSediment
{
	friend class CLandSub;

public:
	CCGSediment();
	virtual ~CCGSediment();

protected:
	long m_ILYR;                                               //粗颗粒沉积物所位于的含水层层位

protected:
	CLandSub* m_pLandSub;                                      //地面沉降模拟类指针
	vector<CCoGrainCell*>m_pCGSedmCellAry;                     //粗颗粒沉积物单元属性数组
	static CMemTable* m_pMemCGDataTable;                       //粗颗粒沉积物模拟结果输出缓存表

public:
	void ReadCGCellData(_RecordsetPtr& pTable);                //读入粗颗粒沉积物单元数据(数据库)
	void ReadCGCellData(CString& strData);                     //读入粗颗粒沉积物单元数据(TXT)
	void InitSimulation();                                     //模拟之前进行初始化
	void ModifyPar();                                          //根据模拟选项修改参数
	void CGOutPut(long PrnID, long iPer,                       //输出粗颗粒沉积物模拟结果(数据库)
		long iStep, double CumTLen);
	void CGOutPut(long PrnID, long iPer,                       //输出粗颗粒沉积物模拟结果(TXT)
		long iStep, double CumTLen, CStdioFile& OutFile);
	static void                                                //构建粗颗粒沉积物模拟结果输出缓存表
		MakeCGDataTable(_RecordsetPtr pTable, CString TbName);
	static void WriteDatabase();                               //写数据库
};

///////////////////////////////////
//介质带类声明
class CMatZone
{
	friend class CDelayIntBed;
	friend class CLandSub;
	friend class CDIntBedCell;

public:
	CMatZone();
	virtual ~CMatZone();

protected:
	long m_IMZ;               //介质带的编号. (编号从1~NMZ). 
	double m_MZVK;            //介质带的垂向渗透系数(L/T)
	double m_MZCR;            //根据<地面沉降_模拟控制参数表>中CSOPT选项而定. CSOPT=1表示该参数为介质带的弹性压缩指数(-); 
	                          //CSOPT=2表示该参数为介质带的弹性贮水率(-)
	double m_MZCC;            //根据<地面沉降_模拟控制参数表>中CSOPT选项而定. CSOPT=1表示该参数为介质带的非弹性压缩指数(-);
	                          //CSOPT=2表示该参数为介质带的非弹性贮水率(-)
	double m_MZTHETA;         //介质带的初始孔隙率(即孔隙体积与总体积之比)(-)
};

///////////////////////////////////
//有延迟夹层体类网格单元类声明
class CDIntBedCell : public CCompSedmCell
{
	friend class CDelayIntBed;
	friend class CLandSub;

public:
	CDIntBedCell();
	virtual ~CDIntBedCell();

protected:
	double m_RNB;                                            //网格单元处有延迟夹层体类的等效夹层体数量(L). 该数值小于1时，认为单元处无该夹层体的分布.
	double m_DSH;                                            //网格单元处等效夹层体的初始水头(L). 该水头值将赋值给等效夹层体的各个空间离散点
	double m_DHC;                                            //网格单元处有延迟夹层体的初始预固结应力,或初始预固结应力与初始有效应力之差(以水柱高计)(L)
	double m_IBCC;                                           //计算非弹性贮水率时所用的系数(-)
	long m_IMZ;                                              //网格单元处等效夹层体对应的介质带的编号(编号从1~NMZ)
	CMatZone* m_pMatZone;                                    //网格单元处等效夹层体对应的介质带对象指针
	long m_Node;                                             //等效夹层体半厚度离散或全厚度离散时的节点单元个数;
	double m_DeltZ;                                          //等效夹层体各节点之间的的垂向离散间距(L)
	double m_KDZ;                                            //等效夹层体的垂向渗透系数除以垂向离散间距的值(1/L)
	double m_CoupItem;                                       //等效夹层体与三维地下水流方程的耦合项(L/T)
	double m_TotComChg;                                      //时段内等效夹层体的总压实/膨胀量(L) 
	double m_ElsComChgR;                                     //时段内等效夹层体的弹性压实/膨胀流量(L/T)
	double m_InElsComChgR;                                   //时段内等效夹层体的非弹性压实/膨胀流量(L/T) 
	double m_WtComChgR;                                      //时段内等效夹层体内水体的压实/膨胀流量(L/T) 
	double m_BndHead;                                        //当前求解计算夹层体水头扩散方程时所用到的边界处水头(L)
	double m_SumStaIn;                                       //等效夹层体贮水累计释放的水量(L)
	double m_SumStaOut;                                      //等效夹层体贮水累计吸收的水量(L)
	double m_SumFluxIn;                                      //从含水层累计进入等效夹层体的水量(L)
	double m_SumFluxOut;                                     //从等效夹层体累计排出给含水层的水量(L)
	double m_RERR;                                           //等效夹层体水头扩散方程模拟的相对水量平衡误差(%)
	double* m_AryHHNew;                                      //等效夹层体水流方程的当前迭代计算水头数组
	double* m_AryHHPre;                                      //等效夹层体水流方程的前次迭代计算水头数组
	double* m_AryHHOld;                                      //等效夹层体水流方程的时段初水头数组
	double* m_AryHHC;                                        //等效夹层体各离散节点的预固结应力数组
	double* m_AryHEST;                                       //等效夹层体各离散节点时段末的有效应力数组
	double* m_AryHESTOLD;                                    //等效夹层体各离散节点时段初的有效应力数组
	double* m_ArySKE;                                        //等效夹层体各离散节点处的综合弹性贮水参数数组
	double* m_ArySKV;                                        //等效夹层体各离散节点处的综合非弹性贮水参数数组
	double* m_AryDU;                                         //等效夹层体水流方程的上对角线项数组         
	double* m_AryDA;                                         //等效夹层体水流方程的主对角线项数组
	double* m_AryR;                                          //等效夹层体水流方程的右端项数组
	double* m_AryThic;                                       //等效夹层体各节点单元当前的厚度(L)
	double* m_AryThicOld;                                    //等效夹层体各节点单元上个时段末的厚度(L)
	double* m_AryTheta;                                      //等效夹层体各节点单元当前的孔隙率(L)
	double* m_AryThetaOld;                                   //等效夹层体各节点单元上个时段末的孔隙率(L)
	double* m_AryNodTCom;                                    //等效夹层体各节点单元当前的压实量(L)
	double* m_AryNodTComPre;                                 //等效夹层体各节点单元上次迭代时的压实量(L)
	double* m_AryKz;                                         //等效夹层体各节点单元单元当前的垂向渗透系数(L/T)
	double* m_AryKzOld;                                      //等效夹层体各节点单元上个时段末的垂向渗透系数(L/T)
	double* m_AryKDZ;                                        //等效夹层体各节点单元上平均垂向渗透系数除以节点间距的值(1/L)
	double* m_AryZOff;                                       //等效夹层体各节点单元中心相对于夹层体中心的距离(L)

public:
	void InitSimulation(long DISOPT, long KMOPT);            //进行模拟之前的初始化
	double SimHDDiff(long iLyr, long DBedID,                 //进行等效夹层体一维水头扩散方程的求解(全厚度离散)
		double CellHead, double DeltT, CLandSub* pLandSub);
	double SimHDDiff2(long iLyr, long DBedID,                //进行等效夹层体一维水头扩散方程的求解(半厚度离散)
		double CellHead, double DeltT, 
		CLandSub* pLandSub);
	long GauSolve(double DA[], double DU[],                  //采用高斯消元法对三对角矩阵(必须是对称阵)进行求解
		double R[], double X[], double X_1[], 
		long NumIter, double& Abs_BigHChg);
	void UpThetaAndThic(long iLyr, long DBedID);             //更新夹层体各节点单元孔隙率和厚度
	void BudgetDIBS(long iLyr, double DeltT,                 //进行水量平衡和压实量统计
		double ThicFact, long DBedID, long DISOPT,
		long KMOPT, CLandSub* pLandSub);
	void CalBalErr(double DeltT, long KMOPT);                //计算水量平衡误差(全厚度离散)
	void CalBalErr2(double DeltT, long KMOPT);               //计算水量平衡误差(半厚度离散)
	void UpdateKz();                                         //更新夹层体各节点单元的垂向渗透系数(全厚度离散)
	void UpdateKz2();                                        //更新夹层体各节点单元的垂向渗透系数(半厚度离散)
	void UpdateZOff();                                       //更新夹层体各节点单元中心点和与夹层体中心点的距离(仅全离散格式下调用)
};

///////////////////////
//无延迟夹层体类类声明
class CNDelayIntBed
{
	friend class CLandSub;

public:
	CNDelayIntBed();
	virtual ~CNDelayIntBed();

protected:
	long m_NDBedID;                                            //无延迟夹层体类的编号(从1开始)
	long m_ILYR;                                               //无延迟夹层体类所位于的含水层层位

protected:
	CLandSub* m_pLandSub;                                      //地面沉降模拟类指针
	vector<CNDIntBedCell*>m_pNDIntBedCellAry;                  //无延迟夹层体类的单元属性数组
	static CMemTable* m_pMemNDBDataTable;                      //无延迟夹层体类模拟结果输出缓存表

public:
	void ReadNDIBCellData(_RecordsetPtr& pTable);              //读入单元数据(数据库)
	void ReadNDIBCellData(CString& strData);                   //读入单元数据(TXT)
	void SetIniPar();                                          //设置初始模拟参数值
	void InitSimulation();                                     //模拟之前进行初始化
	void NDIBOutPut(long PrnID, long iPer,                     //输出无延迟夹层体类模拟结果(数据库)
		long iStep, double CumTLen);                          
	void NDIBOutPut(long PrnID, long iPer,                     //输出无延迟夹层体类模拟结果(TXT)
		long iStep, double CumTLen, CStdioFile& OutFile);
	static void                                                //构建无延迟夹层体类模拟结果输出缓存表
		MakeNDBDataTable(_RecordsetPtr pTable, CString TbName);
	static void WriteDatabase();                               //写数据库
};

///////////////////////
//有延迟夹层体类类声明
class CDelayIntBed
{
	friend class CLandSub;

public:
	CDelayIntBed();
	virtual ~CDelayIntBed();

protected:
	long m_DBedID;                                             //有延迟夹层体类的编号(从1开始)
	long m_ILYR;                                               //有延迟夹层体类所位于的含水层层位

protected:
	CLandSub* m_pLandSub;                                      //地面沉降模拟类指针
	vector<CDIntBedCell*> m_pDIntBedCellAry;                   //有延迟夹层体类的单元属性数组
	vector<CDIntBedCell*> m_ObsDIBCellAry;                     //有延迟夹层体观测点数组
	static CMemTable* m_pMemDBDataTable;                       //有延迟夹层体类模拟结果输出缓存表
	static CMemTable* m_pMemObsDIBCellTable;                   //有延迟夹层体观测点输出数据缓存表

public:
	void ReadDIBCellData(_RecordsetPtr& pTable, long Opt);     //读入单元数据(数据库)
	void ReadDIBCellData(CString& strData, long Opt);          //读入单元数据(TXT)
	void AddObsCell(long iRow, long iCol);                     //添加第iRow行第ICol列的无延迟夹层体观测点
	void InitSimulation(long Opt);                             //模拟之前进行初始化
	void SetIniPar();                                          //设置初始模拟参数值
	void DIBOutPut(long PrnID, long iPer,                      //输出有延迟夹层体模拟结果(数据库)
		long iStep, double CumTLen);                           
	void DIBOutPut(long PrnID, long iPer,                      //输出有延迟夹层体模拟结果(TXT)
		long iStep, double CumTLen, CStdioFile& OutFile);
	void ObsOutPut(long iPer, long iStep, double CumTLen);     //输出有延迟夹层体观测点模拟结果(数据库)
	void ObsOutPut(long iPer, long iStep, double CumTLen,
		CStdioFile& OutFile);                                  //输出有延迟夹层体观测点模拟结果(数据库)
	static void                                                
		MakeDBDataTable(_RecordsetPtr pTable, CString TbName); //构建有延迟夹层体类模拟结果输出缓存表
	static void
		MakeObsDIBCellTable(_RecordsetPtr pTable,
			CString TbName);                                   //构建有延迟夹层体观测点输出缓存表
	static void WriteDatabase();                               //写数据库
};

//////////////////////////////////////////////////////////
//地面沉降网格单元类声明
class CLandSubCell : public CBndCell
{
public:
	CLandSubCell();
	virtual ~CLandSubCell();

public:
	double m_STOPGL;                            //应力期初网格单元处地面以上的应力负荷(以水柱高计)(L)
	double m_ETOPGL;                            //应力期末网格单元处地面以上的应力负荷(以水柱高计)(L)
};

//////////////////////////////////////////////////////////
//地面沉降模拟类声明
class CLandSub : public CBndHandler
{
public:
	CLandSub();
	virtual ~CLandSub();

protected:  //数据表参数
	long m_NNDB;                                               //无延迟夹层体类的数量
	long m_NDB;                                                //有延迟夹层体类的数量
	double m_WSS;                                              //纯水的贮水率(1/L)
	long m_DISOPT;                                             //仅NDB>0时有效, 为等效夹层体采用的离散格式. 1: 采用全厚度离散格式; 2:采用半厚度离散格式.
	long m_NMZ;                                                //仅NDB>0时有效, 为介质带的数量
	long m_NN;                                                 //仅NDB>0时有效, 为有延迟夹层体其等效夹层体全厚度或半厚度上离散点的数量
	long m_DSHOPT;                                             //仅NDB>0时有效, 为如何确定有延迟夹层体初始水头值的选项. 
	                                                           //1: 使用<地面沉降_有延迟夹层体属性表>中的初始水头数据; 2: 使用夹层体所在含水层的初始水头数据
	long m_CSOPT;                                              //夹层体(有/无延迟均适用)贮水参数使用选项. 1: 使用贮水率; 2: 使用压缩系数
	long m_PCSOPT;                                             //夹层体(有/无延迟均适用)初始预固结应力(以水柱高计, 单位为L)输入选项. 1: 输入预固结应力的绝对数值; 
	                                                           //2: 输入预固结应力与初始有效应力(模型自动计算)之差
	long m_KMOPT;                                              //仅NDB>0时有效, 为模拟过程中是否根据形变调整夹层体的渗透系数K值选项. 0: 不调整; 1: 调整
	long m_OBSOPT;                                             //仅NDB > 0时有效, 为是否输出有延迟夹层体观测点模拟结果选项. 0: 不输出; 1: 输出.
	long m_PRNOPT;                                             //是否输出地下水网格单元内粗颗粒沉积物、有/无延迟夹层体模拟数据选项. 0: 不输出; 1: 输出

protected:  //运行变量
	long m_CGBndID;                                            //粗颗粒沉积物弹性释水源汇项统计编号
	long m_IBEBndID;                                           //夹层体弹性释水源汇项统计编号
	long m_IBVBndID;                                           //夹层体非弹性释水源汇项统计编号
	long m_WSBndID;                                            //水体弹性释水源汇项统计编号
	long m_NumLyr;                                             //地下水系统的网格层数
	long m_NumRow;                                             //地下水系统的网格行数
	long m_NumCol;                                             //地下水系统的网格列数
	long * m_AryLyrIBS;                                        //表示模型层是否模拟夹层体贮水的数组(一维)
	double** m_AryTopGL;                                       //网格单元地面以上应力负荷数组(二维)
	double*** m_ArySGM;                                        //网格单元沉积物非饱和比重数组(三维)
	double*** m_ArySGS;                                        //网格单元沉积物饱和比重数组(三维)
	double*** m_AryTotGL;                                      //网格单元上的总地应力(底板处)数组(三维)
	double*** m_AryLyrCent;                                    //网格单元的层中心位置数组(三维)
	double*** m_AryThicFact;                                   //网格单元的饱和厚度比例(三维)
	double*** m_AryEST;                                        //网格单元上当前有效应力(底板处)数组(三维)
	double*** m_AryESTOld;                                     //网格单元上时段初有效应力(底板处)数组(三维)
	double*** m_AryVertComp;                                   //各层网格单元压实量数组(三维)
	double*** m_AryVertDisp;                                   //各层网格单元垂向位移量数组(三维)
	vector<CCGSediment*> m_pCoGrainSedmAry;                    //粗颗粒沉积物指针数组
	vector<CNDelayIntBed*> m_pNDIntBedAry;                     //无延迟夹层体类指针数组
	map<long, CNDelayIntBed*> m_NDIntBetMap;                   //无延迟夹层体编号-指针查询数组
	vector<CDelayIntBed*> m_pDIntBedAry;                       //有延迟夹层体类指针数组
	map<long, CDelayIntBed*> m_DIntBetMap;                     //有延迟夹层体编号-指针查询数组
	vector<CMatZone*> m_pMatZoneAry;                           //介质带指针数组
	static CMemTable* m_pMemSubCellTable;                      //网格单元压实量输出数据缓存表
	CStdioFile m_SubOutFile;                                   //地面沉降量模拟结果输出文件(二进制)
	CStdioFile m_CGOutPutFile;                                 //粗颗粒沉积物模拟结果输出文件(TXT)
	CStdioFile m_NDBOutPutFile;                                //无延迟夹层体模拟结果输出文件(TXT)
	CStdioFile m_DBOutPutFile;                                 //有延迟夹层体模拟结果输出文件(TXT)
	CStdioFile m_OBSOutPutFile;                                //有延迟夹层体观测点模拟结果输出文件(TXT)

protected:
	void AllocMem();                                           //分配数组 
	void ReadLandSubData(_ConnectionPtr pConnection);          //读取地面沉降模拟所需数据(数据库)
	void ReadLandSubData(const CString& strFilePath);          //读取地面沉降模拟所需数据(TXT)
	void ReadCoGrainData(_ConnectionPtr pConnection);          //读取粗颗粒沉积物属性表数据(数据库)
	void ReadCoGrainData(const CString& strFilePath);          //读取粗颗粒沉积物属性表数据(TXT)
	void ReadMatParData(_ConnectionPtr pConnection);	       //读取介质带的参数数据(数据库)
	void ReadMatParData(const CString& strFilePath);	       //读取介质带的参数数据(TXT)
	void ReadSpGravData(_ConnectionPtr pConnection);	       //读取沉积物比重参数数据(数据库)
	void ReadSpGravData(const CString& strFilePath);	       //读取沉积物比重参数数据(TXT)
	void ReadIBSLyrData(_ConnectionPtr pConnection);	       //读取各个夹层体类的层位数据(数据库)
	void ReadIBSLyrData(const CString& strFilePath);	       //读取各个夹层体类的层位数据(TXT)
	void ReadIBSCellData(_ConnectionPtr pConnection);          //读取各个夹层体类的单元数据(数据库)
	void ReadIBSCellData(const CString& strFilePath);          //读取各个夹层体类的单元数据(TXT)
	void ReadObsDIBData(_ConnectionPtr pConnection);           //读取有延迟夹层体观测点数据(数据库)
	void ReadObsDIBData(const CString& strFilePath);           //读取有延迟夹层体观测点数据(TXT)
	void CalThikForCGSedm();                                   //为各层粗颗粒沉积物计算厚度
	void UpdateTopGL();                                        //更新当前应力期的地面应力负荷数据
	void CalLyrCenter();                                       //计算网格单元当前的层中心位置
	void CalThickFact();                                       //计算网格单元在时段内的平均饱和厚度比例
	void CalTotGL();                                           //计算网格单元底板处当前的总地应力
	void CalEST();                                             //计算网格单元底板处当前的有效应力
	void SaveEST();                                            //保存网格单元底板处当前的有效应力到下个时段初	
	void OutPutCellSubData();                                  //输出地面沉降模拟数据

public:
	long GetNumDivN();                                         //返回为有延迟夹层体其等效夹层体半厚度上离散点的数量
	long GetCSOPT();                                           //返回夹层体(有/无延迟均适用)贮水参数使用选项
	long GetPCSOPT();                                          //返回夹层体(有/无延迟均适用)初始预固结应力输入选项
	long GetDISOPT();                                          //返回有延迟夹层体离散格式选项
	long GetKMOPT();                                           //返回有延迟夹层体模拟中是否调整K值选项
	CMatZone* GetMatZone(long MatID);                          //返回介质带对象指针
	double GetWSS();                                           //返回
	double GetTopGL(long iRow, long iCol);                     //返回平面网格单元上的地面应力荷载数据
	double GetSGM(long iLyr, long iRow, long iCol);            //返回网格单元上的沉积物非饱和比重值
	double GetSGS(long iLyr, long iRow, long iCol);            //返回网格单元上的沉积物饱和比重值
	double GetEST(long iLyr, long iRow, long iCol);            //返回网格单元上当前的有效应力值
	double GetESTOld(long iLyr, long iRow, long iCol);         //返回网格单元上之前时段的有效应力值
	double GetLyrCent(long iLyr, long iRow, long iCol);        //返回网格单元的层中心位置
	double GetThickFact(long iLyr, long iRow, long iCol);      //返回网格单元的饱和厚度比例
	double GetTotGL(long iLyr, long iRow, long iCol);          //返回网格单元上的总地应力值
	static void
		MakeSubCellDataTable(_RecordsetPtr pTable,
			CString TbName);                                   //构建网格单元压实量输出缓存表
	static void WriteDatabase();                               //写数据库

public:
	virtual void LinkDataInOut(_ConnectionPtr pConnection,     //关联地面沉降的输入数据和输出项并检查(数据库)
		long& BndID);
	virtual void LinkDataInOut(const CString& strFilePath,     //关联地面沉降的输入数据和输出项并检查(TXT)
		long& BndID);
	virtual void AddBndSinkInfo(long& BndID);                  //添加地面沉降源汇项统计输出编号和名称
	virtual void InitSimulation();                             //模拟之前初始化
	virtual void BndOutPutDB(BOOL bEndPer, long iPer,          //输出地面沉降模拟计算结果(数据库)
		long iStep, double CumTLen, double PerLen, 
		double DeltT);
	virtual void BndOutPutTXT(BOOL bEndPer, long iPer,         //输出地面沉降模拟计算结果(TXT)
		long iStep, double CumTLen, double PerLen,
		double DeltT);
	virtual void PreStepSim(long IBOUND[], double HNEW[],      //时段模拟之前数据准备
		double HOLD[]);

public:
	virtual CBndCell* ReadPerCellData(_RecordsetPtr            //读取应力期网格单元源汇项数据(数据库)
		pCellDataRec);
	virtual CBndCell* ReadPerCellData(CString strData);        //读取应力期网格单元源汇项数据(TXT)
	virtual void FormMatrix(long IBOUND[], double HNEW[],      //根据边界条件形成/处理矩阵方程
		double HCOF[], double RHS[], double HOLD[],
		double HPRE[]);
	virtual void Budget(long IBOUND[], double HNEW[], 
		double HOLD[]);                                        //计算边界处地下水量通量
};

inline double CLandSub::GetWSS()
{
	//纯水的贮水率(1/L)
	return m_WSS;
}

inline double CLandSub::GetSGM(long iLyr, long iRow, long iCol)
{
	//返回网格单元上的沉积物非饱和比重值
	return m_ArySGM[iLyr - 1][iRow - 1][iCol - 1];
}

inline double CLandSub::GetSGS(long iLyr, long iRow, long iCol)
{
	//返回网格单元上的沉积物饱和比重值
	return m_ArySGS[iLyr - 1][iRow - 1][iCol - 1];
}

inline double CLandSub::GetTopGL(long iRow, long iCol)
{
	//返回平面网格单元上的地面应力荷载数据
	return m_AryTopGL[iRow - 1][iCol - 1];
}

inline double CLandSub::GetEST(long iLyr, long iRow, long iCol)
{
	//返回网格单元上当前的有效应力值
	return m_AryEST[iLyr - 1][iRow - 1][iCol - 1];
}

inline double CLandSub::GetESTOld(long iLyr, long iRow, long iCol)
{
	//返回网格单元上之前时段的有效应力值
	return m_AryESTOld[iLyr - 1][iRow - 1][iCol - 1];
}

inline double CLandSub::GetLyrCent(long iLyr, long iRow, long iCol)
{
	//返回网格单元的层中心位置
	return m_AryLyrCent[iLyr - 1][iRow - 1][iCol - 1];
}

inline double CLandSub::GetThickFact(long iLyr, long iRow, long iCol)
{
	//返回网格单元的饱和厚度比例
	return m_AryThicFact[iLyr - 1][iRow - 1][iCol - 1];
}

inline double CLandSub::GetTotGL(long iLyr, long iRow, long iCol)
{
	//返回网格单元的层中心位置
	return m_AryTotGL[iLyr - 1][iRow - 1][iCol - 1];
}

inline long CLandSub::GetDISOPT()
{
	//返回有延迟夹层体离散格式选项
	return m_DISOPT;
}

inline long CLandSub::GetKMOPT()
{
	//返回有延迟夹层体离散格式选项
	return m_KMOPT;
}