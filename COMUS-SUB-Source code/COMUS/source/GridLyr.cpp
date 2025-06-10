#include "StdAfx.h"
#include "ClassInclude.h"

CGridLyr::CGridLyr(void)
{
	m_LYRID = 0;                //含水层编号（从1开始，1表示顶层）
	m_LYRCON = 0;               //含水层类型标示码。每个输入数字表示一个含水层。等于0：承压含水层；等于1：非承压含水层，仅用于最上层；
							    //等于2：导水系数为常数的承压/非承压含水层；等于3：导水系数随厚度变化的承压/非承压含水层。
							    //对于BCF，含水层类型标识可以为0-3中的任意一个；对于LPF，只能为0和3.
	m_LYRTRPY = 0.0;            //等于-1：含水层沿列方向的饱和渗透系数异性值随每个单元变化
							    //大于0：含水层沿列方向的饱和渗透系数异性值全含水层为同一值，大小为输入值。
								//对于BCF，只能为大于0的值；对于LPF，可以为-1或大于0的值.
	m_LYRCBD = 0;               //仅对LPF。含水层是否处理拟三维问题。0：不处理；1：处理
	m_LYRVKA = 0;               //仅对LPF。等于0：表示VKA参数(见<LPF网格单元属性表>)为单元的垂向渗透系数值Kz；
							    //等于1：表示VKA输入值为Kx的比例。
	m_LYRIBS = 0;               //含水层是否模拟夹层体贮水. 0: 不模拟；1: 模拟. 仅当SIMTYPE=1且SIMIBS=1时有效.
	m_pGridCellAry = NULL;      //网格单元数组(二维)
	m_AryKav_R = NULL;          //沿行方向两个相邻单元之间的平均渗透系数数组(L/T), 仅全有效单元法时用到(二维)
	m_AryKav_C = NULL;          //沿列方向两个相邻单元之间的平均渗透系数数组(L/T), 仅全有效单元法时用到(二维)
	m_NumRow = 0;               //网格的行数
	m_NumCol = 0;               //网格的列数
	m_NumRC = 0;                //含水层中网格单元的数量
	m_StrgIn = 0.0;             //当前计算时段内含水层内单元蓄量减少(水头下降)释放的水量(L3)
	m_StrgOut = 0.0;            //当前计算时段内含水层内单元蓄量增加(水头上升)蓄积的水量(L3)
	m_TopIn = 0.0;              //当前计算时段内上部含水层向本含水层越流的水量(L3)
	m_TopOut = 0.0;             //当前计算时段内本含水层向上部含水层越流的水量(L3)
	m_BotIn = 0.0;              //当前计算时段内下部含水层向本含水层越流的水量(L3)
	m_BotOut = 0.0;             //当前计算时段内本含水层向下部含水层越流的水量(L3)
	m_AryLyrSinkIn = NULL;      //当前计算时段内进入含水层的源汇项数组(L3)
	m_AryLyrSinkOut = NULL;     //当前计算时段内离开含水层的源汇项数组(L3)
	m_ResultAry = NULL;         //模拟输出结果存储数组
	m_LyrBalErr = 0.0;          //当前计算时段内的水量平衡误差(L3)
}

CGridLyr::~CGridLyr(void)
{
	//释放网格单元内存,并清除开辟的数组

	long i, j;

	//删除三维数组
	if(m_pGridCellAry != NULL)
	{
		for (i = 0; i < m_NumRow; i++)
		{
			for (j = 0; j < m_NumCol; j++)
			{
				if (m_pGridCellAry[i][j] != NULL)
				{
					delete m_pGridCellAry[i][j];     //先释放CGridCell对象
				}
			}
		}
		for (i = 0; i < m_NumRow; i++)
		{
			delete[] m_pGridCellAry[i];              //释放列指针
		}
		delete[] m_pGridCellAry;                     //释放行指针
		m_pGridCellAry = NULL;
	}
	//删除二维数组
	if (m_AryKav_R != NULL)
	{
		for (i = 0; i < m_NumRow; i++)
		{
			delete[] m_AryKav_R[i];
		}
		delete[] m_AryKav_R;
		m_AryKav_R = NULL;
	}
	if (m_AryKav_C != NULL)
	{
		for (i = 0; i < m_NumRow; i++)
		{
			delete[] m_AryKav_C[i];
		}
		delete[] m_AryKav_C;
		m_AryKav_C = NULL;
	}
	//删除一维数组
	if (m_AryLyrSinkIn != NULL)
	{
		delete[] m_AryLyrSinkIn;
		m_AryLyrSinkIn = NULL;
	}
	if (m_AryLyrSinkOut != NULL)
	{
		delete[] m_AryLyrSinkOut;
		m_AryLyrSinkOut = NULL;
	}
	if (m_ResultAry != NULL)
	{
		delete[] m_ResultAry;
		m_ResultAry = NULL;
	}
}

