#include "StdAfx.h"
#include "ClassInclude.h"

//////////////////////////////////////////////////////////
//随时间变化水头边界单元类函数定义
CSHBCell::CSHBCell()
{
	m_StartHead = 0.0;                            //应力期初始时刻单元的水头(L)
	m_EndHead = 0.0;                              //应力期结束时刻单元的水头(L)
}

CSHBCell::~CSHBCell()
{

}

//////////////////////////////////////////////////////////
//时变定水头处理类函数定义
CSHB::CSHB()
{

}

CSHB::~CSHB()
{

}

void CSHB::PrePerSim(long iPer)
{
	//应力期模拟之前数据准备(TXT)		

	assert(iPer >= 1);
	long iLyr, iRow, iCol, node, CellType;
	CSHBCell* pSHBCell = NULL;
	long* IBOUNDAry = NULL;

	//如果没有应力期数据文件报错
	if (m_PerDataFile.m_hFile == CFile::hFileNull)
	{
		printf(_T("\n"));
		printf(_T("文件<%s>错误, 请检查!\n"), LPCSTR(m_PerDataTbNam + _T(".in")));
		PauAndQuit();
	}
	//如果已到文件末尾, 要用之前的应力期数据
	if (m_PerDataFile.GetPosition() == m_PerDataFile.GetLength())
	{
		//只需改变应力期数据的编号即可
		m_pCurPerData->m_iPeriod = iPer;
		//返回
		return;
	}

	CString strTemp, strTest;
	long DataPer, SumDataRow;
	CBndCell* pBndCell = NULL;
	BOOL bNewData;
	//循环读入编号为iPer的应力期数据
	SumDataRow = 0;
	bNewData = FALSE;
	ULONGLONG PosPre;
	for (;;)
	{
		//逐行读入
		PosPre = m_PerDataFile.GetPosition();
		m_PerDataFile.ReadString(strTemp);
		//累加读入的数据行数
		SumDataRow = SumDataRow + 1;
		//测试读入的数据
		strTest = strTemp;
		strTest.Trim();
		//如果没有读到有效数据
		if (strTest.IsEmpty())
		{
			SumDataRow = SumDataRow - 1;
			if (m_PerDataFile.GetPosition() == m_PerDataFile.GetLength())
			{
				//说明到了文件末尾, 跳出循环
				break;
			}
			else
			{
				//没到文件末尾, 说明是个空行, 略过继续
				continue;
			}
		}
		//读到了1行有效数据
		//检查当前数据行的应力期编号
		if (sscanf_s(strTemp, _T("%ld"), &DataPer) != 1)
		{
			//没有正常读到应力期编号数据, 报错
			printf(_T("\n"));
			printf(_T("文件<%s>中第%d个应力期第%d行数据的应力期编号异常, 请检查!\n"),
				LPCSTR(m_PerDataTbNam), iPer, SumDataRow);
			PauAndQuit();
		}
		if (DataPer < iPer)
		{
			//当前数据行应力期数据的应力期编号不能小于指定的应力期编号
			printf(_T("\n"));
			printf(_T("文件<%s>中的数据没有按照从小到大的应力期次序编号, 请检查第%d个应力期的数据!\n"),
				LPCSTR(m_PerDataTbNam), iPer);
			PauAndQuit();
		}
		if (DataPer > iPer)
		{
			//当前数据行的应力期编号大于指定的应力期编号(表明是后面应力期的数据)
			//读过的那一行必须要回退
			m_PerDataFile.Seek(PosPre, CFile::begin);
			//如果首次就读到了该数据, 说明文件中没有第iPer个应力期的数据
			if (SumDataRow == 1)
			{
				//要用之前应力期的数据
				//只需改变应力期数据的编号即可
				m_pCurPerData->m_iPeriod = iPer;
			}
			//跳出循环
			break;
		}
		//读到了对应iPer编号的应力期数据
		if (SumDataRow == 1)
		{
			//先要将上个应力期指定水头网格单元的模拟时属性还原为变水头网格单元
            //因为每个应力期的随时间变化水头网格单元的位置可能不一样
			IBOUNDAry = GDWMOD.GetAryIBOUND();
			if (m_pCurPerData->m_pBndCellAry.size() > 0)
			{
				for (size_t i = 0; i < m_pCurPerData->m_pBndCellAry.size(); i++)
				{
					pSHBCell = (CSHBCell*)m_pCurPerData->m_pBndCellAry[i];
					iLyr = pSHBCell->m_ILYR;
					iRow = pSHBCell->m_IROW;
					iCol = pSHBCell->m_ICOL;
					node = GDWMOD.GetCellNode(iLyr, iRow, iCol);
					if (IBOUNDAry[node] == -3000)
					{
						//如果上个应力期该单元对应的IBOUND数组元素值为-3000, 将其改为1
						IBOUNDAry[node] = 1;
					}
				}
			}
			//删除之前的应力期数据
			m_pCurPerData->DeleteBndData();
			bNewData = TRUE;
		}
		//读入单元数据
		pBndCell = ReadPerCellData(strTemp);
		m_pCurPerData->AddBndCell(pBndCell, iPer);
	}

	//检查当前应力期数据的合理性
	if (bNewData)
	{
		for (size_t i = 0; i < m_pCurPerData->m_pBndCellAry.size(); i++)
		{
			pSHBCell = (CSHBCell*)m_pCurPerData->m_pBndCellAry[i];
			iLyr = pSHBCell->m_ILYR;
			iRow = pSHBCell->m_IROW;
			iCol = pSHBCell->m_ICOL;
			CellType = GDWMOD.GetCellIniIBOUND(iLyr, iRow, iCol);
			if (CellType <= 0)
			{
				//随时间变化水头单元不能设置在初始无效单元或定常水头单元上，报错
				printf(_T("\n"));
				printf(_T("编号为第%d层第%d行第%d列的网格单元初始为无效单元或定常水头单元,\n"),
					iLyr, iRow, iCol);
				printf(_T("在第%d个应力期时不能设置为随时间变化水头单元,请检查!\n"),
					m_pCurPerData->m_iPeriod);
				PauAndQuit();
			}
		}
	}
}

