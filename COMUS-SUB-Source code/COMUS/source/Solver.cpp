#include "StdAfx.h"
#include "ClassInclude.h"

//////////////////////////////////////////////////////////
//矩阵方程求解方法基类函数定义
CSolver::CSolver(void)
{
	m_NLyr_Def = 0;         //含水层的层数
	m_NRow_Def = 0;         //网格单元的行数
	m_NCol_Def = 0;         //网格单元的列数
	m_NUMNODES = 0;         //地下水系统中网格单元的总数量,其值为m_NLyr_Def,m_NRow_Def,m_NCol_Def的乘积
	m_NODERC = 0;           //单个含水层内的网格单元数量, 其值为m_NRow_Def,m_NCol_Def的乘积
	m_MaxIt = 0;            //矩阵方程求解时最大迭代次数，一般设为200(有效取值区间：50~100000)
	m_HClose = 0.0;         //水位精度控制阈值指标(L)，迭代误差小于该值时认为收敛
	m_RClose = 0.0;         //仅对迭代解法有效, 为矩阵方程残差控制阈值指标(L3/T)，迭代误差小于该值时认为收敛
	m_Damp = 0.0;           //迭代延缓因子
	m_Relax = 0;            //是否启用逐单元松弛迭代算法选项. 0: 不启用; 1: 启用
	m_Theta = 0.0;          //仅IRELAX=1时有效. 为迭代计算过程出现震荡时动态松弛因子的调减系数
	m_Gamma = 0.0;          //仅IRELAX=1时有效. 为连续NITER次迭代计算不出现数值震荡时动态松弛因子的调增系数
	m_Akappa = 0.0;         //仅IRELAX=1时有效. 为连续NITER次迭代计算不出现数值震荡时动态松弛因子的单位调增量
	m_NIter = 0;            //仅IRELAX=1时有效. 为模型调增动态松弛因子时所需的连续无数值震荡次数
	m_SimMthd = 0;          //模拟方法选项. 1: 全有效单元法; 2: 原MODFLOW方法
	m_MinHSat = 0.0;        //为不可疏干单元允许的最小饱和厚度值(L)
	m_AryHCHGNEW = NULL;    //存放本次迭代前后水头变化的数组
	m_AryHCHGOLD = NULL;    //存放上次迭代前后水头变化的数组
	m_AryWSAVE = NULL;      //存放上次动态松弛因子的数组
	m_AryNONOSICUM = NULL;  //存放连续无数值震荡次数的数组
	m_AryBHCHG = NULL;      //保存每次迭代最大水头水头变化值的数组
	m_AryLHCH = NULL;       //保存每次迭代最大水头变化值单元位置(层行列号)的数组
}

CSolver::~CSolver(void)
{
	//清空分配的数组内存
	
	ClearMem();
}

void CSolver::AlloCateMem()
{
	//开辟数组内存

	int i;
	try
	{
		//开辟一维数组
		if (m_Relax == 1)
		{
			//如果采用自动松弛系数法
			m_AryHCHGNEW = new double[m_NUMNODES]();
			m_AryHCHGOLD = new double[m_NUMNODES]();
			m_AryWSAVE = new double[m_NUMNODES]();
			m_AryNONOSICUM = new long[m_NUMNODES]();
		}
		m_AryBHCHG = new double[m_MaxIt]();
		//开辟二维数组
		m_AryLHCH = new long*[m_MaxIt]();
		for (i = 0; i < m_MaxIt; i++)
		{
			m_AryLHCH[i] = new long[3]();
		}
	}
	catch (bad_alloc &e)
	{
		printf(e.what());
		exit(1);
	}
}

void CSolver::ClearMem(void)
{
	//该函数清空之前分配的内存

	//删除一维数组
	if (m_AryHCHGNEW != NULL)
	{
		delete[] m_AryHCHGNEW;
		m_AryHCHGNEW = NULL;
	}
	if (m_AryHCHGOLD != NULL)
	{
		delete[] m_AryHCHGOLD;
		m_AryHCHGOLD = NULL;
	}
	if (m_AryWSAVE != NULL)
	{
		delete[] m_AryWSAVE;
		m_AryWSAVE = NULL;
	}
	if (m_AryNONOSICUM != NULL)
	{
		delete[] m_AryNONOSICUM;
		m_AryNONOSICUM = NULL;
	}
	if (m_AryBHCHG != NULL)
	{
		delete[] m_AryBHCHG;
		m_AryBHCHG = NULL;
	}

	//删除二维数组
	long i;
	if (m_AryLHCH != NULL)
	{
		for (i = 0; i < m_MaxIt; i++)
		{
			delete[] m_AryLHCH[i];
		}
		delete[] m_AryLHCH;
		m_AryLHCH = NULL;
	}
}

void CSolver::Initial(long DefnLyr, long DefnRow, long DefnCol, long MaxIt, double HClose,
	double RClose, double Damp, long Relax, double Theta, double Gamma, double Akappa, 
	long NIter, long SimMthd, double Minsat)
{
	//参数说明
	//DefnLyr:     含水层的层数
	//DefnRow:     网格单元的行数
	//DefnCol:     网格单元的列数
	//MaxIt:       外循环最大允许迭代次数
	//HClose:      水位收敛控制阈值(L)
	//RCLose:      网格单元水量平衡方程残差控制阈值(L3/T)
	//Damp:        全局松弛因子(0.0~1.0), 1.0表示不松弛, 越小松弛程度越大
	//Relax:       逐单元松弛选项标识. 0:不进行逐单元松弛; 1:进行逐单元松弛
	//Theta:       仅Relax=1时有效, 其它情况输入0. 为逐单元松弛时松弛因子的调减系数(0.35~0.95)
	//Gamma:       仅Relax=1时有效, 其它情况输入0. 为逐单元松弛时的惯性量评价系数(1.0~1.5)
	//Akappa:      仅Relax=1时有效, 其它情况输入0. 为逐单元松弛时松弛因子的单位调增量(0.0~0.3)
	//NIter:       仅Relax=1时有效, 其它情况输入0. 为逐单元松弛时调增松弛因子所需的累积无震荡次数阈值
	//SimMthd:     为模拟算法选项. 1:全有效单元法; 2:原MODFLOW算法
	//Minsat:      仅SimMthd=1时有效, 其它情况输入0. 为全有效单元法时不可疏干单元允许的最小饱和厚度(L)

	//先清空已分配的数组(如果之前初始化过的话)
	ClearMem();

	//设置主要计算参数
	assert(DefnLyr>0 && DefnRow>0 && DefnCol>0);
	m_NLyr_Def = DefnLyr;
	m_NRow_Def = DefnRow;
	m_NCol_Def = DefnCol;
	m_NODERC = DefnRow * DefnCol;
	m_NUMNODES = DefnLyr * m_NODERC;

	//设置最大允许迭代次数
	m_MaxIt = MaxIt;
	//设置水头值迭代计算控制精度
	m_HClose = HClose;
	//设置残差计算控制精度
	m_RClose = RClose;
	//设置迭代计算延缓因子
	m_Damp = Damp;
	//设置逐单元松弛选项
	m_Relax = Relax;
	if (m_Relax == 1)
	{
		//设置逐单元松弛时的调减系数
		m_Theta = Theta;
		//设置逐单元松弛时的调增系数
		m_Gamma = Gamma;
		//设置逐单元松弛时的单位调增量
		m_Akappa = Akappa;
		//设置逐单元松弛时的无震荡次数控制参数
		m_NIter = NIter;
	}
	//设置模拟方法
	m_SimMthd = SimMthd;
	if (m_SimMthd == 1)
	{
		//设置不可疏干单元的最小饱和厚度
		m_MinHSat = Minsat;
	}
	//开辟计算所需的数组
	AlloCateMem();
}