vector<CString> CGridLyr::m_ItemNamAry;  //输出项名称数组
vector<int> CGridLyr::m_ItemAtriAry;     //输出项性质数组
void CGridLyr::SetOutputItem()
{
	//设置数据库输出项名称

	CString ItemNam;
	long SimType = GDWMOD.GetSimType();
	size_t iBndOrSink;
	size_t ArySize = CGridCell::m_ActSimBndAry.size();
	m_ItemNamAry.clear();
	m_ItemAtriAry.clear();
	//补给项
	if (SimType == 2)
	{
		//如果是非稳定流模拟
		//蓄量变化(蓄量减少释放的水量)
		m_ItemNamAry.push_back(_T("STAIN"));
		m_ItemAtriAry.push_back(1);
	}
	//上层越流(进)
	m_ItemNamAry.push_back(_T("TOPIN"));
	m_ItemAtriAry.push_back(1);
	//下层越流(进)
	m_ItemNamAry.push_back(_T("BOTIN"));
	m_ItemAtriAry.push_back(1);
	//边界上流量(进)
	for (iBndOrSink = 0; iBndOrSink < ArySize; iBndOrSink++)
	{
		ItemNam = CGridCell::m_ActSimBndAry[iBndOrSink].m_BndNamAbb + _T("IN");
		m_ItemNamAry.push_back(ItemNam);
		m_ItemAtriAry.push_back(1);
	}
	//排泄项(出)
	if (SimType == 2)
	{
		//如果是非稳定流模拟
		//蓄量变化(蓄量增加蓄积的水量)
		m_ItemNamAry.push_back(_T("STAOUT"));
		m_ItemAtriAry.push_back(1);
	}
	//上层越流(出)
	m_ItemNamAry.push_back(_T("TOPOUT"));
	m_ItemAtriAry.push_back(1);
	//下层越流(出)
	m_ItemNamAry.push_back(_T("BOTOUT"));
	m_ItemAtriAry.push_back(1);
	//边界上流量(出)
	for (iBndOrSink = 0; iBndOrSink < ArySize; iBndOrSink++)
	{
		ItemNam = CGridCell::m_ActSimBndAry[iBndOrSink].m_BndNamAbb + _T("OUT");
		m_ItemNamAry.push_back(ItemNam);
		m_ItemAtriAry.push_back(1);
	}
	//水量平衡误差输出
	m_ItemNamAry.push_back(_T("ABER"));
	m_ItemAtriAry.push_back(1);

	//数组不会再增加元素了
	m_ItemNamAry.shrink_to_fit();
	m_ItemAtriAry.shrink_to_fit();
}

void CGridLyr::UpdateLyrCellStatus(double HNEW[], long IBOUND[])
{
	//更新含水层内网格单元的水头和IBOUND值, 清零统计变量

	long NumTask = long(m_NumRC / GDWMOD.m_NUMTD / 2.0) + 1;
#pragma omp parallel for num_threads(GDWMOD.m_NUMTD) schedule(dynamic,NumTask)
	for (long iCell = 0; iCell < m_NumRC; iCell++)
	{
		//注释: 以下变量只能在for块中定义以适应并行运算
		long i, j, node;
		CGridCell* pGridCell = NULL;
		//注释结束
		//先确定网格单元在含水层网格单元数组中的行号和列号
		i = iCell / m_NumCol;
		j = iCell - i * m_NumCol;
		pGridCell = m_pGridCellAry[i][j];
		if (pGridCell->m_INIIBOUND != 0)
		{
			//初始有效的网格单元才执行操作
			node = (m_LYRID - 1) * m_NumRC + i * m_NumCol + j;
			//更新网格单元的水头值和有效性标识(该标识可能在计算中有变化)
			pGridCell->m_hNew = HNEW[node];
			pGridCell->m_IBOUND = IBOUND[node];
			//清零网格单元的水平衡项
			pGridCell->ZeroStepBDItems();
		}
	}
}

void CGridLyr::CalLyrCellStgR(long IBOUND[],double HNEW[], double HOLD[],double SC1[], double SC2[])
{
	//计算含水层内网格单元的蓄量变化

	double TLED = 1.0 / GDWMOD.GetDeltT();
	long NumTask = long(m_NumRC / GDWMOD.m_NUMTD / 2.0) + 1;
#pragma omp parallel for num_threads(GDWMOD.m_NUMTD) schedule(dynamic,NumTask)
	for (long iCell = 0; iCell < m_NumRC; iCell++)
	{
		//注释: 以下变量只能在for块中定义以适应并行运算
		long i, j, node;
		double HSING, HHOLD, RHO1, RHO2, TOP, BOT, SOLD, SNEW;
		CGridCell* pGridCell = NULL;
		//注释结束
		//先确定网格单元在含水层网格单元数组中的行号和列号
		i = iCell / m_NumCol;
		j = iCell - i * m_NumCol;
		//计算网格单元蓄量变化
		node = (m_LYRID - 1) * m_NumRC + i * m_NumCol + j;
		pGridCell = m_pGridCellAry[i][j];
		if (IBOUND[node] > 0)                         //仅非水头单元的有效单元需计算
		{
			HSING = HNEW[node];
			HHOLD = HOLD[node];
			RHO1 = SC1[node] * TLED;
			if (m_LYRCON == 0)
			{
				//如果为纯承压含水层
				pGridCell->m_STRGR = RHO1 * (HSING - HHOLD);
			}
			else if (m_LYRCON == 1)
			{
				//如果为纯潜水含水层
				BOT = pGridCell->m_BOT;
				SOLD = 0.0;
				SNEW = 0.0;
				//1. 先计算时段初水位时的蓄量
				if (HHOLD > BOT)
				{
					//若时段初水位高于网格单元底板高程
					SOLD = RHO1 * (HHOLD - BOT);
				}
				//2. 再计算时段末水位时的蓄量
				if (HSING > BOT)
				{
					//若时段末水位高于网格单元底板高程
					SNEW = RHO1 * (HSING - BOT);
				}
				//3. 相减得到时段内的蓄量变化
				pGridCell->m_STRGR = SNEW - SOLD;
			}
			else if(m_LYRCON == 2)
			{
				//如果为导水系数不变的承压/非承压含水层
				//网格单元没有底板条件(处理方式类似有底板条件, 底板参考高程视为0)
				RHO2 = SC2[node] * TLED;
				TOP = pGridCell->m_TOP;
				SOLD = 0.0;
				SNEW = 0.0;
				//1. 先计算时段初水位时的蓄量
				if (HHOLD <= TOP)
				{
					//若时段初水位低于网格单元的顶板高程
					SOLD = RHO2 * HHOLD;
				}
				if (HHOLD > TOP)
				{
					//若时段初水位高于网格单元的顶板高程
					SOLD = RHO2 * TOP + RHO1 * (HHOLD - TOP);
				}
				//2. 再计算时段末水位时的蓄量
				if (HSING <= TOP)
				{
					//若时段末水位低于网格单元的顶板高程
					SNEW = RHO2 * HSING;
				}
				if (HSING > TOP)
				{
					//若时段末水位高于网格单元的顶板高程
					SNEW = RHO2 * TOP + RHO1 * (HSING - TOP);
				}
				//3. 相减得到时段内的蓄量变化
				pGridCell->m_STRGR = SNEW - SOLD;
			}
			else
			{
				//如果是导水系数可变的承压/非承压含水层
				assert(m_LYRCON == 3);
				RHO2 = SC2[node] * TLED;
				TOP = pGridCell->m_TOP;
				BOT = pGridCell->m_BOT;
				SOLD = 0.0;
				SNEW = 0.0;
				//1. 先计算时段初水位时的蓄量
				if (HHOLD > BOT && HHOLD <= TOP)
				{
					//若时段初水位介于网格单元顶、底板高程之间
					SOLD = RHO2 * (HHOLD - BOT);
				}
				if (HHOLD > TOP)
				{
					//若时段初水位高于网格单元的顶板高程
					SOLD = RHO2 * (TOP - BOT) + RHO1 * (HHOLD - TOP);
				}
				//2. 再计算时段末水位时的蓄量
				if (HSING > BOT && HSING <= TOP)
				{
					//若时段末水位介于网格单元顶、底板高程之间
					SNEW = RHO2 * (HSING - BOT);
				}
				if (HSING > TOP)
				{
					//若时段末水位高于网格单元的顶板高程
					SNEW = RHO2 * (TOP - BOT) + RHO1 * (HSING - TOP);
				}
				//3. 相减得到时段内的蓄量变化
				pGridCell->m_STRGR = SNEW - SOLD;
			}
		}
	}
}

