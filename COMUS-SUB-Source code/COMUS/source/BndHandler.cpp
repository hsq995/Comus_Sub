#include "StdAfx.h"
#include "ClassInclude.h"

//////////////////////////////////////////////////////////
//边界单元类函数定义
CBndCell::CBndCell()
{
	m_ILYR = 0;                               //边界单元对应的网格的层号（从1开始编号）
	m_IROW = 0;                               //边界单元对应的网格的行号（从1开始编号）
	m_ICOL = 0;                               //边界单元对应的网格的列号（从1开始编号）
}

CBndCell::~CBndCell()
{

}

//////////////////////////////////////////////////////////
//应力期边界数据类函数定义
CPerBndData::CPerBndData(void)
{
	m_iPeriod = 0;                     //应力期编号
}

CPerBndData::~CPerBndData(void)
{
	DeleteBndData();
}

void CPerBndData::AddBndCell(CBndCell* pBndCell, long iPer)
{
	//添加对应应力期的源汇项网格单元数据

	//检查网格单元源汇项数据应力期与当前应力期的一致性
	if (m_pBndCellAry.size() > 0 && iPer != m_iPeriod)
	{
		//应力期数据对象中有数据,且要添加的网格单元源汇项
		//数据的应力期与当前应力期不一致, 不允许添加到数组
		printf(_T("\n"));
		printf(_T("要添加的网格单元源汇项的应力期与当前应力期不一致, 请检查!\n"));
		PauAndQuit();
	}

	//添加应力期网格单元源汇项数据
	long NodeID;
	if (pBndCell->m_ILYR >= 1)
	{
		GDWMOD.CheckCellLimit(pBndCell->m_ILYR, pBndCell->m_IROW, pBndCell->m_ICOL);
		NodeID = GDWMOD.GetCellNode(pBndCell->m_ILYR, pBndCell->m_IROW, pBndCell->m_ICOL);
	}
	else
	{
		GDWMOD.CheckCellLimit(pBndCell->m_IROW, pBndCell->m_ICOL);
		NodeID = GDWMOD.GetCellNode(1, pBndCell->m_IROW, pBndCell->m_ICOL);
	}
	if (m_pBndCellAry.size() == 0)
	{
		//应力期的首个网格单元源汇项数据
		//设置当前应力期编号
		m_iPeriod = iPer;
		//将网格单元源汇项数据加入数组
		m_pBndCellAry.push_back(pBndCell);
		m_NodeMap[NodeID] = pBndCell;
	}
	else
	{
		//检查网格单元数据是否重复
		auto result = m_NodeMap.find(NodeID);
		if (result == m_NodeMap.end())
		{
			m_pBndCellAry.push_back(pBndCell);
			m_NodeMap[NodeID] =  pBndCell;
		}
		else
		{
			//有重复记录.报错
			printf(_T("\n"));
			printf(_T("%d应力期%d层%d行%d列的源汇项数据有重复!请检查!\n"),
				m_iPeriod, pBndCell->m_ILYR, pBndCell->m_IROW,
				pBndCell->m_ICOL);
			PauAndQuit();
		}
	}
}

void CPerBndData::DeleteBndData()
{
	//删除现有的边界/源汇项数据

	long i;
	for (i = 0; i < m_pBndCellAry.size(); i++)
	{
		delete m_pBndCellAry[i];
	}
	m_pBndCellAry.clear();
	m_NodeMap.clear();
	m_iPeriod = 0;
}

//////////////////////////////////////////////////////////
//源汇项处理类模板函数定义
CBndHandler::CBndHandler()
{
	m_BndID = 0;                       //边界/源汇项统计输出时的编号
	m_iPrnOpt = 0;                     //边界/源汇项的模拟结果输出控制. 0:不输出；1:逐应力期内模拟时段输出；2:逐应力期输出
	m_pPerDataTable = NULL;            //应力期数据记录集对象
	m_pCurPerData = NULL;              //当前应力期数据指针
}