BOOL CSolver::Solve(long KITER, long IBOUND[], double HNEW[], double CR[], 
	double CC[], double CV[], double HCOF[], double RHS[], double HPRE[])
{
	//该函数求解方程组, 仅进行1次迭代处理, 如此次迭代收敛返回TRUE, 否则返回FALSE
	//求解结果输出到数组HNEW，其他数组保持不变

	//参数说明
	//KITER:  当前外循环迭代次数(从1开始)
	//IBOUND: 网格有效单元标示数组
	//HNEW:   当前迭代计算时的单元水头值数组
	//HPRE:   前1次迭代计算时单元水头值数组
	//CR:     单元行方向水力传导系数数组
	//CC:     单元列方向水力传导系数数组
	//CV:     单元层方向水力传导系数数组
	//HCOF:   中央对角线数组
	//RHS:    右端项数组
	
	//1. 保存迭代计算初始时的水头值
	CopyMemory(HPRE, HNEW, sizeof(double) * m_NUMNODES);

	//2. 对矩阵方程进行求解
	SolveMatrix(KITER, IBOUND, HNEW, CR, CC, CV, HCOF, RHS, HPRE);

	//3. 如果启用了逐单元松弛迭代算法, 进行逐单元松弛
	if (m_Relax == 1)
	{
		UnderRelax(KITER, IBOUND, HNEW, HPRE);
	}

	//4. 如果采用全有效单元法, 对不可疏干单元的水位进行处理 
	if (m_SimMthd == 1)
	{
		TreatCellCantDry(IBOUND, HNEW, HPRE);
	}

	//5. 判断迭代计算是否收敛
	return JudgeCnvg(KITER, IBOUND, HNEW, HPRE);
}

void CSolver::UnderRelax(long KITER, long IBOUND[], double HNEW[], double HPRE[])
{
	//对当前迭代计算的水头值进行松弛

	//该函数仅在启用了逐单元松弛迭代算法时调用
	if (m_Relax != 1)
	{
		return;
	}

	long NumTask = long(m_NODERC / GDWMOD.m_NUMTD / 2.0) + 1;
#pragma omp parallel for num_threads(GDWMOD.m_NUMTD) schedule(dynamic,NumTask)
	for (long iCell = 0; iCell < m_NUMNODES; iCell++)
	{
		//注释: 以下几个变量的定义必须放在for块中不能共用, 以适应并行运算要求
		double WW;
		//注释完毕
		if (IBOUND[iCell] > 0)
		{
			//如果网格单元有效, 评估并保存当前迭代计算后的水头变化
			m_AryHCHGNEW[iCell] = HNEW[iCell] - HPRE[iCell];
			if (KITER == 1)
			{
				//如果是模拟时段的首次迭代计算
				m_AryHCHGOLD[iCell] = m_AryHCHGNEW[iCell];
				m_AryWSAVE[iCell] = 1.0;
				m_AryNONOSICUM[iCell] = 0;
			}
			WW = m_AryWSAVE[iCell];
			if (m_AryHCHGNEW[iCell] * m_AryHCHGOLD[iCell] < 0.0)
			{
				//本次模拟该网格单元的水头模拟值有震荡
				if (fabs(m_AryHCHGNEW[iCell]) > min(1E-6, m_HClose))
				{
					//如果当前水位变化幅度高于限值, 调小松弛因子
					WW = m_Theta * WW;
				}
				//清零迭代计算没有发生震荡的累计次数
				m_AryNONOSICUM[iCell] = 0;
			}
			else
			{
				//本次模拟该网格单元的水头模拟值没有震荡
				//累计迭代计算没有发生震荡的次数
				m_AryNONOSICUM[iCell] = m_AryNONOSICUM[iCell] + 1;
				if (m_AryNONOSICUM[iCell] > m_NIter)
				{
					//如果累计没有震荡的次数大于m_NITER次, 调大松弛因子
					WW = m_Gamma * WW + m_Akappa;
					//清零迭代计算没有发生震荡的累计次数
					m_AryNONOSICUM[iCell] = 0;
				}
			}
			//动态松弛因子最小不能低于AKAPPA
			WW = max(WW, m_Akappa);
			//动态松弛因子最大不能超过1.0
			WW = min(WW, 1.0);
			m_AryWSAVE[iCell] = WW;
			//计算松弛后的水头值
			HNEW[iCell] = HPRE[iCell] + WW * m_AryHCHGNEW[iCell];
			//保存当前水头变化值到前一次变化值
			m_AryHCHGOLD[iCell] = HNEW[iCell] - HPRE[iCell];
		}
	}
}

void CSolver::TreatCellCantDry(long IBOUND[], double HNEW[], double HPRE[])
{
	//处理不可疏干单元水头

	//该函数仅在采用了全有效单元法时调用
	if (m_SimMthd != 1)
	{
		return;
	}

	//从地下水对象得到不可疏干单元数组
	vector<CGridCell*>& CantDryCellAry = GDWMOD.GetCantDryCellAry();
	//得到不可疏干单元的数量
	long NCantDryCell = (long)CantDryCellAry.size();
	long NumTask = long(NCantDryCell / GDWMOD.m_NUMTD / 2.0) + 1;
#pragma omp parallel for num_threads(GDWMOD.m_NUMTD) schedule(dynamic,NumTask)
	for (long iCell = 0; iCell < NCantDryCell; iCell++)
	{
		//注释: 以下几个变量的定义必须放在for块中不能共用, 以适应并行运算要求
		long Node;
		CGridCell* pGridCell = NULL;
		double ThickNow, ThickPre, CellBot, Alpha;
		//注释完毕
		//确定网格单元的指针及其编号
		pGridCell = CantDryCellAry[iCell];
		Node = GDWMOD.GetCellNode(pGridCell->GetCellLyr(), pGridCell->GetCellRow(), pGridCell->GetCellCol());
		if (IBOUND[Node] > 0)
		{
			//如果网格单元有效
			//计算当前迭代计算前和后的饱和厚度
			CellBot = pGridCell->GetCellBot();
			ThickNow = HNEW[Node] - CellBot;
			ThickPre = HPRE[Node] - CellBot;
			if (ThickNow < 0.0)
			{
				//如果网格单元进入疏干状态(当前饱和厚度小于0.0)
				Alpha = min(1.0 - ThickPre / (ThickPre - ThickNow), 0.6);
				HNEW[Node] = CellBot + Alpha * ThickPre;
			}
			//网格单元的水头不能太接近底板高程
			HNEW[Node] = max(HNEW[Node], CellBot + m_MinHSat);
			if (m_Relax == 1)
			{
				//如果采用逐单元松弛迭代法, 保存当前水头变化值到前一次变化值
				m_AryHCHGOLD[Node] = HNEW[Node] - HPRE[Node];
			}
		}
	}
}

BOOL CSolver::JudgeCnvg(long KITER, long IBOUND[], double HNEW[], double HPRE[])
{
	//判断是否迭代收敛

	long i, j, k, node;
	double HCHG, Abs_HCHG;
	double Abs_BIGHCHG = 0.0;
	long KB = 0;
	long IB = 0;
	long JB = 0;
	double BIGHCHG = 0.0;
	//找到本次迭代过程中水头变化最大的单元
	//注: 本循环不可使用并行算法
	for (k = 0; k < m_NLyr_Def; k++)
	{
		for (i = 0; i < m_NRow_Def; i++)
		{
			for (j = 0; j < m_NCol_Def; j++)
			{
				node = k * m_NODERC + i * m_NCol_Def + j;
				if (IBOUND[node] > 0)
				{
					//计算绝对水头变化
					HCHG = HNEW[node] - HPRE[node];
					Abs_HCHG = fabs(HCHG);
					//记录最大水头变化及其三元坐标
					if (Abs_HCHG > Abs_BIGHCHG)
					{
						Abs_BIGHCHG = Abs_HCHG;
						BIGHCHG = HCHG;
						KB = k + 1;
						IB = i + 1;
						JB = j + 1;
					}
				}
			}
		}
	}

	//保存本次迭代过程中水头变化最大的单元
	m_AryBHCHG[KITER - 1] = BIGHCHG;
	m_AryLHCH[KITER - 1][0] = KB;
	m_AryLHCH[KITER - 1][1] = IB;
	m_AryLHCH[KITER - 1][2] = JB;

	//输出迭代信息
	printf(_T("Iteration %7d:   Max head change = %15.8e   [K,I,J]  %4d %4d %4d\n"),
		KITER, BIGHCHG, KB, IB, JB);

	//判断当前计算时段是否收敛
	BOOL bCNVG = FALSE;
	if (Abs_BIGHCHG <= m_HClose)
	{
		//外部循环迭代收敛
		bCNVG = TRUE;
	}
	else
	{
		if (KITER >= m_MaxIt)
		{
			printf(_T("\n"));
			printf(_T("允许的最大迭代次数为: MXITER  =  %d\n"), m_MaxIt);
			printf(_T("计算超过最大迭代次数还未收敛,请检查!\n"));
			printf(_T("水头变化最大位置为: 第%d 层第%d 行第%d 列\n"), KB, IB, JB);
			printf(_T("水头变化值为: %e\n"), BIGHCHG);
			PauAndQuit();
		}
	}

	return bCNVG;
}