void CGridLyr::CalLyrCellFlowR(long IBOUND[], double HNEW[], double CR[], double CC[], double CV[], long iCHFlg)
{
	//计算含水层内网格单元的单元间流量

	long NUMLYR = GDWMOD.GetNumLyr();
	long SimMethod = GDWMOD.GetSIMMTHD();
	//逐网格单元计算单元间流量
	long NumTask = long(m_NumRC / GDWMOD.m_NUMTD / 2.0) + 1;
#pragma omp parallel for num_threads(GDWMOD.m_NUMTD) schedule(dynamic,NumTask)
	for (long iCell = 0; iCell < m_NumRC; iCell++)
	{
		//注释: 以下变量只能在for块中定义以适应并行运算
		long i, j, node, noderight, nodefront, nodedown, LyrConBot;
		double HDIFF, HDOWN, MaxBot, MinHH;
		CGridCell* pGridCell = NULL;
		CGridCell* pGridCellRight = NULL;
		CGridCell* pGridCellFront = NULL;
		CGridCell* pGridCellDown = NULL;
		BOOL lNeedCal;
		//注释结束
		//先确定网格单元在含水层网格单元数组中的行号和列号
		i = iCell / m_NumCol;
		j = iCell - i * m_NumCol;
		node = (m_LYRID - 1) * m_NumRC + i * m_NumCol + j;
		if (IBOUND[node] == 0)
		{
			//如果本网格单元无效, 无需计算各方向上的流量
			continue;
		}
		pGridCell = m_pGridCellAry[i][j];
		//1、计算通过右侧的流量
		HDIFF = 0.0;
		if (m_NumCol > 1 && j < m_NumCol - 1)
		{
			//至少两列以上才有右侧流量的计算, 且最右侧(最后1列)的网格单元无需计算
			noderight = node + 1;
			pGridCellRight = m_pGridCellAry[i][j + 1];
			lNeedCal = TRUE;
			if (IBOUND[noderight] == 0)
			{
				//如果右侧网格单元是无效单元, 无需计算通过本网格单元右侧的流量
				lNeedCal = FALSE;
			}
			else
			{
				//如果右侧网格单元是有效单元
				if (iCHFlg == 0)
				{
					//如果规定对两个定水头网格单元之间不计算流量
					if (IBOUND[node] < 0 && IBOUND[noderight] < 0)
					{
						//如果本网格单元和右侧网格单元都是定水头单元, 无需计算通过本网格单元右侧的流量
						lNeedCal = FALSE;
					}
				}
			}
			if (lNeedCal)
			{
				//如果需要计算通过本网格单元右侧的流量
				if (SimMethod == 1 && (m_LYRCON == 1 || m_LYRCON == 3))
				{
					//如果为全有效单元法且本含水层为可疏干含水层
					if (CR[node] > 0.0)
					{
						//如果本单元与右侧单元之间的水力传导系数不为零
						MaxBot = max(pGridCell->m_BOT, pGridCellRight->m_BOT);
						MinHH = min(HNEW[node], HNEW[noderight]);
						if (MinHH <= MaxBot)
						{
							//当两个单元之间相对较低的水头低于两个单元之间相对较高的底板高程时
							//两个单元之间的水平向渗流量与下风单元的水头无关
							if (HNEW[node] > MaxBot)
							{
								//本单元为上风单元
								HDIFF = HNEW[node] - MaxBot;
							}
							else
							{
								//本单元为下风单元
								HDIFF = - (HNEW[noderight] - MaxBot);
							}
						}
						else
						{
							//两个单元之间的水平向渗流量与上风和下风单元的水头都有关
							HDIFF = HNEW[node] - HNEW[noderight];
						}
					}
				}
				else
				{
					//如果为其他情况
					HDIFF = HNEW[node] - HNEW[noderight];
				}
				//计算通过本网格单元右侧的流量
				pGridCell->m_FlowRight = HDIFF * CR[node];
				//右侧网格单元的左侧流量为本网格单元右侧流量的负值
				pGridCellRight->m_FlowLeft = - pGridCell->m_FlowRight;
			}
		}
		//2、计算通过前侧的流量
		HDIFF = 0.0;
		if (m_NumRow > 1 && i < m_NumRow - 1)
		{
			//至少2行以上才有前侧流量的计算, 且最前侧(最后1行)的网格单元无需计算
			nodefront = node + m_NumCol;
			pGridCellFront = m_pGridCellAry[i + 1][j];
			lNeedCal = TRUE;
			if (IBOUND[nodefront] == 0)
			{
				//如果前侧网格单元是无效单元, 无需计算通过本网格单元前侧的流量
				lNeedCal = FALSE;
			}
			else
			{
				//如果前侧网格单元是有效单元
				if (iCHFlg == 0)
				{
					//如果规定对两个定水头网格单元之间不计算流量
					if (IBOUND[node] < 0 && IBOUND[nodefront] < 0)
					{
						//如果本网格单元和前侧网格单元都是定水头单元, 无需计算通过本网格单元前侧的流量
						lNeedCal = FALSE;
					}
				}
			}
			if (lNeedCal)
			{
				//如果需要计算通过本网格单元前侧的流量
				if (SimMethod == 1 && (m_LYRCON == 1 || m_LYRCON == 3))
				{
					//如果为全有效单元法且本含水层为可疏干含水层
					if (CC[node] > 0.0)
					{
						//如果本单元与前侧单元之间的水力传导系数不为零
						MaxBot = max(pGridCell->m_BOT, pGridCellFront->m_BOT);
						MinHH = min(HNEW[node], HNEW[nodefront]);
						if (MinHH <= MaxBot)
						{
							//当两个单元之间相对较低的水头低于两个单元之间相对较高的底板高程时
							//两个单元之间的水平向渗流量与下风单元的水头无关
							if (HNEW[node] > MaxBot)
							{
								//本单元为上风单元
								HDIFF = HNEW[node] - MaxBot;
							}
							else
							{
								//本单元为下风单元
								HDIFF = -(HNEW[nodefront] - MaxBot);
							}
						}
						else
						{
							//两个单元之间的水平向渗流量与上风和下风单元的水头都有关
							HDIFF = HNEW[node] - HNEW[nodefront];
						}
					}
				}
				else
				{
					//如果为其他情况
					HDIFF = HNEW[node] - HNEW[nodefront];
				}
				//计算通过本网格单元前侧的流量
				pGridCell->m_FlowFront = HDIFF * CC[node];
				//前侧网格单元的后侧流量为本网格单元前侧流量的负值
				pGridCellFront->m_FlowBack = - pGridCell->m_FlowFront;
			}
		}
		//3、计算通过下侧的流量
		HDIFF = 0.0;
		if (NUMLYR > 1 && m_LYRID != NUMLYR)
		{
			//系统中的含水层数量至少有2层以上, 且本层不是最底层时才计算
			//获得本层含水层下层含水层的属性
			LyrConBot = GDWMOD.GetLyrCon(m_LYRID + 1);           
			nodedown = node + m_NumRC;
			pGridCellDown = GDWMOD.GetGridCell(m_LYRID + 1, i + 1, j + 1);
			lNeedCal = TRUE;
			if (IBOUND[nodedown] == 0)
			{
				//如果下侧网格单元是无效单元, 无需计算通过本网格单元下侧的流量
				lNeedCal = FALSE;
			}
			else
			{
				//如果下侧网格单元是有效单元
				if (iCHFlg == 0)
				{
					//如果规定对两个定水头网格单元之间不计算流量
					if (IBOUND[node] < 0 && IBOUND[nodedown] < 0)
					{
						//如果本网格单元和下侧网格单元都是定水头单元, 无需计算通过本网格单元下侧的流量
						lNeedCal = FALSE;
					}
				}
			}
			if (lNeedCal)
			{
				//如果需要计算通过本网格单元下侧的流量
				HDOWN = HNEW[nodedown];
				if ((LyrConBot == 2 || LyrConBot == 3))
				{
					//如果下侧网格单元有顶板条件
					if ((m_LYRCON == 0 || m_LYRCON == 2) ||
						((m_LYRCON == 1 || m_LYRCON == 3) && HNEW[node] > pGridCell->m_BOT))
					{
						//如果本单元是湿润的单元(注:全有效单元法时有效的单元也可能是疏干的单元)
						//根据下侧网格单元是否处于承压状态确定计算参数
						HDOWN = max(HDOWN, pGridCellDown->m_TOP);
					}
				}
				HDIFF = HNEW[node] - HDOWN;
				pGridCell->m_FlowDown = HDIFF * CV[node];
				//下侧网格单元的上侧流量为本网格单元下侧流量的负值
				pGridCellDown->m_FlowUp = - pGridCell->m_FlowDown;
			}
		}
	}
}