CBndHandler::~CBndHandler()
{
	//删除应力期数据对象
	if (m_pCurPerData != NULL)
	{
		delete m_pCurPerData;
		m_pCurPerData = NULL;
	}

	//关闭应力期数据表
	if (m_pPerDataTable != NULL)
	{
		if (m_pPerDataTable->State)
		{
			m_pPerDataTable->Close();
		}
	}

	//关闭应力期数据文件
	if (m_PerDataFile.m_hFile != CStdioFile::hFileNull)
	{
		m_PerDataFile.Close();
	}
}

void CBndHandler::SetBndBaseInfo(CString StrObjName, CString StrBndNamAbb, CString StrBndMean, CString StrPerDataTB)
{
	//设置边界/源汇项对象相关的名称信息
	//注: StrObjName: 边界/源汇项的名称(中文), 数据有问题时用于输出信息
	//StrBndNamAbb: 边界/源汇项的英文简称(只能3个字符), 用于源汇项的统计
	//StrBndMean: 边界/源汇项的英文释义(只能16个字符), 用于源汇项的输出
	//StrPerDataTB: 该边界/源汇项对象的Sql数据库应力期数据表/TXT文件名称

	m_BndObjNam = StrObjName;
	m_BndNamAbb = StrBndNamAbb;
	m_BndMean = StrBndMean;
	m_PerDataTbNam = StrPerDataTB;
	//去掉字符串中的空格
	TrimBlank(m_BndObjNam);
	TrimBlank(m_PerDataTbNam);

	//检查字符串的长度
	CString strError;
	if (m_BndNamAbb.GetLength() != 3)
	{
		printf(_T("\n"));
		strError.Format(_T("边界/源汇项的简称必须是3个字符, 请检查: %s!\n"), m_BndNamAbb);
		printf(strError);
		PauAndQuit();
	}
	if (m_BndMean.GetLength() != 16)
	{
		printf(_T("\n"));
		strError.Format(_T("边界/源汇项的释义必须是16个字符, 请检查: %s!\n"), m_BndMean);
		printf(strError);
		PauAndQuit();
	}
}

void CBndHandler::SetPrnOpt(long iPrnOpt)
{
	//设置边界/源汇项模拟结果输出控制

	m_iPrnOpt = iPrnOpt;
}

void CBndHandler::SetPerData(const CString& strFilePath)
{
	//设置应力期数据表(TXT)

	CString strTbInfo, strTemp, strTest;
	//输出应力期数据表名称
	strTbInfo = m_PerDataTbNam + _T(".in\n");
	printf(strTbInfo);

	//产生应力期数据对象
	m_pCurPerData = new CPerBndData;

	//打开应力期数据文件
	strTbInfo = strFilePath + m_PerDataTbNam + ".in";
	if (!m_PerDataFile.Open(strTbInfo, CStdioFile::modeRead | CStdioFile::shareExclusive | CStdioFile::typeText))
	{
		printf(_T("\n"));
		printf(_T("打开文件<%s>失败!\n"), LPCSTR(m_PerDataTbNam + ".in"));
		PauAndQuit();
	}

	//略过表头
	m_PerDataFile.ReadString(strTemp);

	//检查编号为1的应力期数据是否存在
	for (;;)
	{
		//读入1行数据
		m_PerDataFile.ReadString(strTemp);
		//测试读入的数据
		strTest = strTemp;
		strTest.Trim();
		if (strTest.IsEmpty())
		{
			//没有读到数据
			if (m_PerDataFile.GetPosition() == m_PerDataFile.GetLength())
			{
				//到了文件末尾还未读到第1行有效数据, 报错
				printf(_T("\n"));
				printf(_T("文件<%s>中没有第1个应力期的数据!\n"), LPCSTR(m_PerDataTbNam));
				PauAndQuit();
			}
			else
			{
				//没到文件末尾, 说明是个空行, 略过继续
				continue;
			}
		}
		//读到了第1行有效数据
		//测试应力期编号是否从1开始
		long FirstPer;
		if (sscanf_s(strTemp, _T("%ld"), &FirstPer) != 1)
		{
			printf(_T("\n"));
			printf(_T("文件<%s>中第一行的数据缺失或有其他问题!\n"), LPCSTR(m_PerDataTbNam));
			PauAndQuit();
		}
		if (FirstPer != 1)
		{
			//如果第一行数据的应力期编号不为1
			printf(_T("\n"));
			printf(_T("文件<%s>中应力期数据的编号没从1开始!\n"), LPCSTR(m_PerDataTbNam));
			PauAndQuit();
		}
		//跳出循环
		break;
	}
	//回退到文件数据初始行
	m_PerDataFile.SeekToBegin();
	//略过表头
	m_PerDataFile.ReadString(strTemp);
}

