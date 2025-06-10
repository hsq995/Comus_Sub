#include "StdAfx.h"
#include "ClassInclude.h"

//////////////////////////////////////////////////////////
//面状补给边界单元类函数定义
CRchgCell::CRchgCell()
{
	m_IRECHRG = 0;                                //面上补给计算选项. 0: 按指定层位单元补给；1: 按最高的有效层位单元补给
	m_RechRate = 0.0;                             //对应的面上补给强度(L/T)
}

CRchgCell::~CRchgCell()
{

}

//////////////////////////////////////////////////////////
//面状补给处理类函数定义
CRchg::CRchg()
{

}

CRchg::~CRchg()
{

}

CBndCell* CRchg::ReadPerCellData(CString strData)
{
	//读取应力期网格单元源汇项数据(TXT)

	assert(!strData.IsEmpty());
	CRchgCell* pRchgCell = NULL;
	long iPer, iLyr, iRow, iCol, iRechrg;
	double rechrate;
	CString strFormat;
	strFormat = _T("%ld %ld %ld %ld %ld %lg");
	if (sscanf_s(strData, strFormat, &iPer, &iLyr, &iRow, &iCol, &iRechrg, &rechrate) != 6)
	{
		printf(_T("\n"));
		printf(_T("面上补给_应力期数据表.in 文件中的数据异常, 请检查!\n"));
		PauAndQuit();
	}
	//检查数据
	if (iRechrg < 1 || iRechrg > 2)
	{
		printf(_T("\n"));
		printf(_T("面上补给选项参数IRECH只能为1或2, 请检查 面上补给_应力期数据表.in ！\n"));
		PauAndQuit();
	}
	if (iRechrg == 1)
	{
		GDWMOD.CheckCellLimit(iLyr, iRow, iCol);
	}
	else
	{
		GDWMOD.CheckCellLimit(iRow, iCol);
	}
	if (rechrate < 0.0)
	{
		printf(_T("\n"));
		printf(_T("面上补给量数据必须大于等于0, 请检查 面上补给_应力期数据表.in !\n"));
		PauAndQuit();
	}
	pRchgCell = new CRchgCell;
	pRchgCell->m_ILYR = iLyr;
	pRchgCell->m_IROW = iRow;
	pRchgCell->m_ICOL = iCol;
	pRchgCell->m_IRECHRG = iRechrg;
	pRchgCell->m_RechRate = rechrate;
	//计算单元上的面上补给强度(L3/T)
	pRchgCell->m_RechRate = rechrate *
		GDWMOD.GetCellArea(iRow, iCol);

	//返回数据指针
	return pRchgCell;
}

CBndCell* CRchg::ReadPerCellData(_RecordsetPtr pCellDataRec)
{
	//读取应力期网格单元源汇项数据(数据库)

	_variant_t Value;
	long iLyr, iRow, iCol, iRechrg;
	double rechrate;
	CRchgCell* pRchgCell = NULL;
	Value = pCellDataRec->GetCollect(_T("IRECH"));
	iRechrg = Value.intVal;
	if (iRechrg < 1 || iRechrg > 2)
	{
		printf(_T("\n"));
		printf(_T("面上补给选项参数IRECH只能为1或2, 请检查<面上补给_应力期数据表>！\n"));
		PauAndQuit();
	}
	if (iRechrg == 1)
	{
		Value = pCellDataRec->GetCollect(_T("ILYR"));
		iLyr = Value.intVal;
	}
	else
	{
		iLyr = 0;
	}
	Value = pCellDataRec->GetCollect(_T("IROW"));
	iRow = Value.intVal;
	Value = pCellDataRec->GetCollect(_T("ICOL"));
	iCol = Value.intVal;
	if (iRechrg == 1)
	{
		GDWMOD.CheckCellLimit(iLyr, iRow, iCol);
	}
	else
	{
		GDWMOD.CheckCellLimit(iRow, iCol);
	}
	Value = pCellDataRec->GetCollect(_T("RECHR"));
	rechrate = Value.dblVal;
	if (rechrate < 0.0)
	{
		printf(_T("\n"));
		printf(_T("面上补给量数据必须大于等于0, 请检查<面上补给_应力期数据表>!\n"));
		PauAndQuit();
	}
	pRchgCell = new CRchgCell;
	pRchgCell->m_ILYR = iLyr;
	pRchgCell->m_IROW = iRow;
	pRchgCell->m_ICOL = iCol;
	pRchgCell->m_IRECHRG = iRechrg;
	//计算单元上的面上补给强度(L3/T)
	pRchgCell->m_RechRate = rechrate *
		GDWMOD.GetCellArea(iRow, iCol);

	//返回数据指针
	return pRchgCell;
}