void CGridLyr::SaveLyrCellResult(long CellBDPrn, long CellFlowPrn)
{
	//保存含水层内网格单元的计算结果

	long NumTask = long(m_NumRC / GDWMOD.m_NUMTD / 2.0) + 1;
#pragma omp parallel for num_threads(GDWMOD.m_NUMTD) schedule(dynamic,NumTask)
	for (long iCell = 0; iCell < m_NumRC; iCell++)
	{
		//注释: 以下几个变量的定义必须放在for块中不能共用, 以适应并行运算要求
		long i, j;
		//注释完毕
		//先确定网格单元在含水层网格单元数组中的行号和列号
		i = iCell / m_NumCol;
		j = iCell - i * m_NumCol;
		if (m_pGridCellAry[i][j]->m_IBOUND != 0)
		{
			m_pGridCellAry[i][j]->SaveResult(CellBDPrn, CellFlowPrn);
		}
	}
}

void CGridLyr::SaveResult(long iLyrPrint)
{
	//保存计算结果

	//计算本含水层的水量平衡误差
	CalBalError();

	if (iLyrPrint == 0)
	{
		//如果含水层结果不输出
		//可以返回了
		return;
	}

	//将计算结果保存到结果输出数组
	size_t i = 0;
	size_t iBndOrSink;
	size_t ArySize = CGridCell::m_ActSimBndAry.size();
	long SimType = GDWMOD.GetSimType();
	//进入含水层的水量
	if (SimType == 2)
	{
		//如果是非稳定流模拟
		//蓄量变化(进)
		assert(m_ItemNamAry[i] == _T("STAIN"));
		m_ResultAry[i].step_val = m_StrgIn;
		i = i + 1;
	}
	//上层越流(进)
	assert(m_ItemNamAry[i] == _T("TOPIN"));
	m_ResultAry[i].step_val = m_TopIn;
	i = i + 1;
	//下层越流(进)
	assert(m_ItemNamAry[i] == _T("BOTIN"));
	m_ResultAry[i].step_val = m_BotIn;
	i = i + 1;
	//边界上流量(进)
	for (iBndOrSink = 0; iBndOrSink < ArySize; iBndOrSink++)
	{
		assert(m_ItemNamAry[i] == CGridCell::m_ActSimBndAry[iBndOrSink].m_BndNamAbb + _T("IN"));
		m_ResultAry[i].step_val = m_AryLyrSinkIn[iBndOrSink];
		i = i + 1;
	}
	//离开含水层的水量
	if (SimType == 2)
	{
		//如果是非稳定流模拟
		//蓄量变化(出)
		assert(m_ItemNamAry[i] == _T("STAOUT"));
		m_ResultAry[i].step_val = m_StrgOut;
		i = i + 1;
	}
	//上层越流(出)
	assert(m_ItemNamAry[i] == _T("TOPOUT"));
	m_ResultAry[i].step_val = m_TopOut;
	i = i + 1;
	//下层越流(出)
	assert(m_ItemNamAry[i] == _T("BOTOUT"));
	m_ResultAry[i].step_val = m_BotOut;
	i = i + 1;
	//边界上流量(出)
	for (iBndOrSink = 0; iBndOrSink < ArySize; iBndOrSink++)
	{
		assert(m_ItemNamAry[i] == CGridCell::m_ActSimBndAry[iBndOrSink].m_BndNamAbb + _T("OUT"));
		m_ResultAry[i].step_val = m_AryLyrSinkOut[iBndOrSink];
		i = i + 1;
	}
	//保存水量平衡误差
	assert(m_ItemNamAry[i] == _T("ABER"));
	m_ResultAry[i].step_val = m_LyrBalErr;

	//统计累加量结果
	for(i=0; i<m_ItemNamAry.size(); i++)
	{
		if(m_ItemAtriAry[i] == 0)
		{
			//非可累加计算结果,直接赋值
		    m_ResultAry[i].cum_val = m_ResultAry[i].step_val;
		}
		else if(m_ItemAtriAry[i] == 1)
		{
			//对于可累加计算结果,进行累加
		    m_ResultAry[i].cum_val = m_ResultAry[i].cum_val +
				m_ResultAry[i].step_val;
		}
		else
		{
			//不可能执行到这里
			assert(FALSE);
		}
	}
}

