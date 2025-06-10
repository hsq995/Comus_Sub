#include "StdAfx.h"
#include "ClassInclude.h"

//////////////////////////////////////////////////////////
//开采井边界单元类函数定义
CWellCell::CWellCell()
{
	m_PumpRate = 0.0;                         //对应的开采强度(L3/T)
	m_SatThr =0.0;                            //仅模拟网格单元的干-湿转化，井流量为负(开采)，网格单元所属含水层类型为1或3时有效, 
											  //为开采量线性减少时的含水层饱和厚度阈值(L)
}

CWellCell::~CWellCell()
{

}

//////////////////////////////////////////////////////////
//井流处理类函数定义
CWell::CWell()
{

}

CWell::~CWell()
{

}

CBndCell* CWell::ReadPerCellData(CString strData)
{
	//读取应力期网格单元源汇项数据(TXT)

	long iPer, iLyr, iRow, iCol, LyrCon;
	double PumpRate, SatThr;
	CWellCell* pWellCell = NULL;
	CString strFormat;
	strFormat = _T("%ld %ld %ld %ld %lg %lg");
	if (sscanf_s(strData, strFormat, &iPer, &iLyr, &iRow, &iCol, &PumpRate, &SatThr) != 6)
	{
		printf(_T("\n"));
		printf(_T("井流_应力期数据表.in 文件中的数据异常, 请检查!\n"));
		PauAndQuit();
	}
	//检查数据
	GDWMOD.CheckCellLimit(iLyr, iRow, iCol);
	BOOL bSimDryWet = FALSE;
	if (GDWMOD.GetSIMMTHD() == 1 || (GDWMOD.GetSIMMTHD() == 2 && GDWMOD.GetIWDFLG() == 1))
	{
		bSimDryWet = TRUE;
	}
	if (bSimDryWet && PumpRate < 0.0)
	{
		//如果模拟网格单元的干-湿转化且为开采井
		LyrCon = GDWMOD.GetLyrCon(iLyr);
		if (LyrCon == 1 || LyrCon == 3)
		{
			//如果开采井位于含水层类型1或3
			if (SatThr <= 0.0)
			{
				printf(_T("\n"));
				printf(_T("模型已选择模拟网格单元的干-湿转化, \n"));
				printf(_T("网格单元(K=%d,I=%d,J=%d)的饱和厚度阈值不能小于等于0.0, 请检查 井流_应力期数据表.in !\n"),
					iLyr, iRow, iCol);
				PauAndQuit();
			}
		}
	}
	pWellCell = new CWellCell;
	pWellCell->m_ILYR = iLyr;
	pWellCell->m_IROW = iRow;
	pWellCell->m_ICOL = iCol;
	pWellCell->m_PumpRate = PumpRate;
	if (SatThr > 0.0)
	{
		pWellCell->m_SatThr = SatThr;
	}

	//返回数据指针
	return pWellCell;
}