//////////////////////////////////////////////////////////
//矩阵方程强隐式解法类函数定义
CSsip::CSsip(void)
{
	//参数和数组归零
	m_Applied = 0;          //标明CalSeeds是否执行过的标示号
	m_AryIBOUND = NULL;     //网格单元映射数组,对应solve函数的IBOUND数组
	m_AryCR = NULL;         //行方向水力传导系数映射数组,对应solve函数的CR数组
	m_AryCC = NULL;         //列方向水力传导系数映射数组,对应solve函数的CC数组
	m_AryCV = NULL;         //垂直方向水力传导系数映射数组,对应solve函数的CV数组
	m_AryEL = NULL;         //存放en值的数组,见强隐式法算法说明
	m_AryFL = NULL;         //存放fn值的数组,见强隐式法算法说明
	m_AryGL = NULL;         //存放gn值的数组,见强隐式法算法说明
	m_AryV = NULL;          //存放vn值的数组,见强隐式法算法说明
	ZeroMemory(m_W, sizeof(double) * 5);
}

CSsip::~CSsip(void)
{
	//清空分配的数组内存

	ClearMem();
}

void CSsip::AlloCateMem()
{
	//开辟数组内存

	//开辟父类所需数组
	CSolver::AlloCateMem();

	//自身开辟所需数组
	try
	{
		m_AryEL = new double[m_NUMNODES]();
		m_AryFL = new double[m_NUMNODES]();
		m_AryGL = new double[m_NUMNODES]();
		m_AryV = new double[m_NUMNODES]();
		ZeroMemory(m_W, sizeof(double) * 5);

		//开辟映射数组
		//开辟层映射数组，层数为DefnLyr
		m_AryIBOUND = new long **[m_NLyr_Def]();
		m_AryCR = new double **[m_NLyr_Def]();
		m_AryCC = new double **[m_NLyr_Def]();
		m_AryCV = new double **[m_NLyr_Def]();

		//对每层开辟行映射数组，每层的行数为DefnRow
		long k;
		for (k = 0; k < m_NLyr_Def; k++)
		{
			m_AryIBOUND[k] = new long*[m_NRow_Def]();
			m_AryCR[k] = new double*[m_NRow_Def]();
			m_AryCC[k] = new double*[m_NRow_Def]();
			m_AryCV[k] = new double*[m_NRow_Def]();
		}
	}
	catch (bad_alloc &e)
	{
		printf(e.what());
		exit(1);
	}
}

void CSsip::ClearMem(void)
{
	//该函数清空之前分配的内存

	//清空父类开辟的数组
	CSolver::ClearMem();

	//删除自身开辟的数组内存
	//1. 删除映射数组
	long k;
	if (m_AryIBOUND != NULL) 
	{ 
		for (k = 0; k < m_NLyr_Def; k++)
		{
			delete[] m_AryIBOUND[k];
		}
		delete[] m_AryIBOUND; 
		m_AryIBOUND = NULL;
	}
	if (m_AryCR != NULL) 
	{ 
		for (k = 0; k < m_NLyr_Def; k++)
		{
			delete[] m_AryCR[k];
		}
		delete[] m_AryCR; 
		m_AryCR = NULL; 
	}
	if (m_AryCC != NULL) 
	{ 
		for (k = 0; k < m_NLyr_Def; k++)
		{
			delete[] m_AryCC[k];
		}
		delete[] m_AryCC; 
		m_AryCC = NULL;
	}
	if (m_AryCV != NULL) 
	{ 
		for (k = 0; k < m_NLyr_Def; k++)
		{
			delete[] m_AryCV[k];
		}
		delete[] m_AryCV;
		m_AryCV = NULL; 
	}

	//2. 删除计算数组
	if (m_AryEL != NULL) 
	{ 
		delete[] m_AryEL; 
		m_AryEL = NULL; 
	}
	if (m_AryFL != NULL) 
	{ 
		delete[] m_AryFL; 
		m_AryFL = NULL; 
	}
	if (m_AryGL != NULL)
	{ 
		delete[] m_AryGL; 
		m_AryGL = NULL; 
	}
	if (m_AryV != NULL) 
	{ 
		delete[] m_AryV; 
		m_AryV = NULL; 
	}
}

void CSsip::CalSeeds(long IBOUND[],double CR[], double CC[], double CV[])
					 