void CGridLyr::CalBalError()
{
	//计算时段内地下水平衡误差

	//初始化各项水量平衡项
	m_StrgIn = 0.0;             //当前计算时段内含水层内单元蓄量减少(水头下降)释放的水量(L3)
	m_StrgOut = 0.0;            //当前计算时段内含水层内单元蓄量增加(水头上升)蓄积的水量(L3)
	m_TopIn = 0.0;              //当前计算时段内上部含水层向本含水层越流的水量(L3)
	m_TopOut = 0.0;             //当前计算时段内本含水层向上部含水层越流的水量(L3)
	m_BotIn = 0.0;              //当前计算时段内下部含水层向本含水层越流的水量(L3)
	m_BotOut = 0.0;             //当前计算时段内本含水层向下部含水层越流的水量(L3)
	m_LyrBalErr = 0.0;          //当前计算时段内的水量平衡误差(L3)

	//清零源汇项统计数组
	size_t iBndOrSink;
	size_t ArySize = CGridCell::m_ActSimBndAry.size();
	for (iBndOrSink = 0; iBndOrSink < ArySize; iBndOrSink++)
	{
		m_AryLyrSinkIn[iBndOrSink] = 0.0;
		m_AryLyrSinkOut[iBndOrSink] = 0.0;
	}

	//将网格单元各项平衡量统计到含水层各项平衡量
	long i, j;
	CGridCell* pGridCell = NULL;
	double delt = GDWMOD.GetDeltT();
	for (i = 0; i < m_NumRow; i++)
	{
		for (j = 0; j < m_NumCol; j++)
		{
			pGridCell = m_pGridCellAry[i][j];
			if (pGridCell->m_IBOUND != 0)
			{
				if (pGridCell->m_STRGR < 0.0)
				{
					m_StrgIn = m_StrgIn - pGridCell->m_STRGR * delt;
				}
				else
				{
					m_StrgOut = m_StrgOut + pGridCell->m_STRGR * delt;
				}
				if (pGridCell->m_FlowUp < 0.0)
				{
					m_TopIn = m_TopIn - pGridCell->m_FlowUp * delt;
				}
				else
				{
					m_TopOut = m_TopOut + pGridCell->m_FlowUp * delt;
				}
				if (pGridCell->m_FlowDown < 0.0)
				{
					m_BotIn = m_BotIn - pGridCell->m_FlowDown * delt;
				}
				else
				{
					m_BotOut = m_BotOut + pGridCell->m_FlowDown * delt;
				}
				for (iBndOrSink = 0; iBndOrSink < ArySize; iBndOrSink++)
				{
					m_AryLyrSinkIn[iBndOrSink] = m_AryLyrSinkIn[iBndOrSink] +
						((pGridCell->m_ArySinkR[iBndOrSink] > 0.0) ? pGridCell->m_ArySinkR[iBndOrSink] : 0.0) * delt;
					m_AryLyrSinkOut[iBndOrSink] = m_AryLyrSinkOut[iBndOrSink] -
						((pGridCell->m_ArySinkR[iBndOrSink] < 0.0) ? pGridCell->m_ArySinkR[iBndOrSink] : 0.0) * delt;
				}
			}
		}
	}

	//统计含水层总补/排源汇量
	double SumSinkIn, SumSinkOut;
	SumSinkIn = 0.0;
	SumSinkOut = 0.0;
	for (iBndOrSink = 0; iBndOrSink < ArySize; iBndOrSink++)
	{
		SumSinkIn = SumSinkIn + m_AryLyrSinkIn[iBndOrSink];
		SumSinkOut = SumSinkOut + m_AryLyrSinkOut[iBndOrSink];
	}

	//水量平衡计算
	m_LyrBalErr = (m_StrgIn + m_TopIn + m_BotIn + SumSinkIn) - 
		(m_StrgOut + m_TopOut + m_BotOut + SumSinkOut);
}