void CSHB::PrePerSim(_ConnectionPtr pConnection, long iPer)
{
	//应力期模拟之前数据准备(iPer从1开始)

	assert(iPer >= 1);
	long i, nCount, node, iLyr, iRow, iCol, CellType;
	CSHBCell* pSHBCell = NULL;
	CBndCell* pBndCell = NULL;
	long* IBOUNDAry = NULL;

	//读入当前应力期数据
	//注：必须用try块,否则数据表不存在时程序直接会报错,无法继续执行
	try
	{
		//打开数据表读数据
		CString strSQL, strPer;
		strSQL = _T("SELECT * FROM ");
		strSQL = strSQL + m_PerDataTbNam;
		strSQL = strSQL + _T(" WHERE IPER = ");
		strPer.Format(_T("%d"), iPer);
		strSQL = strSQL + strPer;
		m_pPerDataTable->Open(LPCSTR(strSQL),
			pConnection.GetInterfacePtr(),
			adOpenStatic,
			adLockOptimistic,
			adCmdText);
		if (m_pPerDataTable->BOF)
		{
			//当前应力期数据不存在, 要用之前的应力期数据
			//只需改变应力期数据的编号即可
			m_pCurPerData->m_iPeriod = iPer;
		}
		else
		{
			//当前应力期数据存在
			//先要将上个应力期指定水头网格单元的模拟时属性还原为变水头网格单元
			//因为每个应力期的随时间变化水头网格单元的位置可能不一样
			IBOUNDAry = GDWMOD.GetAryIBOUND();
			if (m_pCurPerData->m_pBndCellAry.size() > 0)
			{
				for (i = 0; i < m_pCurPerData->m_pBndCellAry.size(); i++)
				{
					pSHBCell = (CSHBCell*)m_pCurPerData->m_pBndCellAry[i];
					iLyr = pSHBCell->m_ILYR;
					iRow = pSHBCell->m_IROW;
					iCol = pSHBCell->m_ICOL;
					node = GDWMOD.GetCellNode(iLyr, iRow, iCol);
					if (IBOUNDAry[node] == -3000)
					{
						//如果上个应力期该单元对应的IBOUND数组元素值为-3000, 将其改为1
						IBOUNDAry[node] = 1;
					}
				}
			}
			//删除之前的应力期数据
			m_pCurPerData->DeleteBndData();
			//读入当前的应力期数据
			m_pPerDataTable->MoveLast();
			nCount = m_pPerDataTable->GetRecordCount();  //数据个数
			m_pPerDataTable->MoveFirst();
			for (i = 0; i < nCount; i++)
			{
				pBndCell = ReadPerCellData(m_pPerDataTable);
				m_pCurPerData->AddBndCell(pBndCell, iPer);
				m_pPerDataTable->MoveNext();                                       //定位到下一条数据
			}			
			//检查当前应力期数据的合理性
			for (i = 0; i < m_pCurPerData->m_pBndCellAry.size(); i++)
			{
				pSHBCell = (CSHBCell*)m_pCurPerData->m_pBndCellAry[i];
				iLyr = pSHBCell->m_ILYR;
				iRow = pSHBCell->m_IROW;
				iCol = pSHBCell->m_ICOL;
				CellType = GDWMOD.GetCellIniIBOUND(iLyr, iRow, iCol);
				if (CellType <= 0)
				{
					//随时间变化水头单元不能设置在初始无效单元或定常水头单元上，报错
					printf(_T("\n"));
					printf(_T("编号为第%d层第%d行第%d列的网格单元初始为无效单元或定常水头单元,\n"),
						iLyr, iRow, iCol);
					printf(_T("在第%d个应力期时不能设置为随时间变化水头单元,请检查!\n"), 
						m_pCurPerData->m_iPeriod);
					PauAndQuit();
				}
			}
		}
		//关闭数据表
		m_pPerDataTable->Close();
	}
	catch (_com_error e)
	{
		printf(LPCTSTR(e.Description()));
		exit(1);
	}
}