{
	//计算种子并设置迭代参数数组m_W
	//不修改任何传入的数据,输出m_W
	//该函数仅需执行一次

	if(m_Applied == 0)    //若该函数未执行过
	{
		m_Applied = 1;    //将其改为1标明执行过了
	}
	else
	{
		return;           //该函数已经执行过了,直接退出
	}

	//映射传进来的数组
	long k, i, j, ID = 0;
	for(k=0 ; k<m_NLyr_Def; k++) 
	{
		for(i=0; i<m_NRow_Def; i++)
		{
			ID = k * m_NRow_Def * m_NCol_Def + i * m_NCol_Def; 
			m_AryIBOUND[k][i] = &IBOUND[ID];  
			m_AryCR[k][i] = &CR[ID];  
		    m_AryCC[k][i] = &CC[ID];  
			m_AryCV[k][i] = &CV[ID];  
		}
	}

	double WMINMN = 1.0;           //全网格单元的最小种子
    long NODES= 0;                 //变水头网格单元数
    double AVGSUM = 0.0;           //平均种子值

    double CCOL=9.86960440109 / (2.0*m_NCol_Def*m_NCol_Def);
    double CROW=9.86960440109 / (2.0*m_NRow_Def*m_NRow_Def);
    double CLAY=9.86960440109 / (2.0*m_NLyr_Def*m_NLyr_Def);

	double dd = 0.0;
	double ff = 0.0;
	double bb = 0.0;
	double hh = 0.0;
	double zz = 0.0;
	double ss = 0.0;
	for(k=0; k<m_NLyr_Def; k++)
	{
		for(i=0; i<m_NRow_Def; i++)
		{
			for(j=0; j<m_NCol_Def; j++)
			{
				if(m_AryIBOUND[k][i][j] <= 0)
				{
					continue;
				}
				else
				{
					//获得与该单元与相邻的6个单元之间的水力传导系数
				    dd = 0.0;
				    ff = 0.0;
				    bb = 0.0;
				    hh = 0.0;
				    zz = 0.0;
				    ss = 0.0;
					if(j != 0) dd = m_AryCR[k][i][j-1];
					if(j != m_NCol_Def-1) ff = m_AryCR[k][i][j];
					if(i != 0) bb = m_AryCC[k][i-1][j];
					if(i != m_NRow_Def-1) hh = m_AryCC[k][i][j];
					if(k != 0) zz = m_AryCV[k-1][i][j];
					if(k != m_NLyr_Def-1) ss = m_AryCV[k][i][j];

					//在每个坐标方向上确定最大和最小的水力传导系数
					double dfmax = max(dd, ff);
					double bhmax = max(bb, hh);
					double zsmax = max(zz, ss);
					double dfmin = min(dd, ff);
					double bhmin = min(bb, hh);
					double zsmin = min(zz, ss);
					if(fabs(dfmin) < 1e-30) dfmin = dfmax;
					if(fabs(bhmin) < 1e-30) bhmin = bhmax;
					if(fabs(zsmin) < 1e-30) zsmin = zsmax;

					//对每个坐标方向计算一个种子
					double wCol = 1.0;
					if(fabs(dfmin) > 1e-30) wCol = CCOL / (1.0 + (bhmax + zsmax) / dfmin);
					double wRow = 1.0;
					if(fabs(bhmin) > 1e-30) wRow = CROW / (1.0 + (dfmax + zsmax) / bhmin);
					double wLay = 1.0;
					if(fabs(zsmin) > 1e-30) wLay = CLAY / (1.0 + (dfmax + bhmax) / zsmin);

					//选择该单元的种子(三个方向中最小的那个)
					double wMin = min(wLay, min(wCol,wRow));
					//调整全网格单元的最小种子
					WMINMN = min(WMINMN, wMin);

					//对计算的种子进行累加以得到平均种子值
					AVGSUM = AVGSUM + wMin;
					NODES = NODES + 1;
				}
			}
		}
	}

	//计算平均种子值
	double AVGMIN;
	if (NODES > 0)
	{
		AVGMIN = AVGSUM / NODES;
	}
	else
	{
		AVGMIN = 0.0001;
	}

	//从平均种子值计算迭代参数
	double p1 = -1.0;
	double p2 = 4.0;
	for(i=0; i<5; i++)
	{
		p1 = p1 +1.0;
		m_W[i] = 1.0 - pow(AVGMIN, p1/p2);
	}

	//删除映射数组
	//1. 删除映射数组
	if (m_AryIBOUND != NULL)
	{
		for (k = 0; k < m_NLyr_Def; k++)
		{
			delete[] m_AryIBOUND[k];
		}
		delete[] m_AryIBOUND;
		m_AryIBOUND = NULL;
	}
	if (m_AryCR != NULL)
	{
		for (k = 0; k < m_NLyr_Def; k++)
		{
			delete[] m_AryCR[k];
		}
		delete[] m_AryCR;
		m_AryCR = NULL;
	}
	if (m_AryCC != NULL)
	{
		for (k = 0; k < m_NLyr_Def; k++)
		{
			delete[] m_AryCC[k];
		}
		delete[] m_AryCC;
		m_AryCC = NULL;
	}
	if (m_AryCV != NULL)
	{
		for (k = 0; k < m_NLyr_Def; k++)
		{
			delete[] m_AryCV[k];
		}
		delete[] m_AryCV;
		m_AryCV = NULL;
	}
}

void CSsip::SolveMatrix(long KITER, long IBOUND[], double HNEW[], double CR[], 
	 double CC[], double CV[], double HCOF[], double RHS[], double HPRE[])
{
	//该函数求解方程组, 仅进行1次迭代处理, 如此次迭代收敛返回TRUE, 否则返回FALSE
	//求解结果输出到数组HNEW，其他数组保持不变

	//参数说明
	//KITER: 当前迭代次数(从1开始)
    //IBOUND: 网格有效单元标示数组
    //HNEW: 水头数组
    //CR: 单元行方向水力传导系数数组
    //CC: 单元列方向水力传导系数数组
    //CV: 单元层方向水力传导系数数组
    //HCOF: 中央对角线数组
    //RHS: 右端项数组

	//如果是第1次执行, 计算迭代种子
	if (m_Applied == 0)
	{
		CalSeeds(IBOUND, CR, CC, CV);
	}

	//对迭代过程中的常量进行赋值
	long NRC = m_NRow_Def * m_NCol_Def;
	long NTH = (KITER - 1) % 5;
	double DITPAR = m_W[NTH];         //种子参数
	double BIGG = 0.0;                //迭代过程中最大的计算单元水头变化量

	//清空计算数组
	ZeroMemory(m_AryEL,sizeof(double)*m_NUMNODES);
	ZeroMemory(m_AryFL,sizeof(double)*m_NUMNODES);
	ZeroMemory(m_AryGL,sizeof(double)*m_NUMNODES);
	ZeroMemory(m_AryV,sizeof(double)*m_NUMNODES);

	//设置正向/反向迭代标志位(1/-1),并且根据迭代方向计算单元标号
	long IDIR = 1;
	if(KITER % 2 == 0) IDIR = -1;
	long IDNRC=IDIR * NRC;
    long IDNCOL=IDIR * m_NCol_Def;          
		 
	long kk,ii,jj;
    long NN,NRN,NRL,NCN,NCL,NLN,NLL;
	long NCF,NCD,NRB,NRH,NLS,NLZ;
	double BB,ELNRL,FLNRL,GLNRL,BHNEW,VNRL;
	double HH,HHNEW;
	double DD,ELNCL,FLNCL,GLNCL,DHNEW,VNCL;
	double FF,FHNEW;
	double ZZ,ELNLL,FLNLL,GLNLL,ZHNEW,VNLL;
	double SS,SHNEW;
	double EE;
	double AL,BL,CL,AP,CP,GP,RP,TP,UP,HHCOF,DL;
	double RRHS,RES;
	long NC,NR,NL;
	double ELXI,FLXI,GLXI;
	long k,i,j;
	//进行系数矩阵分解
	for(k=0; k<m_NLyr_Def; k++)
	{
		for(i=0; i<m_NRow_Def; i++)
		{
			for(j=0; j<m_NCol_Def; j++)
			{
				//设置当前单元的下标，取决于迭代方向
				if(IDIR > 0)
				{
					kk = k;
					ii = i;
					jj = j;
				}
				else
				{
					kk = m_NLyr_Def - k - 1;
					ii = m_NRow_Def - i - 1;
					jj = j;
				}

				//从三元下标计算单一下标
				NN = jj + ii * m_NCol_Def + kk * NRC;
				if(IBOUND[NN] > 0)
				{
					//确定与该单元相邻的六个单元的单一下标
                    NRN=NN + IDNCOL;      //正行方向(即沿着迭代进行的方向)
                    NRL=NN - IDNCOL;      //反行方向
                    NCN=NN + 1;           //正列方向
                    NCL=NN - 1;           //反列方向
                    NLN=NN + IDNRC;       //正下方向
                    NLL=NN - IDNRC;       //反下方向

					//确定该单元与相邻六个单元之间水力传导系数的单一下标,与迭代进行方向有关
					if(IDIR > 0)
					{
                        NCF=NN;           //正列方向水力传导系数标号
                        NCD=NCL;          //反列方向水力传导系数标号
                        NRB=NRL;          //反行方向水力传导系数标号
                        NRH=NN;           //正行方向水力传导系数标号
                        NLS=NN;           //正下方向水力传导系数标号
                        NLZ=NLL;          //反下方向水力传导系数标号
					}
					else
					{
                        NCF=NN;
                        NCD=NCL;
                        NRB=NN;
                        NRH=NRN;
                        NLS=NLN;
                        NLZ=NN;
					}

					//对涉及相邻单元的矩阵元素赋值
					//相邻单元为向后一行
                    BB = 0.0;
                    ELNRL = 0.0;
                    FLNRL = 0.0;
                    GLNRL = 0.0;
                    BHNEW = 0.0;
                    VNRL = 0.0;
					if(i != 0)
					{
					    BB = CC[NRB];
                        ELNRL = m_AryEL[NRL];
                        FLNRL = m_AryFL[NRL];
                        GLNRL = m_AryGL[NRL];
                        BHNEW = BB * HNEW[NRL];
                        VNRL = m_AryV[NRL];
					}

					//相邻单元为向前一行
					HH = 0.0;
					HHNEW = 0.0;
					if(i != m_NRow_Def-1)
					{
						HH = CC[NRH];
						HHNEW = HH * HNEW[NRN];
					}

					//相邻单元为向后一列
                    DD = 0.0;
                    ELNCL = 0.0;
                    FLNCL = 0.0;
                    GLNCL = 0.0;
                    DHNEW = 0.0;
                    VNCL = 0.0;
				    if(j != 0)
					{
                        DD = CR[NCD];
                        ELNCL = m_AryEL[NCL];
                        FLNCL = m_AryFL[NCL];
                        GLNCL = m_AryGL[NCL];
                        DHNEW = DD * HNEW[NCL];
                        VNCL = m_AryV[NCL];
					}

					//相邻单元为向前一列
                    FF = 0.0;
                    FHNEW = 0.0;
					if(j != m_NCol_Def-1)
					{
						FF = CR[NCF];
						FHNEW = FF * HNEW[NCN];
					}

					//相邻单元为向后一层
					ZZ = 0.0;
                    ELNLL = 0.0;
                    FLNLL = 0.0;
                    GLNLL = 0.0;
                    ZHNEW = 0.0;
                    VNLL = 0.0;
					if(k != 0)
					{
                        ZZ = CV[NLZ];
                        ELNLL = m_AryEL[NLL];
                        FLNLL = m_AryFL[NLL];
                        GLNLL = m_AryGL[NLL];
                        ZHNEW = ZZ * HNEW[NLL];
                        VNLL = m_AryV[NLL];
					}

					//相邻单元为向前一层
                    SS = 0.0;
                    SHNEW = 0.0;
					if(k != m_NLyr_Def-1)
					{
                        SS = CV[NLS];
                        SHNEW = SS * HNEW[NLN];
					}

					//统计所有六个相邻单元的水力传导系数
					EE = -ZZ-BB-DD-FF-HH-SS;

					//计算上三角矩阵和下三角矩阵的元素，
                    AL = ZZ / (1.0 + DITPAR * (ELNLL + FLNLL));
                    BL = BB / (1.0 + DITPAR * (ELNRL + GLNRL));
                    CL = DD / (1.0 + DITPAR * (FLNCL + GLNCL));
                    AP = AL * ELNLL;
                    CP = BL * ELNRL;
                    GP = CL * FLNCL;
                    RP = CL * GLNCL;
                    TP = AL * FLNLL;
                    UP = BL * GLNRL;
                    HHCOF = HCOF[NN];
                    DL = EE + HHCOF + DITPAR * (AP + TP + CP + GP + UP+ RP) 
					     - AL * GLNLL - BL * FLNRL - CL * ELNCL;
					if(fabs(DL) < 1e-30)
					{
						IBOUND[NN] = 0;
						HCOF[NN] = 0.0;
						RHS[NN] = 0.0;
						HNEW[NN] = GDWMOD.GetHNOFLO();
						//输出异常信息
						printf(_T("\n"));
						printf(_T("网格单元(层=%d, 行=%d, 列=%d)与其所有相邻网格单元间的水力传导系数都为0, 或出现其他异常情况！\n"),
							k + 1, i + 1, j + 1);
						printf(_T("模型将该网格单元置为无效网格单元！\n"));
						printf(_T("\n"));
						//跳过本网格单元的后续处理
						continue;
					}
                    m_AryEL[NN] = (FF - DITPAR * (AP + CP)) / DL;
                    m_AryFL[NN] = (HH - DITPAR * (TP + GP)) / DL;
                    m_AryGL[NN] = (SS - DITPAR * (RP + UP)) / DL;

					//计算残差
                    RRHS = RHS[NN];
                    RES = RRHS - ZHNEW - BHNEW - DHNEW -EE * HNEW[NN] - HHCOF * HNEW[NN]
						   - FHNEW - HHNEW - SHNEW;
				    
				    //计算中间向量V
                    m_AryV[NN] = (m_Damp * RES -AL * VNLL -BL * VNRL - CL * VNCL) / DL;
				}
			}
		}
	}

	//用回代求解未知水头
	for(k=0; k<m_NLyr_Def; k++)
	{
		for(i=0; i<m_NRow_Def; i++)
		{
			for(j=0; j<m_NCol_Def; j++)
			{
				if(IDIR >= 0)
				{
					kk = m_NLyr_Def - k - 1;
					ii = m_NRow_Def - i - 1;
					jj = m_NCol_Def - j - 1;
				}
				else
				{
					kk = k;
					ii = i;
					jj = m_NCol_Def - j - 1;
				}

				//从三元下标计算单一下标
				NN = jj + ii * m_NCol_Def + kk * NRC;
				if(IBOUND[NN] > 0)
				{
					//计算当前单元后面三个相邻单元的一元坐标(相对于扫描方向)
                    NC=NN+1;
                    NR=NN+IDNCOL;
                    NL=NN+IDNRC;

					//向后替代
                    ELXI = 0.0;
                    FLXI = 0.0;
                    GLXI = 0.0;
					if(jj != m_NCol_Def-1) ELXI = m_AryEL[NN] * m_AryV[NC];
					if(i != 0) FLXI = m_AryFL[NN] * m_AryV[NR];
					if(k != 0) GLXI = m_AryGL[NN] * m_AryV[NL];
					m_AryV[NN] = m_AryV[NN] - ELXI - FLXI - GLXI;
					//更新水头值
                    HNEW[NN] = HNEW[NN] + m_AryV[NN];
				}
			}
		}
	}
}