void CGridLyr::InitSimulation()
{
	//模拟之前进行初始化工作

	//对网格单元进行初始化
	long iRow, iCol;
	for (iRow = 0; iRow < m_NumRow; iRow++)
	{
		for (iCol = 0; iCol < m_NumCol; iCol++)
		{
			m_pGridCellAry[iRow][iCol]->InitSimulation();
		}
	}

	//计算各网格单元中心的坐标
	double xStart = GDWMOD.GetXStCord();
	double yStart = GDWMOD.GetYStCord();
	double deltR, deltC;
	double SumYCord = yStart;
	double SumXCord;
	for (iRow = 0; iRow < m_NumRow; iRow++)
	{
		deltC = GDWMOD.GetDimRow(iRow + 1);
		SumYCord = SumYCord - deltC;
		SumXCord = xStart;
		for (iCol = 0; iCol<m_NumCol; iCol++)
		{
			deltR = GDWMOD.GetDimCol(iCol + 1);
			SumXCord = SumXCord + deltR;
			m_pGridCellAry[iRow][iCol]->m_XCord = SumXCord - deltR / 2.0;   //单元中心的X坐标
			m_pGridCellAry[iRow][iCol]->m_YCord = SumYCord + deltC / 2.0;   //单元中心的Y坐标
		}
	}

	//如果采用全有效单元法, 且含水层类型为1或3, 
	//开辟并初始化单元间平均渗透系数数组(二维)
	long SimMthd = GDWMOD.GetSIMMTHD();
	long IntBlkM = GDWMOD.GetINTBLKM();
	if (SimMthd == 1)
	{
		//如果采用全有效单元法
		if (IntBlkM == 2 || (IntBlkM == 1 && (m_LYRCON == 1 || m_LYRCON == 3)))
		{
			//如果为LPF格式, 或者为BCF格式且含水层类型为1或3
			try
			{
				m_AryKav_R = new double*[m_NumRow]();
				m_AryKav_C = new double*[m_NumRow]();
				for (iRow = 0; iRow < m_NumRow; iRow++)
				{
					m_AryKav_R[iRow] = new double[m_NumCol]();
					m_AryKav_C[iRow] = new double[m_NumCol]();
				}
			}
			catch (bad_alloc &e)
			{
				printf(e.what());
				exit(1);
			}
		}
	}
	
	//设置模拟结果输出数组
	long ItemArySize = long(m_ItemNamAry.size());
	m_ResultAry = new CResultVal[ItemArySize]();

	//设置边界上出入流量数组
	long ArySize = long(CGridCell::m_ActSimBndAry.size());
	m_AryLyrSinkIn = new double[ArySize]();
	m_AryLyrSinkOut = new double[ArySize]();
}

void CGridLyr::OutPutLyrCellHH(long PrnID, long iPer, long iStep, double PerTim, double CumTLen, CStdioFile& OutFile)
{
	//输出含水层内网格单元的水位(二进制)

	//输出含水层内网格单元的水位(TXT)

	long i, j, SimMthd;
	double HNewOut, HeadDry, HeadMin;
	HeadDry = GDWMOD.GetHNOFLO();
	SimMthd = GDWMOD.GetSIMMTHD();
	CGridCell* pGridCell = NULL;
	//写入各网格单元的水头
	for (i = 0; i < m_NumRow; i++)
	{
		for (j = 0; j < m_NumCol; j++)
		{
			pGridCell = m_pGridCellAry[i][j];
			if (pGridCell->m_IBOUND == 0)
			{
				//如果网单元无效
				HNewOut = HeadDry;
			}
			else
			{
				//如果网格单元有效
				HNewOut = pGridCell->m_hNew;
				//检查特殊情况
				if (SimMthd == 1 && (m_LYRCON == 1 || m_LYRCON == 3))
				{
					//如果为全有效单元法, 且网格单元位于导水系数可变含水层
					if (pGridCell->m_DryHint == 0)
					{
						//如果为可疏干单元
						if (pGridCell->m_hNew <= pGridCell->m_BOT)
						{
							//如果单元水头低于或等于底板高程
							HNewOut = HeadDry;
						}
					}
					else
					{
						//如果为不可疏干单元
						HeadMin = pGridCell->m_BOT + GDWMOD.GetMinHSat();
						if (pGridCell->m_hNew < HeadMin)
						{
							//如果单元水头低于底板高程加上饱和厚度阈值
							HNewOut = HeadMin;
						}
					}
				}
			}
			fprintf(OutFile.m_pStream, _T(" %-10d"), iPer);
			if (PrnID == 1)
			{
				//时段输出控制
				fprintf(OutFile.m_pStream, _T(" %-10d"), iStep);
			}
			else
			{
				//应力期输出控制
				fprintf(OutFile.m_pStream, _T(" %-10d"), long(-1));
			}
			fprintf(OutFile.m_pStream, _T(" %-16.8f"), CumTLen);
			fprintf(OutFile.m_pStream, _T(" %-10d"), pGridCell->m_ILYR);
			fprintf(OutFile.m_pStream, _T(" %-10d"), pGridCell->m_IROW);
			fprintf(OutFile.m_pStream, _T(" %-10d"), pGridCell->m_ICOL);
			fprintf(OutFile.m_pStream, _T(" %-16.8f"), pGridCell->m_XCord);
			fprintf(OutFile.m_pStream, _T(" %-16.8f"), pGridCell->m_YCord);
			fprintf(OutFile.m_pStream, _T(" %-20.8f"), HNewOut);
			fprintf(OutFile.m_pStream, _T("\n"));
		}
	}
}