void CBndHandler::SetPerData(_ConnectionPtr pConnection)
{
	//设置Sql数据库应力期数据表(数据库)

	//输出数据表名称
	CString strTbInfo;
	strTbInfo = _T("<");
	strTbInfo = strTbInfo + m_PerDataTbNam;
	strTbInfo = strTbInfo + _T(">\n");
	printf(strTbInfo);

	//产生应力期数据对象
	m_pCurPerData = new CPerBndData;

	//实例化应力期数据表对象
	m_pPerDataTable.CreateInstance(__uuidof(Recordset));
	//将数据集游标设置在客户端(可提高性能)
	m_pPerDataTable->CursorLocation = adUseClient;
	//设置数据集缓存(1000条)
	m_pPerDataTable->CacheSize = 1000;
	try
	{
		//检查编号为1的应力期数据是否存在
	    long FirstPer;
		CString strSQL, strErrInfo;
		strSQL = _T("SELECT * FROM ");
		strSQL = strSQL + m_PerDataTbNam;
		strSQL = strSQL + _T(" WHERE IPER = 1");
		m_pPerDataTable->Open(LPCSTR(strSQL),
			pConnection.GetInterfacePtr(),
			adOpenStatic,
			adLockOptimistic,
			adCmdText);
		if (m_pPerDataTable->BOF)
		{
			//表中没有编号为1的应力期数据, 报错退出
			printf(_T("\n"));
			strErrInfo = _T("<源汇项模拟选项表>中");
			strErrInfo = strErrInfo + _T("已选择模拟");
			strErrInfo = strErrInfo + m_BndObjNam;
			strErrInfo = strErrInfo + _T("\n");
			printf(strErrInfo);
			strErrInfo = _T("<");
			strErrInfo = strErrInfo + m_PerDataTbNam;
			strErrInfo = strErrInfo + _T(">");
			strErrInfo = strErrInfo + _T("中至少要有编号为1的应力期数据,请检查！\n");
			printf(strErrInfo);
			PauAndQuit();
		}
		m_pPerDataTable->Close();

		//检查首个应力期数据编号是否从1开始
		//注: 有应力期编号为1的数据并不能代表应力期数据编号是从1开始的
		strSQL = _T("SELECT TOP 1 * FROM ");
		strSQL = strSQL + m_PerDataTbNam;
		strSQL = strSQL + _T(" ORDER BY IPER");
		m_pPerDataTable->Open(LPCSTR(strSQL),
			pConnection.GetInterfacePtr(),
			adOpenStatic,
			adLockOptimistic,
			adCmdText);
		_variant_t Value;
		Value = m_pPerDataTable->GetCollect(_T("IPER"));
		FirstPer = Value.intVal;
		if (FirstPer != 1)
		{
			printf(_T("\n"));
			strErrInfo = _T("<");
			strErrInfo = strErrInfo + m_PerDataTbNam;
			strErrInfo = strErrInfo + _T(">");
			strErrInfo = strErrInfo + _T("中应力期数据的编号没有从1开始,请检查！\n");
			printf(strErrInfo);
			PauAndQuit();
		}
		m_pPerDataTable->Close();
	}
	catch (_com_error e)
	{
		(e.Description());
		exit(1);
	}
}

void CBndHandler::LinkDataInOut(const CString& strFilePath, long& BndID)
{
	//关联边界/源汇项的输入数据和输出项并检查(TXT)

	//设置应力期数据表
	SetPerData(strFilePath);
	//添加边界/源汇项统计输出编号和名称
	AddBndSinkInfo(BndID);
}

void CBndHandler::LinkDataInOut(_ConnectionPtr pConnection, long& BndID)
{
	//关联边界/源汇项的输入数据和输出项并检查(数据库)

	//设置应力期数据表
	SetPerData(pConnection);
	//添加边界/源汇项统计输出编号和名称
	AddBndSinkInfo(BndID);
}

