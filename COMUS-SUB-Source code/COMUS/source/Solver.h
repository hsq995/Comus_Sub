#pragma once
#include "ClassDef.h"

//////////////////////////////////////////////////////////
//矩阵方程求解方法基类声明
class CSolver
{
public:
	CSolver(void);
	virtual ~CSolver(void);

protected:
	//计算控制参数
	long m_NLyr_Def;         //含水层的层数
	long m_NRow_Def;         //网格单元的行数
	long m_NCol_Def;         //网格单元的列数
	long m_NUMNODES;         //地下水系统中网格单元的总数量,其值为m_NLyr_Def,m_NRow_Def,m_NCol_Def的乘积
	long m_NODERC;           //单个含水层内的网格单元数量, 其值为m_NRow_Def,m_NCol_Def的乘积
	long m_MaxIt;            //矩阵方程求解时最大迭代次数，一般设为200(有效取值区间：50~100000)
	double m_HClose;         //水位精度控制阈值指标(L), 一般设为0.0001m(有效取值区间：1e-8~1e-1)
	double m_RClose;         //仅对迭代解法有效, 为矩阵方程残差控制阈值指标(L3/T)，迭代误差小于该值时认为收敛
	double m_Damp;           //迭代延缓因子
	long m_Relax;            //是否启用逐单元松弛迭代算法选项. 0: 不启用; 1: 启用
	double m_Theta;          //仅IRELAX=1时有效. 为迭代计算过程出现震荡时动态松弛因子的调减系数
	double m_Gamma;          //仅IRELAX=1时有效. 为连续NITER次迭代计算不出现数值震荡时动态松弛因子的调增系数
	double m_Akappa;         //仅IRELAX=1时有效. 为连续NITER次迭代计算不出现数值震荡时动态松弛因子的单位调增量
	long m_NIter;            //仅IRELAX=1时有效. 为模型调增动态松弛因子时所需的连续无数值震荡次数
	long m_SimMthd;          //模拟方法选项. 1: 全有效单元法; 2: 原MODFLOW方法
	double m_MinHSat;        //为不可疏干单元允许的最小饱和厚度值(L)
	double* m_AryHCHGNEW;    //存放本次迭代前后水头变化的数组
	double* m_AryHCHGOLD;    //存放上次迭代前后水头变化的数组
	double* m_AryWSAVE;      //存放上次动态松弛因子的数组
	long* m_AryNONOSICUM;    //存放连续无数值震荡次数的数组
	double* m_AryBHCHG;      //保存每次迭代最大水头水头变化值的数组
	long** m_AryLHCH;        //保存每次迭代最大水头变化值单元位置(层行列号)的

	//内部函数
protected:
	virtual void AlloCateMem();                                            //开辟数组内存
	virtual void ClearMem(void);                                           //清除开辟的数组内存
	virtual void SolveMatrix(long KITER, long IBOUND[], double HNEW[],     //求解矩阵方程
		double CR[], double CC[], double CV[], double HCOF[], 
		double RHS[], double HPRE[]) = 0;
	void UnderRelax(long KITER, long IBOUND[], double HNEW[],              //仅启用了逐单元松弛迭代算法时调用
		double HPRE[]);             
	void TreatCellCantDry(long IBOUND[], double HNEW[], double HPRE[]);    //仅采用了全有效单元法时调用, 处理不可疏干单元水头
	BOOL JudgeCnvg(long KITER, long IBOUND[], double HNEW[],               //判断迭代计算是否收敛
		double HPRE[]);              

	//外部接口函数
public:
	BOOL Solve(long KITER, long IBOUND[], double HNEW[], double CR[],      //求解矩阵方程
		double CC[], double CV[], double HCOF[], double RHS[], 
		double HPRE[]);
	void Initial(long DefnLyr, long DefnRow, long DefnCol,                 //进行参数初始化
		long MaxIt, double HClose, double RClose, double Damp, long Relax,
		double Theta, double Gamma, double Akappa, long NIter, 
		long SimMthd, double Minsat);    
};

//////////////////////////////////////////////////////////
//矩阵方程强隐式解法类声明
class CSsip : public CSolver
{
public:
	CSsip(void);
	virtual ~CSsip(void);

protected:
	//内部变量
	long m_Applied;          //标明CalSeeds是否执行过的标示号
	long *** m_AryIBOUND;    //网格单元映射数组,对应solve函数的IBOUND数组
	double *** m_AryCR;      //行方向水力传导系数映射数组,对应solve函数的CR数组
	double *** m_AryCC;      //列方向水力传导系数映射数组,对应solve函数的CC数组
	double *** m_AryCV;      //垂直方向水力传导系数映射数组,对应solve函数的CV数组
	double * m_AryEL;        //存放en值的数组,见强隐式法算法说明
	double * m_AryFL;        //存放fn值的数组,见强隐式法算法说明
	double * m_AryGL;        //存放gn值的数组,见强隐式法算法说明
	double * m_AryV;         //存放vn值的数组,见强隐式法算法说明
	double m_W[5];           //种子数组(默认5个)

	//内部功能函数
protected:
	virtual void AlloCateMem();                                            //开辟数组内存
	virtual void ClearMem(void);                                           //清除开辟的数组内存
	virtual void SolveMatrix(long KITER, long IBOUND[], double HNEW[],     //求解矩阵方程
		double CR[], double CC[], double CV[], double HCOF[], 
		double RHS[], double HPRE[]);
	void CalSeeds(long IBOUND[],double CR[], double CC[], double CV[]);    //计算种子和迭代参数数组,在进入迭代过程之前调用
};

//////////////////////////////////////////////////////////
//矩阵方程预处理共轭梯度解法类声明
class CPcg : public CSolver
{
public:
	CPcg(void);
	virtual ~CPcg(void);

protected:
	//内部变量
	long m_MxInIter;         //内部最大允许迭代次数
	double * m_ArySS;        //存放当前梯度方向的数组, m_NUMNODES个元素
	double * m_AryP;         //存放下次搜索方向的数组, m_NUMNODES个元素
	double * m_AryV;         //存放预处理矩阵方程解的数组, m_NUMNODES个元素
	double * m_AryCD;        //存放预处理矩阵对角线项的数组, m_NUMNODES个元素
	double * m_AryRES;       //存放当前残差的数组, m_NUMNODES个元素
	double * m_AryHCHGN;     //存放单元水头变化的数组, m_NUMNODES个元素

	//内部功能函数
protected:
	virtual void AlloCateMem();                                            //开辟数组内存
	virtual void ClearMem(void);                                           //清除开辟的数组内存
	virtual void SolveMatrix(long KITER, long IBOUND[], double HNEW[],     //求解矩阵方程
		double CR[], double CC[], double CV[], double HCOF[],
		double RHS[], double HPRE[]);
};