void CGridLyr::OutPutLyrCellHH(long PrnID, long iPer, long iStep, double CumTLen)
{
	//输出含水层内网格单元的水位(数据库)

	long i, j;
	for (i = 0; i < m_NumRow; i++)
	{
		for (j = 0; j < m_NumCol; j++)
		{
			m_pGridCellAry[i][j]->OutPutHH(PrnID, iPer, iStep, CumTLen);
		}
	}
}

void CGridLyr::OutPutLyrCellDD(long PrnID, long iPer, long iStep, double PerTim, double CumTLen, CStdioFile& OutFile)
{
	//输出含水层内网格单元的水位降深(TXT)

	long i, j, SimMthd;
	double HeadMin, DDOut;
	SimMthd = GDWMOD.GetSIMMTHD();
	CGridCell* pGridCell = NULL;
	//写入各网格单元的降深信息
	for (i = 0; i < m_NumRow; i++)
	{
		for (j = 0; j < m_NumCol; j++)
		{
			pGridCell = m_pGridCellAry[i][j];
			if (pGridCell->m_IBOUND == 0)
			{
				//如果网单元无效(也有可能是运行当中变为无效的)
				DDOut = pGridCell->m_SHEAD - pGridCell->m_BOT;
			}
			else
			{
				//如果网格单元有效
				if (m_LYRCON == 1 || m_LYRCON == 3)
				{
					//如果网格单元位于导水系数可变含水层
					if (SimMthd == 1)
					{
						//如果为全有效单元法
						if (pGridCell->m_DryHint == 0)
						{
							//如果为可疏干单元
							HeadMin = pGridCell->m_BOT;
						}
						else
						{
							//如果为不可疏干单元
							HeadMin = pGridCell->m_BOT + GDWMOD.GetMinHSat();
						}
						DDOut = pGridCell->m_SHEAD - max(pGridCell->m_hNew, HeadMin);
					}
					else
					{
						//如果为原MODFLOW模拟方法
						DDOut = pGridCell->m_SHEAD - max(pGridCell->m_hNew, pGridCell->m_BOT);
					}
				}
				else
				{
					//如果网格单元位于导水系数不变含水层
					DDOut = pGridCell->m_SHEAD - pGridCell->m_hNew;
				}
			}
			fprintf(OutFile.m_pStream, _T(" %-10d"), iPer);
			if (PrnID == 1)
			{
				//时段输出控制
				fprintf(OutFile.m_pStream, _T(" %-10d"), iStep);
			}
			else
			{
				//应力期输出控制
				fprintf(OutFile.m_pStream, _T(" %-10d"), long(-1));
			}
			fprintf(OutFile.m_pStream, _T(" %-16.8f"), CumTLen);
			fprintf(OutFile.m_pStream, _T(" %-10d"), pGridCell->m_ILYR);
			fprintf(OutFile.m_pStream, _T(" %-10d"), pGridCell->m_IROW);
			fprintf(OutFile.m_pStream, _T(" %-10d"), pGridCell->m_ICOL);
			fprintf(OutFile.m_pStream, _T(" %-16.8f"), pGridCell->m_XCord);
			fprintf(OutFile.m_pStream, _T(" %-16.8f"), pGridCell->m_YCord);
			fprintf(OutFile.m_pStream, _T(" %-20.8f"), DDOut);
			fprintf(OutFile.m_pStream, _T("\n"));
		}
	}
}

void CGridLyr::OutPutLyrCellDD(long PrnID, long iPer, long iStep, double CumTLen)
{
	//输出含水层内网格单元的水位降深(数据库)

	long i, j;
	for (i = 0; i < m_NumRow; i++)
	{
		for (j = 0; j < m_NumCol; j++)
		{
			m_pGridCellAry[i][j]->OutPutDD(PrnID, iPer, iStep, CumTLen);
		}
	}
}

void CGridLyr::OutPutLyrCellFlow(long PrnID, long iPer, long iStep, double PerLen, double DeltT)
{
	//输出含水层内网格单元的流量(数据库)

	long i, j;
	for (i = 0; i < m_NumRow; i++)
	{
		for (j = 0; j < m_NumCol; j++)
		{
			m_pGridCellAry[i][j]->OutPutFlow(PrnID, iPer, iStep, PerLen, DeltT);
		}
	}
}

void CGridLyr::OutPutLyrCellBD(long PrnID, long iPer, long iStep, double PerLen, double DeltT)
{
	//输出含水层内网格单元的水平衡模拟结果(数据库)

	long i, j;
	CGridCell* pGridCell = NULL;
	for (i = 0; i < m_NumRow; i++)
	{
		for (j = 0; j < m_NumCol; j++)
		{
			pGridCell = m_pGridCellAry[i][j];
			if (pGridCell->m_INIIBOUND != 0)
			{
				//仅初始有效单元输出结果
				m_pGridCellAry[i][j]->OutPutBD(PrnID, iPer, iStep, PerLen, DeltT);
			}
		}
	}
}