CBndCell* CSHB::ReadPerCellData(CString strData)
{
	//读取应力期网格单元源汇项数据(TXT)

	assert(!strData.IsEmpty());
	CSHBCell* pSHBCell = NULL;
	CGridCell* pGridCell = NULL;
	long iPer, iLyr, iRow, iCol, LyrCon;
	double StartHead, EndHead, CellBot;
	CString strFormat;
	strFormat = _T("%ld %ld %ld %ld %lg %lg");
	if (sscanf_s(strData, strFormat, &iPer, &iLyr, &iRow, &iCol, &StartHead, &EndHead) != 6)
	{
		printf(_T("\n"));
		printf(_T("时变水头_应力期数据表.in 文件中的数据异常, 请检查!\n"));
		PauAndQuit();
	}
	//检查数据
	GDWMOD.CheckCellLimit(iLyr, iRow, iCol);
	LyrCon = GDWMOD.GetLyrCon(iLyr);
	if (LyrCon == 1 || LyrCon == 3)
	{
		//如果时变水头位于含水层类型1或3
		pGridCell = GDWMOD.GetGridCell(iLyr, iRow, iCol);
		CellBot = pGridCell->GetCellBot();
		if (StartHead <= CellBot || EndHead <= CellBot)
		{
			printf(_T("\n"));
			printf(_T("网格单元(K=%d,I=%d,J=%d)处的时变水头不能低于或等于网格单元的底板高程, 请检查 时变水头_应力期数据表.in !\n"),
				iLyr, iRow, iCol);
			PauAndQuit();
		}
	}
	pSHBCell = new CSHBCell;
	pSHBCell->m_ILYR = iLyr;
	pSHBCell->m_IROW = iRow;
	pSHBCell->m_ICOL = iCol;
	pSHBCell->m_StartHead = StartHead;
	pSHBCell->m_EndHead = EndHead;

	//返回数据指针
	return pSHBCell;
}

CBndCell* CSHB::ReadPerCellData(_RecordsetPtr pCellDataRec)
{
	//读取应力期网格单元源汇项数据(数据库)

	_variant_t Value;
	long iLyr, iRow, iCol, LyrCon;
	double StartHead, EndHead, CellBot;
	CSHBCell* pSHBCell = NULL;
	CGridCell* pGridCell = NULL;
	CPerBndData* pPerGHdData = NULL;
	Value = pCellDataRec->GetCollect(_T("ILYR"));
	iLyr = Value.intVal;
	Value = pCellDataRec->GetCollect(_T("IROW"));
	iRow = Value.intVal;
	Value = pCellDataRec->GetCollect(_T("ICOL"));
	iCol = Value.intVal;
	GDWMOD.CheckCellLimit(iLyr, iRow, iCol);
	Value = pCellDataRec->GetCollect(_T("SHEAD"));
	StartHead = Value.dblVal;
	Value = pCellDataRec->GetCollect(_T("EHEAD"));
	EndHead = Value.dblVal;
	LyrCon = GDWMOD.GetLyrCon(iLyr);
	if (LyrCon == 1 || LyrCon == 3)
	{
		//如果时变水头位于含水层类型1或3
		pGridCell = GDWMOD.GetGridCell(iLyr, iRow, iCol);
		CellBot = pGridCell->GetCellBot();
		if (StartHead <= CellBot || EndHead <= CellBot)
		{
			printf(_T("\n"));
			printf(_T("网格单元(K=%d,I=%d,J=%d)处的时变水头不能低于或等于网格单元的底板高程, 请检查<时变水头_应力期数据表>！\n"),
				iLyr, iRow, iCol);
			PauAndQuit();
		}
	}
	pSHBCell = new CSHBCell;
	pSHBCell->m_ILYR = iLyr;
	pSHBCell->m_IROW = iRow;
	pSHBCell->m_ICOL = iCol;
	pSHBCell->m_StartHead = StartHead;
	pSHBCell->m_EndHead = EndHead;

	//返回数据指针
	return pSHBCell;
}