//////////////////////////////////////////////////////////
//矩阵方程预处理共轭梯度解法类函数定义
CPcg::CPcg(void)
{
	//参数和数组归零
	m_MxInIter = 0;         //内部最大迭代计算循环次数
	m_ArySS = NULL;         //存放当前梯度方向的数组, m_NUMNODES个元素
	m_AryP = NULL;          //存放下次搜索方向的数组, m_NUMNODES个元素
	m_AryV = NULL;          //存放预处理矩阵方程解的数组, m_NUMNODES个元素
	m_AryCD = NULL;         //存放预处理矩阵对角线项的数组, m_NUMNODES个元素
	m_AryRES = NULL;        //存放当前残差的数组, m_NUMNODES个元素
	m_AryHCHGN = NULL;      //存放单元水头变化的数组, m_NUMNODES个元素
}

CPcg::~CPcg(void)
{
	//清空分配的数组内存

	ClearMem();
}

void CPcg::AlloCateMem()
{
	//开辟数组内存

	//开辟父类所需数组
	CSolver::AlloCateMem();

	//自身开辟所需数组
	try
	{
		m_ArySS = new double[m_NUMNODES]();
		m_AryP = new double[m_NUMNODES]();
		m_AryV = new double[m_NUMNODES]();
		m_AryCD = new double[m_NUMNODES]();
		m_AryRES = new double[m_NUMNODES]();
		m_AryHCHGN = new double[m_NUMNODES]();
	}
	catch (bad_alloc &e)
	{
		printf(e.what());
		exit(1);
	}
}

void CPcg::ClearMem(void)
{
	//该函数清空之前分配的内存

	//清空父类所需数组
	CSolver::ClearMem();

	//清空自身开辟的数组
	if (m_ArySS != NULL)
	{ 
		delete[] m_ArySS; 
		m_ArySS = NULL; 
	}
	if (m_AryP != NULL) 
	{ 
		delete[] m_AryP; 
		m_AryP = NULL; 
	}
	if (m_AryV != NULL) 
	{ 
		delete[] m_AryV; 
		m_AryV = NULL; 
	}
	if (m_AryCD != NULL) 
	{
		delete[] m_AryCD; 
		m_AryCD = NULL; 
	}
	if (m_AryRES != NULL)
	{ 
		delete[] m_AryRES; 
		m_AryRES = NULL; 
	}
	if (m_AryHCHGN != NULL)
	{
		delete[] m_AryHCHGN;
		m_AryHCHGN = NULL;
	}
}