void CRchg::FormMatrix(long IBOUND[], double HNEW[],      
	double HCOF[], double RHS[], double HOLD[], double HPRE[])
{
	//根据面状补给边界条件形成/处理矩阵方程

	long NUMLYR = GDWMOD.GetNumLyr();
	long NumBndCell = long(m_pCurPerData->m_pBndCellAry.size());
	//逐个面上补给单元进行循环
	long NumTask = long(NumBndCell / GDWMOD.m_NUMTD / 2.0) + 1;
#pragma omp parallel for num_threads(GDWMOD.m_NUMTD) schedule(dynamic,NumTask)
	for (long i = 0; i < NumBndCell; i++)
	{
		//注释: 以下几个变量的定义必须放在for块中不能共用, 以适应并行运算要求
		long iLyr, iRow, iCol, k, node;
		CRchgCell* pRechCell = NULL;
		double rechrate = 0.0;
		BOOL lFind = FALSE;
		//注释完毕
		pRechCell = (CRchgCell*)m_pCurPerData->m_pBndCellAry[i];
		iLyr = pRechCell->m_ILYR;
		iRow = pRechCell->m_IROW;
		iCol = pRechCell->m_ICOL;
		if (pRechCell->m_IRECHRG == 0)
		{
			//按指定层位网格单元计算面上补给
			node = GDWMOD.GetCellNode(iLyr, iRow, iCol);
			if (IBOUND[node] > 0)
			{
				//网格单元为变水头网格单元
				lFind = TRUE;
			}
		}
		else
		{
			//按最高层位变水头网格单元计算面上补给
			//从顶层开始,往下找到第一个变水头网格单元
			for (k = 1; k <= NUMLYR; k++)
			{
				node = GDWMOD.GetCellNode(k, iRow, iCol);
				if (IBOUND[node] > 0)
				{
					//网格单元为变水头网格单元
					lFind = TRUE;
					break;
				}
				else
				{
					if (IBOUND[node] < 0)
					{
						//网格单元为定水头网格单元
						//认为补给量被该层定水头网格单元拦截
						//以下层都不用判断了
						break;
					}
				}
			}
		}
		if (lFind)
		{
			//如果找到了变水头网格单元
			rechrate = pRechCell->m_RechRate;
			RHS[node] = RHS[node] - rechrate;
		}
	}
}

void CRchg::Budget(long IBOUND[], double HNEW[], double HOLD[])
{
	//计算面状补给边界处水量通量

	long NUMLYR = GDWMOD.GetNumLyr();
	long NumBndCell = long(m_pCurPerData->m_pBndCellAry.size());
	//逐个面上补给单元进行循环
	long NumTask = long(NumBndCell / GDWMOD.m_NUMTD / 2.0) + 1;
#pragma omp parallel for num_threads(GDWMOD.m_NUMTD) schedule(dynamic,NumTask)
	for (long i = 0; i < NumBndCell; i++)
	{
		//注释: 以下几个变量的定义必须放在for块中不能共用, 以适应并行运算要求
		long iLyr, iRow, iCol, k, node;
		CRchgCell* pRechCell = NULL;
		CGridCell * pGridCell = NULL;
		double rechrate = 0.0;
		BOOL lFind = FALSE;
		//注释完毕
		pRechCell = (CRchgCell*)m_pCurPerData->m_pBndCellAry[i];
		iLyr = pRechCell->m_ILYR;
		iRow = pRechCell->m_IROW;
		iCol = pRechCell->m_ICOL;
		if (pRechCell->m_IRECHRG == 0)
		{
			//按指定层位网格单元计算面上补给
			node = GDWMOD.GetCellNode(iLyr, iRow, iCol);
			if (IBOUND[node] > 0)
			{
				pGridCell = GDWMOD.GetGridCell(iLyr, iRow, iCol);
				lFind = TRUE;
			}
		}
		else
		{
			//按最高层位变水头网格单元计算面上补给
			//从顶层开始,往下找到第一个变水头网格单元
			for (k = 1; k <= NUMLYR; k++)
			{
				node = GDWMOD.GetCellNode(k, iRow, iCol);
				if (IBOUND[node] > 0)
				{
					//网格单元为变水头网格单元
					pGridCell = GDWMOD.GetGridCell(k, iRow, iCol);
					lFind = TRUE;
					break;
				}
				else
				{
					if (IBOUND[node] < 0)
					{
						//网格单元为定水头网格单元
						//认为补给量被该层定水头网格单元拦截
						//以下层都不用判断了
						break;
					}
				}
			}
		}
		if (lFind)
		{
			//如果找到了有效单元
			rechrate = pRechCell->m_RechRate;
			pGridCell->AddSinkR(m_BndID, rechrate);
		}
	}
}