void CSHB::PreStepSim(long IBOUND[], double HNEW[], double HOLD[])
{
	//时段模拟之前数据准备
	//注: 设置当前计算时段的单元水头条件

	//获得当前应力期时长
	double PerLen = GDWMOD.GetPerLen();
	//获得从当前应力开始到当前的计算时长
	double PerCumDeltT = GDWMOD.GetPerCumDeltT();
	long NumBndCell = long(m_pCurPerData->m_pBndCellAry.size());
	//逐个时变水头单元进行循环
	long NumTask = long(NumBndCell / GDWMOD.m_NUMTD / 2.0) + 1;
#pragma omp parallel for num_threads(GDWMOD.m_NUMTD) schedule(dynamic,NumTask)
	for (long i = 0; i < NumBndCell; i++)
	{
		//注释: 以下几个变量的定义必须放在for块中不能共用, 以适应并行运算要求
		long iLyr, iRow, iCol, node, LyrCon;
		CSHBCell* pSHBCell = NULL;
		CGridCell* pGridCell = NULL;
		double CellHead;
		//注释完毕
		pSHBCell = (CSHBCell*)m_pCurPerData->m_pBndCellAry[i];
		iLyr = pSHBCell->m_ILYR;
		iRow = pSHBCell->m_IROW;
		iCol = pSHBCell->m_ICOL;
		node = GDWMOD.GetCellNode(iLyr, iRow, iCol);
		pGridCell = GDWMOD.GetGridCell(iLyr, iRow, iCol);
		LyrCon = GDWMOD.GetLyrCon(iLyr);
		//先将网格单元处的IBOUND设置为1(表示该单元为变水头单元)
		IBOUND[node] = 1;
		//计算当前计算时段网格单元处水头值
		CellHead = pSHBCell->m_StartHead + (pSHBCell->m_EndHead - pSHBCell->m_StartHead) *
			PerCumDeltT / PerLen;
		if (LyrCon == 1 || LyrCon == 3)
		{
			//如果网格单元所在含水层类型为1或3
			//只有在水头值高于含水层底板时才有意义
			if (CellHead > pGridCell->m_BOT)
			{
				IBOUND[node] = -3000;
				HNEW[node] = CellHead;
				HOLD[node] = CellHead;
			}
		}
		else
		{
			//如果网格单元所在含水层类型为0或2
			//水头值的设定没有任何限制
			IBOUND[node] = -3000;
			HNEW[node] = CellHead;
			HOLD[node] = CellHead;
		}
	}
}

void CSHB::FormMatrix(long IBOUND[], double HNEW[],
	double HCOF[], double RHS[], double HOLD[], double HPRE[])
{ 
	//根据随时间变化水头边界条件形成/处理矩阵方程

	//直接返回
	return;
}

void CSHB::Budget(long IBOUND[], double HNEW[], double HOLD[])
{
	//计算随时间变化水头边界处水量通量

	long NumBndCell = long(m_pCurPerData->m_pBndCellAry.size());
	//逐个时变水头单元进行循环
	long NumTask = long(NumBndCell / GDWMOD.m_NUMTD / 2.0) + 1;
#pragma omp parallel for num_threads(GDWMOD.m_NUMTD) schedule(dynamic,NumTask)
	for (long i = 0; i < NumBndCell; i++)
	{
		//注释: 以下几个变量的定义必须放在for块中不能共用, 以适应并行运算要求
		long iLyr, iRow, iCol, node;
		CSHBCell* pSHBCell = NULL;
		CGridCell* pGridCell = NULL;
		double SHBFlow;
		//注释完毕
		pSHBCell = (CSHBCell*)m_pCurPerData->m_pBndCellAry[i];
		iLyr = pSHBCell->m_ILYR;
		iRow = pSHBCell->m_IROW;
		iCol = pSHBCell->m_ICOL;
		node = GDWMOD.GetCellNode(iLyr, iRow, iCol);
		pGridCell = GDWMOD.GetGridCell(iLyr, iRow, iCol);
		if (IBOUND[node] == -3000)
		{
			//统计流入/流出量
			SHBFlow = pGridCell->m_FlowRight + pGridCell->m_FlowLeft + pGridCell->m_FlowFront +
				pGridCell->m_FlowBack + pGridCell->m_FlowUp + pGridCell->m_FlowDown;
			//数据统计到地下水网格单元上
			pGridCell->AddSinkR(m_BndID, SHBFlow);
		}
	}
}