void CPcg::SolveMatrix(long KITER, long IBOUND[], double HNEW[], double CR[], 
	double CC[], double CV[], double HCOF[], double RHS[], double HPRE[])
{
	//该函数求解方程组, 如此次迭代收敛返回TRUE, 否则返回FALSE
	//求解结果输出到数组HNEW

	//参数说明
	//KITER: 当前迭代次数(从1开始)
	//IBOUND: 网格有效单元标示数组
	//HNEW: 水头数组
	//CR: 单元行方向水力传导系数数组
	//CC: 单元列方向水力传导系数数组
	//CV: 单元层方向水力传导系数数组
	//HCOF: 中央对角线数组
	//RHS: 右端项数组

	//清空计算数组
	ZeroMemory(m_ArySS, sizeof(double) * m_NUMNODES);
	ZeroMemory(m_AryP, sizeof(double) * m_NUMNODES);
	ZeroMemory(m_AryV, sizeof(double) * m_NUMNODES);
	ZeroMemory(m_AryCD, sizeof(double) * m_NUMNODES);
	ZeroMemory(m_AryRES, sizeof(double) * m_NUMNODES);

	//置零最末层行列处和无效网格单元的矩阵系数
	long i, j, k, Node;
	for (k = 0; k < m_NLyr_Def; k++)
	{
		for (i = 0; i < m_NRow_Def; i++)
		{
			for (j = 0; j < m_NCol_Def; j++)
			{
				//注: 本循环不可并行(切记), 因为可能对同一数组变量写入
				//从三元下标计算一维下标
				Node = k * m_NODERC + i * m_NCol_Def + j;
				if (i == m_NRow_Def - 1)
				{
					//如果本单元在最后一行
					CC[Node] = 0.0;
				}
				if (j == m_NCol_Def - 1)
				{
					//如果本单元在最后一列
					CR[Node] = 0.0;
				}
				if (k == m_NLyr_Def - 1)
				{
					//如果本单元在最底层
					CV[Node] = 0.0;
				}
				if (IBOUND[Node] == 0)
				{
					//如果本单元是无效网格单元
					if (i != 0)
					{
						//如果本单元不在第一行
						//将本单元与后侧网格单元之间的水力传导系数置零
						CC[Node - m_NCol_Def] = 0.0;
					}
					if (j != 0)
					{
						//如果本单元不在第一列
						//将本单元与左侧网格单元之间的水力传导系数置零
						CR[Node - 1] = 0.0;
					}
					if (k != 0)
					{
						//如果本单元不在最顶层
						//将上层网格单元与本网格单元之间的水力传导系数置零
						CV[Node - m_NODERC] = 0.0;
					}
					if (i != m_NRow_Def - 1)
					{
						//如果本单元不在最后一行
					    //将本单元与前侧网格单元之间的水力传导系数置零
						CC[Node] = 0.0;
					}
					if (j != m_NCol_Def - 1)
					{
						//如果本单元不在最后一列
						//将本单元与右侧网格单元之间的水力传导系数置零
						CR[Node] = 0.0;
					}
					if (k != m_NLyr_Def - 1)
					{
						//如果本单元不在最底层
						CV[Node] = 0.0;
						//将本单元与下层网格单元之间的水力传导系数置零
					}
					//将本单元的对角项和右端项置零
					HCOF[Node] = 0.0;
					RHS[Node] = 0.0;
				}
			}
		}
	}

	//计算矩阵方程的初始残差
	long NumTask = long(m_NUMNODES / GDWMOD.m_NUMTD / 2.0) + 1;
#pragma omp parallel for num_threads(GDWMOD.m_NUMTD) schedule(dynamic,NumTask)
	for (long iCell = 0; iCell < m_NUMNODES; iCell++)
	{
		//注释: 以下几个变量的定义必须放在for块中不能共用, 以适应并行运算要求
		long iLyr, iRow, iCol;
		long NRN, NRL, NCN, NCL, NLN, NLL;
		long NCF, NCD, NRB, NRH, NLS, NLZ;
		double B, H, D, F, Z, S, BHNEW, HHNEW, DHNEW, FHNEW, ZHNEW, SHNEW,
			RRHS, HHCOF;
		//注释完毕
		//先确定网格单元在数组中的层号、行号和列号
		iLyr = iCell / m_NODERC;
		iRow = (iCell - iLyr * m_NODERC) / m_NCol_Def;
		iCol = iCell - iLyr * m_NODERC - iRow * m_NCol_Def;
		//计算初始残差
		if (IBOUND[iCell] > 0)
		{
			//如果为变水头的网格单元
			//计算本网格单元周围六个相邻单元的一维下标
			NRN = iCell + m_NCol_Def;    //前侧单元
			NRL = iCell - m_NCol_Def;    //后侧单元
			NCN = iCell + 1;             //右侧单元
			NCL = iCell - 1;             //左侧单元
			NLN = iCell + m_NODERC;      //下侧单元
			NLL = iCell - m_NODERC;      //上侧单元

			//计算本网格单元与相六个邻网格单元之间水力传导系数的一维下标
			NCF = iCell;                 //与右侧
			NRH = iCell;                 //与前侧
			NLS = iCell;                 //与下侧
			NRB = iCell - m_NCol_Def;    //与后侧
			NCD = iCell - 1;             //与左侧
			NLZ = iCell - m_NODERC;      //与上侧

			//找到与相邻单元的水力传导系数
			//与后侧单元
			B = 0.0;
			BHNEW = 0.0;
			if (iRow != 0)
			{
				//如果不是第1行
				B = CC[NRB];
				BHNEW = B * (HNEW[NRL] - HNEW[iCell]);
			}
			//与前侧单元
			H = 0.0;
			HHNEW = 0.0;
			if (iRow != m_NRow_Def - 1)
			{
				//如果不是最后一行
				H = CC[NRH];
				HHNEW = H * (HNEW[NRN] - HNEW[iCell]);
			}
			//与左侧单元
			D = 0.0;
			DHNEW = 0.0;
			if (iCol != 0)
			{
				//如果不是第1列
				D = CR[NCD];
				DHNEW = D * (HNEW[NCL] - HNEW[iCell]);
			}
			//与右侧单元
			F = 0.0;
			FHNEW = 0.0;
			if (iCol != m_NCol_Def - 1)
			{
				//如果不是最后一列
				F = CR[NCF];
				FHNEW = F * (HNEW[NCN] - HNEW[iCell]);
			}
			//与上侧单元
			Z = 0.0;
			ZHNEW = 0.0;
			if (iLyr != 0)
			{
				//如果不是第1层
				Z = CV[NLZ];
				ZHNEW = Z * (HNEW[NLL] - HNEW[iCell]);
			}
			//与下侧单元
			S = 0.0;
			SHNEW = 0.0;
			if (iLyr != m_NLyr_Def - 1)
			{
				//如果不是最后一层
				S = CV[NLS];
				SHNEW = S * (HNEW[NLN] - HNEW[iCell]);
			}
			//计算与相邻单元间总的水力传导系数
			if ((B + H + D + F + Z + S) == 0.0)
			{
				//如果所有相邻网格单元间的水力传导系数都为0
				//将其置为无效单元, 不纳入本次循环迭代计算
				IBOUND[iCell] = 0;
				HCOF[iCell] = 0.0;
				RHS[iCell] = 0.0;
				HNEW[iCell] = GDWMOD.GetHNOFLO();
				//输出异常信息
				printf(_T("\n"));
				printf(_T("网格单元(层=%d, 行=%d, 列=%d)与其所有相邻网格单元间的水力传导系数都为0！\n"),
					iLyr + 1, iRow + 1, iCol + 1);
				printf(_T("模型将该网格单元置为无效网格单元！\n"));
				printf(_T("\n"));
			}
			else
			{
				//计算残差并放到数组m_AryRES中
				RRHS = RHS[iCell];
				HHCOF = HNEW[iCell] * HCOF[iCell];
				m_AryRES[iCell] = RRHS - ZHNEW - BHNEW - DHNEW - HHCOF - FHNEW -
					HHNEW - SHNEW;
			}
		}
		else
		{
			//如果是定水头单元, 残差为0.0
			m_AryRES[iCell] = 0.0;
		}
	}

	//采用预处理共轭梯度法对方程组Ax=b进行迭代求解
	long IC, IR, IL;
	long NC, NR, NL;
	double H, VCC, F, VCR, S, VCV;
	double CBack, CFront, CLeft, CRight, CTop, CBot, HHCOF;
	double CDCR, CDCC, CDCV, FCC, FCR, FCV, FV;
	double SSCR, SSCC, SSCV, VN;
	double BIGH = 0.0;
	double BIGR = 0.0;
	double DEL = 0.0;
	double CD1 = 0.0;
	double SRNew = 0.0;
	double SROld = 0.0;
	double PAP = 0.0;
	double ALPHA = 0.0;
	//进入共轭梯度法内部循环迭代过程
	long InIter = 0;
	m_MxInIter = 30;
	for (InIter = 1; InIter <= m_MxInIter; InIter++)
	{
		//重置网格单元最大的水头变化和通量变化
		BIGH = 0.0;
		BIGR = 0.0;
		DEL = 0.0;

		//进行矩阵预处理
		//注: m_AryCD为对角线元素数组, m_AryV为临时解数组
		for (k = 0; k < m_NLyr_Def; k++)
		{
			for (i = 0; i < m_NRow_Def; i++)
			{
				for (j = 0; j < m_NCol_Def; j++)
				{
					//注: 本循环不可并行(切记), 因为m_AryCD存在前后计算相关的问题
					//从三元下标计算一维下标
					Node = k * m_NODERC + i * m_NCol_Def + j;
					if (IBOUND[Node] < 1)
					{
						//如果是无效单元或定水头单元
						//跳过以下处理
						continue;
					}
					//计算临时解, 存入m_AryV数组
					H = 0.0;
					VCC = 0.0;
					IC = Node - m_NCol_Def;
					if (i != 0)
					{
						//如果不是第1行的网格单元
						H = CC[IC];
						if (m_AryCD[IC] != 0.0)
						{
							VCC = H * m_AryV[IC] / m_AryCD[IC];
						}
					}
					F = 0.0;
					VCR = 0.0;
					IR = Node - 1;
					if (j != 0)
					{
						//如果不是第1列的网格单元
						F = CR[IR];
						if (m_AryCD[IR] != 0.0)
						{
							VCR = F * m_AryV[IR] / m_AryCD[IR];
						}
					}
					S = 0.0;
					VCV = 0.0;
					IL = Node - m_NODERC;
					if (k != 0)
					{
						//如果不是第1层的网格单元
						S = CV[IL];
						if (m_AryCD[IL] != 0.0)
						{
							VCV = S * m_AryV[IL] / m_AryCD[IL];
						}
					}
					m_AryV[Node] = m_AryRES[Node] - VCR - VCC - VCV;

					//计算CHOLESKY矩阵的对角线元素(仅首次内部迭代)
					if (InIter == 1)
					{
						CDCR = 0.0;
						CDCC = 0.0;
						CDCV = 0.0;
						FCC = 0.0;
						FCR = 0.0;
						FCV = 0.0;
						FV = 0.0;
						if (IR >= 0)
						{
							if (m_AryCD[IR] != 0.0)
							{
								CDCR = F * F / m_AryCD[IR];
							}
						}
						if (IC >= 0)
						{
							if (m_AryCD[IC] != 0.0)
							{
								CDCC = H * H / m_AryCD[IC];
							}
						}
						if (IL >= 0)
						{
							if (m_AryCD[IL] != 0.0)
							{
								CDCV = S * S / m_AryCD[IL];
							}
						}
						if (IR >= 0)
						{
							FV = CV[IR];
							if (k == m_NLyr_Def - 1 && (j + i) > 0)
							{
								FV = 0.0;
							}
							if (m_AryCD[IR] != 0.0)
							{
								FCR = (F / m_AryCD[IR]) * (CC[IR] + FV);
							}
						}
						if (IC >= 0)
						{
							FV = CV[IC];
							if (k == m_NLyr_Def - 1 && i > 0)
							{
								FV = 0.0;
							}
							if (m_AryCD[IC] != 0.0)
							{
								FCC = (H / m_AryCD[IC]) * (CR[IC] + FV);
							}
						}
						if (IL >= 0)
						{
							if (m_AryCD[IL] != 0.0)
							{
								FCV = (S / m_AryCD[IL])*(CR[IL] + CC[IL]);
							}
						}
						CBack = 0.0;
						CFront = 0.0;
						CLeft = 0.0;
						CRight = 0.0;
						CTop = 0.0;
						CBot = 0.0;
						HHCOF = 0.0;
						if (i != 0)
						{
							CBack = CC[IC];
						}
						if (i != m_NRow_Def - 1)
						{
							CFront = CC[Node];
						}
						if (j != 0)
						{
							CLeft = CR[IR];
						}
						if (j != m_NCol_Def - 1)
						{
							CRight = CR[Node];
						}
						if (k != 0)
						{
							CTop = CV[IL];
						}
						if (k != m_NLyr_Def - 1)
						{
							CBot = CV[Node];
						}
						HHCOF = HCOF[Node] - CTop - CBack - CLeft - CRight - CFront - CBot;
						m_AryCD[Node] = (1.0 + DEL) * HHCOF - CDCR - CDCC - CDCV - (FCR + FCC + FCV);
						if (fabs(m_AryCD[Node]) < 1E-30)
						{
							IBOUND[Node] = 0;
							HCOF[Node] = 0.0;
							RHS[Node] = 0.0;
							HNEW[Node] = GDWMOD.GetHNOFLO();
							//输出异常信息
							printf(_T("\n"));
							printf(_T("网格单元(层=%d, 行=%d, 列=%d)在矩阵预处理过程中出现对角线元素为0的异常情况！\n"),
								k + 1, i + 1, j + 1);
							printf(_T("模型将该网格单元置为无效网格单元！\n"));
							printf(_T("\n"));
							//跳过本网格单元的后续处理
							continue;
						}
						if (CD1 == 0.0 && m_AryCD[Node] != 0.0)
						{
							CD1 = m_AryCD[Node];
						}
						if (m_AryCD[Node] * CD1 <= 0.0)
						{
							DEL = 1.5 * DEL + 0.001;
							if (DEL > 0.5)
							{
								printf(_T("\n"));
								printf(_T("共轭梯度法失败, 矩阵方程严重不对角占优,请检查输入数据！\n"));
								PauAndQuit();
							}
						}
					}
				}
			}
		}

		//进行共轭梯度法求解
		//1. 求解预处理方程(追赶法), 结果放到m_ArySS数组
		for (k = m_NLyr_Def - 1; k >= 0; k--)
		{
			for (i = m_NRow_Def - 1; i >= 0; i--)
			{
				for (j = m_NCol_Def - 1; j >= 0; j--)
				{
					//注: 本循环不可并行(切记), 因为m_ArySS存在前后计算相关的问题
					//从三元下标计算一维下标
					Node = k * m_NODERC + i * m_NCol_Def + j;
					if (IBOUND[Node] < 1)
					{
						//如果是无效单元或定水头单元
						//跳过以下处理
						continue;
					}
					NC = Node + 1;
					NR = Node + m_NCol_Def;
					NL = Node + m_NODERC;
					SSCR = 0.0;
					SSCC = 0.0;
					SSCV = 0.0;
					VN = 0.0;
					if (j != m_NCol_Def - 1) SSCR = CR[Node] * m_ArySS[NC] / m_AryCD[Node];
					if (i != m_NRow_Def - 1) SSCC = CC[Node] * m_ArySS[NR] / m_AryCD[Node];
					if (k != m_NLyr_Def - 1) SSCV = CV[Node] * m_ArySS[NL] / m_AryCD[Node];
					VN = m_AryV[Node] / m_AryCD[Node];
					m_ArySS[Node] = VN - SSCR - SSCC - SSCV;
				}
			}
		}

		//2. 计算共轭梯度法的P向量(搜索方向向量)
		SROld = SRNew;
		SRNew = 0.0;
		for (Node = 0; Node < m_NUMNODES; Node++)
		{
			//注: 本循环不可并行(切记), 因为SRNew的计算存在变量共享的问题
			//也别想用规约操作来累加, 因为浮点数有截断误差, 每次SRNew的计算值不固定
			if (IBOUND[Node] > 0)
			{
				SRNew += m_ArySS[Node] * m_AryRES[Node];
			}
		}
		if (InIter == 1)
		{
			//如果是首次内部迭代
			CopyMemory(m_AryP, m_ArySS, sizeof(double) * m_NUMNODES);
		}
		else
		{
#pragma omp parallel for num_threads(GDWMOD.m_NUMTD) schedule(dynamic,NumTask)
			for (long iCell = 0; iCell < m_NUMNODES; iCell++)
			{
				m_AryP[iCell] = m_ArySS[iCell] + (SRNew / SROld) * m_AryP[iCell];
			}
		}
		
		//3. 计算共轭梯度法的ALPHA值(搜索步长)
#pragma omp parallel for num_threads(GDWMOD.m_NUMTD) schedule(dynamic,NumTask)
		for (long iCell = 0; iCell < m_NUMNODES; iCell++)
		{
			//注释: 以下几个变量的定义必须放在for块中不能共用, 以适应并行运算要求
			long iLyr, iRow, iCol;
			long NRN, NRL, NCN, NCL, NLN, NLL;
			long NCF, NCD, NRB, NRH, NLS, NLZ;
			double B, H, D, F, Z, S, BHNEW, HHNEW, DHNEW, FHNEW, ZHNEW, SHNEW,
				PN, VN;
			//注释完毕
			//先确定网格单元在数组中的层号、行号和列号
			iLyr = iCell / m_NODERC;
			iRow = (iCell - iLyr * m_NODERC) / m_NCol_Def;
			iCol = iCell - iLyr * m_NODERC - iRow * m_NCol_Def;
			//更新m_AryV
			m_AryV[iCell] = 0.0;
			if (IBOUND[iCell] > 0)
			{
				//仅对变水头网格单元进行处理
				//计算本网格单元周围六个相邻单元的一维下标
				NRN = iCell + m_NCol_Def;    //前侧单元
				NRL = iCell - m_NCol_Def;    //后侧单元
				NCN = iCell + 1;             //右侧单元
				NCL = iCell - 1;             //左侧单元
				NLN = iCell + m_NODERC;      //下侧单元
				NLL = iCell - m_NODERC;      //上侧单元
				//计算本网格单元与相六个邻网格单元之间水力传导系数的一维下标
				NRH = iCell;                 //与前侧
				NRB = iCell - m_NCol_Def;    //与后侧
				NCF = iCell;                 //与右侧
				NCD = iCell - 1;             //与左侧
				NLS = iCell;                 //与下侧
				NLZ = iCell - m_NODERC;      //与上侧

				//找到与相邻单元的水力传导系数
				//与前侧单元
				H = 0.0;
				if (iRow != m_NRow_Def - 1)
				{
					//如果不是最后一行
					H = CC[NRH];
				}
				//与后侧单元
				B = 0.0;
				if (iRow != 0)
				{
					//如果不是第一行
					B = CC[NRB];
				}
				//与右侧单元
				F = 0.0;
				if (iCol != m_NCol_Def - 1)
				{
					//如果不是最后一列
					F = CR[NCF];
				}
				//与左侧单元
				D = 0.0;
				if (iCol != 0)
				{
					//如果不是第一列
					D = CR[NCD];
				}
				//与下侧单元
				S = 0.0;
				if (iLyr != m_NLyr_Def - 1)
				{
					//如果不是最后一层
					S = CV[NLS];
				}
				//与上侧单元
				Z = 0.0;
				if (iLyr != 0)
				{
					//如果不是第一层
					Z = CV[NLZ];
				}
				PN = m_AryP[iCell];
				BHNEW = 0.0;
				HHNEW = 0.0;
				DHNEW = 0.0;
				FHNEW = 0.0;
				ZHNEW = 0.0;
				SHNEW = 0.0;
				if (NRL >= 0)
				{
					BHNEW = B*(m_AryP[NRL] - PN);
				}
				if (NRN < m_NUMNODES)
				{
					HHNEW = H*(m_AryP[NRN] - PN);
				}
				if (NCL >= 0)
				{
					DHNEW = D*(m_AryP[NCL] - PN);
				}
				if (NCN < m_NUMNODES)
				{
					FHNEW = F*(m_AryP[NCN] - PN);
				}
				if (NLL >= 0)
				{
					ZHNEW = Z*(m_AryP[NLL] - PN);
				}
				if (NLN < m_NUMNODES)
				{
					SHNEW = S*(m_AryP[NLN] - PN);
				}
				//计算矩阵A与数组P的乘积, 结果放到数组m_AryV
				PN = HCOF[iCell] * m_AryP[iCell];
				VN = ZHNEW + BHNEW + DHNEW + PN + FHNEW + HHNEW + SHNEW;
				m_AryV[iCell] = VN;
			}
		}
		//统计PAP
		PAP = 0.0;
        for (Node = 0; Node < m_NUMNODES; Node++)
		{
			//注: 本循环不可并行(切记), 因为SRNew的计算存在变量共享的问题
			//也别想用reduction(+:)操作来累加, 因为浮点数有截断误差, 每次PAP的计算值不固定
			if (IBOUND[Node] > 0)
			{
				PAP += m_AryP[Node] * m_AryV[Node];
			}
		}
		//计算ALPHA值
		ALPHA = 1.0;
		if (PAP == 0.0)
		{
			printf(_T("\n"));
			printf(_T("共轭梯度法失败,请检查矩阵方程数据合理性！\n"));
			PauAndQuit();
		}
		if (PAP != 0.0)
		{
			ALPHA = SRNew / PAP;
		}

		//4. 计算新的水头值和残差
#pragma omp parallel for num_threads(GDWMOD.m_NUMTD) schedule(dynamic,NumTask)
		for (long iCell = 0; iCell < m_NUMNODES; iCell++)
		{
			if (IBOUND[iCell] > 0)
			{
				//如果网格单元为变水头单元
				m_AryHCHGN[iCell] = ALPHA * m_AryP[iCell];
				HNEW[iCell] = HNEW[iCell] + m_AryHCHGN[iCell];
				//计算残差(矩阵A乘以向量P，得到向量V)
				m_AryRES[iCell] = m_AryRES[iCell] - ALPHA * m_AryV[iCell];
			}
		}

		//5. 计算本次内部循环的最大水头值变化和残差变化
		for (Node = 0; Node < m_NUMNODES; Node++)
		{
			//注: 本循环不可并行, 因为涉及共享变量BIGH和BIGR
			//另外, OpenMP2.0还不支持reduction(max:)操作
			if (IBOUND[Node] > 0)
			{
				BIGH = max(BIGH, fabs(m_AryHCHGN[Node]));
				BIGR = max(BIGR, fabs(m_AryRES[Node]));
			}
		}
		//减缓水头值和残差
		BIGH = BIGH * m_Damp;
		BIGR = BIGR * m_Damp;

		//6. 判断内部循环迭代是否收敛
		if (BIGH <= m_HClose && BIGR <= m_RClose)
		{
			//水头变化和残差均满足条件
			//内部迭代计算收敛, 跳出内部循环
			//否则需在迭代m_MxInIter次后自动跳出循环
			break;
		}
	}
	
	if (fabs(m_Damp - 1.0) > 1e-30)
	{
		//如果需要延缓迭代计算结果
#pragma omp parallel for num_threads(GDWMOD.m_NUMTD) schedule(dynamic,NumTask)
		for (long iCell = 0; iCell < m_NUMNODES; iCell++)
		{
			if (IBOUND[iCell] > 0)
			{
				//应用延缓因子参数计算本次外部循环迭代的水头
				HNEW[iCell] = (1.0 - m_Damp) * HPRE[iCell] + m_Damp * HNEW[iCell];
			}
		}
	}
}