CMemTable* CGridLyr::m_pMemDataTable = new CMemTable;
void CGridLyr::MakeDataTable(_RecordsetPtr pTable,CString TbName)
{
	//构建缓存表

	m_pMemDataTable->MakeDataTable(pTable,TbName);
}

void CGridLyr::WriteDatabase()
{
	//将内存数据写入数据库

	m_pMemDataTable->WriteDatabase();
}

void CGridLyr::CheckData()
{
	//检查和处理含水层输入数据

	if (GDWMOD.GetINTBLKM() == 1)
	{
		//BCF含水层数据
		//含水层性质
		if (m_LYRCON < 0 ||m_LYRCON > 3)
		{
			printf(_T("\n"));
			printf(_T("LYRCON 字段的值必须为 0-3 , 请检查!\n"));
			PauAndQuit();
		}
		//含水层K值在Y方向上的异性参数
		if (m_LYRTRPY < 0.0)
		{
			printf(_T("\n"));
			printf(_T("LYRTRPY 字段的值必须大于等于0.0, 请检查!\n"));
			PauAndQuit();
		}
		if (GDWMOD.GetSimType() == 2 && GDWMOD.GetSimIBS() == 1)
		{
			//如果模拟非稳定流且模拟夹层体贮水
			if (m_LYRIBS < 0 || m_LYRIBS > 1)
			{
				printf(_T("\n"));
				printf(_T("LYRIBS 字段的值必须为0-1, 请检查!\n"));
				PauAndQuit();
			}
		}
		//检查一下含水层的合理性
		if (m_LYRID > 1 && m_LYRCON == 1)
		{
			//潜水含水层一般仅用于含水层的顶层,不能用于其他层
			printf(_T("\n"));
			printf(_T("注意: 编号为%d的含水层在数据中定义为潜水含水层,但其不是含水层的顶层. \n"),
				m_LYRID);
			printf(_T("\n"));
		}
	}
	else
	{
		//LPF含水层数据
		//含水层性质
		if (m_LYRCON < 0 || m_LYRCON > 1)
		{
			printf(_T("\n"));
			printf(_T("LYRTYPE 字段的值必须为 0-1 , 请检查!\n"));
			PauAndQuit();
		}
		//含水层K值在Y方向上的异性参数
		if (m_LYRTRPY <= 0.0)
		{
			if (fabs(m_LYRTRPY + 1.0) > 1e-10 || m_LYRTRPY == 0.0)
			{
				printf(_T("\n"));
				printf(_T("LYRHANI 字段的值必须等于-1或大于0.0, 请检查!\n"));
				PauAndQuit();
			}
		}
		//含水层K值在Z方向上的异性参数
		if (m_LYRVKA < 0 || m_LYRVKA > 1)
		{
			printf(_T("\n"));
			printf(_T("LYRVKA 字段的值必须为0或1, 请检查!\n"));
			PauAndQuit();
		}
		//含水层是否模拟拟三维选项
		if (m_LYRCBD < 0 || m_LYRCBD > 1)
		{
			printf(_T("\n"));
			printf(_T("LYRCBD 字段的值必须为0或1, 请检查!\n"));
			PauAndQuit();
		}
		if (GDWMOD.GetSimType() == 2 && GDWMOD.GetSimIBS() == 1)
		{
			//如果模拟非稳定流且模拟夹层体贮水
			if (m_LYRIBS < 0 || m_LYRIBS > 1)
			{
				printf(_T("\n"));
				printf(_T("LYRIBS 字段的值必须为0或1, 请检查!\n"));
				PauAndQuit();
			}
		}
		//若为承压-非承压含水层, 改写LPF下的含水层属性值到3
		if (m_LYRCON == 1)
		{
			m_LYRCON = 3;
		}
	}
}

void CGridLyr::LyrOutPut(long PrnID, long iPer, long iStep, double CumTLen, CStdioFile& OutFile)
{
	//输出含水层水量平衡计算结果(TXT)

	fprintf(OutFile.m_pStream, _T(" %-10d"), iPer);
	if (PrnID == 1)
	{
		fprintf(OutFile.m_pStream, _T(" %-10d"), iStep);
	}
	else
	{
		fprintf(OutFile.m_pStream, _T(" %-10d"), long(-1));
	}
	fprintf(OutFile.m_pStream, _T(" %-16.8g"), CumTLen);
	fprintf(OutFile.m_pStream, _T(" %-10d"), m_LYRID);
	for (size_t i = 0; i < m_ItemNamAry.size(); i++)
	{
		fprintf(OutFile.m_pStream, _T(" %-16.8g"), m_ResultAry[i].cum_val);
	}
	fprintf(OutFile.m_pStream, _T("\n"));
}

void CGridLyr::LyrOutPut(long PrnID, long iPer, long iStep, double CumTLen)
{
	//输出含水层水量平衡计算结果

	size_t i = 0;
	String^ strTemp;

	//输出含水层结果
	try
	{
		DataRow^ dr;
		for (i = 0; i < m_ItemNamAry.size(); i++)
		{
			dr = m_pMemDataTable->GetDataTable()->NewRow();
			dr[0] = iPer;
			if (PrnID == 1)
			{
				dr[1] = iStep;
			}
			else
			{
				dr[1] = long(-1);
			}
			dr[2] = CumTLen;
			dr[3] = m_LYRID;
			dr[4] = i + 1;
			strTemp = gcnew String(m_ItemNamAry[i]);
			dr[5] = strTemp;
			dr[6] = m_ResultAry[i].cum_val;
			m_pMemDataTable->GetDataTable()->Rows->Add(dr);
		}
	}
	catch (_com_error *e)
	{
		printf((LPCTSTR)e->ErrorMessage());
	}
}