CBndCell* CWell::ReadPerCellData(_RecordsetPtr pCellDataRec)
{
	//读取应力期网格单元源汇项数据(数据库)

	_variant_t Value;
	long iLyr, iRow, iCol, LyrCon;
	double PumpRate, SatThr, CellTop, CellBot;
	CGridCell* pGridCell = NULL;
	CWellCell* pWellCell = NULL;
	Value = pCellDataRec->GetCollect(_T("ILYR"));
	iLyr = Value.intVal;
	Value = pCellDataRec->GetCollect(_T("IROW"));
	iRow = Value.intVal;
	Value = pCellDataRec->GetCollect(_T("ICOL"));
	iCol = Value.intVal;
	GDWMOD.CheckCellLimit(iLyr, iRow, iCol);
	Value = pCellDataRec->GetCollect(_T("WELLR"));
	PumpRate = Value.dblVal;
	SatThr = 0.0;
	//检查数据
	BOOL bSimDryWet = FALSE;
	if (GDWMOD.GetSIMMTHD() == 1 || (GDWMOD.GetSIMMTHD() == 2 && GDWMOD.GetIWDFLG() == 1))
	{
		bSimDryWet = TRUE;
	}
	if (bSimDryWet && PumpRate < 0.0)
	{
		//如果模拟网格单元的干-湿转化且为开采井
		LyrCon = GDWMOD.GetLyrCon(iLyr);
		if (LyrCon == 1 || LyrCon == 3)
		{
			//如果开采井位于含水层类型1或3
			Value = pCellDataRec->GetCollect(_T("SATTHR"));
			SatThr = Value.dblVal;
			if (SatThr <= 0.0)
			{
				printf(_T("\n"));
				printf(_T("模型已选择模拟网格单元的干-湿转化, \n"));
				printf(_T("网格单元(K=%d,I=%d,J=%d)的饱和厚度阈值不能小于等于0.0, 请检查<井流_应力期数据表>！\n"),
					iLyr, iRow, iCol);
				PauAndQuit();
			}
			if (LyrCon == 3)
			{
				pGridCell = GDWMOD.GetGridCell(iLyr, iRow, iCol);
				CellTop = pGridCell->GetCellTop();
				CellBot = pGridCell->GetCellBot();
				if (SatThr > CellTop - CellBot)
				{
					printf(_T("\n"));
					printf(_T("网格单元(K=%d,I=%d,J=%d)的饱和厚度阈值不能超过网格单元的层厚, 请检查<井流_应力期数据表>！\n"),
						iLyr, iRow, iCol);
					PauAndQuit();
				}
			}
		}
	}
	pWellCell = new CWellCell;
	pWellCell->m_ILYR = iLyr;
	pWellCell->m_IROW = iRow;
	pWellCell->m_ICOL = iCol;
	pWellCell->m_PumpRate = PumpRate;
	if (SatThr > 0.0)
	{
		pWellCell->m_SatThr = SatThr;
	}

	//返回数据指针
	return pWellCell;
}

void CWell::FormMatrix(long IBOUND[], double HNEW[],  
	double HCOF[], double RHS[], double HOLD[], double HPRE[])
{
    //根据开采井边界条件形成/处理矩阵方程

	BOOL bSimDryWet = FALSE;
	if (GDWMOD.GetSIMMTHD() == 1 || (GDWMOD.GetSIMMTHD() == 2 && GDWMOD.GetIWDFLG() == 1))
	{
		bSimDryWet = TRUE;
	}
	long NumBndCell = long(m_pCurPerData->m_pBndCellAry.size());
	//逐个井流单元进行循环
	long NumTask = long(NumBndCell / GDWMOD.m_NUMTD / 2.0) + 1;
#pragma omp parallel for num_threads(GDWMOD.m_NUMTD) schedule(dynamic,NumTask)
	for (long i = 0; i < NumBndCell; i++)
	{
		//注释: 以下几个变量的定义必须放在for块中不能共用, 以适应并行运算要求
		long iLyr, iRow, iCol, node, LyrCon;
		CWellCell* pWellCell = NULL;
		CGridCell* pGridCell = NULL;
		double PumpRate, Bot, HH;
		//注释完毕
		pWellCell = (CWellCell*)m_pCurPerData->m_pBndCellAry[i];
		iLyr = pWellCell->m_ILYR;
		iRow = pWellCell->m_IROW;
		iCol = pWellCell->m_ICOL;
		node = GDWMOD.GetCellNode(iLyr, iRow, iCol);
		if (IBOUND[node] <= 0)
		{
			//不计算无效单元和水头单元
			continue;
		}
		PumpRate = pWellCell->m_PumpRate;
		if (!bSimDryWet)
		{
			//如果不模拟网格单元的干湿转化, 无论注水井还是开采井直接处理
			RHS[node] = RHS[node] - PumpRate;
		}
		else
		{
			//如果模拟网格单元的干湿转化, 分情况进行处理
			if (PumpRate >= 0.0)
			{
				//注水井的情况, 任何情况都没问题
				RHS[node] = RHS[node] - PumpRate;
			}
			else
			{
				//开采井的情况
				LyrCon = GDWMOD.GetLyrCon(iLyr);
				if (LyrCon == 0 || LyrCon == 2)
				{
					//含水层类型0和2的情况下网格单元不可能疏干
					RHS[node] = RHS[node] - PumpRate;
				}
				else
				{
					//含水层类型1和3的情况下网格单元开采量可能发生变化
					pGridCell = GDWMOD.GetGridCell(iLyr, iRow, iCol);
					Bot = pGridCell->GetCellBot();
					HH = HNEW[node] - Bot;
					if (HH >= pWellCell->m_SatThr)
					{
						//网格单元的饱和厚度大于等于厚度阈值, 按指定开采量处理
						RHS[node] = RHS[node] - PumpRate;
					}
					else if (HH > 0.0 && HH <= pWellCell->m_SatThr)
					{
						//网格单元的饱和厚度介于0.0和厚度阈值之间, 开采量线性衰减处理
						HCOF[node] = HCOF[node] + PumpRate / pWellCell->m_SatThr;
						RHS[node] = RHS[node] + PumpRate * Bot / pWellCell->m_SatThr;
					}
					else
					{
						//网格单元的饱和厚度小于0.0, 全有效单元法下网格单元处于疏干状态, 不做任何处理
						continue;
					}
				}
			}
		}
	}
}