void CBndHandler::AddBndSinkInfo(long& BndID)
{
	//添加边界/源汇项统计输出编号和名称

	m_BndID = BndID;
	CGridCell::AddBndObjItem(BndID, m_BndNamAbb, m_BndMean);
}

void CBndHandler::PrePerSim(long iPer)
{
	//应力期模拟之前数据准备(TXT)
	//iPer: 应力期编号(从1开始)

	//如果没有应力期数据表直接返回
	if (m_PerDataTbNam.IsEmpty())
	{
		return;
	}

	assert(iPer >= 1);
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
	ULONGLONG PosPre;
	//循环读入编号为iPer的应力期数据
	SumDataRow = 0;
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
			printf(_T("文件<%s>中第%d个应力期第%d行数据的应力期编号数据异常, 请检查!\n"),
				LPCSTR(m_PerDataTbNam), iPer, SumDataRow);
			PauAndQuit();
		}
		if (DataPer < iPer)
		{
			//当前数据行的应力期编号不能小于指定的应力期编号
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
			//如果是首次读到, 删除之前的应力期数据
			m_pCurPerData->DeleteBndData();
		}
		//读入单元数据
		pBndCell = ReadPerCellData(strTemp);
		m_pCurPerData->AddBndCell(pBndCell, iPer);
	}
}

void CBndHandler::PrePerSim(_ConnectionPtr pConnection, long iPer)
{
    //应力期模拟之前数据准备
	//iPer: 应力期编号(从1开始)

	//如果没有应力期数据表直接返回
	if (m_PerDataTbNam.IsEmpty())
	{
		return;
	}

	assert(iPer >= 1);
	//读入当前应力期数据
	//注：必须用try块,否则数据表不存在时程序直接会报错,无法继续执行
    CString strSQL, strPer;
	CBndCell* pBndCell = NULL;
	long i, nCount;
	try
	{
		//打开数据表读数据
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
			//当前应力期数据存在,
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

void CBndHandler::InitSimulation()
{
	//进行边界/源汇项应力期数据的初始化
	//该函数在CGDWater对象的initsimulation()函数中调用
	//具体对象可根据自身情况对该函数进行重载

	return;
}

void CBndHandler::PreStepSim(long IBOUND[], double HNEW[], double HOLD[])
{
	//时段模拟之前数据准备
	//该函数在CGDWater对象的SimOneStep()函数中调用
	//具体对象可根据自身情况对该函数进行重载

	return;
}

void CBndHandler::SaveResult()
{
	//保存边界/源汇项的计算结果
	//该函数在CGDWater对象的SaveResult()函数中调用
	//具体对象可根据自身情况对该函数进行重载

	return;
}

void CBndHandler::BndOutPutTXT(BOOL bEndPer, long iPer, long iStep, double CumTLen, double PerLen, double DeltT)
{
	//输出边界/源汇项的模拟计算结果(TXT)
	//该函数在CGDWater对象的OutPutAllTXT函数中调用
	//具体对象可根据自身情况对该函数进行重载
	//bEndPer: 应力期是否模拟结束表示
	//iPer: 应力期编号
	//iStep: 模拟时段编号
	//CumTLen: 从模拟开始到当前累计的模拟计算时长(T)
	//PerLen: 当前应力期的时长(T)
	//DeltT: 当前模拟时段的时长(T)

	return;
}

void CBndHandler::BndOutPutDB(BOOL bEndPer, long iPer, long iStep, double CumTLen, double PerLen, double DeltT)
{
	//输出边界/源汇项的模拟计算结果(数据库)
	//该函数在CGDWater对象的OutPutAllDB函数中调用
	//具体对象可根据自身情况对该函数进行重载
	//bEndPer: 应力期是否模拟结束表示
	//iPer: 应力期编号
	//iStep: 模拟时段编号
	//CumTLen: 从模拟开始到当前累计的模拟计算时长(T)
	//PerLen: 当前应力期的时长(T)
	//DeltT: 当前模拟时段的时长(T)

	return;
}