void CWell::Budget(long IBOUND[], double HNEW[], double HOLD[])
{
	//计算开采井边界处水量通量

	BOOL bSimDryWet = FALSE;
	if (GDWMOD.GetSIMMTHD() == 1 || (GDWMOD.GetSIMMTHD() == 2 && GDWMOD.GetIWDFLG() == 1))
	{
		bSimDryWet = TRUE;
	}
	long NumBndCell = long(m_pCurPerData->m_pBndCellAry.size());
	//逐个井流单元进行循环
	long NumTask = long(NumBndCell / GDWMOD.m_NUMTD / 2.0) + 1;
#pragma omp parallel for num_threads(GDWMOD.m_NUMTD) schedule(dynamic,NumTask)
	for (long i = 0; i < NumBndCell; i++)
	{
		//注释: 以下几个变量的定义必须放在for块中不能共用, 以适应并行运算要求
		long iLyr, iRow, iCol, node, LyrCon;
		CWellCell* pWellCell = NULL;
		CGridCell * pGridCell = NULL;
		double PumpRate, Bot, HH, SinkPump;
		//注释完毕
		pWellCell = (CWellCell*)m_pCurPerData->m_pBndCellAry[i];
		iLyr = pWellCell->m_ILYR;
		iRow = pWellCell->m_IROW;
		iCol = pWellCell->m_ICOL;
		node = GDWMOD.GetCellNode(iLyr, iRow, iCol);
		pGridCell = GDWMOD.GetGridCell(iLyr, iRow, iCol);
		if (IBOUND[node] <= 0)
		{
			//不计算无效单元和水头单元
			continue;
		}
		PumpRate = pWellCell->m_PumpRate;
		SinkPump = 0.0;
		if (!bSimDryWet)
		{
			//如果不模拟网格单元的干湿转化, 无论注水井还是开采井直接处理
			SinkPump = PumpRate;
		}
		else
		{
			//如果模拟网格单元的干湿转化, 分情况进行处理
			if (PumpRate >= 0.0)
			{
				//注水井的情况, 任何情况都没问题
				SinkPump = PumpRate;
			}
			else
			{
				//开采井的情况
				LyrCon = GDWMOD.GetLyrCon(iLyr);
				if (LyrCon == 0 || LyrCon == 2)
				{
					//含水层类型0和2的情况下网格单元不可能疏干
					SinkPump = PumpRate;
				}
				else
				{
					//含水层类型1和3的情况下网格单元开采量可能发生变化
					pGridCell = GDWMOD.GetGridCell(iLyr, iRow, iCol);
					Bot = pGridCell->GetCellBot();
					HH = HNEW[node] - Bot;
					if (HH >= pWellCell->m_SatThr)
					{
						//网格单元的饱和厚度大于等于厚度阈值, 按指定开采量处理
						SinkPump = PumpRate;
					}
					else if (HH > 0.0 && HH <= pWellCell->m_SatThr)
					{
						//网格单元的饱和厚度介于0.0和厚度阈值之间, 开采量线性衰减处理
						SinkPump = PumpRate * HH / pWellCell->m_SatThr;
					}
					else
					{
						//网格单元的饱和厚度小于0.0, 全有效单元法下网格单元处于疏干状态, 不做任何处理
						continue;
					}
				}
			}
		}

		//数据统计到单元
		pGridCell->AddSinkR(m_BndID, SinkPump);
	}
}