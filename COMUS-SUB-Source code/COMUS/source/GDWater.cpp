#include "StdAfx.h"
#include "ClassInclude.h"
using namespace System::Data; 

//设置全局变量
CGDWater GDWMOD;

//////////////////////////////////////////////////////////
//暂停并退出函数定义
void PauAndQuit(void)
{
	printf(_T("\n"));
	system(_T("pause"));
	exit(0);
}

//////////////////////////////////////////////////////////
//判断文件是否存在
BOOL CheckFileExist(const CString &strFileNam)
{
	ifstream FileTest(strFileNam);
	return FileTest.good();
}

//////////////////////////////////////////////////////////
//判断目录是否存在
BOOL CheckFolderExist(const CString &strPath)
{
	WIN32_FIND_DATA wfd;
	BOOL  bValue = FALSE;
	HANDLE hFind = FindFirstFile(strPath, &wfd);
	if ((hFind != INVALID_HANDLE_VALUE) && (wfd.dwFileAttributes  &  FILE_ATTRIBUTE_DIRECTORY))
	{
		bValue = TRUE;
	}
	FindClose(hFind);
	return bValue;
}
//////////////////////////////////////////////////////////
//清空文件夹下的所有文件
void DelDirFiles(CString pathStr)
{
	CFileFind find;
	CString myDataPath, fdPath;
	myDataPath = pathStr + "*.*";
	BOOL bf = find.FindFile(myDataPath);
	while (bf)
	{
		bf = find.FindNextFile();
		fdPath = find.GetFilePath();
		if (!find.IsDots())
		{
			DeleteFile(fdPath);
		}
	}
	find.Close();
}

//////////////////////////////////////////////////////////
//判断文件是否为不带BOM的UTF-8文件
bool IsUTF8WithoutBOM(char* _data, int size)
{
	char *data = new char[size];
	memset(data, 0, size);
	memcpy(data, _data, size);
	int encodingBytesCount = 0;
	bool allTextsAreASCIIChars = true;
	for (int i = 0; i<size; i++)
	{
		char current = data[i];
		//判断是否都是ASCII字符，当包括中文字符时allTextsAreASCIIChars为false
		if ((current & 0x80) == 0x80)
		{
			allTextsAreASCIIChars = false;
		}
		//判断是否为一个字符的开始字节
		if (encodingBytesCount == 0)
		{
			if ((current & 0x80) == 0)
			{
				// ASCII字符的区间为0x00-0x7F
				continue;
			}
			if ((current & 0xC0) == 0xC0)   ///大于等于两字节
			{
				encodingBytesCount = 1;
				current <<= 2;
				//超过两个字节数据用来编码UNICODE字符
				//计算实际的长度
				while ((current & 0x80) == 0x80)
				{
					current <<= 1;  //判断下一位是否为1
					encodingBytesCount++;   //当前字符编码字节数
				}
			}
			else
			{
				//不符合UTF8编码规则的文件格式
				return false;
			}
		}
		else
		{
			//后续字节必须以10开头
			if ((current & 0xC0) == 0x80)   ///当前字节是否以10开头
			{
				encodingBytesCount--;
			}
			else
			{
				//不符合UTF8编码规则的文件格式
				return false;
			}
		}
	}
	if (encodingBytesCount != 0)
	{
		//不符合UTF8编码规则的文件格式
		return false;
	}
	return !allTextsAreASCIIChars;
}

//////////////////////////////////////////////////////////
//判断TXT文件文件编码类型
CString JudgeTxtFileCode(CString TxtFileNam)
{
	CString strFileCode;
	std::ifstream file;
	file.open(TxtFileNam, std::ios_base::in);
	if (file.good())
	{
		unsigned char s2;
		file.read((char*)&s2, 1);
		int p = s2 << 8;
		file.read((char*)&s2, 1);
		p |= s2;
		file.close();
		switch (p)
		{
		case 0xfffe:
			strFileCode = _T("UTF-16 LE");
			break;
		case 0xfeff:
			strFileCode = _T("UTF-16 BE");
			break;
		case 0xefbb:
			strFileCode = _T("UTF-8 with BOM");
			break;
		default:
			CFile QFile;
			if (QFile.Open(TxtFileNam, CFile::modeRead))
			{
				UINT size = (UINT)QFile.GetLength();
				char* buffer = new char[size];
				QFile.Read(buffer, size);
				QFile.Close();
				if (IsUTF8WithoutBOM(buffer, size))
				{
					//文件中不都为字符, 说明为不带BOM, 但与ANSI不兼容的UTF-8文件
					strFileCode = _T("UTF-8 without BOM Uncompatible to ANSI");
					break;
				}
				else
				{
					//文件中都为字符(即ANSI文件; 或为不带BOM, 但与ANSI完全兼容的UTF-8文件)
					strFileCode = _T("ANSI or UTF-8 without BOM Compatible to ANSI");
					break;
				}
			}
			//不认识
			strFileCode = _T("Unrecognizable");
			break;
		}
	}

	if (strFileCode == _T("UTF - 16 LE") || strFileCode == _T("UTF-16 BE"))
	{
		printf(_T("\n"));
		printf(_T("文本文件无效, 本程序仅支持<ANSI>以及<UTF-8>编码的文本文件!\n"));
		PauAndQuit();
	}
	return strFileCode;
}

//////////////////////////////////////////////////////////
//解决读取UTF-8文件时字符串出现乱码的问题
void CharToUTF8(CString &str)
{
	char *szBuf = new char[str.GetLength() + 1];//注意“+1”，char字符要求结束符，而CString没有
	memset(szBuf, '\0', str.GetLength() + 1);
	int i;
	for (i = 0; i < str.GetLength(); i++)
	{
		szBuf[i] = (char)str.GetAt(i);
	}
	int nLen;
	WCHAR *ptch;
	CString strOut;
	if (szBuf == NULL)
	{
		return;
	}
	nLen = MultiByteToWideChar(CP_UTF8, 0, szBuf, -1, NULL, 0);//获得需要的宽字符字节数
	ptch = new WCHAR[nLen];
	memset(ptch, '\0', nLen);
	MultiByteToWideChar(CP_UTF8, 0, szBuf, -1, ptch, nLen);
	str = ptch;
	if (NULL != ptch)
	{
		delete[] ptch;
	}
	ptch = NULL;
	if (NULL != szBuf)
	{
		delete[]szBuf;
	}
	szBuf = NULL;
}

//////////////////////////////////////////////////////////
//消除字符串中空格-回车等字符函数定义
void TrimBlank(CString &str)
{
	str.Remove(' ');
	str.Remove('\n');
	str.Remove('\r');
}

//////////////////////////////////////////////////////////
//地下水主模型类函数定义
CGDWater::CGDWater(void)
{
	m_DataContext =0;                //模拟工作的数据环境(1表示TXT数据, 2表示sql-Server数据库)
	m_NUMLYR = 0;                    //含水层的分层数
	m_NUMROW = 0;                    //含水层的行数
	m_NUMCOL = 0;                    //含水层的列数
	m_NUMRC = 0;                     //单层含水层中的网格单元数量
	m_XSTCORD = 0.0;                 //网格左上角的X坐标
	m_YSTCORD = 0.0;                 //网格左上角的Y坐标
	m_SIMMTHD = 0;                   //模拟方法选项. 1: 全有效单元法; 2: 原MODFLOW方法
	m_SIMTYPE = 0;                   //模拟类型. 1:稳定流；2:非稳定流
	m_LAMBDA = 0.0;                  //仅SIMMTHD=1且SIMTYPE=1时有效, 为网格单元差分方程右端附加项中的阻滞系数
	m_INTBLKM = 0;                   //含水层类型与网格单元数据的输入格式选项. 1: BCF格式; 2: LPF格式
	m_ISOLVE = 0;                    //矩阵方程的求解方式选项. 1: 强隐式法(Sip); 2: 预处理共轭梯度法(PCG)
	m_MAXIT = 0;                     //矩阵求解时最大迭代次数, 至少设为200(有效取值区间: 200~100000)
	m_DAMP = 0.0;                    //迭代计算延缓因子，一般设为1.0(有效取值区间：0.0001~1.0)
	m_HCLOSE = 0.0;                  //水位精度控制阈值指标(L), 一般设为0.0001m(有效取值区间：1e-8~1e-1)
	m_RCLOSE = 0.0;                  //仅ISOLVE=2时有效, 为矩阵方程残差控制阈值(L3/T)
	m_IRELAX = 0;                    //是否启用逐单元松弛迭代算法选项. 0: 不启用; 1: 启用
	m_THETA = 0.0;                   //仅IRELAX=1时有效. 为迭代计算过程出现震荡时动态松弛因子的调减系数
	m_GAMMA = 0.0;                   //仅IRELAX=1时有效. 为连续NITER次迭代计算不出现数值震荡时动态松弛因子的调增系数
	m_AKAPPA = 0.0;                  //仅IRELAX=1时有效. 为连续NITER次迭代计算不出现数值震荡时动态松弛因子的单位调增量
	m_NITER = 0;                     //仅IRELAX=1时有效. 为模型调增动态松弛因子时所需的连续无数值震荡次数
	m_ICHFLG = 0;                    //是否计算相邻两个水头单元间流量选项。0: 不计算；1: 计算
	m_HNOFLO = 0.0;                  //无效或不透水计算单元的水头值(一般为一个负的极大值)
	m_IWDFLG = 0;                    //仅SIMMTD=2时有效, 为是否模拟单元的干湿转化选项. 0: 不模拟; 1: 采用试算法模拟
	m_WETFCT = 0.0;                  //单元从疏干状态向湿润状态转化计算时试算水头的相乘因子(有效值0.0-1.0, 需大于0.0可等于1.0)
	m_NWETIT = 0;                    //迭代过程中每隔m_NWETIT个迭代尝试湿润疏干的单元(至少设置为间隔1次)
	m_IHDWET = 0;                    //单元从疏干向湿润转换时算法选项. 1:不使用指定的WETDRY数据；2：使用指定的WETDRY数据
	m_IREGSTA = 0;                   //是否启用分区水量平衡统计功能. 0: 不启用; 1: 启用.需要在<水量平衡统计分区表>中输入数据.
	m_IOBSOUT = 0;                   //是否输出网格单元观测点模拟结果选项. 0: 不输出; 1: 输出.若选择输出, 用户需要在<网格单元观测点表>中准备数据
	m_IMULTD = 0;                    //是否启用多线程并行计算选项. 0: 不启用; 1: 启用
	m_NUMTD = 0;                     //仅IMULTD=1时有效. 为并行计算时采用的线程数
	m_SIMSHB = 0;                    //是否模拟时变水头选项. 0:不模拟; 1: 模拟
	m_SIMRCH = 0;                    //是否模拟面上补给选项. 0:不模拟; 1: 模拟
	m_SIMWEL = 0;                    //是否模拟井流选项. 0:不模拟; 1: 模拟
	m_SIMSUB = 0;                    //仅模拟非稳定流时有效. 为是否模拟地面沉降选项. 0:不模拟; 1: 模拟
	m_IGDWBDPRN = 0;                 //地下水全系统水量平衡输出控制：0、不输出；1、逐应力期内模拟时段输出；2、逐应力期输出
	m_ILYRBDPRN = 0;                 //含水层水量平衡输出控制：0、不输出；1、逐应力期内模拟时段输出；2、逐应力期输出
	m_ICELLBDPRN = 0;                //网格单元水量平衡输出控制：0、不输出；1、逐应力期内模拟时段输出；2、逐应力期输出
	m_ICELLHHPRN = 0;                //网格单元水位输出控制：0、不输出；1、逐应力期内模拟时段输出；2、逐应力期输出
	m_ICELLDDPRN = 0;                //网格单元水位降深输出控制：0、不输出；1、逐应力期内模拟时段输出；2、逐应力期输出
	m_ICELLFLPRN = 0;                //网格单元流量输出控制：0、不输出；1、逐应力期内模拟时段输出；2、逐应力期输出
	m_LAKBDPRN = 0;                  //仅模拟湖泊时有效.为湖泊水量平衡输出控制选项. 0: 不输出; 1: 逐应力期内模拟时段输出; 2: 逐应力期输出
	m_SEGMBDPRN = 0;                 //仅模拟季节性河流时有效. 为季节性河流河道水量平衡输出控制选项. 0: 不输出; 1: 逐应力期内模拟时段输出; 2: 逐应力期输出
	m_RECHBDPRN = 0;                 //仅模拟季节性河流时有效. 为季节性河流河段水量平衡输出控制选项. 0: 不输出; 1: 逐应力期内模拟时段输出; 2: 逐应力期输出
	m_IBSPRN = 0;                    //仅模拟夹层体贮水时有效. 为夹层体模拟结果输出控制选项. 0: 不输出; 1: 逐应力期内模拟时段输出; 2: 逐应力期输出
	m_SUBPRN = 0;                    //仅模拟地面沉降时有效, 为网格单元地面沉降模拟结果输出控制选项. 0: 不输出; 1: 逐应力期内模拟时段输出; 2: 逐应力期输出
	m_REGBDPRN = 0;                  //仅启用分区水量平衡统计功能时有效. 为统计分区水量平衡输出控制选项. 0: 不输出; 1: 逐应力期内模拟时段输出; 2: 逐应力期输出
	m_iPer = 0;                      //运行期间的应力期,从1开始计数
	m_iStep = 0;                     //应力期内模拟时段编号,从1开始计数
	m_iIter = 0;                     //时段内迭代次数编号,从1开始计数
	m_DeltT = 0.0;                   //运行期间的计算步长
	m_CumTLen = 0.0;                 //从模拟开始到当前的累计计算时长
	m_PerCumDeltT = 0.0;             //从当前应力期开始到当前的累积计算时长(T)
	m_NumNodes = 0;                  //总的单元个数
	m_SimIBS = 0;                    //是否模拟夹层体贮水
	m_pSolver = NULL;                //矩阵方程强隐式解法求解对象
	m_pRegBudget = NULL;             //分区水量平衡统计对象指针
	m_AryIBOUND = NULL;              //网格单元有效性指示数组
	m_AryWetFlag = NULL;             //在迭代过程中标识网格单元是否被重新湿润的标识符数组
	m_MinHSat = 0.0;                 //仅SIMMTHD=2时有效, 为不可疏干单元允许的最小饱和厚度值(L)
	m_AryHNEW = NULL;                //存放新求解水头值的数组
	m_AryHOLD = NULL;                //存放旧水头值的数组
	m_AryHPRE = NULL;                //存放矩阵方程上次迭代求解水头的数组
	m_AryDelR = NULL;                //沿行方向的离散间距数组
	m_AryDelC = NULL;                //沿列方向的离散间距数组
	m_AryTR = NULL;                  //沿行方向上的导水系数T数组
	m_AryCR = NULL;                  //沿行方向上的水力传导系数数组
	m_AryCC = NULL;                  //沿列方向上的水力传导系数数组
	m_AryCV = NULL;                  //沿垂向上的水力传导系数数组
	m_AryHCOF = NULL;                //矩阵对角线元素数组
	m_AryRHS = NULL;                 //矩阵右端项向量数组
	m_AryCVWD = NULL;                //仅BCF. m_AryCV的备份数组,供单元干湿转化时用
	m_ArySC1 = NULL;                 //存放第一类贮水量的数组
	m_ArySC2 = NULL;                 //存放第二类贮水量的数组
	m_StrgIn = 0.0;                  //当前计算时段内地下水系统单元蓄量减少(水头下降)释放的水量(L3)
	m_StrgOut = 0.0;                 //当前计算时段内地下水系统单元蓄量增加(水头上升)蓄积的水量(L3)
	m_StepABEr = 0.0;                //当前计算时段内的水量平衡误差(L3)
	m_SumFlowIn = 0.0;               //应力期内累计总进入含水层系统的水量(L3)
	m_SumFlowOut = 0.0;              //应力期内累计总离开含水层系统的水量(L3)
	m_AryGDWSinkIn = NULL;           //当前计算时段内进入地下水系统的源汇项数组(L3)
	m_AryGDWSinkOut = NULL;          //当前计算时段内离开地下水系统的源汇项数组(L3)
	m_CnhBndID = 0;                  //定常水头边界对应的边界/源汇项编号
	m_ResultAry = NULL;              //模拟输出结果存储数组
	omp_init_lock(&m_GDWOmpLock);    //初始化并行互斥锁
}

CGDWater::~CGDWater(void)
{
	//释放含水层对象内存空间

	size_t i;
	if(m_pGDLayers != NULL) 
	{
		for (i = 0; i < m_NUMLYR; i++)
		{
		    delete m_pGDLayers[i];       //先删除含水层对象
		}
		delete[] m_pGDLayers;            //再删除指针数组
		m_pGDLayers = NULL;
	}

	//删除CSolver对象
	if (m_pSolver != NULL) 
	{ 
		delete m_pSolver; 
		m_pSolver = NULL;
	}

    //删除开辟的一维数组
	if (m_AryIBOUND != NULL) 
	{
		delete[] m_AryIBOUND; 
		m_AryIBOUND = NULL;
	}
	if (m_AryWetFlag != NULL) 
	{
		delete[] m_AryWetFlag;
		m_AryWetFlag = NULL;
	}
	if (m_AryDelC != NULL) 
	{ 
		delete[] m_AryDelC;
		m_AryDelC = NULL;
	}
	if (m_AryDelR != NULL)
	{ 
		delete[] m_AryDelR; 
		m_AryDelR = NULL;
	}
	if (m_AryTR != NULL) 
	{ 
		delete[] m_AryTR;
		m_AryTR = NULL;
	}
	if (m_AryCR != NULL)
	{
		delete[] m_AryCR;
		m_AryCR = NULL;
	}
	if (m_AryCC != NULL) 
	{ 
		delete[] m_AryCC; 
		m_AryCC = NULL;
	}
	if (m_AryCV != NULL) 
	{
		delete[] m_AryCV;
		m_AryCV = NULL;
	}
	if (m_AryHCOF != NULL) 
	{ 
		delete[] m_AryHCOF; 
		m_AryHCOF = NULL;
	}
	if (m_AryRHS != NULL)
	{
		delete[] m_AryRHS;
		m_AryRHS = NULL;
	}
	if (m_AryHNEW != NULL) 
	{
		delete[] m_AryHNEW; 
		m_AryHNEW = NULL;
	}
	if (m_AryHOLD != NULL) 
	{ 
		delete[] m_AryHOLD;
		m_AryHOLD = NULL;
	}
	if (m_AryHPRE != NULL)
	{
		delete[] m_AryHPRE;
		m_AryHPRE = NULL;
	}
	if (m_AryCVWD != NULL)
	{
		delete[] m_AryCVWD; 
		m_AryCVWD = NULL;
	}
	if (m_ArySC1 != NULL) 
	{
		delete[] m_ArySC1; 
		m_ArySC1 = NULL;
	}
	if (m_ArySC2 != NULL) 
	{ 
		delete[] m_ArySC2;
		m_ArySC2 = NULL;
	}
	if (m_AryGDWSinkIn != NULL)
	{
		delete[] m_AryGDWSinkIn;
		m_AryGDWSinkIn = NULL;
	}
	if (m_AryGDWSinkOut != NULL)
	{
		delete[] m_AryGDWSinkOut;
		m_AryGDWSinkOut = NULL;
	}
	if (m_ResultAry != NULL)
	{
		delete[] m_ResultAry;
		m_ResultAry = NULL;
	}

	//删除应力期控制数据对象
	for (i = 0; i < m_pPPeriodAry.size(); i++)
	{
		delete m_pPPeriodAry[i];
	}
	m_pPPeriodAry.clear();

	//删除定常水头单元数据对象
	for (i = 0; i < m_pCHBCellAry.size(); i++)
	{
		delete m_pCHBCellAry[i];
	}
	m_pCHBCellAry.clear();

	//删除边界对象数据
	for (i = 0; i < m_pSimBndObjAry.size(); i++)
	{
		delete m_pSimBndObjAry[i];
	}
	m_pSimBndObjAry.clear();

	//删除统计分区指针
	if (m_pRegBudget != NULL)
	{
		delete m_pRegBudget;
		m_pRegBudget = NULL;
	}

	//管理数据库连接
	if (m_pConnection != NULL)
	{
		if (m_pConnection->State)
		{
			m_pConnection->Close();
		}
		m_pConnection = NULL;
	}

	//关闭模拟结果输出文件
	if (m_GDWOutPutFile.m_hFile != CFile::hFileNull)
	{
		m_GDWOutPutFile.Close();
	}
	if (m_LyrOutPutFile.m_hFile != CFile::hFileNull)
	{
		m_LyrOutPutFile.Close();
	}
	if (m_HeadOutFile.m_hFile != CFile::hFileNull)
	{
		m_HeadOutFile.Close();
	}
	if (m_HDownOutFile.m_hFile != CFile::hFileNull)
	{
		m_HDownOutFile.Close();
	}
	if (m_FLowOutFile.m_hFile != CFile::hFileNull)
	{
		m_FLowOutFile.Close();
	}
	if (m_CellBDOutFile.m_hFile != CFile::hFileNull)
	{
		m_CellBDOutFile.Close();
	}
	if (m_OBSCellOutFile.m_hFile != CFile::hFileNull)
	{
		m_OBSCellOutFile.Close();
	}

	//释放并行互斥锁
	omp_destroy_lock(&m_GDWOmpLock);
}

void CGDWater::ReadModDataTXT()
{
	//从TXT文件读入模型数据(所有的空间参数和计算参数)

	CString strError;
	CString FileNam;
	CString FileCode;
	CStdioFile FileTxtIn;

	//注：以下读入顺序不能变
	printf(_T("Necessary model data files: \n"));
	printf(_T("\n"));
	//读入地下水模拟控制参数
	printf(_T("CtrlPar.in\n"));
	FileNam = m_strDataInDir + _T("CtrlPar.in");
	FileCode = JudgeTxtFileCode(FileNam);
	if (!FileTxtIn.Open(FileNam, CFile::modeRead | CFile::typeText))
	{
		printf(_T("\n"));
		strError = _T("Fail to open");
		strError = strError + _T(" CtrlPar.in file!\n");
		printf(strError);
		PauAndQuit();
	}
	ReadCtrPar(FileTxtIn, FileCode);
	FileTxtIn.Close();

	//读入源汇项模拟选项表	
	printf(_T("BndOpt.in\n"));
	FileNam = m_strDataInDir + _T("BndOpt.in");
	if (!FileTxtIn.Open(FileNam, CFile::modeRead | CFile::typeText))
	{
		printf(_T("\n"));
		strError = _T("Fail to open");
		strError = strError + _T(" BndOpt.in file!\n");
		printf(strError);
		PauAndQuit();
	}
	ReadBndOpt(FileTxtIn);
	FileTxtIn.Close();

	//读入模拟输出选项表
	printf(_T("OutOpt.in\n"));
	FileNam = m_strDataInDir + _T("OutOpt.in");
	if (!FileTxtIn.Open(FileNam, CFile::modeRead | CFile::typeText))
	{
		printf(_T("\n"));
		strError = _T("Fail to open");
		strError = strError + _T(" OutOpt.in file!\n");
		printf(strError);
		PauAndQuit();
	}
	ReadPrnOpt(FileTxtIn);
	FileTxtIn.Close();

	//应力期属性表
	printf(_T("Period.in\n"));
	FileNam = m_strDataInDir + _T("Period.in");
	if (!FileTxtIn.Open(FileNam, CFile::modeRead | CFile::typeText))
	{
		printf(_T("\n"));
		strError = _T("Fail to open");
		strError = strError + _T(" Period.in file!\n");
		printf(strError);
		PauAndQuit();
	}
	ReadPerCtrl(FileTxtIn);
	FileTxtIn.Close();

	//应力期属性表
	printf(_T("Discrete.in\n"));
	FileNam = m_strDataInDir + _T("Discrete.in");
	if (!FileTxtIn.Open(FileNam, CFile::modeRead | CFile::typeText))
	{
		printf(_T("\n"));
		strError = _T("Fail to open");
		strError = strError + _T(" Discrete.in file!\n");
		printf(strError);
		PauAndQuit();
	}
	ReadDimRC(FileTxtIn);
	FileTxtIn.Close();

	//读入含水层属性表
	CString TbNamOpt;
	printf(_T("Layer.in\n"));
	TbNamOpt = _T("Layer.in");
	FileNam = m_strDataInDir + TbNamOpt;
	if (!FileTxtIn.Open(FileNam, CFile::modeRead | CFile::typeText))
	{
		printf(_T("\n"));
		strError = _T("Fail to open");
		strError = strError + TbNamOpt + _T("  file!\n");
		printf(strError);
		PauAndQuit();
	}

	ReadLyrData(FileTxtIn);
	FileTxtIn.Close();

	//读入网格单元属性表
	printf(_T("Cell.in\n"));
	TbNamOpt = _T("Cell.in");
	FileNam = m_strDataInDir + TbNamOpt;
	if (!FileTxtIn.Open(FileNam, CFile::modeRead | CFile::typeText))
	{
		printf(_T("\n"));
		strError = _T("Fail to open");
		strError = strError + TbNamOpt + _T("  file!\n");
		printf(strError);
		PauAndQuit();
	}
	ReadCellData(FileTxtIn);
	FileTxtIn.Close();

	printf(_T("\n"));
	printf(_T("Optional model data files: \n"));
	printf(_T("\n"));

	//可选数据表读入
	long IBndID = 0;
	CGridCell::m_ActSimBndAry.clear();
	for (long i = 0; i < (long)m_pSimBndObjAry.size(); i++)
	{
		m_pSimBndObjAry[i]->LinkDataInOut(m_strDataInDir, IBndID);
	}

	//读入地下水数值模拟统计分区表
	if (m_pRegBudget != NULL)
	{
		m_pRegBudget->ReadRBData(m_strDataInDir);
	}

	//读入网格单元观测点表
	if (m_IOBSOUT == 1)
	{
		printf(_T("ObsCell.in\n"));
		FileNam = m_strDataInDir + _T("ObsCell.in");
		if (!FileTxtIn.Open(FileNam, CFile::modeRead | CFile::typeText))
		{
			printf(_T("\n"));
			strError = _T("Fail to open");
			strError = strError + _T(" ObsCell.in file!\n");
			printf(strError);
			PauAndQuit();
		}
		ReadObsCellData(FileTxtIn);
	}
}

void CGDWater::ReadModDataDB()
{
	//从数据库读入模型数据(所有的空间参数和计算参数)

	_RecordsetPtr pTable;
    pTable.CreateInstance(__uuidof(Recordset));
	//将数据集游标设置在客户端(可提高性能)
	pTable->CursorLocation = adUseClient;
	//设置数据集缓存(1000条)
	pTable->CacheSize = 1000;
	CString strSQL;
	long i, nCount;
	_ConnectionPtr pConnection = m_pConnection;
	try
	{
		printf(_T("必须的模型数据表: \n"));
		printf(_T("\n"));
		//注：以下读入顺序不能变
		//读入模拟控制参数表
		printf(_T("<模拟控制参数表>\n"));
        strSQL = _T("SELECT * FROM 模拟控制参数表");
		pTable->Open(LPCSTR(strSQL),               
			pConnection.GetInterfacePtr(),	 
			adOpenStatic,
			adLockOptimistic,
			adCmdText);
		if (pTable->BOF)
		{
			printf(_T("\n"));
			printf(_T("<模拟控制参数表>不能为空表,请检查！\n"));
			PauAndQuit();
		}
        pTable->MoveLast();
	    nCount = pTable->GetRecordCount();  //得到数据表的记录个数
	    pTable->MoveFirst();  
		if (nCount != 1)
		{
			printf(_T("\n"));
			printf(_T("<模拟控制参数表>只能有1行数据记录,请检查！\n"));
			PauAndQuit();
		}
		ReadCtrPar(pTable);
	    pTable->Close();

		//读入源汇项模拟选项表
		printf(_T("<源汇项模拟选项表>\n"));
		strSQL = _T("SELECT * FROM 源汇项模拟选项表");
		pTable->Open(LPCSTR(strSQL),
			pConnection.GetInterfacePtr(),
			adOpenStatic,
			adLockOptimistic,
			adCmdText);
		if (pTable->BOF)
		{
			printf(_T("\n"));
			printf(_T("<源汇项模拟选项表>不能为空表,请检查！\n"));
			PauAndQuit();
		}
		pTable->MoveLast();
		nCount = pTable->GetRecordCount();  //得到应力期数量
		pTable->MoveFirst();
		if (nCount != 1)
		{
			printf(_T("\n"));
			printf(_T("<源汇项模拟选项表>只能有1行数据记录,请检查！\n"));
			PauAndQuit();
		}
		ReadBndOpt(pTable);
		pTable->Close();

		//读入模拟输出选项表
		printf(_T("<模拟输出选项表>\n"));
		strSQL = _T("SELECT * FROM 模拟输出选项表");
		pTable->Open(LPCSTR(strSQL),
			pConnection.GetInterfacePtr(),
			adOpenStatic,
			adLockOptimistic,
			adCmdText);
		if (pTable->BOF)
		{
			printf(_T("\n"));
			printf(_T("<模拟输出选项表>不能为空表,请检查！\n"));
			PauAndQuit();
		}
		pTable->MoveLast();
		nCount = pTable->GetRecordCount();  //得到数据表的记录个数
		pTable->MoveFirst();
		if (nCount != 1)
		{
			printf(_T("\n"));
			printf(_T("<模拟输出选项表>只能有1行数据记录,请检查！\n"));
			PauAndQuit();
		}
		ReadPrnOpt(pTable);
		pTable->Close();

		//读入应力期属性表
		printf(_T("<应力期属性表>\n"));
		strSQL = _T("SELECT * FROM 应力期属性表 ORDER BY IPER");
		pTable->Open(LPCSTR(strSQL),
			pConnection.GetInterfacePtr(),
			adOpenStatic,
			adLockOptimistic,
			adCmdText);
		if (pTable->BOF)
		{
			printf(_T("\n"));
			printf(_T("<应力期属性表>不能为空表,请检查！\n"));
			PauAndQuit();
		}
		ReadPerCtrl(pTable);
		pTable->Close();

		//地下水网格间距信息
		//读入沿行方向的离散间距信息
		printf(_T("<网格单元水平向间距表>\n"));
		strSQL = _T("SELECT * FROM 网格单元水平向间距表 ORDER BY ATTI DESC, NUMID");
		pTable->Open(LPCSTR(strSQL),
			pConnection.GetInterfacePtr(),	 
			adOpenStatic,
			adLockOptimistic,
			adCmdText);
		if (pTable->BOF)
		{
			printf(_T("\n"));
			printf(_T("<网格单元水平向间距表>不能为空表,请检查！\n"));
			PauAndQuit();
		}
		ReadDimRC(pTable);
	    pTable->Close();

		//读入含水层属性表
		if (m_INTBLKM == 1)
		{
			printf(_T("<BCF含水层属性表>\n"));
			strSQL = _T("SELECT * FROM BCF含水层属性表 ORDER BY LYRID");
			pTable->Open(LPCSTR(strSQL),
				pConnection.GetInterfacePtr(),
				adOpenStatic,
				adLockOptimistic,
				adCmdText);
			if (pTable->BOF)
			{
				printf(_T("\n"));
				printf(_T("<BCF含水层属性表>不能为空表,请检查！\n"));
				PauAndQuit();
			}
		}
		else
		{
			printf(_T("<LPF含水层属性表>\n"));
			strSQL = _T("SELECT * FROM LPF含水层属性表 ORDER BY LYRID");
			pTable->Open(LPCSTR(strSQL),
				pConnection.GetInterfacePtr(),
				adOpenStatic,
				adLockOptimistic,
				adCmdText);
			if (pTable->BOF)
			{
				printf(_T("\n"));
				printf(_T("<LPF含水层属性表>不能为空表,请检查！\n"));
				PauAndQuit();
			}
		}
		ReadLyrData(pTable);
		pTable->Close();

		//读入网格单元属性表
		if (m_INTBLKM == 1)
		{
			printf(_T("<BCF网格单元属性表>\n"));
			strSQL = _T("SELECT * FROM BCF网格单元属性表 ORDER BY ILYR, IROW, ICOL");
			pTable->Open(LPCSTR(strSQL),
				pConnection.GetInterfacePtr(),
				adOpenStatic,
				adLockOptimistic,
				adCmdText);
			if (pTable->BOF)
			{
				printf(_T("\n"));
				printf(_T("<BCF网格单元属性表>不能为空表,请检查！\n"));
				PauAndQuit();
			}
		}
		else
		{
			printf(_T("<LPF网格单元属性表>\n"));
			strSQL = _T("SELECT * FROM LPF网格单元属性表 ORDER BY ILYR, IROW, ICOL");
			pTable->Open(LPCSTR(strSQL),
				pConnection.GetInterfacePtr(),
				adOpenStatic,
				adLockOptimistic,
				adCmdText);
			if (pTable->BOF)
			{
				printf(_T("\n"));
				printf(_T("<LPF网格单元属性表>不能为空表,请检查！\n"));
				PauAndQuit();
			}
		}
		ReadCellData(pTable);
		pTable->Close();

		printf(_T("\n"));
		printf(_T("可选的模型数据表: \n"));
		printf(_T("\n"));

		//可选数据表读入
		long IBndID = 0;
		CGridCell::m_ActSimBndAry.clear();
		for (i = 0; i < (long)m_pSimBndObjAry.size(); i++)
		{
			m_pSimBndObjAry[i]->LinkDataInOut(pConnection, IBndID);
		}

		//读入地下水数值模拟统计分区表
		if (m_pRegBudget != NULL)
		{
			m_pRegBudget->ReadRBData(pConnection);
		}

		//读入网格单元观测点表
		if (m_IOBSOUT == 1)
		{
			printf(_T("<网格单元观测点表>\n"));
			strSQL = _T("SELECT * FROM 网格单元观测点表");
			pTable->Open(LPCSTR(strSQL),
				pConnection.GetInterfacePtr(),
				adOpenStatic,
				adLockOptimistic,
				adCmdText);
			if (!pTable->BOF)
			{
				ReadObsCellData(pTable);
			}
			pTable->Close();
		}
	}
	catch(_com_error *e)
	{
		printf((LPCTSTR)e->ErrorMessage());
	} 
}

void CGDWater::ReadCtrPar(CStdioFile& FileTxtIn, CString FileCode)
{
	//读入地下水数值计算控制参数

	CString strTemp, strTest, strFormat;
	long NUMLYR, NUMROW, NUMCOL, SIMMTHD, SIMTYPE, INTBLKM, ISOLVE, MAXIT, IRELAX, NITER, 
		ICHFLG, IWDFLG, NWETIT, IHDWET, IREGSTA, IOBSOUT, IMULTD, NUMTD;
	char DIMUNIT[50] = {0};
	char TIMEUNIT[50] = {0};
	double XSTCORD, YSTCORD, LAMBDA, DAMP, HCLOSE, RCLOSE, THETA, GAMMA, AKAPPA, HNOFLO, WETFCT;

	//略过表头
	FileTxtIn.ReadString(strTemp);
	//读取数据
	for (;;)
	{
		FileTxtIn.ReadString(strTemp);
		//测试读入的数据
		strTest = strTemp;
		strTest.Trim();
		//如果没有读到有效数据
		if (strTest.IsEmpty())
		{
			if (FileTxtIn.GetPosition() == FileTxtIn.GetLength())
			{
				//到了文件末尾依然没有读到有效数据, 报错!
				printf(_T("\n"));
				printf(_T("文件 模拟控制参数表.in 中没有数据, 请检查!\n"));
				PauAndQuit();
			}
			else
			{
				//没到文件末尾, 说明是个空行, 略过继续
				continue;
			}
		}
		//读到了有效数据, 跳出循环
		break;
	}
	//解析数据
	if (FileCode != _T("ANSI or UTF-8 without BOM Compatible to ANSI"))
	{
		//该文件不与ANSI兼容, 需进行字符串转换
		CharToUTF8(strTemp);
	}
	strFormat = _T("%ld %ld %ld %s %s %lg %lg %ld %ld %lg %ld %ld %ld %lg %lg %lg %ld %lg %lg %lg %ld %lg %ld %ld");
	strFormat = strFormat + _T(" %lg %ld %ld %ld %ld %ld %ld");
	if (sscanf_s(strTemp, strFormat, &NUMLYR, &NUMROW, &NUMCOL, DIMUNIT, 50, TIMEUNIT, 50, &XSTCORD,
		&YSTCORD, &SIMMTHD, &SIMTYPE, &LAMBDA, &INTBLKM, &ISOLVE, &MAXIT, &DAMP, &HCLOSE, &RCLOSE, &IRELAX,
		&THETA, &GAMMA, &AKAPPA, &NITER, &HNOFLO, &ICHFLG, &IWDFLG, &WETFCT, &NWETIT, &IHDWET, 
		&IREGSTA, &IOBSOUT, &IMULTD, &NUMTD) != 31)
	{
		printf(_T("\n"));
		printf(_T("数据有缺失, 请检查 模拟控制参数表.in !\n"));
		PauAndQuit();
	}
	//记录数据
	m_NUMLYR = NUMLYR;
	m_NUMROW = NUMROW;
	m_NUMCOL = NUMCOL;
	m_DIMUNIT = DIMUNIT;
	m_TIMEUNIT = TIMEUNIT;
	m_XSTCORD = XSTCORD;
	m_YSTCORD = YSTCORD;
	m_SIMMTHD = SIMMTHD;
	m_SIMTYPE = SIMTYPE;
	m_LAMBDA = LAMBDA;
	m_INTBLKM = INTBLKM;
	m_ISOLVE = ISOLVE;
	m_MAXIT = MAXIT;
	m_DAMP = DAMP;
	m_HCLOSE = HCLOSE;
	m_RCLOSE = RCLOSE;
	m_IRELAX = IRELAX;
	m_THETA = THETA;
	m_GAMMA = GAMMA;
	m_AKAPPA = AKAPPA;
	m_NITER = NITER;
	m_HNOFLO = HNOFLO;
	m_ICHFLG = ICHFLG;
	m_IWDFLG = IWDFLG;
	m_WETFCT = WETFCT;
	m_NWETIT = NWETIT;
	m_IHDWET = IHDWET;
	m_IREGSTA = IREGSTA;
	m_IOBSOUT = IOBSOUT;
	m_IMULTD = IMULTD;
	m_NUMTD = NUMTD;
	//检查数据
	CheckCtrlParData();
}

void CGDWater::ReadCtrPar(_RecordsetPtr& pTable)
{
	//读入地下水模拟控制参数数据
	try
	{
        _variant_t Value;
	    Value = pTable->GetCollect(_T("NUMLYR"));
		m_NUMLYR = Value.intVal;
		Value = pTable->GetCollect(_T("NUMROW"));
		m_NUMROW = Value.intVal;
	    Value = pTable->GetCollect(_T("NUMCOL"));
		m_NUMCOL = Value.intVal;
	    Value = pTable->GetCollect(_T("DIMUNIT"));
		m_DIMUNIT = (LPCSTR)_bstr_t(Value);
		TrimBlank(m_DIMUNIT);
		Value = pTable->GetCollect(_T("TIMEUNIT"));
		m_TIMEUNIT = (LPCSTR)_bstr_t(Value);
		TrimBlank(m_TIMEUNIT);
		Value = pTable->GetCollect(_T("XSTCORD"));
		m_XSTCORD = Value.dblVal;
		Value = pTable->GetCollect(_T("YSTCORD"));
		m_YSTCORD = Value.dblVal;
		Value = pTable->GetCollect(_T("SIMMTHD"));
		m_SIMMTHD = Value.intVal;
		Value = pTable->GetCollect(_T("SIMTYPE"));
		m_SIMTYPE = Value.intVal;
		if (m_SIMMTHD == 1 && m_SIMTYPE == 1)
		{
			Value = pTable->GetCollect(_T("LAMBDA"));
			m_LAMBDA = Value.dblVal;
		}
		Value = pTable->GetCollect(_T("INTBLKM"));
		m_INTBLKM = Value.intVal;
		Value = pTable->GetCollect(_T("ISOLVE"));
		m_ISOLVE = Value.intVal;
	    Value = pTable->GetCollect(_T("MAXIT"));
		m_MAXIT = Value.intVal;
	    Value = pTable->GetCollect(_T("DAMP"));
		m_DAMP = Value.dblVal;
	    Value = pTable->GetCollect(_T("HCLOSE"));
		m_HCLOSE = Value.dblVal;
		if (m_ISOLVE == 2)
		{
			Value = pTable->GetCollect(_T("RCLOSE"));
			m_RCLOSE = Value.dblVal;
		}
		//读入是否启用逐单元松弛迭代算法选项
		Value = pTable->GetCollect(_T("IRELAX"));
		m_IRELAX = Value.intVal;
		if (m_IRELAX == 1)
		{
			//如果启用逐单元松弛迭代算法, 继续读入控制性参数
			Value = pTable->GetCollect(_T("THETA"));
			m_THETA = Value.dblVal;
			Value = pTable->GetCollect(_T("GAMMA"));
			m_GAMMA = Value.dblVal;
			Value = pTable->GetCollect(_T("AKAPPA"));
			m_AKAPPA = Value.dblVal;
			Value = pTable->GetCollect(_T("NITER"));
			m_NITER = Value.intVal;
		}
	    Value = pTable->GetCollect(_T("HNOFLO"));
		m_HNOFLO = Value.dblVal;
	    Value = pTable->GetCollect(_T("ICHFLG"));
		m_ICHFLG = Value.intVal;
		if (m_SIMMTHD == 2)
		{
			//如果采用原MODFLOW方法模拟, 读入是否模拟网格单元干湿转化选项
			Value = pTable->GetCollect(_T("IWDFLG"));
			m_IWDFLG = Value.intVal;
			if (m_IWDFLG == 1)
			{
				//采用试算法模拟网格单元的干-湿转换, 继续读入控制性参数
				Value = pTable->GetCollect(_T("WETFCT"));
				m_WETFCT = Value.dblVal;
				Value = pTable->GetCollect(_T("NWETIT"));
				m_NWETIT = Value.intVal;
				Value = pTable->GetCollect(_T("IHDWET"));
				m_IHDWET = Value.intVal;
			}
		}
		Value = pTable->GetCollect(_T("IREGSTA"));
		m_IREGSTA = Value.intVal;
		Value = pTable->GetCollect(_T("IOBSOUT"));
		m_IOBSOUT = Value.intVal;
		Value = pTable->GetCollect(_T("IMULTD"));
		m_IMULTD = Value.intVal;
		if (m_IMULTD == 1)
		{
			Value = pTable->GetCollect(_T("NUMTD"));
			m_NUMTD = Value.intVal;
		}
	}
	catch(_com_error *e)
	{
		printf((LPCTSTR)e->ErrorMessage());
	}

	//检查数据
	CheckCtrlParData();
}

void CGDWater::CheckCtrlParData()
{
	//检查地下水数值计算控制参数

	if (m_NUMLYR < 1)
	{
		//地下水含水层数量必须大于等于1层
		printf(_T("\n"));
		printf(_T("NUMLYR参数必须大于等于1, 请检查!\n"));
		PauAndQuit();
	}
	if (m_NUMROW < 1)
	{
		//网格单元行数量必须大于等于1行
		printf(_T("\n"));
		printf(_T("NUMROW参数必须大于等于1, 请检查!\n"));
		PauAndQuit();
	}
	if (m_NUMCOL < 1)
	{
		//网格单元列数量必须大于等于1列
		printf(_T("\n"));
		printf(_T("NUMCOL参数必须大于等于1, 请检查!\n"));
		PauAndQuit();
	}
	if (m_SIMMTHD < 1 || m_SIMMTHD > 2)
	{
		printf(_T("\n"));
		printf(_T("SIMMTHD参数必须为1或2, 请检查!\n"));
		PauAndQuit();
	}
	if (m_SIMTYPE < 1 || m_SIMTYPE > 2)
	{
		printf(_T("\n"));
		printf(_T("SIMTYPE参数必须为1或2, 请检查!\n"));
		PauAndQuit();
	}
	if (m_SIMMTHD == 1 && m_SIMTYPE == 1)
	{
		if (m_LAMBDA > 0.0)
		{
			if (m_LAMBDA < 1E-4 || m_LAMBDA > 1E-3)
			{
				printf(_T("\n"));
				printf(_T("LAMBDA参数的有效取值区间为0.0001~0.001, 请检查!\n"));
				PauAndQuit();
			}
		}
		else
		{
			if (fabs(m_LAMBDA + 1.0) > 1e-10)
			{
				printf(_T("\n"));
				printf(_T("LAMBDA参数的有效取值区间为0.0001~0.001, 或为-1.0, 请检查!\n"));
				PauAndQuit();
			}
		}
	}
	else
	{
		m_LAMBDA = -1.0;
	}
	if (m_INTBLKM < 1 || m_INTBLKM > 2)
	{
		printf(_T("\n"));
		printf(_T("INTBLKM参数必须为1或2, 请检查!\n"));
		PauAndQuit();
	}
	if (m_ISOLVE != 1 && m_ISOLVE != 2)
	{
		printf(_T("\n"));
		printf(_T("ISOLVE参数必须为1或2, 请检查!\n"));
		PauAndQuit();
	}
	if (m_MAXIT < 200 || m_MAXIT > 1000000)
	{
		printf(_T("\n"));
		printf(_T("MAXIT参数有效取值区间为200~1000000, 请检查!\n"));
		PauAndQuit();
	}
	if (m_DAMP < 0.0001 || m_DAMP > 1.0)
	{
		printf(_T("\n"));
		printf(_T("DAMP参数有效取值区间为0.0001~1.0, 请检查!\n"));
		PauAndQuit();
	}
	if (m_HCLOSE < 1e-8 || m_HCLOSE > 1e-1)
	{
		printf(_T("\n"));
		printf(_T("HCLOSE参数有效取值区间为0.1~1e-8, 请检查!\n"));
		PauAndQuit();
	}
	if (m_ISOLVE == 2)
	{
		if (m_RCLOSE < 1e-8 || m_RCLOSE > 1e-1)
		{
			printf(_T("\n"));
			printf(_T("RCLOSE参数有效取值区间为0.1~1e-8, 请检查!\n"));
			PauAndQuit();
		}
	}
	else
	{
		m_RCLOSE = 0.0;
	}
	if (m_IRELAX < 0 || m_IRELAX > 1)
	{
		printf(_T("\n"));
		printf(_T("IRELAX参数值必须为0或1, 请检查!\n"));
		PauAndQuit();
	}
	if (m_IRELAX == 1)
	{
		//如果启用逐单元松弛迭代算法
		if (m_THETA < 0.35 || m_THETA > 0.95)
		{
			printf(_T("\n"));
			printf(_T("THETA参数有效取值区间为0.35~0.95, 请检查!\n"));
			PauAndQuit();
		}
		if (m_GAMMA < 1.0 || m_GAMMA > 5.0)
		{
			printf(_T("\n"));
			printf(_T("GAMMA参数有效取值区间为1.0~5.0, 请检查!\n"));
			PauAndQuit();
		}
		if (m_AKAPPA <= 0.0 || m_AKAPPA > 0.2)
		{
			printf(_T("\n"));
			printf(_T("AKAPPA参数有效取值区间为0.0(不含)~0.2, 请检查!\n"));
			PauAndQuit();
		}
		if (m_NITER < 0 || m_NITER > 100)
		{
			printf(_T("\n"));
			printf(_T("NITER参数有效取值区间为0~100, 请检查!\n"));
			PauAndQuit();
		}
	}
	else
	{
		m_THETA = 0.0;
		m_GAMMA = 0.0;
		m_AKAPPA = 0.0;
		m_NITER = 0;
	}
	if (m_ICHFLG < 0 || m_ICHFLG>1)
	{
		printf(_T("\n"));
		printf(_T("ICHFLG参数必须为0或1, 请检查!\n"));
		PauAndQuit();
	}
	if (m_SIMMTHD == 2)
	{
		//如果采用原MODFLOW方法模拟
		if (m_IWDFLG < 0 || m_IWDFLG > 1)
		{
			printf(_T("\n"));
			printf(_T("IWDFLG参数必须为0或1, 请检查!\n"));
			PauAndQuit();
		}
		if (m_IWDFLG == 1)
		{
			//采用试算法模拟网格单元的干-湿转换
			if (m_WETFCT < -1.0 || (m_WETFCT > -1.0 && m_WETFCT <= 0.0) || m_WETFCT > 1.0)
			{
				printf(_T("\n"));
				printf(_T("WETFCT参数值需输入-1, 或输入在0.0~1.0之间的有效值(注: 需大于0.0, 可等于1.0), 请检查!\n"));
				PauAndQuit();
			}
			if (m_NWETIT < 1 || m_NWETIT > 4)
			{
				printf(_T("\n"));
				printf(_T("NWETIT参数有效取值区间为1~4, 请检查!\n"));
				PauAndQuit();
			}
			if (m_IHDWET < -1 || m_IHDWET == 0 || m_IHDWET > 2)
			{
				printf(_T("\n"));
				printf(_T("IHDWET参数必须为-1、1或2, 请检查!\n"));
				PauAndQuit();
			}
			if (m_IHDWET == -1)
			{
				//用户让模型自动选择
				m_IHDWET = 2;
			}
		}
		else
		{
			m_WETFCT = 0.0;
			m_NWETIT = 0;
			m_IHDWET = 0;
		}
	}
	else
	{
		m_IWDFLG = 0;
		m_WETFCT = 0.0;
		m_NWETIT = 0;
		m_IHDWET = 0;
	}
	if (m_IREGSTA < 0 || m_IREGSTA > 1)
	{
		printf(_T("\n"));
		printf(_T("IREGSTA参数必须为0或1, 请检查!\n"));
		PauAndQuit();
	}
	if (m_IOBSOUT < 0 || m_IOBSOUT > 1)
	{
		printf(_T("\n"));
		printf(_T("IOBSOUT参数必须为0或1, 请检查!\n"));
		PauAndQuit();
	}
	if (m_IMULTD < 0 || m_IMULTD > 1)
	{
		printf(_T("\n"));
		printf(_T("IMULTD参数必须为0或1, 请检查!\n"));
		PauAndQuit();
	}
	if (m_IMULTD == 1)
	{
		if (m_NUMTD < -1 || m_NUMTD == 0 || m_NUMTD == 1 || m_NUMTD > 256)
		{
			printf(_T("\n"));
			printf(_T("NUMTD参数必须等于-1，或2-256之间, 请检查!\n"));
			PauAndQuit();
		}
	}
	else
	{
		m_NUMTD = 0;
	}

	//计算每层含水层中的总网格数
	m_NUMRC = m_NUMROW * m_NUMCOL;
	//进行基础性数据处理
	BaseInit();
}

void CGDWater::BaseInit()
{
	//进行基础性数据处理

	//开辟基础数组
	try
	{
		int iRow, iCol, iLyr;
		//开辟含水层指针数组
		m_pGDLayers = new CGridLyr*[m_NUMLYR]();
		for (iLyr = 0; iLyr < m_NUMLYR; iLyr++)
		{
			//生成含水层对象
			m_pGDLayers[iLyr] = new CGridLyr;
			m_pGDLayers[iLyr]->m_NumRow = m_NUMROW;
			m_pGDLayers[iLyr]->m_NumCol = m_NUMCOL;
			m_pGDLayers[iLyr]->m_NumRC = m_NUMRC;
			//开辟含水层网格单元数组(二维)
			m_pGDLayers[iLyr]->m_pGridCellAry = new CGridCell**[m_NUMROW]();
			for (iRow = 0; iRow < m_NUMROW; iRow++)
			{
				m_pGDLayers[iLyr]->m_pGridCellAry[iRow] = new CGridCell*[m_NUMCOL]();
				//生成网格单元对象
				for (iCol = 0; iCol < m_NUMCOL; iCol++)
				{
					m_pGDLayers[iLyr]->m_pGridCellAry[iRow][iCol] = new CGridCell;
				}
			}
		}
		//开辟沿行方向的离散间距数组
		m_AryDelR = new double[m_NUMCOL]();
		//开辟沿列方向的离散间距数组
		m_AryDelC = new double[m_NUMROW]();
	}
	catch (bad_alloc &e)
	{
		printf(e.what());
		exit(1);
	}

	//计算总网格单元数量
	m_NumNodes = m_NUMLYR * m_NUMRC;
	//确定并行运算时线程数量
	if (m_IMULTD == 1)
	{
		//如果启用了并行运算
		if (m_NUMTD == -1)
		{
			//取机器支持的物理核心数量作为并行运算时的线程数量.
			//注: 经过测试, 虽然机器的物理核心支持超线程, 但并行线
			//程数量的超过物理核心的数量之后计算性能反而显著下降.
			//原因可能是本模型并行运算时每个线程的负载粒度很小,
			//并行的线程过多会导致频繁的任务分配开销, 得不偿失
			typedef BOOL(WINAPI *LPFN_GLPI)(PSYSTEM_LOGICAL_PROCESSOR_INFORMATION, PDWORD);
			LPFN_GLPI glpi;
			BOOL done = FALSE;
			PSYSTEM_LOGICAL_PROCESSOR_INFORMATION buffer = NULL;
			PSYSTEM_LOGICAL_PROCESSOR_INFORMATION ptr = NULL;
			DWORD returnLength = 0;
			DWORD processorCoreCount = 0;
			DWORD byteOffset = 0;
			//获取机器的物理核心(非逻辑核心)数量
			glpi = (LPFN_GLPI)GetProcAddress(GetModuleHandle(TEXT(_T("kernel32"))),
				_T("GetLogicalProcessorInformation"));
			if (NULL == glpi)
			{
				printf(_T("\n"));
				printf(_T("系统不支持GetLogicalProcessorInformation方法, 无法自动选择合适的并行线程数量!\n"));
				printf(_T("请在<模拟控制参数表>中明确指出NUMTD的数值!\n"));
				PauAndQuit();
			}
			while (!done)
			{
				DWORD rc = glpi(buffer, &returnLength);
				if (rc == FALSE)
				{
					if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
					{
						if (buffer)
						{
							free(buffer);
						}
						buffer = (PSYSTEM_LOGICAL_PROCESSOR_INFORMATION)malloc(returnLength);
						if (buffer == NULL)
						{
							printf(_T("\n"));
							printf(_T("无法分配内存!\n"));
							PauAndQuit();
						}
					}
				}
				else
				{
					done = TRUE;
				}
			}
			ptr = buffer;
			while (byteOffset + sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION) <= returnLength)
			{
				if (ptr->Relationship == RelationProcessorCore)
				{
					processorCoreCount++;
				}
				byteOffset += sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION);
				ptr++;
			}
			if (buffer) free(buffer);
			m_NUMTD = processorCoreCount;
		}
	}
	else
	{
		//如果不启用并行运算
		m_NUMTD = 1;
	}

	//生成统计分区对象
	if (m_IREGSTA == 1)
	{
		m_pRegBudget = new CRegBudget;
	}
}

void CGDWater::ReadBndOpt(CStdioFile& FileTxtIn)
{
	//读入边界条件模拟选项

	CString strTemp, strTest, strFormat;
	CString strErrInfo;
	long SIMSHB, SIMRCH, SIMWELL, SIMSUB;

	//略过表头
	FileTxtIn.ReadString(strTemp);
	//读取数据
	for (;;)
	{
		FileTxtIn.ReadString(strTemp);
		//测试读入的数据
		strTest = strTemp;
		strTest.Trim();
		//如果没有读到有效数据
		if (strTest.IsEmpty())
		{
			if (FileTxtIn.GetPosition() == FileTxtIn.GetLength())
			{
				//到了文件末尾依然没有读到有效数据, 报错!
				printf(_T("\n"));
				printf(_T("文件 源汇项模拟选项表.in 中没有数据, 请检查!\n"));
				PauAndQuit();
			}
			else
			{
				//没到文件末尾, 说明是个空行, 略过继续
				continue;
			}
		}
		//读到了有效数据, 跳出循环
		break;
	}
	//解析数据
	strFormat = _T("%ld %ld %ld %ld");
	if (sscanf_s(strTemp, strFormat, &SIMSHB, &SIMRCH, &SIMWELL, &SIMSUB) != 4)
	{
		printf(_T("\n"));
		printf(_T("数据有缺失, 请检查 源汇项模拟选项表.in ！\n"));
		PauAndQuit();
	}

	//记录数据
	m_SIMSHB = SIMSHB;
	m_SIMRCH = SIMRCH;
	m_SIMWEL = SIMWELL;
	m_SIMSUB = SIMSUB;

	//检查并处理源汇项选项数据
	CheckBndOptData();
}

void CGDWater::ReadBndOpt(_RecordsetPtr& pTable)
{
	//读入边界条件模拟选项

	CString strErrInfo;
	try
	{
		_variant_t Value;
		//随时间变化水头边界
		Value = pTable->GetCollect(_T("SIMSHB"));
		m_SIMSHB = Value.intVal;
		//面上补给
		Value = pTable->GetCollect(_T("SIMRCH"));
		m_SIMRCH = Value.intVal;
		//井流
		Value = pTable->GetCollect(_T("SIMWELL"));
		m_SIMWEL = Value.intVal;
		//地面沉降
		if (m_SIMTYPE == 2)
		{
			//仅在模拟非稳定流时地面沉降模拟才有意义
			Value = pTable->GetCollect(_T("SIMSUB"));
			m_SIMSUB = Value.intVal;
		}
	}
	catch (_com_error *e)
	{
		printf((LPCTSTR)e->ErrorMessage());
	}

	//检查并处理源汇项选项数据
	CheckBndOptData();
}

void CGDWater::CheckBndOptData()
{
	//检查并处理边界模拟选项数据

	CString strErrInfo;

	//随时间变化水头边界
	if (m_SIMSHB < 0 || m_SIMSHB > 1)
	{
		printf(_T("\n"));
		strErrInfo = _T("SIMSHB 参数无效，该值必须为0或1，请检查!\n");
		printf(strErrInfo);
		PauAndQuit();
	}
	else
	{
		if (m_SIMSHB == 1)
		{
			CSHB* pSHB = new CSHB;
			pSHB->SetBndBaseInfo(_T("时变定水头"), _T("SHB"), _T("  SPECIFIED HEAD"), _T("时变水头_应力期数据表"));
			m_pSimBndObjAry.push_back(pSHB);
		}
	}

	//面上补给
	if (m_SIMRCH < 0 || m_SIMRCH > 1)
	{
		printf(_T("\n"));
		strErrInfo = _T("SIMRCH 参数无效，该值必须为0或1，请检查!\n");
		printf(strErrInfo);
		PauAndQuit();
	}
	else
	{
		if (m_SIMRCH == 1)
		{
			CRchg* pRchg = new CRchg;
			pRchg->SetBndBaseInfo(_T("面上补给"), _T("RCH"), _T("        RECHARGE"), _T("面上补给_应力期数据表"));
			m_pSimBndObjAry.push_back(pRchg);
		}
	}

	//开采井/注水井
	if (m_SIMWEL < 0 || m_SIMWEL > 1)
	{
		printf(_T("\n"));
		strErrInfo = _T("SIMWEL 参数无效，该值必须为0或1，请检查!\n");
		printf(strErrInfo);
		PauAndQuit();
	}
	else
	{
		if (m_SIMWEL == 1)
		{
			CWell* pWell = new CWell;
			pWell->SetBndBaseInfo(_T("井流"), _T("WEL"), _T("           WELLS"), _T("井流_应力期数据表"));
			m_pSimBndObjAry.push_back(pWell);
		}
	}

	//地面沉降
	if (m_SIMTYPE == 2)
	{
		//仅在模拟非稳定流时地面沉降模拟才有意义
		if (m_SIMSUB < 0 || m_SIMSUB > 1)
		{
			printf(_T("\n"));
			strErrInfo = _T("SIMSUB 参数无效，该值必须为0或1，请检查!\n");
			printf(strErrInfo);
			PauAndQuit();
		}
		else
		{
			if (m_SIMSUB == 1)
			{
				//地面沉降模拟必须使用LPF渗流计算方式
				if (m_INTBLKM != 2)
				{
					printf(_T("\n"));
					strErrInfo = _T("地面沉降模拟必须使用LPF格式，请在<模拟控制参数表>中正确设置INTBLKM参数!\n");
					printf(strErrInfo);
					PauAndQuit();
				}
				//地面沉降模拟和夹层体模拟不能相互冲突
				CBndHandler* pIBS = GetSimBndObj(_T("夹层体贮水"));
				if (pIBS != NULL)
				{
					printf(_T("\n"));
					strErrInfo = _T("夹层体贮水模拟和地面沉降模拟只能选择其一，请检查<源汇项模拟选项表>!\n");
					printf(strErrInfo);
					PauAndQuit();
				}
				CLandSub* pLandSub = new CLandSub;
				pLandSub->SetBndBaseInfo(_T("地面沉降"), _T("SUB"), _T("      SUBSIDENCE"), _T("Sub_Stress"));
				m_pSimBndObjAry.push_back(pLandSub);
			}
		}
	}
}

void CGDWater::ReadPrnOpt(CStdioFile& FileTxtIn)
{
	//读入模拟结果输出控制参数(TXT文件)

	CString strTemp, strTest, strFormat;
	long GDWBDPRN, LYRBDPRN, CELLBDPRN, CELLHHPRN, CELLDDPRN, CELLFLPRN, SUBPRN, REGBDPRN;

	//略过表头
	FileTxtIn.ReadString(strTemp);
	//读取数据
	for (;;)
	{
		FileTxtIn.ReadString(strTemp);
		//测试读入的数据
		strTest = strTemp;
		strTest.Trim();
		//如果没有读到有效数据
		if (strTest.IsEmpty())
		{
			if (FileTxtIn.GetPosition() == FileTxtIn.GetLength())
			{
				//到了文件末尾依然没有读到有效数据, 报错!
				printf(_T("\n"));
				printf(_T("文件 模拟输出选项表.in 中没有数据, 请检查!\n"));
				PauAndQuit();
			}
			else
			{
				//没到文件末尾, 说明是个空行, 略过继续
				continue;
			}
		}
		//读到了有效数据, 跳出循环
		break;
	}
	//解析数据
	strFormat = _T("%ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld");
	if (sscanf_s(strTemp, strFormat, &GDWBDPRN, &LYRBDPRN, &CELLBDPRN, &CELLHHPRN, &CELLDDPRN,
		&CELLFLPRN, &SUBPRN, &REGBDPRN) != 8)
	{
		printf(_T("\n"));
		printf(_T("数据有缺失, 请检查 模拟输出选项表.in ！\n"));
		PauAndQuit();
	}
	//记录数据
	m_IGDWBDPRN = GDWBDPRN;
	m_ILYRBDPRN = LYRBDPRN;
	m_ICELLBDPRN = CELLBDPRN;
	m_ICELLHHPRN = CELLHHPRN;
	m_ICELLDDPRN = CELLDDPRN;
	m_ICELLFLPRN = CELLFLPRN;
	m_SUBPRN = SUBPRN;
	m_REGBDPRN = REGBDPRN;
	//检查并处理模拟输出选项
	CheckPrnOptData();
}

void CGDWater::ReadPrnOpt(_RecordsetPtr& pTable)
{
	//读入模拟结果输出控制参数
	try
	{
		_variant_t Value;
		//地下水系统水量平衡输出
		Value = pTable->GetCollect(_T("GDWBDPRN"));
		m_IGDWBDPRN = Value.intVal;
		//含水层水量平衡输出
		Value = pTable->GetCollect(_T("LYRBDPRN"));
		m_ILYRBDPRN = Value.intVal;
		//网格单元水量平衡输出
		Value = pTable->GetCollect(_T("CELLBDPRN"));
		m_ICELLBDPRN = Value.intVal;
		//网格单元水位输出
		Value = pTable->GetCollect(_T("CELLHHPRN"));
		m_ICELLHHPRN = Value.intVal;
		//网格单元水位降深输出
		Value = pTable->GetCollect(_T("CELLDDPRN"));
		m_ICELLDDPRN = Value.intVal;
		//网格单元流量输出
		Value = pTable->GetCollect(_T("CELLFLPRN"));
		m_ICELLFLPRN = Value.intVal;
		//地面沉降输出
		CLandSub* pLandSub = (CLandSub*)(GDWMOD.GetSimBndObj(_T("地面沉降")));
		if (pLandSub != NULL)
		{
			//沉降量输出控制
			Value = pTable->GetCollect(_T("SUBPRN"));
			m_SUBPRN = Value.intVal;
		}
		//统计分区水量平衡输出
		if (m_pRegBudget != NULL)
		{
			Value = pTable->GetCollect(_T("REGBDPRN"));
			m_REGBDPRN = Value.intVal;
		}
	}
	catch (_com_error *e)
	{
		printf((LPCTSTR)e->ErrorMessage());
	}

	//检查并处理模拟输出选项
	CheckPrnOptData();
}

void CGDWater::CheckPrnOptData()
{
	//检查模拟结果输出控制参数

	//地下水系统水量平衡输出
	if (m_IGDWBDPRN<0 || m_IGDWBDPRN>2)
	{
		printf(_T("\n"));
		printf(_T("GDWBDPRN参数无效,最小0最大2. 请检查!\n"));
		PauAndQuit();
	}
	if (m_SIMTYPE == 1)
	{
		//如果是稳定流模拟, 如果要输出则默认逐时段输出
		if (m_IGDWBDPRN > 0)
		{
			m_IGDWBDPRN = 1;
		}
	}
	//含水层水量平衡输出
	if (m_ILYRBDPRN<0 || m_ILYRBDPRN>2)
	{
		printf(_T("\n"));
		printf(_T("LYRBDPRN参数无效,最小0最大2. 请检查!\n"));
		PauAndQuit();
	}
	if (m_SIMTYPE == 1)
	{
		//如果是稳定流模拟, 如果要输出则默认逐时段输出
		if (m_ILYRBDPRN > 0)
		{
			m_ILYRBDPRN = 1;
		}
	}
	//网格单元水量平衡输出
	if (m_ICELLBDPRN<0 || m_ICELLBDPRN>2)
	{
		printf(_T("\n"));
		printf(_T("CELLBDPRN参数无效,最小0最大2. 请检查!\n"));
		PauAndQuit();
	}
	if (m_SIMTYPE == 1)
	{
		//如果是稳定流模拟, 如果要输出则默认逐时段输出
		if (m_ICELLBDPRN > 0)
		{
			m_ICELLBDPRN = 1;
		}
	}
	//网格单元水位输出
	if (m_ICELLHHPRN<0 || m_ICELLHHPRN>2)
	{
		printf(_T("\n"));
		printf(_T("CELLHHPRN参数无效,最小0最大2. 请检查!\n"));
		PauAndQuit();
	}
	if (m_SIMTYPE == 1)
	{
		//如果是稳定流模拟, 如果要输出则默认逐时段输出
		if (m_ICELLHHPRN > 0)
		{
			m_ICELLHHPRN = 1;
		}
	}
	//网格单元水位降深输出
	if (m_ICELLDDPRN<0 || m_ICELLDDPRN>2)
	{
		printf(_T("\n"));
		printf(_T("CELLDDPRN参数无效,最小0最大2. 请检查!\n"));
		PauAndQuit();
	}
	if (m_SIMTYPE == 1)
	{
		//如果是稳定流模拟, 如果要输出则默认逐时段输出
		if (m_ICELLDDPRN > 0)
		{
			m_ICELLDDPRN = 1;
		}
	}
	//网格单元流量输出
	if (m_ICELLFLPRN<0 || m_ICELLFLPRN>2)
	{
		printf(_T("\n"));
		printf(_T("CELLFLPRN参数无效,最小0最大2. 请检查!\n"));
		PauAndQuit();
	}
	if (m_SIMTYPE == 1)
	{
		//如果是稳定流模拟, 如果要输出则默认逐时段输出
		if (m_ICELLFLPRN > 0)
		{
			m_ICELLFLPRN = 1;
		}
	}
	//地面沉降输出
	CLandSub* pLandSub = (CLandSub*)(GDWMOD.GetSimBndObj(_T("地面沉降")));
	if (pLandSub != NULL)
	{
		//沉降量输出控制
		if (m_SUBPRN < 0 || m_SUBPRN > 2)
		{
			printf(_T("\n"));
			printf(_T("SUBPRN参数无效,最小0最大2. 请检查!\n"));
			PauAndQuit();
		}
		pLandSub->SetPrnOpt(m_SUBPRN);
	}
	else
	{
		m_SUBPRN = 0;
	}
	//统计分区水量平衡输出
	if (m_pRegBudget != NULL)
	{
		if (m_REGBDPRN < 0 || m_REGBDPRN > 2)
		{
			printf(_T("\n"));
			printf(_T("REGBDPRN参数无效,最小0最大2. 请检查!\n"));
			PauAndQuit();
		}
		if (m_SIMTYPE == 1)
		{
			//如果是稳定流模拟, 如果要输出则默认逐时段输出
			if (m_REGBDPRN > 0)
			{
				m_REGBDPRN = 1;
			}
		}
		m_pRegBudget->SetPrnOpt(m_REGBDPRN);
	}
	else
	{
		m_REGBDPRN = 0;
	}
}

void CGDWater::ReadPerCtrl(CStdioFile& FileTxtIn)
{
	//读入应力期控制参数(TXT文件)

	CString strTemp, strTest, strFormat;
	CPressPeriod * pPPeriod = NULL;
	long IPER, NSTEP;
	double PERLEN, MULTR;
	//略过表头
	FileTxtIn.ReadString(strTemp);

	//读取数据
	strFormat = _T("%ld %lg %ld %lg");
	long SumDataRow = 0;
	for (;;)
	{
		//逐行读入
		FileTxtIn.ReadString(strTemp);
		//累加读入的数据行数
		SumDataRow = SumDataRow + 1;
		//测试读入的数据
		strTest = strTemp;
		strTest.Trim();
		//如果没有读到有效数据
		if (strTest.IsEmpty())
		{
			SumDataRow = SumDataRow - 1;
			if (FileTxtIn.GetPosition() == FileTxtIn.GetLength())
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
		pPPeriod = new CPressPeriod;
		if (sscanf_s(strTemp, strFormat, &IPER, &PERLEN, &NSTEP, &MULTR) != 4)
		{
			printf(_T("\n"));
			printf(_T("数据有缺失,请检查 应力期属性表.in 第%d行的数据!\n"), SumDataRow);
			PauAndQuit();
		}
		if (IPER != SumDataRow)
		{
			printf(_T("\n"));
			printf(_T("应力期编号必须从1开始并连续,请检查 应力期属性表.in 中第%d行的 IPER 字段数据!\n"),
				SumDataRow);
			PauAndQuit();
		}
		if (PERLEN <= 0.0)
		{
			printf(_T("\n"));
			printf(_T("应力期时长必须大于0.0,请检查 应力期属性表.in 中第%d行的 PERLEN 字段数据!\n"),
				SumDataRow);
			PauAndQuit();
		}
		if (NSTEP < 1)
		{
			printf(_T("\n"));
			printf(_T("应力期所分模拟时段的个数必须大于等于1,请检查 应力期属性表.in 中第%d行的 NSTEP 字段数据!\n"),
				SumDataRow);
			PauAndQuit();
		}
		if (MULTR <= 0.0)
		{
			printf(_T("\n"));
			printf(_T("模拟时段的时长倍率因子必须需大于0.0,请检查 应力期属性表.in 中第%d行的 MULTR 字段数据!\n"),
				SumDataRow);
			PauAndQuit();
		}
		//数据加入到应力期数组
		pPPeriod->m_iPeriod = IPER;
		pPPeriod->m_PeridLen = PERLEN;
		pPPeriod->m_NStep = NSTEP;
		pPPeriod->m_MultR = MULTR;
		m_pPPeriodAry.push_back(pPPeriod);
	}

	//至少要有一个应力期控制数据
	if (m_pPPeriodAry.size() == 0)
	{
		printf(_T("\n"));
		printf(_T("至少要有1个应力期控制数据,请检查 应力期属性表.in !\n"));
		PauAndQuit();
	}
}

void CGDWater::ReadPerCtrl(_RecordsetPtr& pTable)
{
	//读入应力期控制参数(数据库)

	long i, nCount;
	_variant_t Value;
	CPressPeriod * pPPeriod = NULL;
	try
	{
		pTable->MoveLast();
		nCount = pTable->GetRecordCount();  //得到数据表的记录个数
		pTable->MoveFirst();
		for (i = 0; i < nCount; i++)
		{
			pPPeriod = new CPressPeriod;
			Value = pTable->GetCollect(_T("IPER"));
			pPPeriod->m_iPeriod = Value.intVal;
			Value = pTable->GetCollect(_T("PERLEN"));
			pPPeriod->m_PeridLen = Value.dblVal;
			Value = pTable->GetCollect(_T("NSTEP"));
			pPPeriod->m_NStep = Value.intVal;
			Value = pTable->GetCollect(_T("MULTR"));
			pPPeriod->m_MultR = Value.dblVal;
			if (pPPeriod->m_iPeriod != i + 1)
			{
				printf(_T("\n"));
				printf(_T("应力期编号必须从1开始并连续, 请检查<应力期属性表>中的 IPER 字段数据！\n"));
				PauAndQuit();
			}
			if (pPPeriod->m_PeridLen <= 0.0)
			{
				printf(_T("\n"));
				printf(_T("应力期时长必须大于0.0, 请检查<应力期属性表>中的 PERLEN 字段数据！\n"));
				PauAndQuit();
			}
			if (pPPeriod->m_NStep < 1)
			{
				printf(_T("\n"));
				printf(_T("应力期所分模拟时段的个数必须大于等于1, 请检查<应力期属性表>中的 NSTEP 字段数据！\n"));
				PauAndQuit();
			}
			if (pPPeriod->m_MultR <= 0.0)
			{
				printf(_T("\n"));
				printf(_T("模拟时段的时长倍率因子必须需大于0.0,  请检查<应力期属性表>中的 MULTR 字段数据！\n"));
				PauAndQuit();
			}
			//加入到应力期数组
			m_pPPeriodAry.push_back(pPPeriod);
			pTable->MoveNext();                               //定位到下一条数据
		}
	}
	catch (_com_error *e)
	{
		printf((LPCTSTR)e->ErrorMessage());
	}

	//至少要有一个应力期控制数据
	if (m_pPPeriodAry.size() == 0)
	{
		printf(_T("\n"));
		printf(_T("只要要有1个应力期控制数据,请检查<应力期属性表>!\n"));
		PauAndQuit();
	}
}

void CGDWater::ReadDimRC(CStdioFile& FileTxtIn)
{
	//读入沿行和沿列方向的离散间距数据

	CString strTemp, strTest, strFormat;
	//略过表头
	FileTxtIn.ReadString(strTemp);

	//读取数据
	char Indicator[50] = {0};
	CString strIndicator;
	long NUMID;
	double DeltLen;
	long CumRow, CumCol;
	CumRow = 0;
	CumCol = 0;
	strFormat = _T("%s %ld %lg");
	long SumDataRow = 0;
	for (;;)
	{
		//逐行读入
		FileTxtIn.ReadString(strTemp);
		//累加读入的数据行数
		SumDataRow = SumDataRow + 1;
		//测试读入的数据
		strTest = strTemp;
		strTest.Trim();
		//如果没有读到有效数据
		if (strTest.IsEmpty())
		{
			SumDataRow = SumDataRow - 1;
			if (FileTxtIn.GetPosition() == FileTxtIn.GetLength())
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
		if (sscanf_s(strTemp, strFormat, Indicator, 50, &NUMID, &DeltLen) != 3)
		{
			printf(_T("\n"));
			printf(_T("数据有缺失,请检查 网格单元水平向间距表.in 中第%d行的数据!\n"),
				SumDataRow);
			PauAndQuit();
		}
		strIndicator = Indicator;
		if (strIndicator != _T("R") && strIndicator != _T("C"))
		{
			printf(_T("\n"));
			printf(_T("ATTI 字段的值必须为 R 或 C, 请检查 网格单元水平向间距表.in 中第%d行的数据!\n"),
				SumDataRow);
			PauAndQuit();
		}
		if (DeltLen <= 0.0)
		{
			printf(_T("\n"));
			printf(_T("DELT 字段的数据必须大于0.0, 网格单元水平向间距表.in 中第%d行的数据!\n"),
				SumDataRow);
			PauAndQuit();
		}
		if (strIndicator == _T("R"))
		{
			//沿行方向的离散间距数据DeltR, 应该有NUMCOL个数据
			CumCol = CumCol + 1;
			if (NUMID < 1 || NUMID > m_NUMCOL)
			{
				printf(_T("\n"));
				printf(_T("沿行方向的离散间距编号错误,必须在1-%d之间, 请检查 网格单元水平向间距表.in 中第%d行的数据!\n"),
					m_NUMCOL, SumDataRow);
				PauAndQuit();
			}
			if (NUMID != CumCol)
			{
				printf(_T("\n"));
				printf(_T("沿行方向的离散间距编号没从1开始或不连续, 请检查 网格单元水平向间距表.in 中第%d行的数据!\n"),
					SumDataRow);
				PauAndQuit();
			}
			m_AryDelR[NUMID - 1] = DeltLen;
		}
		else
		{
			//沿列方向的离散间距数据DeltC, 应该有NUMROW个数据
			CumRow = CumRow + 1;
			if (NUMID < 1 || NUMID > m_NUMROW)
			{
				printf(_T("\n"));
				printf(_T("沿列方向的离散间距编号错误,必须在1-%d之间, 请检查 网格单元水平向间距表.in 中第%d行的数据!\n"),
					m_NUMROW, SumDataRow);
				PauAndQuit();
			}
			if (NUMID != CumRow)
			{
				printf(_T("\n"));
				printf(_T("沿列方向的离散间距编号没从1开始或不连续, 请检查 网格单元水平向间距表.in 中第%d行的数据!\n"),
					SumDataRow);
				PauAndQuit();
			}
			m_AryDelC[NUMID - 1] = DeltLen;
		}
	}

	//检查数据是否完整
	if (CumRow != m_NUMROW)
	{
		printf(_T("\n"));
		printf(_T("缺失 %d 个沿列方向的离散间距数据, 请检查 网格单元水平向间距表.in !\n"), m_NUMROW - CumRow);
		PauAndQuit();
	}
	if (CumCol != m_NUMCOL)
	{
		printf(_T("\n"));
		printf(_T("缺失 %d 个沿行方向的离散间距数据, 请检查 网格单元水平向间距表.in !\n"), m_NUMCOL - CumCol);
		PauAndQuit();
	}
}

void CGDWater::ReadDimRC(_RecordsetPtr& pTable)
{
	//读入沿行和沿列方向的离散间距数据(数据库)

	long i, nCount;
    _variant_t Value;
	CString strTemp;
	long NUMID, CumRow, CumCol;
	double DeltLen;
	try
	{
        pTable->MoveLast();
		nCount = pTable->GetRecordCount();  //得到数据表的记录个数
		pTable->MoveFirst();
		CumRow = 0;
		CumCol = 0;
		for (i = 0; i < nCount; i++)
		{
			Value = pTable->GetCollect(_T("ATTI"));
			strTemp = Value.bstrVal;
			if (strTemp != _T("R") && strTemp != _T("C"))
			{
				printf(_T("\n"));
				printf(_T("ATTI 字段的值必须为 R 或 C, 请检查<网格单元水平向间距表>!\n"));
				PauAndQuit();
			}
			Value = pTable->GetCollect(_T("NUMID"));
			NUMID = Value.intVal;
			Value = pTable->GetCollect(_T("DELT"));
			DeltLen = Value.dblVal;
			if (DeltLen <= 0.0)
			{
				printf(_T("\n"));
				printf(_T("DELT 字段的数据必须大于0.0, 请检查<网格单元水平向间距表>!\n"));
				PauAndQuit();
			}
			if (strTemp == _T("R"))
			{
				//沿行方向的离散间距数据DeltR, 应该有NUMCOL个数据
				CumCol = CumCol + 1;
				if (NUMID < 1 || NUMID > m_NUMCOL)
				{
					printf(_T("\n"));
					printf(_T("沿行方向的离散间距编号错误,必须在1-%d之间, 请检查<网格单元水平向间距表>!\n"),
						m_NUMCOL);
					PauAndQuit();
				}
				if (NUMID != CumCol)
				{
					printf(_T("\n"));
					printf(_T("沿行方向的离散间距编号没从1开始或不连续, 请检查<网格单元水平向间距表>!\n"));
					PauAndQuit();
				}
				m_AryDelR[NUMID - 1] = DeltLen;
			}
			else
			{
				//沿列方向的离散间距数据DeltC, 应该有NUMROW个数据
				CumRow = CumRow + 1;
				if (NUMID < 1 || NUMID > m_NUMROW)
				{
					printf(_T("\n"));
					printf(_T("沿列方向的离散间距编号错误,必须在1-%d之间, 请检查<网格单元水平向间距表>!\n"), 
						m_NUMROW);
					PauAndQuit();
				}
				if (NUMID != CumRow)
				{
					printf(_T("\n"));
					printf(_T("沿列方向的离散间距编号没从1开始或不连续, 请检查<网格单元水平向间距表>!\n"));
					PauAndQuit();
				}
				m_AryDelC[NUMID - 1] = DeltLen;
			}
			pTable->MoveNext();
		}
	}
	catch(_com_error *e)
	{
		printf((LPCTSTR)e->ErrorMessage());
	}

	//检查数据是否完整
	if (CumRow != m_NUMROW)
	{
		printf(_T("\n"));
		printf(_T("沿列方向的离散间距数据缺失, 请检查<网格单元水平向间距表>!\n"));
		PauAndQuit();
	}
	if (CumCol != m_NUMCOL)
	{
		printf(_T("\n"));
		printf(_T("沿行方向的离散间距数据缺失, 请检查<网格单元水平向间距表>!\n"));
		PauAndQuit();
	}
}

void CGDWater::ReadLyrData(_RecordsetPtr& pTable)
{
	//读入含水层属性数据(数据库)

	long nCount;
	_variant_t Value;
	CString strTemp;
	long iLyr;
	CGridLyr* pGridLyr = NULL;
	try
	{
		pTable->MoveLast();
		nCount = pTable->GetRecordCount();  //得到数据表的记录个数
		pTable->MoveFirst();
		if (nCount != m_NUMLYR)
		{
			printf(_T("\n"));
			printf(_T("缺失 %d 个含水层的属性数据, 请检查!\n"), m_NUMLYR - nCount);
			PauAndQuit();
		}
		for (iLyr = 0; iLyr < nCount; iLyr++)
		{
			pGridLyr = GetGridLyr(iLyr + 1);
			//读取含水层编号
			Value = pTable->GetCollect(_T("LYRID"));
			pGridLyr->m_LYRID = Value.intVal;
			if (pGridLyr->m_LYRID != iLyr + 1)
			{
				printf(_T("\n"));
				printf(_T("含水层的编号没从1开始或不连续, 请检查!\n"));
				PauAndQuit();
			}
			if (m_INTBLKM == 1)
			{
				//BCF含水层数据
				//读取含水层性质
				Value = pTable->GetCollect(_T("LYRCON"));
				pGridLyr->m_LYRCON = Value.intVal;
				//读取含水层K值在Y方向上的异性参数
				Value = pTable->GetCollect(_T("LYRTRPY"));
				pGridLyr->m_LYRTRPY = Value.dblVal;
				if (m_SIMTYPE == 2 && m_SimIBS == 1)
				{
					//如果模拟非稳定流且模拟夹层体贮水, 读入含水层的夹层体模拟选项
					Value = pTable->GetCollect(_T("LYRIBS"));
					pGridLyr->m_LYRIBS = Value.intVal;
				}
			}
			else
			{
				//LPF含水层数据
				//读取含水层性质
				Value = pTable->GetCollect(_T("LYRTYPE"));
				pGridLyr->m_LYRCON = Value.intVal;
				//读取含水层K值在Y方向上的异性参数
				Value = pTable->GetCollect(_T("LYRHANI"));
				pGridLyr->m_LYRTRPY = Value.dblVal;
				//读取含水层K值在Z方向上的异性参数
				Value = pTable->GetCollect(_T("LYRVKA"));
				pGridLyr->m_LYRVKA = Value.intVal;
				//读取含水层是否模拟拟三维选项
				Value = pTable->GetCollect(_T("LYRCBD"));
				pGridLyr->m_LYRCBD = Value.intVal;
				if (m_SIMTYPE == 2 && m_SimIBS == 1)
				{
					//如果模拟非稳定流且模拟夹层体贮水, 读入含水层的夹层体模拟选项
					Value = pTable->GetCollect(_T("LYRIBS"));
					pGridLyr->m_LYRIBS = Value.intVal;
				}
			}
			//含水层检查自身数据
			pGridLyr->CheckData();
			pTable->MoveNext();
		}
	}
	catch (_com_error *e)
	{
		printf((LPCTSTR)e->ErrorMessage());
	}
}

void CGDWater::ReadLyrData(CStdioFile& FileTxtIn)
{
	//读入含水层属性数据(TXT文件)

	CString strTemp, strTest, strFormat;
	//略过表头
	FileTxtIn.ReadString(strTemp);

	//读取数据
	long LYRID, LYRCON, LYRIBS, LYRVKA, LYRCBD;
	double LYRTRPY;
	CGridLyr* pGridLyr = NULL;
	if (m_INTBLKM == 1)
	{
		//BCF含水层
		strFormat = _T("%ld %ld %lg %ld");
	}
	else
	{
		//LPF含水层
		strFormat = _T("%ld %ld %lg %ld %ld %ld");
	}
	long SumDataRow = 0;
	for (;;)
	{
		//逐行读入
		FileTxtIn.ReadString(strTemp);
		//累加读入的数据行数
		SumDataRow = SumDataRow + 1;
		//测试读入的数据
		strTest = strTemp;
		strTest.Trim();
		//如果没有读到有效数据
		if (strTest.IsEmpty())
		{
			SumDataRow = SumDataRow - 1;
			if (FileTxtIn.GetPosition() == FileTxtIn.GetLength())
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
		if (m_INTBLKM == 1)
		{
			//BCF含水层数据
			if (sscanf_s(strTemp, strFormat, &LYRID, &LYRCON, &LYRTRPY, &LYRIBS) != 4)
			{
				printf(_T("\n"));
				printf(_T("数据有缺失,请检查!\n"));
				PauAndQuit();
			}
			if (!(m_SIMTYPE == 2 && m_SimIBS == 1))
			{
				//仅模拟非稳定流和夹层体贮水时该参数有效
				LYRIBS = 0;
			}
			//检查数据
			if (LYRID != SumDataRow)
			{
				printf(_T("\n"));
				printf(_T("含水层的编号没从1开始或不连续, 请检查!\n"));
				PauAndQuit();
			}
			pGridLyr = GetGridLyr(LYRID);
			pGridLyr->m_LYRID = LYRID;
			pGridLyr->m_LYRCON = LYRCON;
			pGridLyr->m_LYRTRPY = LYRTRPY;
			pGridLyr->m_LYRIBS = LYRIBS;
		}
		else
		{
			//LPF含水层数据
			if (sscanf_s(strTemp, strFormat, &LYRID, &LYRCON, &LYRTRPY, &LYRVKA, &LYRCBD, &LYRIBS) != 6)
			{
				printf(_T("\n"));
				printf(_T("数据有缺失,请检查!\n"));
				PauAndQuit();
			}
			if (!(m_SIMTYPE == 2 && m_SimIBS == 1))
			{
				//仅模拟非稳定流和夹层体贮水时该参数有效
				LYRIBS = 0;
			}
			//检查数据
			if (LYRID != SumDataRow)
			{
				printf(_T("\n"));
				printf(_T("含水层的编号没从1开始或不连续, 请检查!\n"));
				PauAndQuit();
			}
			pGridLyr = GetGridLyr(LYRID);
			pGridLyr->m_LYRID = LYRID;
			pGridLyr->m_LYRCON = LYRCON;
			pGridLyr->m_LYRTRPY = LYRTRPY;
			pGridLyr->m_LYRVKA = LYRVKA;
			pGridLyr->m_LYRCBD = LYRCBD;
			pGridLyr->m_LYRIBS = LYRIBS;
		}
		//含水层检查自身数据
		pGridLyr->CheckData();
	}

	if (SumDataRow != m_NUMLYR)
	{
		printf(_T("\n"));
		printf(_T("缺失 %d 个含水层的属性数据, 请检查!\n"), m_NUMLYR - SumDataRow);
		PauAndQuit();
	}
}

void CGDWater::ReadCellData(_RecordsetPtr& pTable)
{
	//读入网格单元属性数据(数据库)

	long nCount;
	_variant_t Value;
	CString strTemp;
	long iCell, ILYR, IROW, ICOL, INIIBOUND;
	CGridLyr* pGridLyr = NULL;
	CGridCell* pGridCell = NULL;
	long LyrCon;   //含水层的属性
	double CHANI;  //含水层沿列方向饱和渗透系数各项异性值
	long LYRVKA;   //含水层所含网格单元的VKA参数含义
	long LYRCBD;   //含水层是否模拟拟三维问题
	try
	{
		pTable->MoveLast();
		nCount = pTable->GetRecordCount();  //得到数据表的记录个数
		pTable->MoveFirst();
		if (nCount != m_NumNodes)
		{
			printf(_T("\n"));
			printf(_T("缺失 %d 个网格单元的属性数据, 请检查!\n"), m_NumNodes - nCount);
			PauAndQuit();
		}
		for (iCell = 0; iCell < nCount; iCell++)
		{
			//读入层号
			Value = pTable->GetCollect(_T("ILYR"));
			ILYR = Value.intVal;
			//读入行号
			Value = pTable->GetCollect(_T("IROW"));
			IROW = Value.intVal;
			//读入列号
			Value = pTable->GetCollect(_T("ICOL"));
			ICOL = Value.intVal;
			GDWMOD.CheckCellLimit(ILYR, IROW, ICOL);
			if (iCell != GetCellNode(ILYR, IROW, ICOL))
			{
				printf(_T("\n"));
				printf(_T("网格单元必须按照(层,行,列)的顺序逐一编号输入, 请检查!\n"));
				PauAndQuit();
			}
			//读入单元初始属性
			Value = pTable->GetCollect(_T("IBOUND"));
			INIIBOUND = Value.intVal;
			if (INIIBOUND < -1 || INIIBOUND >1)
			{
				printf(_T("\n"));
				printf(_T("编号为第%d层第%d行第%d列的网格单元的 IBOUND 值必须为-1、0、1其中之一,请检查!\n"),
					ILYR, IROW, ICOL);
				PauAndQuit();
			}
			//获得网格单元的指针
			pGridCell = GetGridCell(ILYR, IROW, ICOL);
			pGridCell->m_ILYR = ILYR;
			pGridCell->m_IROW = IROW;
			pGridCell->m_ICOL = ICOL;
			pGridCell->m_INIIBOUND = INIIBOUND;
			pGridLyr = GetGridLyr(ILYR);
			LyrCon = pGridLyr->GetLyrCon();
			if (m_INTBLKM == 1)
			{
				//BCF含水层网格单元数据
				if (INIIBOUND != 0)
				{
					//以下数据仅针对初始有效单元读入
					if (LyrCon == 2 || LyrCon == 3)
					{
						//承压/非承压含水层(2,3)需要TOP数据,其他类型不需要
						Value = pTable->GetCollect(_T("CELLTOP"));
						pGridCell->m_TOP = Value.dblVal;
					}
					if (LyrCon == 1 || LyrCon == 3)
					{
						//潜水含水层和导水系数可变的承压/非承压含水层需要BOT数据,其他类型不需要
						Value = pTable->GetCollect(_T("CELLBOT"));
						pGridCell->m_BOT = Value.dblVal;
					}
					if (LyrCon == 0 || LyrCon == 2)
					{
						//纯承压层(0)和导水系数不变的承压/非承压含水层(2)类型需要指定导水系数
						Value = pTable->GetCollect(_T("TRANSM"));
						pGridCell->m_TRAN = Value.dblVal;
					}
					if (LyrCon == 1 || LyrCon == 3)
					{
						//纯潜水含水层(1)和导水系数可变的承压/非承压含水层(3)需要指定渗透系数
						Value = pTable->GetCollect(_T("HK"));
						pGridCell->m_HK = Value.dblVal;
					}
					if (ILYR != m_NUMLYR)
					{
						//如果不是最后一层,需要指定越流系数
						Value = pTable->GetCollect(_T("VCONT"));
						pGridCell->m_VCONT = Value.dblVal;
					}
					//如果是非稳定流模拟
					if (m_SIMTYPE == 2)
					{
						//读入第一类贮水系数。对纯潜水该值为给水度;对纯承压水该值为贮水系数
						//对承压/非承压含水层为承压态时贮水系数
						Value = pTable->GetCollect(_T("SC1"));
						pGridCell->m_SC1 = Value.dblVal;
						if (LyrCon == 2 || LyrCon == 3)
						{
							//如果是承压/非承压含水层(2,3), 继续读入第二类贮水系数(即处于非承压状态时的给水度)
							Value = pTable->GetCollect(_T("SC2"));
							pGridCell->m_SC2 = Value.dblVal;
						}
					}
					//如果采用MODFLOW试算法模拟单元干湿变化且网格单元初始为变水头单元, 读入干湿转化参数
					if (m_SIMMTHD == 2 && m_IWDFLG == 1 && INIIBOUND > 0)
					{
						if (LyrCon == 1 || LyrCon == 3)
						{
							//仅对导水系数可变的含水层单元才需要读入
							Value = pTable->GetCollect(_T("WETDRY"));
							pGridCell->m_WETDRY = Value.dblVal;
						}
					}
					//读入初始水头
					Value = pTable->GetCollect(_T("SHEAD"));
					pGridCell->m_SHEAD = Value.dblVal;
				}
			}
			else
			{
				//LPF含水层网格单元数据
				CHANI = pGridLyr->GetLyrTRPY();
				LYRVKA = pGridLyr->GetLyrVKA();
				LYRCBD = pGridLyr->GetLyrCBD();
				assert(LyrCon == 0 || LyrCon == 3);
				//如果是第1层读入含水层的顶板高程
				if (ILYR == 1)
				{
					Value = pTable->GetCollect(_T("CELLTOP"));
					pGridCell->m_TOP = Value.dblVal;
				}
				//读入网格单元的底板高程
				Value = pTable->GetCollect(_T("CELLBOT"));
				pGridCell->m_BOT = Value.dblVal;
				if (INIIBOUND != 0)
				{
					//以下数据仅针对初始有效单元读入
					//读入沿行方向的渗透系数
					Value = pTable->GetCollect(_T("HK"));
					pGridCell->m_HK = Value.dblVal;
					//如果含水层的CHANI小于0(等于-1)，读入网格单元沿列方向的渗透系数比例因子
					if (CHANI < 0)
					{
						Value = pTable->GetCollect(_T("HANI"));
						pGridCell->m_HANI = Value.dblVal;
					}
					//读入网格单元的垂向渗透系数
					Value = pTable->GetCollect(_T("VKA"));
					pGridCell->m_VKA = Value.dblVal;
					//如果含水层模拟拟三维问题,且不是最底层,读入网格单元底部低渗透性介质的垂向渗透系数和厚度
					if (LYRCBD == 1 && ILYR != m_NUMLYR)
					{
						Value = pTable->GetCollect(_T("VKCB"));
						pGridCell->m_VKCB = Value.dblVal;
						Value = pTable->GetCollect(_T("TKCB"));
						pGridCell->m_TKCB = Value.dblVal;
					}
					//如果模拟非稳定流
					if (m_SIMTYPE == 2)
					{
						//读入含水层的第1类贮水系数(承压状态时的贮水率)	
						Value = pTable->GetCollect(_T("SC1"));
						pGridCell->m_SC1 = Value.dblVal;
						if (LyrCon == 3)
						{
							//如果含水层类型属性为3，继续读入含水层的第2类贮水系数(非承压状态时的给水度)
							Value = pTable->GetCollect(_T("SC2"));
							pGridCell->m_SC2 = Value.dblVal;
						}
					}
					//如果采用MODFLOW试算法模拟单元干湿变化且网格单元初始为变水头单元, 读入干湿转化参数
					if (m_SIMMTHD == 2 && m_IWDFLG == 1 && INIIBOUND > 0)
					{
						if (LyrCon == 3)
						{
							//仅对导水系数可变的含水层单元才需要读入
							Value = pTable->GetCollect(_T("WETDRY"));
							pGridCell->m_WETDRY = Value.dblVal;
						}
					}
					//读入初始水头
					Value = pTable->GetCollect(_T("SHEAD"));
					pGridCell->m_SHEAD = Value.dblVal;
				}
			}
			//网格单元自身检查输入数据
			pGridCell->CheckCellData();
			pTable->MoveNext();
		}
	}
	catch (_com_error *e)
	{
		printf((LPCTSTR)e->ErrorMessage());
	}
}

void CGDWater::ReadCellData(CStdioFile& FileTxtIn)
{
	//读入网格单元属性数据(TXT文件)

	CString strTemp, strTest, strFormat;
	//略过表头
	FileTxtIn.ReadString(strTemp);

	//读取数据
	long iCell, ILYR, IROW, ICOL, INIIBOUND;
	double CELLTOP, CELLBOT, TRANSM, HK, HANI, VCONT, VKA, VKCB, TKCB, SC1, SC2, WETDRY, SHEAD;
	long LyrCon, LYRCBD;
	double LYRHANI;
	CGridCell* pGridCell = NULL;
	if (m_INTBLKM == 1)
	{
		//BCF含水层
		strFormat = _T("%ld %ld %ld %ld %lg %lg %lg %lg %lg %lg %lg %lg %lg");
	}
	else
	{
		//LPF含水层
		strFormat = _T("%ld %ld %ld %lg %lg %ld %lg %lg %lg %lg %lg %lg %lg %lg %lg");
	}
	long SumDataRow = 0;
	for (;;)
	{
		//逐行读入
		FileTxtIn.ReadString(strTemp);
		//累加读入的数据行数
		SumDataRow = SumDataRow + 1;
		//测试读入的数据
		strTest = strTemp;
		strTest.Trim();
		//如果没有读到有效数据
		if (strTest.IsEmpty())
		{
			SumDataRow = SumDataRow - 1;
			if (FileTxtIn.GetPosition() == FileTxtIn.GetLength())
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
		if (m_INTBLKM == 1)
		{
			//BCF含水层数据
			if (sscanf_s(strTemp, strFormat, &ILYR, &IROW, &ICOL, &INIIBOUND, &CELLTOP, &CELLBOT,
				&TRANSM, &HK, &VCONT, &SC1, &SC2, &WETDRY, &SHEAD) != 13)
			{
				printf(_T("\n"));
				printf(_T("数据有缺失,请检查!\n"));
				PauAndQuit();
			}
			CheckCellLimit(ILYR, IROW, ICOL);
			iCell = GetCellNode(ILYR, IROW, ICOL) + 1;
			if (iCell != SumDataRow)
			{
				printf(_T("\n"));
				printf(_T("网格单元必须按照(层,行,列)的顺序逐一编号输入, 请检查!\n"));
				PauAndQuit();
			}
			if (INIIBOUND < -1 || INIIBOUND >1)
			{
				printf(_T("\n"));
				printf(_T("编号为第%d层第%d行第%d列的网格单元的 IBOUND 值必须为-1、0、1其中之一,请检查!\n"),
					ILYR, IROW, ICOL);
				PauAndQuit();
			}
			if (INIIBOUND != 0)
			{
				//如果为有效的网格单元
				LyrCon = GetLyrCon(ILYR);
				//根据含水层类型和模拟选项确定哪些数据应该清零
				if (LyrCon == 0 || LyrCon == 1)
				{
					//如果含水层类型为0或1, 顶板数据无效
					CELLTOP = 0.0;
				}
				if (LyrCon == 0 || LyrCon == 2)
				{
					//如果含水层类型为0或2
					//底板数据无效
					CELLBOT = 0.0;
					//渗透系数无效
					HK = 0.0;
				}
				if (LyrCon == 1 || LyrCon == 3)
				{
					//如果含水层类型为1或3, 导水系数数据无效
					TRANSM = 0.0;
				}
				if (ILYR == m_NUMLYR)
				{
					//如果是最后一层, 越流系数无效
					VCONT = 0.0;
				}
				if (m_SIMTYPE == 1)
				{
					//如果是稳定流模拟, 第1类和第2类贮水系数无效
					SC1 = 0.0;
					SC2 = 0.0;
				}
				if (LyrCon == 0 || LyrCon == 1)
				{
					//如果含水层类型为0和1, 第2类贮水系数无效
					SC2 = 0.0;
				}
				if (!(m_SIMMTHD == 2 && m_IWDFLG == 1 && (LyrCon == 1 || LyrCon == 3) && INIIBOUND > 0))
				{
					//干湿转化参数无效
					WETDRY = 0.0;
				}
			}
			else
			{
				//无效的网格单元
				CELLTOP = 0.0;
				CELLBOT = 0.0;
				TRANSM = 0.0;
				HK = 0.0;
				VCONT = 0.0;
				SC1 = 0.0;
				SC2 = 0.0;
				WETDRY = 0.0;
				SHEAD = 0.0;
			}
			//设置网格单元数据
			pGridCell = GetGridCell(ILYR, IROW, ICOL);
			pGridCell->m_ILYR = ILYR;
			pGridCell->m_IROW = IROW;
			pGridCell->m_ICOL = ICOL;
			pGridCell->m_INIIBOUND = INIIBOUND;
			pGridCell->m_TOP = CELLTOP;
			pGridCell->m_BOT = CELLBOT;
			pGridCell->m_TRAN = TRANSM;
			pGridCell->m_HK = HK;
			pGridCell->m_VCONT = VCONT;
			pGridCell->m_SC1 = SC1;
			pGridCell->m_SC2 = SC2;
			pGridCell->m_WETDRY = WETDRY;
			pGridCell->m_SHEAD = SHEAD;
		}
		else
		{
			//LPF含水层数据
			if (sscanf_s(strTemp, strFormat, &ILYR, &IROW, &ICOL, &CELLTOP, &CELLBOT, &INIIBOUND,
				&HK, &HANI, &VKA, &VKCB, &TKCB, &SC1, &SC2, &WETDRY, &SHEAD) != 15)
			{
				printf(_T("\n"));
				printf(_T("数据有缺失, 请检查!\n"));
				PauAndQuit();
			}
			CheckCellLimit(ILYR, IROW, ICOL);
			iCell = GetCellNode(ILYR, IROW, ICOL) + 1;
			if (iCell != SumDataRow)
			{
				printf(_T("\n"));
				printf(_T("网格单元必须按照(层,行,列)的顺序逐一编号输入, 请检查!\n"));
				PauAndQuit();
			}
			if (INIIBOUND < -1 || INIIBOUND >1)
			{
				printf(_T("\n"));
				printf(_T("编号为第%d层第%d行第%d列的网格单元的 IBOUND 值必须为-1、0、1其中之一,请检查!\n"),
					ILYR, IROW, ICOL);
				PauAndQuit();
			}
			if (ILYR != 1)
			{
				//如果不为首层网格单元, 顶板高程数据无效
				CELLTOP = 0.0;
			}
			if (INIIBOUND != 0)
			{
				//如果为有效的网格单元
				LyrCon = GetLyrCon(ILYR);
				LYRHANI = GetGridLyr(ILYR)->GetLyrTRPY();
				LYRCBD = GetGridLyr(ILYR)->GetLyrCBD();
				//根据含水层类型和模拟选项确定哪些数据应该清零
				if (fabs(LYRHANI + 1.0) > 1e-10)
				{
					//LYRHANI等于-1表示该含水层每个网格单独设置ky/kx的比值
					//如果不单独设置, 则网格单元上的HANI数据无效
					HANI = 0.0;
				}
				if (LYRCBD == 0 || ILYR == m_NUMLYR)
				{
					//如果含水层不模拟拟三维, 或含水层是底层含水层
					//网格单元上有关拟三维模拟的两个参数无效
					VKCB = 0.0;
					TKCB = 0.0;
				}
				if (m_SIMTYPE == 1)
				{
					//如果模拟稳定流, 承压状态下的贮水率和非承压状态下的给水度参数无效
					SC1 = 0.0;
					SC2 = 0.0;
				}
				if (LyrCon == 0)
				{
					//如果含水层类型为0, 非承压状态下的给水度参数无效
					SC2 = 0.0;
				}
				if (!(m_SIMMTHD == 2 && m_IWDFLG == 1 && LyrCon == 3 && INIIBOUND > 0))
				{
					//干湿转化参数无效
					WETDRY = 0.0;
				}
			}
			else
			{
				//无效的网格单元
				HK = 0.0;
				HANI = 0.0;
				VKA = 0.0;
				VKCB = 0.0;
				TKCB = 0.0;
				SC1 = 0.0;
				SC2 = 0.0;
				WETDRY = 0.0;
				SHEAD = 0.0;
			}
			//设置网格单元数据
			pGridCell = GetGridCell(ILYR, IROW, ICOL);
			pGridCell->m_ILYR = ILYR;
			pGridCell->m_IROW = IROW;
			pGridCell->m_ICOL = ICOL;
			pGridCell->m_TOP = CELLTOP;
			pGridCell->m_BOT = CELLBOT;
			pGridCell->m_INIIBOUND = INIIBOUND;
			pGridCell->m_HK = HK;
			pGridCell->m_HANI = HANI;
			pGridCell->m_VKA = VKA;
			pGridCell->m_VKCB = VKCB;
			pGridCell->m_TKCB = TKCB;
			pGridCell->m_SC1 = SC1;
			pGridCell->m_SC2 = SC2;
			pGridCell->m_WETDRY = WETDRY;
			pGridCell->m_SHEAD = SHEAD;
		}
		//网格单元自身检查输入数据
		pGridCell->CheckCellData();
	}

	//检查网格单元数据是否充分
	if (SumDataRow != m_NumNodes)
	{
		printf(_T("\n"));
		printf(_T("缺失 %d 个网格单元的属性数据, 请检查!\n"), m_NumNodes - SumDataRow);
		PauAndQuit();
	}
}

void CGDWater::AddObsCell(long iLyr, long iRow, long iCol)
{
	//添加网格单元观测点数据

	CheckCellLimit(iLyr, iRow, iCol);
	long IniIBound = GetCellIniIBOUND(iLyr, iRow, iCol);
	if (IniIBound == 0)
	{
		//夹层体位于无效单元或定水头单元上.报错
		printf(_T("\n"));
		printf(_T("第%d层第%d行第%d列的网格单元为无效单元! 请检查<网格单元观测点表>!\n"),
			iLyr, iRow, iCol);
		PauAndQuit();
	}
	CGridCell* pGridCell = GetGridCell(iLyr, iRow, iCol);
	m_pObsCellAry.push_back(pGridCell);
}

void CGDWater::ReadObsCellData(_RecordsetPtr& pTable)
{
	//读入网格单元观测点数据(数据库)

	_variant_t Value;
	CString strTemp;
	long i, nCount, iLyr, iRow, iCol;
	CGridCell* pGridCell = NULL;
	try
	{
		pTable->MoveLast();
		nCount = pTable->GetRecordCount();  //得到数据表的记录个数
		pTable->MoveFirst();
		for (i = 0; i < nCount; i++)
		{
			Value = pTable->GetCollect(_T("ILYR"));
			iLyr = Value.intVal;
			Value = pTable->GetCollect(_T("IROW"));
			iRow = Value.intVal;
			Value = pTable->GetCollect(_T("ICOL"));
			iCol = Value.intVal;
			AddObsCell(iLyr, iRow, iCol);
			pTable->MoveNext();
		}
	}
	catch (_com_error* e)
	{
		printf((LPCTSTR)e->ErrorMessage());
	}
}

void CGDWater::ReadObsCellData(CStdioFile& FileTxtIn)
{
	//读入网格单元观测点数据(TXT文件)

	CString strTemp, strTest, strFormat;
	//略过表头
	FileTxtIn.ReadString(strTemp);

	//读取数据
	long iLyr, iRow, iCol;
	CGridCell* pGridCell = NULL;
	strFormat = _T("%ld %ld %ld");
	long SumDataRow = 0;
	for (;;)
	{
		//逐行读入
		FileTxtIn.ReadString(strTemp);
		//累加读入的数据行数
		SumDataRow = SumDataRow + 1;
		//测试读入的数据
		strTest = strTemp;
		strTest.Trim();
		//如果没有读到有效数据
		if (strTest.IsEmpty())
		{
			SumDataRow = SumDataRow - 1;
			if (FileTxtIn.GetPosition() == FileTxtIn.GetLength())
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
		if (sscanf_s(strTemp, strFormat, &iLyr, &iRow, &iCol) != 3)
		{
			printf(_T("\n"));
			printf(_T("数据有缺失,请检查!\n"));
			PauAndQuit();
		}
		AddObsCell(iLyr, iRow, iCol);
	}
}

void CGDWater::InitSimulation()
{
	//模拟之前进行初始化工作

	printf(_T("Initialize model data...\n\n"));

	//记录定常水头单元
	CGridCell* pGridCell = NULL;
	CCHBCell* pCHBCell = NULL;
	long k, i, j;
	for (k = 0; k < m_NUMLYR; k++)
	{
		for (i = 0; i < m_NUMROW; i++)
		{
			for (j = 0; j < m_NUMCOL; j++)
			{
				pGridCell = m_pGDLayers[k]->m_pGridCellAry[i][j];
				if (pGridCell->m_INIIBOUND < 0)
				{
					assert(pGridCell->m_INIIBOUND == -1);
					pCHBCell = new CCHBCell;
					pCHBCell->m_pGridCell = pGridCell;
					pCHBCell->m_BndHead = pGridCell->m_hNew;
					m_pCHBCellAry.push_back(pCHBCell);
				}
			}
		}
	}
	if (m_pCHBCellAry.size() > 0)
	{
		//存在定常水头边界, 设置边界处理标识
		size_t size = CGridCell::m_ActSimBndAry.size();
		if (size > 0)
		{
			m_CnhBndID = CGridCell::m_ActSimBndAry[size - 1].m_BndID + 1;
		}
		else
		{
			m_CnhBndID = 0;
		}
		CGridCell::AddBndObjItem(m_CnhBndID, CString(_T("CNH")), CString(_T("   CONSTANT HEAD")));
		//注: AddBndObjItem函数自动将输入的源汇项标识号加1, 所以这里需要减1
		m_CnhBndID = m_CnhBndID - 1;
	}
	//数组元素不会再增加了
	CGridCell::m_ActSimBndAry.shrink_to_fit();

	//检查模型的边界/源汇项情况, 至少要有1个边界/源汇项才能够开展模拟
	if (CGridCell::m_ActSimBndAry.size() == 0)
	{
		printf(_T("\n"));
		printf(_T("至少需具备1个边界/源汇项条件(包括定常水头)才能开展模拟! 请检查!\n"));
		PauAndQuit();
	}

	//设置模型输出项名称(最合适在此调用该函数)
	SetModOutPutItem();

	//若采用全有效单元法, 设置不可疏干单元的最小允许饱和厚度
	if (m_SIMMTHD == 1)
	{
		m_MinHSat = 1E-8;
	}

    //开辟数值模拟计算所需要的一维数组
	try
	{
		long NumNode = m_NumNodes;
		m_AryIBOUND = new long[NumNode]();
		m_AryTR = new double[NumNode]();
		m_AryCR = new double[NumNode]();
		m_AryCC = new double[NumNode]();
		m_AryCV = new double[NumNode]();
		m_AryHCOF = new double[NumNode]();
		m_AryRHS = new double[NumNode]();
		m_AryHNEW = new double[NumNode]();
		m_AryHOLD = new double[NumNode]();
		m_AryHPRE = new double[NumNode]();
		if (m_INTBLKM == 1)
		{
			//仅BCF数据格式时需要
			m_AryCVWD = new double[NumNode]();
		}
		m_ArySC1 = new double[NumNode]();
		m_ArySC2 = new double[NumNode]();
		if (m_SIMMTHD == 2 && m_IWDFLG == 1)
		{
			//仅采用原MODFLOW算法且模拟干-湿转化时需要
			m_AryWetFlag = new long[NumNode]();
		}
	}
	catch (bad_alloc &e)
	{
		printf(e.what());
		exit(1);
	}
	
	//初始化矩阵方程求解对象
	if (m_ISOLVE == 1)
	{
		m_pSolver = new CSsip;
	}
	else
	{
		m_pSolver = new CPcg;
	}
	m_pSolver->Initial(m_NUMLYR, m_NUMROW, m_NUMCOL, m_MAXIT, m_HCLOSE, m_RCLOSE, m_DAMP, m_IRELAX, 
		m_THETA, m_GAMMA, m_AKAPPA, m_NITER, m_SIMMTHD, m_MinHSat);
	
	//对各含水层进行初始化
	for (k = 0; k < m_NUMLYR; k++)
	{
		m_pGDLayers[k]->InitSimulation();
	}

	//检查应力期控制数据的合理性
	if (m_pPPeriodAry.size() < 1)
	{
		printf(_T("\n"));
		printf(_T("至少需要一个地下水数值模拟期控制数据! 请检查!\n"));
		PauAndQuit();
	}
	else
	{
		//应力期控制数据必须连续
		for (i = 0; i < m_pPPeriodAry.size(); i++)
		{
			if (m_pPPeriodAry[i]->m_iPeriod != i + 1)
			{
				printf(_T("\n"));
				printf(_T("应力期控制数据的编号必须从1开始并且连续! 请检查!\n"));
				PauAndQuit();
			}
		}
	}

	//进行数组数据初始化
	InitAndCheck();

	//各边界/源汇项对象进行初始化
	for (i = 0; i < m_pSimBndObjAry.size(); i++)
	{
		m_pSimBndObjAry[i]->InitSimulation();
	}	

	//对统计分区对象进行初始化
	if (m_pRegBudget != NULL)
	{
		m_pRegBudget->InitSimulation();
	}

	//注: 必须在InitAndCheck()函数执行完后调用
	//保存时段初和迭代计算初始时的水头值
	CopyMemory(m_AryHOLD, m_AryHNEW, sizeof(double)* m_NumNodes);
	CopyMemory(m_AryHPRE, m_AryHNEW, sizeof(double) * m_NumNodes);

	//将累积量置0
	m_SumFlowIn = 0.0;          //应力期内累计总进入含水层系统的水量(L3)
	m_SumFlowOut = 0.0;         //应力期内累计总离开含水层系统的水量(L3)

	//设置模拟结果输出数组
	long ItemArySize = long(m_ItemNamAry.size());
	m_ResultAry = new CResultVal[ItemArySize]();

	//边界上流量(进)
	long ArySize = long(CGridCell::m_ActSimBndAry.size());
	m_AryGDWSinkIn = new double[ArySize]();
	m_AryGDWSinkOut = new double[ArySize]();

	if (m_DataContext == 1)
	{
		//TXT文件数据环境, 打开模拟结果输出表
		CString FilePath, strOutFile;
		CString FileHead;
		FilePath = m_strDataOutDir;
		//地下水系统模拟结果输出表
		if (m_IGDWBDPRN != 0)
		{
			strOutFile = FilePath + _T("GWBD.out");
			if (!m_GDWOutPutFile.Open(strOutFile, CFile::modeCreate | CFile::modeWrite | CFile::typeText | CFile::shareDenyNone))
			{
				printf(_T("\n"));
				printf(_T("打开文件失败!\n"));
				PauAndQuit();
			}
			//写表头
			FileHead = _T(" IPER  ISTEP  TLEN");
			for (size_t i = 0; i < m_ItemNamAry.size(); i++)
			{
				FileHead = FileHead + _T("  ") + m_ItemNamAry[i];
			}
			FileHead = FileHead + _T("\n");
			m_GDWOutPutFile.WriteString(FileHead);
		}
		//含水层模拟结果输出表
		if (m_ILYRBDPRN != 0)
		{
			strOutFile = FilePath + _T("LyrBD.out");
			if (!m_LyrOutPutFile.Open(strOutFile, CFile::modeCreate | CFile::modeWrite | CFile::typeText | CFile::shareDenyNone))
			{
				printf(_T("\n"));
				printf(_T("打开文件失败!\n"));
				PauAndQuit();
			}
			//写表头
			FileHead = _T(" IPER  ISTEP  TLEN  ILYR");
			for (size_t i = 0; i < CGridLyr::m_ItemNamAry.size(); i++)
			{
				FileHead = FileHead + _T("  ") + CGridLyr::m_ItemNamAry[i];
			}
			FileHead = FileHead + _T("\n");
			m_LyrOutPutFile.WriteString(FileHead);
		}
		//网格单元观测点模拟结果输出表
		if (m_IOBSOUT != 0 && m_pObsCellAry.size() > 0)
		{
			strOutFile = FilePath + _T("网格单元观测点输出表.out");
			if (!m_OBSCellOutFile.Open(strOutFile, CFile::modeCreate | CFile::modeWrite | CFile::typeText | CFile::shareDenyNone))
			{
				printf(_T("\n"));
				printf(_T("打开文件失败!\n"));
				PauAndQuit();
			}
			//写表头
			FileHead = _T(" IPER  ISTEP  TLEN  DTLEN  ILYR  IROW  ICOL  HEAD  FFRONT  FBACK  FLEFT  FRIGHT  FUP  FDOWN");
			for (size_t i = 0; i < CGridCell::m_ItemNamAry.size(); i++)
			{
				FileHead = FileHead + _T("  ") + CGridCell::m_ItemNamAry[i];
			}
			FileHead = FileHead + _T("\n");
			m_OBSCellOutFile.WriteString(FileHead);
		}
		//水头模拟结果输出表
		if (m_ICELLHHPRN != 0)
		{
			strOutFile = FilePath + _T("CellHead.dat");
			if (!m_HeadOutFile.Open(strOutFile, CFile::modeCreate | CFile::modeWrite | CFile::typeText))
			{
				printf(_T("\n"));
				printf(_T("Fail to open <%s> file!\n"), _T("CellHead.dat"));
				PauAndQuit();
			}
			//写表头
			FileHead = _T(" IPER  ISTEP           TLEN           ILYR    IROW   ICOL");
			FileHead = FileHead + _T("            XCORD               YCORD                HEAD");
			FileHead = FileHead + _T("\n");
			m_HeadOutFile.WriteString(FileHead);
		}
		//降深模拟结果输出表
		if (m_ICELLDDPRN != 0)
		{
			strOutFile = FilePath + _T("CellDD.dat");
			if (!m_HDownOutFile.Open(strOutFile, CFile::modeCreate | CFile::modeWrite | CFile::typeText))
			{
				printf(_T("\n"));
				printf(_T("Fail to open <%s> file!\n"), _T("CellDD.dat"));
				PauAndQuit();
			}
			//写表头
			FileHead = _T(" IPER  ISTEP           TLEN           ILYR    IROW   ICOL");
			FileHead = FileHead + _T("            XCORD               YCORD             DrawDown");
			FileHead = FileHead + _T("\n");
			m_HDownOutFile.WriteString(FileHead);
		}
		//流量模拟结果输出表
		if (m_ICELLFLPRN != 0)
		{
			strOutFile = FilePath + _T("CellFlow.dat");
			if (!m_FLowOutFile.Open(strOutFile, CFile::modeCreate | CFile::modeWrite | CFile::typeText))
			{
				printf(_T("\n"));
				printf(_T("Fail to open <%s> file!\n"), _T("CellFlow.dat"));
				PauAndQuit();
			}
			//写表头
			FileHead = _T(" IPER  ISTEP           DLEN           ILYR    IROW   ICOL");
			FileHead = FileHead + _T("            XCORD               YCORD");
			FileHead = FileHead + _T("                      FlowX                    FlowY                   FlowZ");
			FileHead = FileHead + _T("\n");
			m_FLowOutFile.WriteString(FileHead);
		}
		//网格单元_水量平衡输出表
		if (m_ICELLBDPRN != 0)
		{
			strOutFile = FilePath + _T("CellBD.dat");
			if (!m_CellBDOutFile.Open(strOutFile, CFile::modeCreate | CFile::modeWrite | CFile::typeText))
			{
				printf(_T("\n"));
				printf(_T("Fail to open <%s> file!\n"), _T("CellBD.dat"));
				PauAndQuit();
			}
			//写表头
			FileHead = _T(" IPER  ISTEP           DLEN           ILYR    IROW   ICOL");
			for (size_t i = 0; i < CGridCell::m_ItemNamAry.size(); i++)
			{
				FileHead = FileHead + _T("                     ") + CGridCell::m_ItemNamAry[i];
			}
			FileHead = FileHead + _T("\n");
			m_CellBDOutFile.WriteString(FileHead);
		}
	}
}

void CGDWater::Simulate(CString StrPath)
{
	//进行地下水数值模拟

	//连接数据库
	printf(_T("Link Data...\n"));
	printf(_T("\n"));
	LinkDataSource(StrPath);

	//读入输入数据
	printf(_T("Read Input Data...\n"));
	printf(_T("\n"));
	if (m_DataContext == 1)
	{
		//TXT数据环境
		ReadModDataTXT();
	}
	else
	{
		//sql-Server数据库环境
		ReadModDataDB();
	}

	//进行模拟之前的初始化
	printf(_T("\n"));
	InitSimulation();

	printf(_T("Start simulation...\n"));
	//开始循环模拟
	long iPer, iStep;
	CPressPeriod* pPPerid = NULL;
	long NumSimBndObj = long(m_pSimBndObjAry.size());
	m_CumTLen = 0.0;
	for (iPer = 0; iPer < m_pPPeriodAry.size(); iPer++)
	{
	    //应力期循环
		//设置应力期
		pPPerid = m_pPPeriodAry[iPer];
		m_iPer = iPer + 1;
		m_PerCumDeltT = 0.0;

		//各边界/源汇项对象准备当前应力期数据
#pragma omp parallel for num_threads(m_NUMTD) schedule(static,1)
		for (long iBndObj = 0; iBndObj < NumSimBndObj; iBndObj++)
		{
			//注释: 以下几个变量的定义必须放在for块中不能共用, 以适应并行运算要求
			CBndHandler * pBndHandler = NULL;
			//注释完毕
			pBndHandler = m_pSimBndObjAry[iBndObj];
			if (m_DataContext == 1)
			{
				//TXT数据环境
				pBndHandler->PrePerSim(m_iPer);
			}
			else
			{
				//sql-Server数据库环境
				pBndHandler->PrePerSim(m_pConnection, m_iPer);
			}
		}

		//时间步长循环
		for (iStep = 0; iStep < pPPerid->m_NStep; iStep++)
		{
		    //先计算时间步长
			SetDelt(iStep);

			//输出时段信息
			printf(_T("\n"));
			printf(_T("Period:  %8d    ##   Step:  %5d    ##   Cumulative time: %16.15g\n"),
				m_iPer, m_iStep, m_CumTLen);

			//进行本时间步长内的模拟
			SimOneStep();

			//保存计算结果
			SaveResult();

			//输出计算成果
			if (m_DataContext == 1)
			{
				//TXT数据环境
				//输出全体模拟计算结果到TXT文件
				OutPutAllTXT();
			}
			else
			{
				//sql-Server数据库环境
				//输出全体模拟计算结果到缓存表
				OutPutAllDB();
				//将缓存表中的数据输出到数据库
				WriteDatabase();
			}

			//如果模拟稳定流，计算完毕可以退出
			if (m_SIMTYPE == 1)
			{
				return;
			}
		}
	}
}
void CGDWater::OutPutAllTXT()
{
	//输出全体模拟计算结果(TXT）

	CPressPeriod* pPPerid = m_pPPeriodAry[m_iPer - 1];
	BOOL bEndPer = FALSE;
	if (pPPerid->m_NStep == m_iStep)
	{
		bEndPer = TRUE;
	}

#pragma omp parallel sections
	{
		//输出地下水系统水量平衡计算结果
#pragma omp section
		{
			if (m_IGDWBDPRN == 1 || (m_IGDWBDPRN == 2 && bEndPer == TRUE))
			{
				GDWOutPutTXT();
				//写入文件
				m_GDWOutPutFile.Flush();
			}
		}
		//输出含水层水量平衡计算结果
#pragma omp section
		{
			if (m_ILYRBDPRN == 1 || (m_ILYRBDPRN == 2 && bEndPer == TRUE))
			{
				for (long k = 0; k < m_NUMLYR; k++)
				{
					m_pGDLayers[k]->LyrOutPut(m_ILYRBDPRN, m_iPer, m_iStep, m_CumTLen, m_LyrOutPutFile);
				}
				//写入文件
				m_LyrOutPutFile.Flush();
			}
		}
		//输出网格单元水头计算结果
#pragma omp section
		{
			if (m_ICELLHHPRN == 1 || (m_ICELLHHPRN == 2 && bEndPer == TRUE))
			{
				for (long k = 0; k < m_NUMLYR; k++)
				{
					m_pGDLayers[k]->OutPutLyrCellHH(m_ICELLHHPRN, m_iPer, m_iStep, m_PerCumDeltT, m_CumTLen, m_HeadOutFile);
				}
				//写入文件
				m_HeadOutFile.Flush();
			}
		}
		//输出网格单元降深计算结果
#pragma omp section
		{
			if (m_ICELLDDPRN == 1 || (m_ICELLDDPRN == 2 && bEndPer == TRUE))
			{
				for (long k = 0; k < m_NUMLYR; k++)
				{
					m_pGDLayers[k]->OutPutLyrCellDD(m_ICELLDDPRN, m_iPer, m_iStep, m_PerCumDeltT, m_CumTLen, m_HDownOutFile);
				}
				//写入文件
				m_HDownOutFile.Flush();
			}
		}
		//输出网格单元流量计算结果
#pragma omp section
		{
			if (m_ICELLFLPRN == 1 || (m_ICELLFLPRN == 2 && bEndPer == TRUE))
			{
				OutPutFlowTXT();
				//写入文件
				m_FLowOutFile.Flush();
			}
		}
		//输出网格单元水平衡计算结果
#pragma omp section
		{
			if (m_ICELLBDPRN == 1 || (m_ICELLBDPRN == 2 && bEndPer == TRUE))
			{
				OutPutCellBDTXT();
				//写入文件
				m_CellBDOutFile.Flush();
			}
		}
		//输出各源汇项水量平衡统计结果
#pragma omp section
		{
			long NumBndObj = long(m_pSimBndObjAry.size());
			for (long iBndObj = 0; iBndObj < NumBndObj; iBndObj++)
			{
				m_pSimBndObjAry[iBndObj]->BndOutPutTXT(bEndPer, m_iPer, m_iStep, m_CumTLen, pPPerid->m_PeridLen, m_DeltT);
			}
		}
		//输出统计分区水量平衡结果
#pragma omp section
		{
			if (m_pRegBudget != NULL)
			{
				m_pRegBudget->BudgetOutPutTXT(bEndPer, m_iPer, m_iStep, m_CumTLen);
			}
		}
#pragma omp section
		{
			if (m_IOBSOUT != 0 && m_pObsCellAry.size() > 0)
			{
				OutPutObsCellTXT();
				//写入文件
				m_OBSCellOutFile.Flush();
			}
		}
	}
}

void CGDWater::OutPutAllDB()
{
	//输出全体模拟计算结果(数据库)

	CPressPeriod* pPPerid = m_pPPeriodAry[m_iPer - 1];
	BOOL bEndPer = FALSE;
	if (pPPerid->m_NStep == m_iStep)
	{
		bEndPer = TRUE;
	}

#pragma omp parallel sections
	{
	//输出地下水系统水量平衡计算结果
#pragma omp section
		{
			if (m_IGDWBDPRN == 1 || (m_IGDWBDPRN == 2 && bEndPer == TRUE))
			{
				GDWOutPutDB();
			}
		}
		//输出含水层水量平衡计算结果
#pragma omp section
		{
			if (m_ILYRBDPRN == 1 || (m_ILYRBDPRN == 2 && bEndPer == TRUE))
			{
				for (long k = 0; k < m_NUMLYR; k++)
				{
					m_pGDLayers[k]->LyrOutPut(m_ILYRBDPRN, m_iPer, m_iStep, m_CumTLen);
				}
			}
		}
		//输出网格单元水头计算结果
#pragma omp section
		{
			if (m_ICELLHHPRN == 1 || (m_ICELLHHPRN == 2 && bEndPer == TRUE))
			{
				for (long k = 0; k < m_NUMLYR; k++)
				{
					m_pGDLayers[k]->OutPutLyrCellHH(m_ICELLHHPRN, m_iPer, m_iStep, m_CumTLen);
				}
			}
		}
		//输出网格单元降深计算结果
#pragma omp section
		{
			if (m_ICELLDDPRN == 1 || (m_ICELLDDPRN == 2 && bEndPer == TRUE))
			{
				for (long k = 0; k < m_NUMLYR; k++)
				{
					m_pGDLayers[k]->OutPutLyrCellDD(m_ICELLDDPRN, m_iPer, m_iStep, m_CumTLen);
				}
			}
		}
		//输出网格单元流量计算结果
#pragma omp section
		{
			if (m_ICELLFLPRN == 1 || (m_ICELLFLPRN == 2 && bEndPer == TRUE))
			{
				OutPutFlowDB();
			}
		}
		//输出网格单元水平衡计算结果
#pragma omp section
		{
			if (m_ICELLBDPRN == 1 || (m_ICELLBDPRN == 2 && bEndPer == TRUE))
			{
				OutPutCellBDDB();
			}
		}
		//输出各源汇项水量平衡统计结果
#pragma omp section
		{
			long NumBndObj = long(m_pSimBndObjAry.size());
			for (long iBndObj = 0; iBndObj < NumBndObj; iBndObj++)
			{
				m_pSimBndObjAry[iBndObj]->BndOutPutDB(bEndPer, m_iPer, m_iStep, m_CumTLen, pPPerid->m_PeridLen, m_DeltT);
			}
		}
		//输出统计分区水量平衡结果
#pragma omp section
		{
			if (m_pRegBudget != NULL)
			{
				m_pRegBudget->BudgetOutPutDB(bEndPer, m_iPer, m_iStep, m_CumTLen);
			}
		}
#pragma omp section
		{
			if (m_IOBSOUT != 0 && m_pObsCellAry.size() > 0)
			{
				OutPutObsCellDB();
			}
		}
	}
}

void CGDWater::SimOneStep()
{
	//进行一个时间步长的地下水数值模拟

	//时段模拟之前先处理前一个时间步长模拟的水头
	DealWithHOLD();       

	//各边界/源汇项对象进行当前时段模拟准备
	size_t iBndObj;
	for (iBndObj = 0; iBndObj < m_pSimBndObjAry.size(); iBndObj++)
	{
		m_pSimBndObjAry[iBndObj]->PreStepSim(m_AryIBOUND, m_AryHNEW, m_AryHOLD);
	}

	//开始迭代求解过程
	m_iIter = 1;
	BOOL lcnvg = FALSE;             //迭代是否收敛标示
	for(; ;)
	{
		//先不管源汇项构建矩阵方程
		BASICFM(m_iIter);

		//处理各项地下水边界条件
		for (iBndObj = 0; iBndObj < m_pSimBndObjAry.size(); iBndObj++)
		{
			m_pSimBndObjAry[iBndObj]->FormMatrix(m_AryIBOUND, m_AryHNEW, m_AryHCOF,
				m_AryRHS, m_AryHOLD, m_AryHPRE);
		}

		//求解矩阵方程
		lcnvg = m_pSolver->Solve(m_iIter, m_AryIBOUND, m_AryHNEW, m_AryCR, m_AryCC,
			 m_AryCV, m_AryHCOF, m_AryRHS, m_AryHPRE);

		//如果计算收敛,退出迭代过程
		if (lcnvg && m_iIter > 1)
		{
			//如果迭代计算收敛, 且迭代计算次数为2次以上则结束时段模拟
			break;
		}
		//累计迭代次数
		m_iIter = m_iIter + 1;
	}

	//计算各网格单元间渗流量、网格单元源汇项和蓄量变化情况
	CellBudget();
}

void CGDWater::DealWithHOLD()
{
	//处理HOLD值

	long k, LyrCon;
	long NumTask = long(m_NUMRC / m_NUMTD / 2.0) + 1;
	//先将HNEW值拷贝给HOLD(对拷)
	CopyMemory(m_AryHOLD, m_AryHNEW, sizeof(double)*m_NumNodes);

	//如果不采用全有效单元法
	//检查疏干单元并把HOLD值设为网格单元的底界高程
	if (m_SIMMTHD != 1)
	{
		for (k = 0; k < m_NUMLYR; k++)
		{
			LyrCon = m_pGDLayers[k]->GetLyrCon();
			if (LyrCon == 1 || LyrCon == 3)
			{
				//仅对含水层类型1或3进行操作
#pragma omp parallel for num_threads(m_NUMTD) schedule(dynamic,NumTask)
				for (long iCell = 0; iCell < m_NUMRC; iCell++)
				{
					//注释: 以下几个变量的定义必须放在for块中不能共用, 以适应并行运算要求
					long i, j, node;
					CGridCell* pGridCell = NULL;
					//注释完毕
					//先确定网格单元在数组中的行号和列号
					i = iCell / m_NUMCOL;
					j = iCell - i * m_NUMCOL;
					//进行判断和设置
					node = k * m_NUMRC + i * m_NUMCOL + j;
					pGridCell = m_pGDLayers[k]->m_pGridCellAry[i][j];
					if (m_AryIBOUND[node] == 0 && fabs(pGridCell->m_WETDRY)> 1e-30)
					{
						//若网格单元无效且可被湿润
						//将其HOLD值设置到底板高程处
						m_AryHOLD[node] = pGridCell->m_BOT;
					}
				}
			}
		}
	}
}

CBndHandler* CGDWater::GetSimBndObj(CString BndObjNam)
{
	//返回对应边界/源汇项名称的对象指针

	TrimBlank(BndObjNam);
	CBndHandler* pBndObj = NULL;
	for (size_t iBndObj = 0; iBndObj < m_pSimBndObjAry.size(); iBndObj++)
	{
		if (m_pSimBndObjAry[iBndObj]->m_BndObjNam == BndObjNam)
		{
			pBndObj = m_pSimBndObjAry[iBndObj];
			break;
		}
	}

	return pBndObj;
}

CMemTable* CGDWater::m_pMemDataTable = new CMemTable;
CMemTable* CGDWater::m_pMemObsCellTable = new CMemTable;
void CGDWater::MakeDataTable(_RecordsetPtr pTable, CString TbName)
{
	//构建缓存表

	m_pMemDataTable->MakeDataTable(pTable,TbName);
}

void CGDWater::MakeObsCellTable(_RecordsetPtr pTable, CString TbName)
{
	//构建缓存表

	m_pMemObsCellTable->MakeDataTable(pTable, TbName);
}

void CGDWater::WriteDatabase()
{
	//将内存数据写入数据库

	//注: 并行写入数据库经测试没什么效果
	//可能因为数据库不支持并行写入
	CGridCell::WriteDatabase();
	CGridLyr::WriteDatabase();
	CRegion::WriteDatabase();
	CLandSub::WriteDatabase();
	m_pMemDataTable->WriteDatabase();
	m_pMemObsCellTable->WriteDatabase();
}

_ConnectionPtr CGDWater::GetAdoDatabaseCon()
{
	//返回数据库连接

	assert(m_pConnection != NULL);
	return m_pConnection;
}

String^ CGDWater::GetstrSqlCon()
{
	//返回写入数据库时的连接语句

	return m_strSqlCon;
}

int CGDWater::EnableDirAccess(PCTSTR pszPath, PCTSTR pszAccount)
{
	BOOL bSuccess = TRUE;
	EXPLICIT_ACCESS ea;
	PACL pNewDacl = NULL;
	PACL pOldDacl = NULL;
	do
	{
		// 获取文件 (夹) 安全对象的 DACL 列表
		if (ERROR_SUCCESS != GetNamedSecurityInfo((LPTSTR)pszPath, SE_FILE_OBJECT,
			DACL_SECURITY_INFORMATION, NULL, NULL, &pOldDacl, NULL, NULL))
		{
			bSuccess = FALSE;
			break;
		}
		// 此处不可直接用 AddAccessAllowedAce 函数, 因为已有的 DACL 长度是固定, 必须重新创建一个 DACL 对象
		// 生成指定用户帐户的访问控制信息(这里指定赋予全部的访问权限)
		::BuildExplicitAccessWithName(&ea, (LPTSTR)pszAccount, GENERIC_ALL, GRANT_ACCESS, SUB_CONTAINERS_AND_OBJECTS_INHERIT);
		// 创建新的 ACL 对象(合并已有的 ACL 对象和刚生成的用户帐户访问控制信息)
		if (ERROR_SUCCESS != ::SetEntriesInAcl(1, &ea, pOldDacl, &pNewDacl))
		{
			bSuccess = FALSE;
			break;
		}
		// 设置文件 (夹) 安全对象的 DACL 列表
		if (ERROR_SUCCESS != ::SetNamedSecurityInfo((LPTSTR)pszPath, SE_FILE_OBJECT, DACL_SECURITY_INFORMATION,
			NULL, NULL, pNewDacl, NULL))
		{
			bSuccess = FALSE;
		}
	} while (FALSE);
	if (NULL != pNewDacl)
	{
		::LocalFree(pNewDacl);
	}
	return bSuccess;
}

CString CGDWater::GetDbFromServer(_ConnectionPtr& pCon, CString LocalDB)
{
	//查看SQL服务器，看磁盘目录下的数据库是否已经在服务器中存在
	//如果存在，返回该数据库在服务器中的数据库名
	//如果不存在，返回空字符串
	//LocalDB为磁盘目录下的数据库(全路径)

	CString strSQL, strDBName, strServerDB;
	long nCount, i;
	_variant_t var;
	_RecordsetPtr testRec = NULL;
	try
	{
		testRec.CreateInstance(__uuidof(Recordset));
		//将数据集游标设置在客户端(可提高性能)
		testRec->CursorLocation = adUseClient;
		//设置数据集缓存(1000条)
		testRec->CacheSize = 1000;
		//从系统数据库中获得用户数据库名录数据集
		strSQL = _T("select * From sys.databases where database_id > 6");
		testRec->Open(LPCSTR(strSQL),
			pCon.GetInterfacePtr(),
			adOpenStatic,
			adLockOptimistic,
			adCmdText);
		nCount = 0;
		//确定用户数据库的个数
		if (!testRec->BOF)
		{
			testRec->MoveFirst();
			nCount = testRec->GetRecordCount();
		}
		_RecordsetPtr userdbrec = NULL;
		BOOL isFind = FALSE;
		//逐个用户数据库对其磁盘路径进行对比，查看是否有路径名为LocalDB的数据库
		for (i = 0; i<nCount; i++)
		{
			//先获取用户数据库在SQlSERVER中的命名
			var = testRec->GetCollect(_T("name"));
			strDBName = (LPCSTR)_bstr_t(var);
			//再获取该用户数据库的完整磁盘路径
			strSQL = _T("select filename from master..sysdatabases where name='");
			strSQL = strSQL + strDBName;
			strSQL = strSQL + _T("'");
			userdbrec = pCon->Execute(LPCSTR(strSQL), &var, adCmdText);
			var = userdbrec->GetCollect(_T("filename"));
			strServerDB = (LPCSTR)_bstr_t(var);
			userdbrec->Close();
			//进行完整磁盘路径对比
			if (strServerDB == LocalDB)
			{
				isFind = TRUE;
				break;
			}
			testRec->MoveNext();
		}
		testRec->Close();
		if (isFind)
		{
			//如果有与LocalDB同磁盘路径的数据库,返回数据库名
			return strDBName;
		}
		else
		{
			//否则返回_T("NotFind_T("
			return CString(_T("NotFind"));
		}
	}
	catch (_com_error e)
	{
		printf(LPCTSTR(e.Description()));
		exit(1);
	}
	return CString(_T("NotFind"));
}

void CGDWater::SetDelt(long iStep)
{
	//设置计算时间步长

	double DeltT = 0.0;
	CPressPeriod* pPPerid = NULL;
	pPPerid = m_pPPeriodAry[m_iPer - 1];
	if (fabs(pPPerid->m_MultR - 1.0) > 1e-30 && pPPerid->m_NStep > 1)
	{
		//变时间步长模拟
		if (iStep == 0)
		{
			//第1个时间步长
			DeltT = pPPerid->m_PeridLen * (1.0 - pPPerid->m_MultR) /
				(1.0 - pow(pPPerid->m_MultR, pPPerid->m_NStep));
		}
		else
		{
			//后续时间步长
			DeltT = m_DeltT * pPPerid->m_MultR;
		}
	}
	else
	{
		//定时间步长模拟
		DeltT = pPPerid->m_PeridLen / (double)pPPerid->m_NStep;
	}

	m_iStep = iStep + 1;
	m_DeltT = DeltT;
	//累计从模拟开始到当前的时长
	m_CumTLen = m_CumTLen + m_DeltT;
	//累计从应力期开始到当前的时长
	m_PerCumDeltT = m_PerCumDeltT + m_DeltT;
}

vector<CString> CGDWater::m_ItemNamAry;  //输出项名称数组
vector<int> CGDWater::m_ItemAtriAry;     //输出项性质数组
void CGDWater::SetOutputItem()
{
	//设置数据库输出表头名称

	CString ItemNam;
	long SimType = GDWMOD.GetSimType();  //数值模拟类型
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
	//边界上流量(进)
	for (size_t iBndOrSink = 0; iBndOrSink < CGridCell::m_ActSimBndAry.size(); iBndOrSink++)
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
	//边界上流量(出)
	for (size_t iBndOrSink = 0; iBndOrSink < CGridCell::m_ActSimBndAry.size(); iBndOrSink++)
	{
		ItemNam = CGridCell::m_ActSimBndAry[iBndOrSink].m_BndNamAbb + _T("OUT");
		m_ItemNamAry.push_back(ItemNam);
		m_ItemAtriAry.push_back(1);
	}
	//水量平衡误差输出
	m_ItemNamAry.push_back(_T("ABER"));
	m_ItemAtriAry.push_back(1);
	m_ItemNamAry.push_back(_T("RBER"));
	m_ItemAtriAry.push_back(0);

	//数组不会再增加元素了
	m_ItemNamAry.shrink_to_fit();
	m_ItemAtriAry.shrink_to_fit();
}

void CGDWater::SetModOutPutItem()
{
	//设置模型输出表表头字段

CGridCell::SetOutputItem();
CGridLyr::SetOutputItem();
CRegion::SetOutputItem();
CGDWater::SetOutputItem();
}

void CGDWater::SaveResult()
{
	//保存计算结果

	long k;

	//先保存网格单元水量平衡计算结果
	for (k = 0; k < m_NUMLYR; k++)
	{
		m_pGDLayers[k]->SaveLyrCellResult(m_ICELLBDPRN, m_ICELLFLPRN);
	}

	//保存含水层水量平衡计算结果
	if (m_ILYRBDPRN != 0 || m_IGDWBDPRN != 0)
	{
		//如果需要输出含水层水量平衡模拟结果或地下水系统水量平衡模拟结果
#pragma omp parallel for num_threads(m_NUMTD) schedule(static,1)
		for (long iLyr = 0; iLyr < m_NUMLYR; iLyr++)
		{
			m_pGDLayers[iLyr]->SaveResult(m_ILYRBDPRN);
		}
	}

	//保存各源汇项水量平衡统计结果
	long iBndObj;
	for (iBndObj = 0; iBndObj < m_pSimBndObjAry.size(); iBndObj++)
	{
		m_pSimBndObjAry[iBndObj]->SaveResult();
	}

	//保存统计分区水量平衡统计结果
	if (m_pRegBudget != NULL)
	{
		m_pRegBudget->SaveResult();
	}

	//如果不需要输出地下水系统水量平衡模拟结果, 此处直接返回
	if (m_IGDWBDPRN == 0)
	{
		return;
	}

	//计算地下水系统的水量平衡
	CalBalError();

	//保存地下水系统水量平衡结果
	size_t i = 0;
	//进入地下水系统的水量
	if (m_SIMTYPE == 2)
	{
		//若模拟非稳定流
		//蓄量变化(进)
		assert(m_ItemNamAry[i] == _T("STAIN"));
		m_ResultAry[i].step_val = m_StrgIn;
		i = i + 1;
	}
	//边界上流量(进)
	for (size_t iBndOrSink = 0; iBndOrSink < CGridCell::m_ActSimBndAry.size(); iBndOrSink++)
	{
		assert(m_ItemNamAry[i] == CGridCell::m_ActSimBndAry[iBndOrSink].m_BndNamAbb + _T("IN"));
		m_ResultAry[i].step_val = m_AryGDWSinkIn[iBndOrSink];
		i = i + 1;
	}
	//离开地下水系统的水量
	if (m_SIMTYPE == 2)
	{
		//若模拟非稳定流
		//蓄量变化(出)
		assert(m_ItemNamAry[i] == _T("STAOUT"));
		m_ResultAry[i].step_val = m_StrgOut;
		i = i + 1;
	}
	//边界上流量(出)
	for (size_t iBndOrSink = 0; iBndOrSink < CGridCell::m_ActSimBndAry.size(); iBndOrSink++)
	{
		assert(m_ItemNamAry[i] == CGridCell::m_ActSimBndAry[iBndOrSink].m_BndNamAbb + _T("OUT"));
		m_ResultAry[i].step_val = m_AryGDWSinkOut[iBndOrSink];
		i = i + 1;
	}
	//保存水量平衡误差计算结果
	assert(m_ItemNamAry[i] == _T("ABER"));
	m_ResultAry[i].step_val = m_StepABEr;
	i = i + 1;
	assert(m_ItemNamAry[i] == _T("RBER"));
	if ((m_SumFlowIn + m_SumFlowOut) > 0.0)
	{
		m_ResultAry[i].step_val = fabs((m_SumFlowIn - m_SumFlowOut)) /
			(m_SumFlowIn + m_SumFlowOut) * 100.0;
	}
	else
	{
		m_ResultAry[i].step_val = 0.0;
	}

	//统计累加量结果
	for (i = 0; i < m_ItemNamAry.size(); i++)
	{
		if (m_ItemAtriAry[i] == 0)
		{
			//非可累加计算结果,直接赋值
			m_ResultAry[i].cum_val = m_ResultAry[i].step_val;
		}
		else if (m_ItemAtriAry[i] == 1)
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

void CGDWater::CalBalError()
{
	//计算水量平衡

	//先清零上次的值
	m_StrgIn = 0.0;             //当前计算时段内地下水系统单元蓄量减少(水头下降)释放的水量(L3)
	m_StrgOut = 0.0;            //当前计算时段内地下水系统单元蓄量增加(水头上升)蓄积的水量(L3)
	m_StepABEr = 0.0;           //当前计算时段内的水量平衡误差(L3)

	//清零源汇项统计数组
	size_t iBndOrSink;
	size_t ArySize = CGridCell::m_ActSimBndAry.size();
	for (iBndOrSink = 0; iBndOrSink < ArySize; iBndOrSink++)
	{
		m_AryGDWSinkIn[iBndOrSink] = 0.0;
		m_AryGDWSinkOut[iBndOrSink] = 0.0;
	}

	long k;
	for (k = 0; k<m_NUMLYR; k++)
	{
		m_StrgIn = m_StrgIn + m_pGDLayers[k]->m_StrgIn;
		m_StrgOut = m_StrgOut + m_pGDLayers[k]->m_StrgOut;
		for (iBndOrSink = 0; iBndOrSink < ArySize; iBndOrSink++)
		{
			m_AryGDWSinkIn[iBndOrSink] = m_AryGDWSinkIn[iBndOrSink] + m_pGDLayers[k]->m_AryLyrSinkIn[iBndOrSink];
			m_AryGDWSinkOut[iBndOrSink] = m_AryGDWSinkOut[iBndOrSink] + m_pGDLayers[k]->m_AryLyrSinkOut[iBndOrSink];
		}
	}

	//计算地下水系统水量平衡
	//时段步长内总流入
	double FlowIn = m_StrgIn;
	for (iBndOrSink = 0; iBndOrSink < ArySize; iBndOrSink++)
	{
		FlowIn = FlowIn + m_AryGDWSinkIn[iBndOrSink];
	}
		
	//时段步长内总流出
	double FlowOut = m_StrgOut;
	for (iBndOrSink = 0; iBndOrSink < ArySize; iBndOrSink++)
	{
		FlowOut = FlowOut + m_AryGDWSinkOut[iBndOrSink];
	}
	//时段步长内的水量平衡误差
	m_StepABEr = FlowIn - FlowOut;
	//当前累积的进/出水量
	m_SumFlowIn = m_SumFlowIn + FlowIn;          //应力期内累计总进入含水层系统的水量(L3)
	m_SumFlowOut = m_SumFlowOut + FlowOut;       //应力期内累计总离开含水层系统的水量(L3)
}

void CGDWater::SetCellIniIBound(long iLyr, long iRow, long iCol, long IBNDVal)
{
	//设置第iLyr层第iRow行第ICol列的网格单元的INIIBOUND值
	//注: iLyr, iRow和iCol必须都从1开始编号

	long Node, NodeAbove, LyrCon;
	//获得网格单元
	CGridCell* pGridCell = GetGridCell(iLyr, iRow, iCol);
	//获得网格单元的节点号
	Node = GetCellNode(iLyr, iRow, iCol);
	//修改网格单元的m_INIIBOUND值
	pGridCell->m_INIIBOUND = IBNDVal;
	//修改网格单元和数组的IBOUND值
	pGridCell->m_IBOUND = IBNDVal;
	m_AryIBOUND[Node] = IBNDVal;
	if (IBNDVal == 0)
	{
		//如果是将网格单元的初始m_INIIBOUND设置为无效属性
		//修改网格单元的水头值和数组中该网格单元的水头值
		pGridCell->m_hNew = m_HNOFLO;
		m_AryHNEW[Node] = m_HNOFLO;
		m_AryHOLD[Node] = m_HNOFLO;
		//将该单元的垂向水力传导系数设置为0.0(必须)
		m_AryCV[Node] = 0.0;
		if (m_INTBLKM == 1)
		{
			//如果是BCF数据格式, 同步修改CV备份数组
			m_AryCVWD[Node] = 0.0;
		}
		//将其上方的单元的垂向水力传导系数也设为0, 顶层除外
		if (iLyr != 1)
		{
			NodeAbove = Node - m_NUMRC;
			m_AryCV[NodeAbove] = 0.0;
			if (m_INTBLKM == 1)
			{
				//如果是BCF数据格式, 同步修改CV备份数组
				m_AryCVWD[NodeAbove] = 0.0;
			}
		}
		//将沿行方向的导水系数设为0
		m_AryTR[Node] = 0.0;
		if (m_SIMMTHD == 2 && m_IWDFLG == 1)
		{
			//如果采用原MODFLOW方法模拟网格单元的干-湿转化
			LyrCon = GetLyrCon(iLyr);
			if (LyrCon == 1 || LyrCon == 3)
			{
				//如果是可疏干含水层的单元, 令其不可干湿转换
				pGridCell->m_WETDRY = 0.0;
			}
		}
	}
}

void CGDWater::InitAndCheck()
{
	//进行部分数组元素初始化,并进行检查

	long i, j, k, node, nodeabove, LyrCon;
	CGridCell* pGridCell = NULL;
	long lallzero = 0;

	//初始化有关数组元素
	for (k = 0; k < m_NUMLYR; k++)
	{
		LyrCon = m_pGDLayers[k]->GetLyrCon();
		for (i = 0; i < m_NUMROW; i++)
		{
			for (j = 0; j < m_NUMCOL; j++)
			{
				node = k * m_NUMRC + i * m_NUMCOL + j;
				pGridCell = m_pGDLayers[k]->m_pGridCellAry[i][j];
				//初始化IBOUND
				m_AryIBOUND[node] = pGridCell->m_IBOUND;
				//初始化HNEW和HOLD
				m_AryHNEW[node] = pGridCell->m_hNew;
				m_AryHOLD[node] = pGridCell->m_hNew;
				if (LyrCon == 1 || LyrCon == 3)
				{
					//对于含水层类型1和3
					if (m_AryIBOUND[node] != 0)
					{
						//若单元有效
						if (pGridCell->m_hNew <= pGridCell->m_BOT)
						{
							//若单元的初始水位低于底板
							if (m_SIMMTHD != 1)
							{
								//若不采用全有效单元法, 将单元状态设置为无效状态
								m_AryIBOUND[node] = 0;
								m_AryHNEW[node] = m_HNOFLO;
								m_AryHOLD[node] = pGridCell->m_BOT;
								//修正初始水头值为网格单元底板高程
								pGridCell->m_SHEAD = pGridCell->m_BOT;
							}
						}
					}
				}

				//初始化垂向导水系数CV
				if (k < m_NUMLYR - 1) 
				{
					//如果不是最底层含水层
					if (m_INTBLKM == 1)
					{
						//如果BCF(垂向水力传导系数为输入的固定值)
						m_AryCV[node] = pGridCell->m_VCONT;
						m_AryCV[node] = m_AryCV[node] * m_AryDelR[j] * m_AryDelC[i];
						if (m_SIMMTHD == 2 && m_IWDFLG == 1)
						{
							//如果采用原MODFLOW方法模拟网格单元的干-湿转化
							//初始化CVWD(将垂向导水系数保存起来供单元由疏干转湿润时用)
							m_AryCVWD[node] = m_AryCV[node];
						}
					}
					else
					{
						//如果LPF(垂向水力传导系数为与水头相关变化的值)
						//先按垂向渗透系数设置m_AryCV,后面将重新计算
						if (m_pGDLayers[k]->GetLyrCBD() == 0)
						{
							//含水层不模拟拟三维问题
							m_AryCV[node] = pGridCell->m_VKA;
						}
						else
						{
							//含水层模拟拟三维问题
							m_AryCV[node] = min(pGridCell->m_VKA, pGridCell->m_VKCB);
						}
					}
				}
				//初始化沿行方向(X方向)的导水系数TR
				if (LyrCon == 0 || LyrCon == 2)           
				{
					//仅针对导水系数恒定的含水层
					m_AryTR[node] = pGridCell->m_TRAN;
				}
				//设置无效网格单元的相关数据
				if (m_AryIBOUND[node] == 0)
				{
					//如果是无效单元
					//将垂向水力传导系数设为0
					m_AryCV[node] = 0.0;
					if (m_INTBLKM == 1)
					{
						//如果是BCF数据格式, 同步修改CV备份数组
						m_AryCVWD[node] = 0.0;
					}
					if (k != 0)
					{
						//将其上方的单元的垂向水力传导系数也设为0, 顶层除外
						nodeabove = node - m_NUMRC;
						m_AryCV[nodeabove] = 0.0;
						if (m_INTBLKM == 1)
						{
							//如果是BCF数据格式, 同步修改CV备份数组
							m_AryCVWD[nodeabove] = 0.0;
						}
					}
					//将沿行方向的导水系数设为0
					m_AryTR[node] = 0.0;
					//将其水头值设为m_HNOFLO
					pGridCell->m_hNew = m_HNOFLO;
					m_AryHNEW[node] = m_HNOFLO;
					m_AryHOLD[node] = m_HNOFLO;
				}
			}
		}
	}

	//根据网格单元参数数据,剔除计算中不可能为有效的单元(即使目前单元的INIIBOUND不等于0)
	for (k = 0; k<m_NUMLYR; k++)
	{
		LyrCon = m_pGDLayers[k]->GetLyrCon();
		for (i = 0; i<m_NUMROW; i++)
		{
			for (j = 0; j<m_NUMCOL; j++)
			{
				node = k * m_NUMRC + i * m_NUMCOL + j;
				pGridCell = m_pGDLayers[k]->m_pGridCellAry[i][j];
				if (m_AryIBOUND[node] != 0)
				{
					//如果单元有效，确保单元最少有一个非0的导水系数
					if (LyrCon == 0 || LyrCon == 2)
					{
						//如果是导水系数恒定的含水层
						lallzero = 0;
						if (m_AryTR[node]<1e-30)
						{
							//如果单元沿行方向的导水系数为0
							if (m_NUMLYR == 1)
							{
								//如果含水层仅1层,则单元的垂向导水系数也为0,满足无效条件
								lallzero = 1;
							}
							else
							{
								//如果含水层多于1层
								if (m_AryCV[node]<1e-30 && k>0)
								{
									//如果该单元的垂向导水系数为0,同时不是第1层
									nodeabove = node - m_NUMRC;
									if (m_AryCV[nodeabove] < 1e-30)
									{
										//如果其上的单元的垂向导水系数也为0,满足无效条件
										lallzero = 1;
									}
								}
							}
						}
						if (lallzero == 1)
						{
							//该单元一个非0的导水系数也没有,将其修改为无效单元
							pGridCell->m_INIIBOUND = 0;
							pGridCell->m_IBOUND = 0;
							m_AryIBOUND[node] = 0;
							pGridCell->m_hNew = m_HNOFLO;
							m_AryHNEW[node] = m_HNOFLO;
							printf(_T("编号为%d层%d行%d列的单元经检查不可能为有效单元,模型已将其屏蔽!\n"), k, i, j);
						}
					}
					if (LyrCon == 1 || LyrCon == 3)
					{
						//如果是导水系数变化的含水层,单元的HK或CV必须有一个非0
						long lcvzero = 0;
						long lhyzero = 0;
						//先检查CV
						if (m_AryCV[node] < 1e-30) 
						{
							//如果单元的垂向导水系数为0
							if (m_NUMLYR == 1) 
							{ 
								//如果含水层仅1层,垂向上流量永远为0
								lcvzero = 1;
							}
							else  
							{
								//如果含水层为多层
								if (k > 0) 
								{
									//如果不是第1层
									nodeabove = node - m_NUMRC;
									if (m_AryCV[nodeabove] < 1e-30)
									{
										//如果其上单元的CV为0,垂向上流量永远为0
										lcvzero = 1;
									}
								}
							}
						}
						//再检查CC
						if (pGridCell->m_HK < 1e-30)  
						{
							//如果沿行方向饱和渗透系数为0，平面方向上流量永远为0
							lhyzero = 1;
						}
						//根据判断结果进行处理
						if (lcvzero == 1 && lhyzero == 1)
						{
							//该单元一个非0的导水系数也没有,将其修改为无效单元
							pGridCell->m_INIIBOUND = 0;
							pGridCell->m_IBOUND = 0;
							m_AryIBOUND[node] = 0;
							pGridCell->m_hNew = m_HNOFLO;
							m_AryHNEW[node] = m_HNOFLO;
							if (m_SIMMTHD == 2 && m_IWDFLG == 1)
							{
								//如果采用原MODFLOW方法模拟网格单元的干-湿转化, 令其不可转换
								pGridCell->m_WETDRY = 0.0;
							}
							printf(_T("编号为%d层%d行%d列的单元经检查不可能为有效单元,模型已将其屏蔽!\n"), k, i, j);
						}
					}
				}
			}
		}
	}

	//如果采用原MODFLOW的试算法模拟网格单元的干-湿转化, 设置网格单元的WETFCT参数
	if (m_SIMMTHD == 2 && m_IWDFLG == 1)
	{
		for (k = 0; k < m_NUMLYR; k++)
		{
			LyrCon = m_pGDLayers[k]->GetLyrCon();
			if (LyrCon == 1 || LyrCon == 3)
			{
				//仅对含水层类型1或3进行操作
				for (i = 0; i < m_NUMROW; i++)
				{
					for (j = 0; j < m_NUMCOL; j++)
					{
						pGridCell = m_pGDLayers[k]->m_pGridCellAry[i][j];
						if (pGridCell->m_INIIBOUND > 0 && fabs(pGridCell->m_WETDRY) > 1e-30)
						{
							//如果网格单元初始有效且可干湿转化
							if (m_WETFCT != -1)
							{
								//如果用户自己设置WETFCT参数
								pGridCell->m_CellWETFCT = m_WETFCT;
							}
							else
							{
								//用户让模型自动确定WETFCT参数
								//设置网格单元m_CellWFACT参数与m_WETDRY参数的乘积与2倍m_HCLOSE值相当
								pGridCell->m_CellWETFCT = 2.0 * m_HCLOSE / fabs(pGridCell->m_WETDRY);
							}
						}
					}
				}
			}
		}
	}

	//如果采用全有效单元法, 计算网格单元间的平均渗透系数
	CGridCell* pCellRight = NULL;
	CGridCell* pCellFront = NULL;
	BOOL bCANIDis;
	double YX = 0.0;
	if (m_SIMMTHD == 1)
	{
		//如果采用全有效单元法
		for (k = 0; k < m_NUMLYR; k++)
		{
			LyrCon = m_pGDLayers[k]->GetLyrCon();
			if (m_INTBLKM == 2 || (m_INTBLKM == 1 && (LyrCon == 1 || LyrCon == 3)))
			{
				//如果是LPF, 或者是BCF且含水层类型为1或3
				bCANIDis = TRUE;        
				if (m_INTBLKM == 1 || (m_INTBLKM == 2 && m_pGDLayers[k]->m_LYRTRPY > 0.0))
				{
					//如果是BCF，或者是LPF但是含水层沿列方向的渗透系数异性统一
					//则含水层沿列方向的渗透系数异性不是分布式
					bCANIDis = FALSE;
				}
				if (bCANIDis == FALSE)
				{
					//如果含水层沿列方向的渗透系数异性不是分布式, 取得含水层上Ky/Kx的值
					YX = m_pGDLayers[k]->m_LYRTRPY;
				}
				for (i = 0; i < m_NUMROW; i++)
				{
					for (j = 0; j < m_NUMCOL; j++)
					{
						node = k * m_NUMRC + i * m_NUMCOL + j;
						pGridCell = m_pGDLayers[k]->m_pGridCellAry[i][j];
						if (pGridCell->m_INIIBOUND != 0)
						{
							//如果网格单元初始有效
							//先标识不可疏干单元
							if (pGridCell->m_INIIBOUND > 0 && (LyrCon == 1 || LyrCon == 3))
							{
								//如果该单元为变水头单元且含水层类型为1或3
								if (k == m_NUMLYR - 1 || (k != m_NUMLYR - 1 && GetGridCell(k + 1, i + 1, j + 1)->m_INIIBOUND == 0))
								{
									//如果该单元是底层单元, 或该单元下方为无效单元, 该单元不可疏干
									pGridCell->m_DryHint = 1;
									//检查不可疏干单元水头
									if (pGridCell->m_hNew <= pGridCell->m_BOT)
									{
										//若单元的初始水位低于底板, 设置初始水位为底板处加一个极小值
										m_AryHNEW[node] = pGridCell->m_BOT + m_MinHSat;
										m_AryHOLD[node] = pGridCell->m_BOT + m_MinHSat;
										//修正初始水头值为网格单元底板高程加极小值
										pGridCell->m_SHEAD = pGridCell->m_BOT + m_MinHSat;
									}
									m_pCantDryCellAry.push_back(pGridCell);
								}
							}
							if (j != m_NUMCOL - 1)
							{
								//如果不是最后1列
								pCellRight = m_pGDLayers[k]->m_pGridCellAry[i][j + 1];
								if (pCellRight->m_INIIBOUND != 0)
								{
									//如果右侧单元有效
									//计算沿行方向上的单元间平均渗透系数
									m_pGDLayers[k]->m_AryKav_R[i][j] = ((m_AryDelR[j] + m_AryDelR[j + 1]) * pGridCell->m_HK *
										pCellRight->m_HK) / (m_AryDelR[j + 1] * pGridCell->m_HK + m_AryDelR[j] *
											pCellRight->m_HK);
								}
							}
							if (i != m_NUMROW - 1)
							{
								//如果不是最后1行
								pCellFront = m_pGDLayers[k]->m_pGridCellAry[i + 1][j];
								if (pCellFront->m_INIIBOUND != 0)
								{
									//如果前侧单元有效
									//计算沿列方向上的单元间平均渗透系数
									if (bCANIDis == FALSE)
									{
										//含水层沿列方向的渗透系数异性不是分布式
										m_pGDLayers[k]->m_AryKav_C[i][j] = YX * ((m_AryDelC[i] + m_AryDelC[i + 1]) * 
											pGridCell->m_HK * pCellFront->m_HK) / (m_AryDelC[i + 1] * pGridCell->m_HK + 
												m_AryDelC[i] * pCellFront->m_HK);
									}
									else
									{
										//含水层沿列方向的渗透系数异性是分布式
										m_pGDLayers[k]->m_AryKav_C[i][j] = ((m_AryDelC[i] + m_AryDelC[i + 1]) *
											 pGridCell->m_HK * pGridCell->m_HANI * pCellFront->m_HK * pCellFront->m_HANI) / 
											(m_AryDelC[i + 1] * pGridCell->m_HK * pGridCell->m_HANI + 
												m_AryDelC[i] * pCellFront->m_HK * pCellFront->m_HANI);
									}
								}
							}
						}
					}
				}
			}
		}
		//不可疏干单元的数量不会再增加了
		m_pCantDryCellAry.shrink_to_fit();
	}
	
	//计算导水系数不变变含水层的水平向水力传导系数(仍放在CC和CR数组中)
	long iLyr;
	for (iLyr = 1; iLyr <= m_NUMLYR; iLyr++)
	{
		CalCnstHoriCond(iLyr);
	}

	//如果LPF,计算某些含水层的垂向水力传导系数
	if (m_INTBLKM == 2)
	{
		//计算上下两层都是承压含水层时它们之间的垂向水力传导系数
		for (iLyr = 1; iLyr <= m_NUMLYR; iLyr++)
		{
			CalCnstVertCond(iLyr);
		}
	}

	//计算各单元的第一类和第二类贮水量
	for (k = 0; k<m_NUMLYR; k++)
	{
		LyrCon = m_pGDLayers[k]->GetLyrCon();
		for (i = 0; i<m_NUMROW; i++)
		{
			for (j = 0; j<m_NUMCOL; j++)
			{
				node = k * m_NUMRC + i * m_NUMCOL + j;
				pGridCell = m_pGDLayers[k]->m_pGridCellAry[i][j];
				//第一类贮水能力
				if (m_INTBLKM == 1)
				{
					//若BCF, 用户输入的是潜水的给水度, 或含水层处于承压状态时的贮水系数
					m_ArySC1[node] = pGridCell->m_SC1 * m_AryDelR[j] * m_AryDelC[i];
				}
				else
				{
					//若LPF, 用户输入的是含水层承压状态下的贮水率, 需再乘以含水层厚度
					m_ArySC1[node] = pGridCell->m_SC1 * m_AryDelR[j] * m_AryDelC[i] *
						(pGridCell->m_TOP - pGridCell->m_BOT);
				}
				//计算第二类贮水能力
				if (LyrCon == 2 || LyrCon == 3)   
				{
					//如果是承压-非承压可转化含水层, 用户输入数据是非承压状态时的给水度
					m_ArySC2[node] = pGridCell->m_SC2 * m_AryDelR[j] * m_AryDelC[i];
				}
			}
		}
	}
}

void CGDWater::UpHoriTRANS(long LyrID, long KITER)
{
	//更新水力传导系数可变含水层(1,3)沿行方向的导水系数, 非全有效单元法时调用
	//iLyr: 含水层的层号, 必须从1开始编号
	//KITER: 当前迭代次数(从1开始)

	if (m_SIMMTHD == 1)
	{
		//如果采用全有效单元法, 无需更新含水层的导水系数, 直接返回
		return;
	}

	long LyrCon = GetGridLyr(LyrID)->GetLyrCon();
	if (LyrCon == 0 || LyrCon == 2)
	{
		//如果是导水系数不变的含水层直接返回
		return;
	}

	//如果采用试算法模拟网格单元的疏干-湿润, 计算间隔标识符ITFLG(等于0时表示到了间隔次数)
	long ITFLG = 1;
	if (m_IWDFLG == 1)
	{
		//每间隔m_NWETIT个迭代尝试湿润被疏干的网格单元
		ITFLG = KITER % (m_NWETIT + 1);
	}

	//尝试重新湿润被疏干的网格单元
	long NumTask = long(m_NUMRC / m_NUMTD / 2.0) + 1;
	if (m_IWDFLG == 1 && ITFLG == 0)
	{
		//如果采用试算法模拟网格单元的干-湿转换且到了间隔次数
#pragma omp parallel for num_threads(m_NUMTD) schedule(dynamic,NumTask)
		for (long iCell = 0; iCell < m_NUMRC; iCell++)
		{
			//注释: 以下几个变量的定义必须放在for块中不能共用, 以适应并行运算要求
			long i, j, node;
			CGridCell* pGridCell = NULL;
			CGridCell* pGridCellLeft = NULL;
			CGridCell* pGridCellRight = NULL;
			CGridCell* pGridCellFront = NULL;
			CGridCell* pGridCellBack = NULL;
			double TURNON, hTemp;
			long lMatch, nbIBD;
			long nodeabove, nodedown, nodeleft, noderight, nodefront, nodeback;
			//注释完毕
			//先确定网格单元在数组中的行号和列号
			i = iCell / m_NUMCOL;
			j = iCell - i * m_NUMCOL;
			//逐个疏干单元进行重新湿润尝试
			node = (LyrID - 1) * m_NUMRC + i * m_NUMCOL + j;
			pGridCell = m_pGDLayers[LyrID - 1]->m_pGridCellAry[i][j];
			if (pGridCell->m_INIIBOUND > 0 && m_AryIBOUND[node] == 0 && fabs(pGridCell->m_WETDRY) > 1e-30)
			{
				//如果网格单元原始属性为变水头单元，目前处于疏干状态(无效), 且不是不可湿润的
				//计算网格单元被重新湿润时的水头阈值, 该阈值为本网格单元的底板高程加上m_WETDRY参数的绝对值
				TURNON = pGridCell->m_BOT + fabs(pGridCell->m_WETDRY);
				//设置湿润标识初值(0表示未能重新湿润; 1表示成功重新湿润)
				lMatch = 0;
				//先看看下方的网格单元有无可能湿润该单元
				if (LyrID != m_NUMLYR)
				{
					//如果不是最底层含水层
					nodedown = node + m_NUMRC;
					nbIBD = m_AryIBOUND[nodedown];
					if (nbIBD != 0)
					{
						//如果下方网格单元有效
						hTemp = m_AryHNEW[nodedown];
						if (hTemp >= TURNON)
						{
							//如果水头满足阈值条件, 点亮重新湿润标识
							lMatch = 1;
						}
					}
				}
				//如下方网格单元不能湿润该单元,再检查同层邻近的四个网格单元有无可能湿润该单元						
				if (lMatch == 0 && pGridCell->m_WETDRY > 1e-30)
				{
					//仅在单元的m_WETDRY参数大于0(即相邻任何单元均可驱动湿润)时进行以下操作
					if (j != 0)
					{
						//如果不是第1列, 测试左侧网格单元
						nodeleft = node - 1;
						nbIBD = m_AryIBOUND[nodeleft];
						if (nbIBD != 0)
						{
							//如果左侧网格单元有效
							hTemp = m_AryHNEW[nodeleft];
							pGridCellLeft = m_pGDLayers[LyrID - 1]->m_pGridCellAry[i][j - 1];
							if (hTemp >= TURNON && hTemp > pGridCellLeft->m_BOT)
							{
								//如果水头满足阈值条件, 同时左侧网格单元水头值高于自身底板, 点亮重新湿润标识
								lMatch = 1;
							}
						}
					}
					if (lMatch == 0 && j != m_NUMCOL - 1)
					{
						//若未成功且不是最后1列，测试右侧网格单元
						noderight = node + 1;
						nbIBD = m_AryIBOUND[noderight];
						if (nbIBD != 0)
						{
							//如果右侧网格单元有效
							hTemp = m_AryHNEW[noderight];
							pGridCellRight = m_pGDLayers[LyrID - 1]->m_pGridCellAry[i][j + 1];
							if (hTemp >= TURNON && hTemp > pGridCellRight->m_BOT)
							{
								//如果水头满足阈值条件, 同时右侧网格单元水头值高于自身底板, 点亮重新湿润标识
								lMatch = 1;
							}
						}
					}
					if (lMatch == 0 && i != m_NUMROW - 1)
					{
						//若未成功且不是第最后1行，测试前侧网格单元
						nodefront = node + m_NUMCOL;
						nbIBD = m_AryIBOUND[nodefront];
						if (nbIBD != 0)
						{
							//如果前侧网格单元有效
							hTemp = m_AryHNEW[nodefront];
							pGridCellFront = m_pGDLayers[LyrID - 1]->m_pGridCellAry[i + 1][j];
							if (hTemp >= TURNON && hTemp > pGridCellFront->m_BOT)
							{
								//如果水头满足阈值条件, 同时前侧网格单元水头值高于自身底板, 点亮重新湿润标识
								lMatch = 1;
							}
						}
					}
					if (lMatch == 0 && i != 0)
					{
						//若未成功且不是第1行，测试后侧网格单元
						nodeback = node - m_NUMCOL;
						nbIBD = m_AryIBOUND[nodeback];
						if (nbIBD != 0)
						{
							//如果后侧网格单元有效
							hTemp = m_AryHNEW[nodeback];
							pGridCellBack = m_pGDLayers[LyrID - 1]->m_pGridCellAry[i - 1][j];
							if (hTemp >= TURNON && hTemp > pGridCellBack->m_BOT)
							{
								//如果水头满足阈值条件, 同时后侧网格单元水头值高于自身底板, 点亮重新湿润标识
								lMatch = 1;
							}
						}
					}
				}
				//重新湿润尝试完成
				if (lMatch == 1)
				{
					//若该网格单元被成功湿润, 将网格单元被重新湿润的标识符置为1
					m_AryWetFlag[node] = 1;
					//设置被重新湿润的网格单元的水头初值
					if (m_IHDWET == 2)
					{
						//采用本网格单元的重新湿润水层厚度阈值计算本网格单元的重新湿润水头
						m_AryHNEW[node] = pGridCell->m_BOT + pGridCell->m_CellWETFCT * fabs(pGridCell->m_WETDRY);
					}
					else
					{
						//采用湿润本网格单元的相邻网格单元的水头计算本网格单元的重新湿润水头
						m_AryHNEW[node] = pGridCell->m_BOT + pGridCell->m_CellWETFCT * (hTemp - pGridCell->m_BOT);
					}
					//恢复垂向水力传导系数(仅对BCF, LPF将会重新计算垂向水力传导系数)
					if (m_INTBLKM == 1)
					{
						if (LyrID != m_NUMLYR)
						{
							//如果含水层不是最底层
							nodedown = node + m_NUMRC;
							if (m_AryIBOUND[nodedown] != 0)
							{
								//如果下方网格单元为有效单元, 恢复本网格单元与下方网格单元的水力联系
								m_AryCV[node] = m_AryCVWD[node];
							}
						}
						if (LyrID != 1)
						{
							//如果不是第一层
							nodeabove = node - m_NUMRC;
							if (m_AryIBOUND[nodeabove] != 0)
							{
								//如果上方网格单元为有效单元, 恢复上方网格单元与本网格单元的水力联系
								m_AryCV[nodeabove] = m_AryCVWD[nodeabove];
							}
						}
					}
				}
			}
		}
	}

	//根据单元被重新湿润的标识符修改m_AryIBOUND数组
	if (m_IWDFLG == 1 && ITFLG == 0)
	{
		//如果采用试算法模拟网格单元的干-湿转换且到了间隔次数
#pragma omp parallel for num_threads(m_NUMTD) schedule(dynamic,NumTask)
		for (long iCell = 0; iCell < m_NUMRC; iCell++)
		{
			//注释: 以下几个变量的定义必须放在for块中不能共用, 以适应并行运算要求
			long i, j, node;
			//注释完毕
			//先确定网格单元在数组中的行号和列号
			i = iCell / m_NUMCOL;
			j = iCell - i * m_NUMCOL;
			node = (LyrID - 1) * m_NUMRC + i * m_NUMCOL + j;
			if (m_AryWetFlag[node] == 1 && m_AryIBOUND[node] == 0)
			{
				//如果疏干单元被重新湿润的标识符为1
				//恢复网格单元有效性标识
				m_AryIBOUND[node] = 1;
				//重置被重新湿润的标识符
				m_AryWetFlag[node] = 0;
			}
		}
	}

	//更新网格单元沿行方向(X方向)的导水系数T
#pragma omp parallel for num_threads(m_NUMTD) schedule(dynamic,NumTask)
	for (long iCell = 0; iCell < m_NUMRC; iCell++)
	{
		//注释: 以下几个变量的定义必须放在for块中不能共用, 以适应并行运算要求
		long i, j, node, nodeabove;
		double HD, BBOT, TTOP, TICK;
		CGridCell* pGridCell = NULL;
		//注释完毕
		//先确定网格单元在数组中的行号和列号
		i = iCell / m_NUMCOL;
		j = iCell - i * m_NUMCOL;
		//更新网格单元沿行方向的导水系数
		node = (LyrID - 1) * m_NUMRC + i * m_NUMCOL + j;
		pGridCell = m_pGDLayers[LyrID - 1]->m_pGridCellAry[i][j];
		if (m_AryIBOUND[node] != 0)
		{
			//如果网格单元有效
			HD = m_AryHNEW[node];
			BBOT = pGridCell->m_BOT;
			if (LyrCon == 3)
			{
				//如果是导水系数可变的承压/非承压含水层, 网格单元的水层厚度不可能超过顶界
				TTOP = pGridCell->m_TOP;
				if (HD > TTOP)
				{
					HD = TTOP;
				}
			}
			//计算单元的水层厚度
			TICK = HD - BBOT;
			//计算单元沿行方向(X方向)的导水系数T(放在m_AryTR数组中)
			if (TICK > 0.0)
			{
				//如果饱和厚度大于零
				m_AryTR[node] = TICK * pGridCell->m_HK;
			}
			else
			{
				//如果饱和厚度小于等于0, 说明该网格单元被疏干了
				if (m_AryIBOUND[node] < 0)
				{
					//如果是定常水头网格单元, 不可能被疏干
					printf(_T("\n"));
					printf(_T("定常水头单元被疏干,模拟不正常退出!\n"));
					printf(_T("该单元编号为%d层%d行%d列,请检查!\n"), LyrID, i + 1, j + 1);
					PauAndQuit();
				}
				//将该网格单元标示为无效单元
				m_AryIBOUND[node] = 0;
				//将水头设为无效水头的水头值
				m_AryHNEW[node] = m_HNOFLO;
				//将沿行方向(X方向)导水系数设为0;
				m_AryTR[node] = 0.0;
				//设置垂向水力传导系数为0(仅对BCF, LPF将会重新计算垂向水力传导系数)
				if (m_INTBLKM == 1)
				{
					if (LyrID != m_NUMLYR)
					{
						//如果不是最底层, 断开与下层网格单元的垂向水力联系
						m_AryCV[node] = 0.0;
					}
					if (LyrID != 1)
					{
						//如果不是顶层, 断开与上层网格单元的垂向水力联系
						nodeabove = node - m_NUMRC;
						m_AryCV[nodeabove] = 0.0;
					}
				}
			}
		}
	}
}

void CGDWater::CalCnstVertCond(long lyrID)
{
	//仅LPF, 对编号为lyrID的含水层计算固定的垂向水力传导系数CV

	//该函数仅针对LPF方式
	if (m_INTBLKM != 2)
	{
		return;
	}

	//如果是最底层含水层直接返回
	if (lyrID == m_NUMLYR)
	{
		return;
	}

	long LyrCon = GetGridLyr(lyrID)->GetLyrCon();
	long LyrConDown = GetGridLyr(lyrID + 1)->GetLyrCon();
	//如果本层和下层含水层中至少有一个是含水层类型3时直接返回
	if (LyrCon == 3 || LyrConDown == 3)
	{
		return;
	}

	long LyrCBD = GetGridLyr(lyrID)->GetLyrCBD();
	//含水层内逐网格单元进行循环
	long NumTask = long(m_NUMRC / m_NUMTD / 2.0) + 1;
#pragma omp parallel for num_threads(m_NUMTD) schedule(dynamic,NumTask)
	for (long iCell = 0; iCell < m_NUMRC; iCell++)
	{
		//注释: 以下几个变量的定义必须放在for块中不能共用, 以适应并行运算要求
		long i, j;
		long node_k, node_k1;
		double thik_k, thik_k1;
		double BLeak_k, BLeak_k1, BLeak_kb;
		double BLeakAll;
		CGridCell* pCell_k = NULL;
		CGridCell* pCell_k1 = NULL;
		//注释完毕
		//先确定网格单元在数组中的行号和列号
		i = iCell / m_NUMCOL;
		j = iCell - i * m_NUMCOL;
		//计算网格单元的垂向水力传导系数
		node_k = (lyrID - 1) * m_NUMRC + i * m_NUMCOL + j;
		node_k1 = lyrID * m_NUMRC + i * m_NUMCOL + j;
		BLeak_k = 0.0;               //本单元自身的越流系数的倒数
		BLeak_k1 = 0.0;              //本单元下层单元越流系数的倒数
		BLeak_kb = 0.0;              //本单元与下层单元间低渗透介质越流系数的倒数
		thik_k = 0.0;                //上层单元饱和厚度
		thik_k1 = 0.0;               //下层单元饱和厚度
		m_AryCV[node_k] = 0.0;       //将垂向水力传导系数先置零
		if (m_AryIBOUND[node_k] != 0 && m_AryIBOUND[node_k1] != 0)
		{
			//如果本层单元和下层单元都是有效单元
			pCell_k = m_pGDLayers[lyrID - 1]->m_pGridCellAry[i][j];
			pCell_k1 = m_pGDLayers[lyrID]->m_pGridCellAry[i][j];
			//两个单元的垂向饱和渗透系数必定都大于0.0
			assert(pCell_k->m_VKA > 1e-30 && pCell_k1->m_VKA > 1e-30);
			//1. 计算本层单元自身的越流系数的倒数
			thik_k = pCell_k->m_TOP - pCell_k->m_BOT;
			BLeak_k = 0.5 * thik_k / pCell_k->m_VKA;
			//2. 计算本层单元下层单元越流系数的倒数
			thik_k1 = pCell_k1->m_TOP - pCell_k1->m_BOT;
			BLeak_k1 = 0.5 * thik_k1 / pCell_k1->m_VKA;
			//3. 计算本单元与下层单元间低渗透介质越流系数的倒数
			if (LyrCBD == 1)
			{
				//如果本层进行地下水拟三维模拟
				if (pCell_k->m_VKCB > 0.0)
				{
					BLeak_kb = pCell_k->m_TKCB / pCell_k->m_VKCB;
				}
			}
			//计算综合垂向水力传导系数
			BLeakAll = BLeak_k + BLeak_k1 + BLeak_kb;
			if (BLeakAll > 0.0)
			{
				m_AryCV[node_k] = pCell_k->m_CellArea / BLeakAll;
			}
		}
	}
}

void CGDWater::CalVaryVertCond(long lyrID)
{
	//仅LPF, 对编号为lyrID的含水层计算变化的垂向水力传导系数CV

	//该函数仅针对LPF方式
	if (m_INTBLKM != 2)
	{
		return;
	}

	//如果是最底层含水层直接返回
	if (lyrID == m_NUMLYR)
	{
		return;
	}

	long LyrCon = GetGridLyr(lyrID)->GetLyrCon();
	long LyrConDown = GetGridLyr(lyrID + 1)->GetLyrCon();
	if (LyrCon == 0 && LyrConDown == 0)
	{
		//如果本层和下层含水层都是含水层类型0时直接返回
		return;
	}

	long LyrCBD = GetGridLyr(lyrID)->GetLyrCBD();
	//含水层内逐网格单元进行循环
	long NumTask = long(m_NUMRC / m_NUMTD / 2.0) + 1;
#pragma omp parallel for num_threads(m_NUMTD) schedule(dynamic,NumTask)
	for (long iCell = 0; iCell < m_NUMRC; iCell++)
	{
		//注释: 以下几个变量的定义必须放在for块中不能共用, 以适应并行运算要求
		long i, j;
		long node_k, node_k1;
		double thik_k, thik_k1;
		double BLeak_k, BLeak_k1, BLeak_kb;
		double BLeakAll;
		CGridCell* pCell_k = NULL;
		CGridCell* pCell_k1 = NULL;
		//注释完毕
		//先确定网格单元在数组中的行号和列号
		i = iCell / m_NUMCOL;
		j = iCell - i * m_NUMCOL;
		//计算网格单元的垂向水力传导系数
		node_k = (lyrID - 1) * m_NUMRC + i * m_NUMCOL + j;
		node_k1 = lyrID * m_NUMRC + i * m_NUMCOL + j;
		BLeak_k = 0.0;               //本单元自身的越流系数的倒数
		BLeak_k1 = 0.0;              //本单元下层单元越流系数的倒数
		BLeak_kb = 0.0;              //本单元与下层单元间低渗透介质越流系数的倒数
		thik_k = 0.0;                //上层单元饱和厚度
		thik_k1 = 0.0;               //下层单元饱和厚度
		m_AryCV[node_k] = 0.0;       //将垂向水力传导系数先置零
		if (m_AryIBOUND[node_k] != 0 && m_AryIBOUND[node_k1] != 0)
		{
			//如果本层单元和下层单元都是有效单元
			pCell_k = m_pGDLayers[lyrID - 1]->m_pGridCellAry[i][j];
			pCell_k1 = m_pGDLayers[lyrID]->m_pGridCellAry[i][j];
			//两个单元的垂向饱和渗透系数必定都大于0.0
			assert(pCell_k->m_VKA > 1e-30 && pCell_k1->m_VKA > 1e-30);
			//1. 计算本层单元自身的越流系数的倒数
			if (LyrCon == 3)
			{
				//如果本层为承压-非承压可变含水层
				thik_k = min(m_AryHNEW[node_k], pCell_k->m_TOP) - pCell_k->m_BOT;
				thik_k = max(thik_k, 0.0);
			}
			else
			{
				//如果本层为纯承压含水层
				thik_k = pCell_k->m_TOP - pCell_k->m_BOT;
			}
			BLeak_k = 0.5 * thik_k / pCell_k->m_VKA;
			//2. 计算本层单元下层单元越流系数的倒数
			if (LyrConDown == 3)
			{
				//如果下层为承压-非承压可变含水层
				//注: Mod2K的算法认为如果下层单元如果处于非承压状态
				//下层单元的含水层不考虑对越流系数的计算作用, 会导致越流系数计算时不连续
				//有时或特殊情况下会出现大的水量平衡误差或计算不稳定
				//尤其是在启用干-湿转化计算功能时
				//注: 此代码为MOD2K和MOD2005的计算方法					
				/*
				if (m_AryHNEW[node_k1] < pCell_k1->m_TOP)
				{
				    thik_k1 = 0.0;
				}
				else
				{
				    thik_k1 = pCell_k1->m_TOP - pCell_k1->m_BOT;
				}
				*/
				//注: 以下代码为可替代的计算方法(更接近BCF的算法)
				//即认为只要上、下层单元间有越流，下层单元处含水层
				//始终对越流系数的计算有作用
				thik_k1 = pCell_k1->m_TOP - pCell_k1->m_BOT;
			}
			else
			{
				//如果下层为纯承压含水层
				thik_k1 = pCell_k1->m_TOP - pCell_k1->m_BOT;
			}
			BLeak_k1 = 0.5 * thik_k1 / pCell_k1->m_VKA;
			//3. 计算本单元与下层单元间低渗透介质越流系数的倒数
			if (LyrCBD == 1)
			{
				//如果本层进行地下水拟三维模拟
				if (pCell_k->m_VKCB > 0.0)
				{
					BLeak_kb = pCell_k->m_TKCB / pCell_k->m_VKCB;
				}
			}
			//计算综合垂向水力传导系数
			BLeakAll = BLeak_k + BLeak_k1 + BLeak_kb;
			if (BLeakAll > 0.0)
			{
				m_AryCV[node_k] = pCell_k->m_CellArea / BLeakAll;
			}
		}
	}
}

void CGDWater::CalCnstHoriCond(long lyrID)
{
	//对导水系数不变含水层计算水平向水力传导系数
	//CR为沿行方向(X方向)水力传导系数, CC为沿列方向(Y方向)水力传导系数

	long LyrCon = GetGridLyr(lyrID)->GetLyrCon();
	if (LyrCon == 1 || LyrCon == 3)
	{
		//如果本含水层是导水系数可变含水层, 直接返回
		return;
	}

	BOOL bCANIDis = TRUE;        //含水层沿列方向的渗透系数异性是否是分布式
	if (m_INTBLKM == 1 || (m_INTBLKM == 2 && GetGridLyr(lyrID)->m_LYRTRPY > 0.0))
	{
		//如果是BCF，或者是LPF但是含水层沿列方向的渗透系数异性统一
		//则含水层沿列方向的渗透系数异性不是分布式
		bCANIDis = FALSE;
	}

	//计算含水层各网格单元间的水力传导系数
	double YX = 0.0;
	if (bCANIDis == FALSE)
	{
		//如果含水层沿列方向的渗透系数异性不是分布式
		//先计算沿列方向的渗透系数因子
		YX = GetGridLyr(lyrID)->GetLyrTRPY() * 2.0;
	}

	//含水层内逐网格单元计算
	long NumTask = long(m_NUMRC / m_NUMTD / 2.0) + 1;
	if (m_SIMMTHD == 1 && m_INTBLKM == 2)
	{
		//如果模拟方法为全有效单元法且采用LPF格式
#pragma omp parallel for num_threads(m_NUMTD) schedule(dynamic,NumTask)
		for (long iCell = 0; iCell < m_NUMRC; iCell++)
		{
			//注释: 以下几个变量的定义必须放在for块中不能共用, 以适应并行运算要求
			long i, j, nodeself, noderight, nodefront;
			CGridCell* pCellSelf = NULL;
			CGridCell* pCellRight = NULL;
			CGridCell* pCellFront = NULL;
			double Kave_R, Kave_C, HThikUp, HThikLow, HThikAve, DeltLenUp,
				DeltLenLow;
			//注释完毕
			//先确定网格单元在数组中的行号和列号
			i = iCell / m_NUMCOL;
			j = iCell - i * m_NUMCOL;
			//计算网格单元的水平向水力传导系数
			nodeself = (lyrID - 1) * m_NUMRC + i * m_NUMCOL + j;
			pCellSelf = m_pGDLayers[lyrID - 1]->m_pGridCellAry[i][j];
			if (m_AryIBOUND[nodeself] == 0 || pCellSelf->m_HK < 1e-30)
			{
				//如果本网格单元无效或K值为0, CC和CR都为0
				m_AryCR[nodeself] = 0.0;
				m_AryCC[nodeself] = 0.0;
			}
			else
			{
				//如果本网格单元有效且K值大于0, 计算CR和CC
				//1. 如果本网格单元不是最后一列,计算沿行方向上水力传导系数
				if (j != m_NUMCOL - 1)
				{
					noderight = nodeself + 1;
					if (m_AryIBOUND[noderight] != 0)
					{
						//如果右侧网格单元为有效单元, 计算确定CR
						pCellRight = m_pGDLayers[lyrID - 1]->m_pGridCellAry[i][j + 1];
						Kave_R = m_pGDLayers[lyrID - 1]->m_AryKav_R[i][j];
						DeltLenUp = m_AryDelR[j];
						DeltLenLow = m_AryDelR[j + 1];
						HThikUp = max(0.0, pCellSelf->m_TOP - pCellSelf->m_BOT);
						HThikLow = max(0.0, pCellRight->m_TOP - pCellRight->m_BOT);
						//计算两个网格单元之间的平均过水断面厚度
						HThikAve = (HThikUp * DeltLenLow + HThikLow * DeltLenUp) /
							(DeltLenUp + DeltLenLow);
						//计算两个网格单元之间的综合水力传导系数
						m_AryCR[nodeself] = 2.0 * m_AryDelC[i] * Kave_R * HThikAve /
							(m_AryDelR[j] + m_AryDelR[j + 1]);
					}
					else
					{
						//如果右侧网格单元为无效单元, CR=0
						m_AryCR[nodeself] = 0.0;
					}
				}
				else
				{
					//本网格单元为最后一列, CR=0
					m_AryCR[nodeself] = 0.0;
				}
				//2. 如果本网格单元不是最后一行,计算沿列方向的水力传导系数
				if (i != m_NUMROW - 1)
				{
					nodefront = nodeself + m_NUMCOL;
					if (m_AryIBOUND[nodefront] != 0)
					{
						//如果前侧网格单元有效, 计算确定CC
						pCellFront = m_pGDLayers[lyrID - 1]->m_pGridCellAry[i + 1][j];
						Kave_C = m_pGDLayers[lyrID - 1]->m_AryKav_C[i][j];
						DeltLenUp = m_AryDelC[i];
						DeltLenLow = m_AryDelC[i + 1];
						HThikUp = max(0.0, pCellSelf->m_TOP - pCellSelf->m_BOT);
						HThikLow = max(0.0, pCellFront->m_TOP - pCellFront->m_BOT);
						//计算两个网格单元之间的平均饱和厚度
						HThikAve = (HThikUp * DeltLenLow + HThikLow * DeltLenUp) /
							(DeltLenUp + DeltLenLow);
						//计算两个网格单元之间的水力传导系数
						m_AryCC[nodeself] = 2.0 * m_AryDelR[j] * Kave_C * HThikAve /
							(m_AryDelC[i] + m_AryDelC[i + 1]);
					}
					else
					{
						//如果前网格单元为无效单元, CC=0
						m_AryCC[nodeself] = 0.0;
					}
				}
				else
				{
					//本网格单元为最后一行, CC=0
					m_AryCC[nodeself] = 0.0;
				}
			}
		}
	}
	else
	{
		//如果模拟方法为原MODFLOW方法, 或者为全有效单元法且采用BCF格式
#pragma omp parallel for num_threads(m_NUMTD) schedule(dynamic,NumTask)
		for (long iCell = 0; iCell < m_NUMRC; iCell++)
		{
			//注释: 以下几个变量的定义必须放在for块中不能共用, 以适应并行运算要求
			long i, j, node, noderight, nodefront;
			CGridCell* pCellSelf = NULL;
			CGridCell* pCellFront = NULL;
			double T1, T2;
			//注释完毕
			//先确定网格单元在数组中的行号和列号
			i = iCell / m_NUMCOL;
			j = iCell - i * m_NUMCOL;
			//计算网格单元的水平向水力传导系数
			node = (lyrID - 1) * m_NUMRC + i * m_NUMCOL + j;
			pCellSelf = m_pGDLayers[lyrID - 1]->m_pGridCellAry[i][j];
			if (m_AryIBOUND[node] == 0 || pCellSelf->m_TRAN < 1e-30)
			{
				//如果本网格单元无效或导水系数值为0, CC和CR都为0
				m_AryCR[node] = 0.0;
				m_AryCC[node] = 0.0;
			}
			else
			{
				//如果本网格单元有效且K值大于0, 计算CR和CC
				T1 = m_AryTR[node];
				assert(T1 > 0.0);
				//1. 如果本网格单元不是最后一列,计算沿行方向上水力传导系数
				if (j != m_NUMCOL - 1)
				{
					noderight = node + 1;
					if (m_AryIBOUND[noderight] != 0)
					{
						//如果右侧网格单元为有效单元, 计算确定CR
						T2 = m_AryTR[noderight];
						assert(T2 > 0.0);
						m_AryCR[node] = 2.0 * T1 * T2 * m_AryDelC[i] /
							(T1 * m_AryDelR[j + 1] + T2 * m_AryDelR[j]);
					}
					else
					{
						//如果右侧网格单元为无效单元, CR=0
						m_AryCR[node] = 0.0;
					}
				}
				else
				{
					//本网格单元为最后一列, CR=0
					m_AryCR[node] = 0.0;
				}
				//2. 如果本网格单元不是最后一行,计算沿列方向的水力传导系数
				if (i != m_NUMROW - 1)
				{
					nodefront = node + m_NUMCOL;
					if (m_AryIBOUND[nodefront] != 0)
					{
						//如果前侧网格单元有效, 计算确定CC
						T2 = m_AryTR[nodefront];
						assert(T2 > 0.0);
						if (bCANIDis == FALSE)
						{
							//如果各向异性系数是含水层统一值
							m_AryCC[node] = YX * T1 * T2 * m_AryDelR[j] /
								(T1 * m_AryDelC[i + 1] + T2 * m_AryDelC[i]);
						}
						else
						{
							//如果各向异性系数是网格单元分布式
							pCellFront = m_pGDLayers[lyrID - 1]->m_pGridCellAry[i + 1][j];
							m_AryCC[node] = 2.0 * pCellSelf->m_HANI * T1 * pCellFront->m_HANI * T2 *
								m_AryDelR[j] / (pCellSelf->m_HANI * T1 * m_AryDelC[i + 1] +
									pCellFront->m_HANI * T2 * m_AryDelC[i]);
						}
					}
					else
					{
						//如果前网格单元为无效单元, CC=0
						m_AryCC[node] = 0.0;
					}
				}
				else
				{
					//本网格单元为最后一行, CC=0
					m_AryCC[node] = 0.0;
				}
			}
		}
	}
}

void CGDWater::CalVaryHoriCond(long lyrID)
{
	//对导水系数可变含水层计算水平向水力传导系数
	//CR为沿行方向(X方向)水力传导系数, CC为沿列方向(Y方向)水力传导系数

	long LyrCon = GetGridLyr(lyrID)->GetLyrCon();
	if (LyrCon == 0 || LyrCon == 2)
	{
		//如果本含水层是导水系数不变含水层, 直接返回
		return;
	}

	BOOL bCANIDis = TRUE;        //含水层沿列方向的渗透系数异性是否是分布式
	if (m_INTBLKM == 1 || (m_INTBLKM == 2 && GetGridLyr(lyrID)->m_LYRTRPY > 0.0))
	{
		//如果是BCF，或者是LPF但是含水层沿列方向的渗透系数异性统一
		//则含水层沿列方向的渗透系数异性不是分布式
		bCANIDis = FALSE;
	}
	
	//计算含水层各网格单元间的水力传导系数
	double YX = 0.0;
	if (bCANIDis == FALSE)
	{
		//如果含水层沿列方向的渗透系数异性不是分布式
		//先计算沿列方向的渗透系数因子
		YX = GetGridLyr(lyrID)->GetLyrTRPY() * 2.0;
	}

	//含水层内逐网格单元计算
	long NumTask = long(m_NUMRC / m_NUMTD / 2.0) + 1;
	if (m_SIMMTHD == 1)
	{
		//如果模拟方法为全有效单元法
#pragma omp parallel for num_threads(m_NUMTD) schedule(dynamic,NumTask)
		for (long iCell = 0; iCell < m_NUMRC; iCell++)
		{
			//注释: 以下几个变量的定义必须放在for块中不能共用, 以适应并行运算要求
			long i, j, nodeself, noderight, nodefront, NodeUpUse, NodeLowUse;
			CGridCell* pCellSelf = NULL;
			CGridCell* pCellRight = NULL;
			CGridCell* pCellFront = NULL;
			CGridCell* pCellUpUse = NULL;
			CGridCell* pCellLowUse = NULL;
			double Kave_R, Kave_C, MaxBot, HThikUp, HThikLow, HThikAve, DeltLenUp,
				DeltLenLow;
			//注释完毕
			//先确定网格单元在数组中的行号和列号
			i = iCell / m_NUMCOL;
			j = iCell - i * m_NUMCOL;
			//计算网格单元的水平向水力传导系数
			nodeself = (lyrID - 1) * m_NUMRC + i * m_NUMCOL + j;
			pCellSelf = m_pGDLayers[lyrID - 1]->m_pGridCellAry[i][j];
			if (m_AryIBOUND[nodeself] == 0 || pCellSelf->m_HK < 1e-30)
			{
				//如果本网格单元无效或K值为0, CC和CR都为0
				m_AryCR[nodeself] = 0.0;
				m_AryCC[nodeself] = 0.0;
			}
			else
			{
				//如果本网格单元有效且K值大于0, 计算CR和CC
				//1. 如果本网格单元不是最后一列,计算沿行方向上水力传导系数
				if (j != m_NUMCOL - 1)
				{
					noderight = nodeself + 1;
					if (m_AryIBOUND[noderight] != 0)
					{
						//如果右侧网格单元为有效单元, 计算确定CR
						pCellRight = m_pGDLayers[lyrID - 1]->m_pGridCellAry[i][j + 1];
						//根据水位相对关系确定上风单元和下风单元
						if (m_AryHNEW[nodeself] >= m_AryHNEW[noderight])
						{
							pCellUpUse = pCellSelf;
							pCellLowUse = pCellRight;
							NodeUpUse = nodeself;
							NodeLowUse = noderight;
						}
						else
						{
							pCellUpUse = pCellRight;
							pCellLowUse = pCellSelf;
							NodeUpUse = noderight;
							NodeLowUse = nodeself;
						}
						//找到两个网格单元之间底板高程相对较高的一个
						MaxBot = max(pCellUpUse->m_BOT, pCellLowUse->m_BOT);
						if (m_AryHNEW[NodeUpUse] > MaxBot)
						{
							//如果上风网格单元的水头高于相对较高底板高程
							//两个网格单元间的水力传导系数将不为零
							Kave_R = m_pGDLayers[lyrID - 1]->m_AryKav_R[i][j];
							DeltLenUp = m_AryDelR[pCellUpUse->m_ICOL - 1];
							DeltLenLow = m_AryDelR[pCellLowUse->m_ICOL - 1];
							//先分别计算两个网格单元的含水层过水断面厚度
							if (pCellUpUse->m_BOT > pCellLowUse->m_BOT)
							{
								//如果上风单元的底板高程高于下风单元的
								//上风单元过水断面计算
								HThikUp = m_AryHNEW[NodeUpUse] - MaxBot;
								//下风单元过水断面计算
								if (m_AryHNEW[NodeLowUse] > MaxBot)
								{
									//如果下风单元的水头高于上风单元的底板高程, 下风单元的过水断面厚度按下式计算	
									//注: 第二项为两个单元的底板高程差调节项
									HThikLow = m_AryHNEW[NodeLowUse] - MaxBot +
										min(m_AryHNEW[NodeLowUse] - MaxBot, MaxBot - pCellLowUse->m_BOT);
								}
								else
								{
									//如果下风单元的水头不高于上风单元的底板高程, 下风单元的过水断面厚度为零									
									HThikLow = 0.0;
								}
							}
							else
							{
								//如果上风单元的底板高程低于或等于下风单元的
								//上风单元过水断面厚度计算
								//注: 第二项为两个单元的底板高程差调节项
								HThikUp = m_AryHNEW[NodeUpUse] - MaxBot +
									min(m_AryHNEW[NodeUpUse] - MaxBot, MaxBot - pCellUpUse->m_BOT);
								//下风单元过水断面厚度计算
								HThikLow = max(0.0, m_AryHNEW[NodeLowUse] - MaxBot);
							}
							if (LyrCon == 3)
							{
								//如果含水层有顶板条件, 含水层过水断面厚度不能超过顶/底板高程限制
								HThikUp = min(HThikUp, pCellUpUse->m_TOP - pCellUpUse->m_BOT);
								HThikLow = min(HThikLow, pCellLowUse->m_TOP - pCellLowUse->m_BOT);
							}
							//计算两个网格单元之间的平均含水层过水断面厚度
							HThikAve = (HThikUp * DeltLenUp + HThikLow * DeltLenLow) /
								(DeltLenUp + DeltLenLow);
							//计算两个网格单元之间的水力传导系数
							m_AryCR[nodeself] = 2.0 * m_AryDelC[i] * Kave_R * HThikAve /
								(m_AryDelR[j] + m_AryDelR[j + 1]);
						}
						else
						{
							//如果上风网格单元的水头小于等于两个网格单元中的最大底板高程, CR=0
							m_AryCR[nodeself] = 0.0;
						}
					}
					else
					{
						//如果右侧网格单元为无效单元, CR=0
						m_AryCR[nodeself] = 0.0;
					}
				}
				else
				{
					//本网格单元为最后一列, CR=0
					m_AryCR[nodeself] = 0.0;
				}
				//2. 如果本网格单元不是最后一行,计算沿列方向的水力传导系数
				if (i != m_NUMROW - 1)
				{
					nodefront = nodeself + m_NUMCOL;
					if (m_AryIBOUND[nodefront] != 0)
					{
						//如果前侧网格单元有效, 计算确定CC
						pCellFront = m_pGDLayers[lyrID - 1]->m_pGridCellAry[i + 1][j];
						//根据水位相对关系确定上风单元和下风单元
						if (m_AryHNEW[nodeself] >= m_AryHNEW[nodefront])
						{
							pCellUpUse = pCellSelf;
							pCellLowUse = pCellFront;
							NodeUpUse = nodeself;
							NodeLowUse = nodefront;
						}
						else
						{
							pCellUpUse = pCellFront;
							pCellLowUse = pCellSelf;
							NodeUpUse = nodefront;
							NodeLowUse = nodeself;
						}
						//找到两个网格单元之间底板高程相对较高的一个
						MaxBot = max(pCellUpUse->m_BOT, pCellLowUse->m_BOT);
						if (m_AryHNEW[NodeUpUse] > MaxBot)
						{
							//如果上风网格单元的水头高于相对较高底板高程
							//两个网格单元间的水力传导系数将不为零
							Kave_C = m_pGDLayers[lyrID - 1]->m_AryKav_C[i][j];
							DeltLenUp = m_AryDelC[pCellUpUse->m_IROW - 1];
							DeltLenLow = m_AryDelC[pCellLowUse->m_IROW - 1];
							//先分别计算两个网格单元的含水层过水断面厚度
							if (pCellUpUse->m_BOT > pCellLowUse->m_BOT)
							{
								//如果上风单元的底板高程高于下风单元的
								//上风单元过水断面计算
								HThikUp = m_AryHNEW[NodeUpUse] - MaxBot;
								//下风单元过水断面计算
								if (m_AryHNEW[NodeLowUse] > MaxBot)
								{
									//如果下风单元的水头高于上风单元的底板高程, 下风单元的过水断面厚度按下式计算	
									//注: 第二项为两个单元的底板高程差调节项
									HThikLow = m_AryHNEW[NodeLowUse] - MaxBot +
										min(m_AryHNEW[NodeLowUse] - MaxBot, MaxBot - pCellLowUse->m_BOT);
								}
								else
								{
									//如果下风单元的水头不高于上风单元的底板高程, 下风单元的过水断面厚度为零									
									HThikLow = 0.0;
								}
							}
							else
							{
								//如果上风单元的底板高程低于或等于下风单元的
								//上风单元过水断面厚度计算
								//注: 第二项为两个单元的底板高程差调节项
								HThikUp = m_AryHNEW[NodeUpUse] - MaxBot +
									min(m_AryHNEW[NodeUpUse] - MaxBot, MaxBot - pCellUpUse->m_BOT);
								//下风单元过水断面厚度计算
								HThikLow = max(0.0, m_AryHNEW[NodeLowUse] - MaxBot);
							}
							if (LyrCon == 3)
							{
								//如果含水层有顶板条件, 含水层过水断面厚度不能超过顶/底板高程限制
								HThikUp = min(HThikUp, pCellUpUse->m_TOP - pCellUpUse->m_BOT);
								HThikLow = min(HThikLow, pCellLowUse->m_TOP - pCellLowUse->m_BOT);
							}
							//计算两个网格单元之间的平均含水层过水断面厚度
							HThikAve = (HThikUp * DeltLenUp + HThikLow * DeltLenLow) /
								(DeltLenUp + DeltLenLow);
							//计算两个网格单元之间的水力传导系数
							m_AryCC[nodeself] = 2.0 * m_AryDelR[j] * Kave_C * HThikAve /
								(m_AryDelC[i] + m_AryDelC[i + 1]);
						}
						else
						{
							//如果上风网格单元的水头小于等于两个网格单元中的最大底板高程, CC=0
							m_AryCC[nodeself] = 0.0;
						}
					}
					else
					{
						//如果前网格单元为无效单元, CC=0
						m_AryCC[nodeself] = 0.0;
					}
				}
				else
				{
					//本网格单元为最后一行, CC=0
					m_AryCC[nodeself] = 0.0;
				}
			}
		}
	}
	else
	{
		//如果模拟方法为原MODFLOW方法
#pragma omp parallel for num_threads(m_NUMTD) schedule(dynamic,NumTask)
		for (long iCell = 0; iCell < m_NUMRC; iCell++)
		{
			//注释: 以下几个变量的定义必须放在for块中不能共用, 以适应并行运算要求
			long i, j, node, noderight, nodefront;
			CGridCell* pCellSelf = NULL;
			CGridCell* pCellFront = NULL;
			double T1, T2;
			//注释完毕
			//先确定网格单元在数组中的行号和列号
			i = iCell / m_NUMCOL;
			j = iCell - i * m_NUMCOL;
			//计算网格单元的水平向水力传导系数
			node = (lyrID - 1) * m_NUMRC + i * m_NUMCOL + j;
			pCellSelf = m_pGDLayers[lyrID - 1]->m_pGridCellAry[i][j];
			if (m_AryIBOUND[node] == 0 || pCellSelf->m_HK < 1e-30)
			{
				//如果本网格单元无效或K值为0, CC和CR都为0
				m_AryCR[node] = 0.0;
				m_AryCC[node] = 0.0;
			}
			else
			{
				//如果本网格单元有效且K值大于0, 计算CR和CC
				T1 = m_AryTR[node];
				assert(T1 > 0.0);
				//1. 如果本网格单元不是最后一列,计算沿行方向上水力传导系数
				if (j != m_NUMCOL - 1)
				{
					noderight = node + 1;
					if (m_AryIBOUND[noderight] != 0)
					{
						//如果右侧网格单元为有效单元, 计算确定CR
						T2 = m_AryTR[noderight];
						assert(T2 > 0.0);
						m_AryCR[node] = 2.0 * T1 * T2 * m_AryDelC[i] /
							(T1 * m_AryDelR[j + 1] + T2 * m_AryDelR[j]);
					}
					else
					{
						//如果右侧网格单元为无效单元, CR=0
						m_AryCR[node] = 0.0;
					}
				}
				else
				{
					//本网格单元为最后一列, CR=0
					m_AryCR[node] = 0.0;
				}

				//2. 如果本网格单元不是最后一行,计算沿列方向的水力传导系数
				if (i != m_NUMROW - 1)
				{
					nodefront = node + m_NUMCOL;
					if (m_AryIBOUND[nodefront] != 0)
					{
						//如果前侧网格单元有效, 计算确定CC
						T2 = m_AryTR[nodefront];
						assert(T2 > 0.0);
						if (bCANIDis == FALSE)
						{
							//如果各向异性系数是含水层统一值
							m_AryCC[node] = YX * T1 * T2 * m_AryDelR[j] /
								(T1 * m_AryDelC[i + 1] + T2 * m_AryDelC[i]);
						}
						else
						{
							//如果各向异性系数是网格单元分布式
							pCellFront = m_pGDLayers[lyrID - 1]->m_pGridCellAry[i + 1][j];
							m_AryCC[node] = 2.0 * pCellSelf->m_HANI * T1 * pCellFront->m_HANI * T2 *
								m_AryDelR[j] / (pCellSelf->m_HANI * T1 * m_AryDelC[i + 1] +
									pCellFront->m_HANI * T2 * m_AryDelC[i]);
						}
					}
					else
					{
						//如果前网格单元为无效单元, CC=0
						m_AryCC[node] = 0.0;
					}
				}
				else
				{
					//本网格单元为最后一行, CC=0
					m_AryCC[node] = 0.0;
				}
			}
		}
	}
}

void CGDWater::DealWithSC(long lyrID)
{
	//若模拟非稳定流, 对编号为lyrID的含水层处理与时间有关的对角项和右端项
	//注: LyrID必须从1开始编号

	if (m_SIMTYPE != 2)
	{
		//如果不是非稳定流模拟直接返回
		return;
	}

	long LyrCon = GetGridLyr(lyrID)->GetLyrCon();
	double TLED = 1.0 / m_DeltT;

	//逐个含水层网格单元处理对角项和右端项
	long NumTask = long(m_NUMRC / m_NUMTD / 2.0) + 1;
#pragma omp parallel for num_threads(m_NUMTD) schedule(dynamic,NumTask)
	for (long iCell = 0; iCell < m_NUMRC; iCell++)
	{
		//注释: 以下几个变量的定义必须放在for块中不能共用, 以适应并行运算要求
		long i, j, node;
		double RHO1, RHO2, BOT, TOP, HTMP, HOLD;
		CGridCell* pGridCell = NULL;
		//注释完毕
		//先确定网格单元在数组中的行号和列号
		i = iCell / m_NUMCOL;
		j = iCell - i * m_NUMCOL;
		//处理网格单元的对角项和右端项
		node = (lyrID - 1) * m_NUMRC + i * m_NUMCOL + j;
		pGridCell = m_pGDLayers[lyrID - 1]->m_pGridCellAry[i][j];
		if (m_AryIBOUND[node] > 0)
		{
			//仅对有效的网格单元进行处理
			//注: 全有效单元法下疏干的网格单元也是有效的网格单元
			HOLD = m_AryHOLD[node];
			HTMP = m_AryHNEW[node];
			RHO1 = m_ArySC1[node] * TLED;
			if (LyrCon == 0)
			{
				//如果是纯承压水含水层
				//1. 先处理时段初水位对右端项的影响
				m_AryRHS[node] = m_AryRHS[node] - RHO1 * HOLD;
				//2. 再处理时段末水位对对角项的影响
				m_AryHCOF[node] = m_AryHCOF[node] - RHO1;
			}
			else if (LyrCon == 1)
			{
				//如果是纯潜水含水层
				BOT = pGridCell->m_BOT;
				//1. 先处理时段初水位对右端项的影响
				if (HOLD > BOT)
				{
					//若时段初水位高于网格单元底板高程
					m_AryRHS[node] = m_AryRHS[node] - RHO1 * (HOLD - BOT);
				}
				//2. 再处理时段末水位对对角项和右端项的影响
				if (HTMP > BOT)
				{
					//若时段末水位高于网格单元底板高程
					m_AryHCOF[node] = m_AryHCOF[node] - RHO1;
					m_AryRHS[node] = m_AryRHS[node] - RHO1 * BOT;
				}
			}
			else if (LyrCon == 2)
			{
				//如果是导水系数不变的承压/非承压含水层
				//注: 类似导水系数可变的承压/非承压含水层
				//但以BOT=0作为参考网格单元底板高程
				RHO2 = m_ArySC2[node] * TLED;
				TOP = pGridCell->m_TOP;
				//1. 先处理时段初水位对右端项的影响
				if (HOLD <= TOP)
				{
					//若时段初水位低于网格单元顶板高程
					m_AryRHS[node] = m_AryRHS[node] - RHO2 * HOLD;
				}
				if (HOLD > TOP)
				{
					//若时段初水位高于网格单元的顶板高程
					m_AryRHS[node] = m_AryRHS[node] - RHO2 * TOP - RHO1 * (HOLD - TOP);
				}
				//2. 再处理时段末水位对对角项和右端项的影响
				if (HTMP <= TOP)
				{
					//若时段末水位低于网格单元顶板高程
					m_AryHCOF[node] = m_AryHCOF[node] - RHO2;
				}
				if (HTMP > TOP)
				{
					//若时段末水位高于网格单元的顶板高程
					m_AryHCOF[node] = m_AryHCOF[node] - RHO1;
					m_AryRHS[node] = m_AryRHS[node] + RHO2 * TOP - RHO1 * TOP;
				}
			}
			else
			{
				//如果是导水系数可变的承压/非承压含水层
				assert(LyrCon == 3);
				RHO2 = m_ArySC2[node] * TLED;
				TOP = pGridCell->m_TOP;
				BOT = pGridCell->m_BOT;
				//1. 先处理时段初水位对右端项的影响
				if (HOLD > BOT && HOLD <= TOP)
				{
					//若时段初水位介于网格单元顶、底板高程之间
					m_AryRHS[node] = m_AryRHS[node] - RHO2 * (HOLD - BOT);
				}
				if (HOLD > TOP)
				{
					//若时段初水位高于网格单元的顶板高程
					m_AryRHS[node] = m_AryRHS[node] - RHO2 * (TOP - BOT) - RHO1 * (HOLD - TOP);
				}
				//2. 再处理时段末水位对对角项和右端项的影响
				if (HTMP > BOT && HTMP <= TOP)
				{
					//若时段末水位介于网格单元顶、底板高程之间
					m_AryHCOF[node] = m_AryHCOF[node] - RHO2;
					m_AryRHS[node] = m_AryRHS[node] - RHO2 * BOT;
				}
				if (HTMP > TOP)
				{
					//若时段末水位高于网格单元的顶板高程
					m_AryHCOF[node] = m_AryHCOF[node] - RHO1;
					m_AryRHS[node] = m_AryRHS[node] + RHO2 * (TOP - BOT) - RHO1 * TOP;
				}
			}
		}
	}
}

void CGDWater::ModiRHS(long lyrID)
{
	//根据含水层水平向和垂向渗流情况修正矩阵方程的右端项

	//判断含水层类型
	long LyrCon = m_pGDLayers[lyrID - 1]->GetLyrCon();

	//1. 如果为全有效单元法且本含水层是可疏干含水层
	//当下风单元的水头低于两个单元之间相对较高的底板高程时需对右端项进行修正
	long i, j, Node, NodeRight, NodeFront;
	double MaxBot, MinHH;
	CGridCell* pGridCell = NULL;
	CGridCell* pCellRight = NULL;
	CGridCell* pCellFront = NULL;
	if (m_SIMMTHD == 1 && (LyrCon == 1 || LyrCon == 3))
	{
		//含水层内逐网格单元进行循环
		//(注: 本过程切记不可并行, 因为m_AryRHS可能同时被不同行和列的单元修正)
		for (i = 0; i < m_NUMROW; i++)
		{
			for (j = 0; j < m_NUMCOL; j++)
			{
				Node = (lyrID - 1) * m_NUMRC + i * m_NUMCOL + j;
				pGridCell = m_pGDLayers[lyrID - 1]->m_pGridCellAry[i][j];
				//1. 如果本网格单元不是最后一列, 修正沿行方向各网格单元的右端项
				if (j != m_NUMCOL - 1)
				{
					NodeRight = Node + 1;
					if (m_AryIBOUND[Node] != 0 && m_AryIBOUND[NodeRight] != 0)
					{
						//如果本网格单元和右侧网格单元都是有效单元(注: 全有效单元法下有效单元也可能是疏干的单元)
						if (m_AryCR[Node] > 0.0)
						{
							//如果本网格单元和右侧网格单元之间的水平向水力传导系数不为零
							pCellRight = m_pGDLayers[lyrID - 1]->m_pGridCellAry[i][j + 1];
							MaxBot = max(pGridCell->m_BOT, pCellRight->m_BOT);
							MinHH = min(m_AryHNEW[Node], m_AryHNEW[NodeRight]);
							if (MinHH < MaxBot)
							{
								//两个网格单元之间相对较低的水头低于两个网格单元之间相对较高的底板高程时
								//两个网格单元之间的水平向渗流量与下风单元的水头无关
								if (m_AryHNEW[Node] > MaxBot)
								{
									//本网格单元为上风单元
									//(1)先修正本网格单元自身的右端项
									m_AryRHS[Node] = m_AryRHS[Node] - m_AryCR[Node] * (MaxBot - m_AryHNEW[NodeRight]);
									//(2)再修正右侧网格单元的右端项
									m_AryRHS[NodeRight] = m_AryRHS[NodeRight] + m_AryCR[Node] * (MaxBot - m_AryHNEW[NodeRight]);
								}
								else
								{
									//本网格单元为下风单元
									//(1)先修正本网格单元自身的右端项
									m_AryRHS[Node] = m_AryRHS[Node] - m_AryCR[Node] * (m_AryHNEW[Node] - MaxBot);
									//(2)再修正右侧网格单元的右端项
									m_AryRHS[NodeRight] = m_AryRHS[NodeRight] + m_AryCR[Node] * (m_AryHNEW[Node] - MaxBot);
								}
							}
						}
					}
				}
				//2. 如果本网格单元不是最后一行, 修正沿列方向各网格单元的右端项
				if (i != m_NUMROW - 1)
				{
					NodeFront = Node + m_NUMCOL;
					if (m_AryIBOUND[Node] != 0 && m_AryIBOUND[NodeFront] != 0)
					{
						//如果本网格单元和前侧网格单元都是有效单元(注: 全有效单元法下有效单元也可能是疏干的单元)
						if (m_AryCC[Node] > 0.0)
						{
							//如果本网格单元和前侧网格单元之间的水平向水力传导系数不为零
							pCellFront = m_pGDLayers[lyrID - 1]->m_pGridCellAry[i + 1][j];
							MaxBot = max(pGridCell->m_BOT, pCellFront->m_BOT);
							MinHH = min(m_AryHNEW[Node], m_AryHNEW[NodeFront]);
							if (MinHH < MaxBot)
							{
								//两个网格单元之间相对较低的水头低于两个网格单元之间相对较高的底板高程时
								//两个网格单元之间的水平向渗流量与下风单元的水头无关
								if (m_AryHNEW[Node] > MaxBot)
								{
									//本网格单元为上风单元
									//(1)先修正本网格单元自身的右端项
									m_AryRHS[Node] = m_AryRHS[Node] - m_AryCC[Node] * (MaxBot - m_AryHNEW[NodeFront]);
									//(2)再修正前侧网格单元的右端项
									m_AryRHS[NodeFront] = m_AryRHS[NodeFront] + m_AryCC[Node] * (MaxBot - m_AryHNEW[NodeFront]);
								}
								else
								{
									//本网格单元为下风单元
									//(1)先修正本网格单元自身的右端项
									m_AryRHS[Node] = m_AryRHS[Node] - m_AryCC[Node] * (m_AryHNEW[Node] - MaxBot);
									//(2)再修正前侧网格单元的右端项
									m_AryRHS[NodeFront] = m_AryRHS[NodeFront] + m_AryCC[Node] * (m_AryHNEW[Node] - MaxBot);
								}
							}
						}
					}
				}
			}
		}
	}

	//2. 如果本含水层为承压/非承压可转化含水层, 当网格单元处于非承压状态时对右端项进行修正
	long NumTask = long(m_NUMRC / m_NUMTD / 2.0) + 1;
	if (lyrID != 1 && (LyrCon == 2 || LyrCon == 3))
	{
		//如果本层含水层不是顶层含水层, 并且是承压/非承压可转化含水层
		//含水层内逐网格单元进行循环(注: 修正过程可以并行)
#pragma omp parallel for num_threads(m_NUMTD) schedule(dynamic,NumTask)
		for (long iCell = 0; iCell < m_NUMRC; iCell++)
		{
			//注释: 以下几个变量的定义必须放在for块中不能共用, 以适应并行运算要求
			long i, j, Node, NodeAbove, LyrConAbove;
			CGridCell* pGridCell = NULL;
			CGridCell* pCellAbove = NULL;
			double HTMP;
			//注释完毕
			//先确定网格单元在数组中的行号和列号
			i = iCell / m_NUMCOL;
			j = iCell - i * m_NUMCOL;
			//修正网格单元的右端项
			Node = (lyrID - 1) * m_NUMRC + i * m_NUMCOL + j;
			pGridCell = m_pGDLayers[lyrID - 1]->m_pGridCellAry[i][j];
			LyrConAbove = m_pGDLayers[lyrID - 2]->GetLyrCon();
			pCellAbove = m_pGDLayers[lyrID - 2]->m_pGridCellAry[i][j];
			NodeAbove = Node - m_NUMRC;
			if (m_AryIBOUND[Node] != 0 && m_AryIBOUND[NodeAbove] != 0)
			{
				//如果本网格单元和上层网格单元都是有效单元(注: 全有效单元法下有效单元也可能是疏干的单元)
				HTMP = m_AryHNEW[Node];
				if (HTMP < pGridCell->m_TOP)
				{
					//如果本网格单元处于非承压状态(或为疏干单元,这也没有关系)
					if ((LyrConAbove == 0 || LyrConAbove == 2) ||
						((LyrConAbove == 1 || LyrConAbove == 3) && m_AryHNEW[NodeAbove] > pCellAbove->m_BOT))
					{
						//如果上层网格单元不是疏干单元
						//(1)先修正本网格单元自身的右端项
						m_AryRHS[Node] = m_AryRHS[Node] - m_AryCV[NodeAbove] * (HTMP - pGridCell->m_TOP);
						//(2)再修正上层网格单元的右端项
						m_AryRHS[NodeAbove] = m_AryRHS[NodeAbove] + m_AryCV[NodeAbove] * (HTMP - pGridCell->m_TOP);
					}
				}
			}
		}
	}
}

void CGDWater::BASICFM(long KITER)
{
	//进行矩阵方程的基础构建(之后处理其他源汇和边界问题)
	//KITER: 当前迭代次数(从1开始)

	//清空对角线元素数组和右端项向量数组
	ZeroMemory(m_AryHCOF, sizeof(double)*m_NumNodes);
	ZeroMemory(m_AryRHS, sizeof(double)*m_NumNodes);

	long iLyr;
    //1. 如果采用原MODFLOW方法, 更新水力传导系数可变含水层(1,3)的水平向导水系数
	if (m_SIMMTHD == 2)
	{
		for (iLyr = 1; iLyr <= m_NUMLYR; iLyr++)
		{
			UpHoriTRANS(iLyr, KITER);
		}
	}

	//3. 更新导水系数可变含水层(1,3)的水平向水力传导系数
	for (iLyr = 1; iLyr <= m_NUMLYR; iLyr++)
	{
		CalVaryHoriCond(iLyr);
	}

	//6. 如果采用LPF数据格式, 更新垂向水力传导系数(注: 必须在水平向水力传导系数计算完以后再计算)
	if (m_INTBLKM == 2)
	{
		for (iLyr = 1; iLyr <= m_NUMLYR; iLyr++)
		{
			CalVaryVertCond(iLyr);
		}
	}

	//7. 如果模拟非稳定流, 处理与时间有关的对角项和右端项
	if (m_SIMTYPE == 2)
	{
		for (iLyr = 1; iLyr <= m_NUMLYR; iLyr++)
		{
			DealWithSC(iLyr);
		}
	}

	//9. 根据含水层水平向和垂向渗流情况修正矩阵方程的右端项
	for (iLyr = 1; iLyr <= m_NUMLYR; iLyr++)
	{
		ModiRHS(iLyr);
	}

	//10. 全有效单元法且模拟稳定流时给网格单元方程右端增加附加项
	if (m_SIMMTHD == 1 && m_SIMTYPE == 1 && m_LAMBDA > 0.0)
	{
		AddRight();
	}
}

void CGDWater::AddRight()
{
	//全有效单元法且模拟稳定流时给网格单元方程右端增加附加项

	//该函数仅在模拟稳定流且采用全有效单元法时调用
	if (!(m_SIMMTHD == 1 && m_SIMTYPE == 1 && m_LAMBDA > 0.0))
	{
		return;
	}
	
	long NCantDryCell = (long)m_pCantDryCellAry.size();
	long NumTask = long(NCantDryCell / GDWMOD.m_NUMTD / 2.0) + 1;
#pragma omp parallel for num_threads(GDWMOD.m_NUMTD) schedule(dynamic,NumTask)
	for (long iCell = 0; iCell < NCantDryCell; iCell++)
	{
		//注释: 以下几个变量的定义必须放在for块中不能共用, 以适应并行运算要求
		long Node;
		CGridCell* pGridCell = NULL;
		//注释完毕
		//确定网格单元的指针及其编号
		pGridCell = m_pCantDryCellAry[iCell];
		Node = GDWMOD.GetCellNode(pGridCell->m_ILYR, pGridCell->m_IROW, pGridCell->m_ICOL);
		if (m_AryIBOUND[Node] > 0)
		{
			//如果网格单元有效
			m_AryHCOF[Node] = m_AryHCOF[Node] - m_LAMBDA;
			m_AryRHS[Node] = m_AryRHS[Node] - m_LAMBDA * m_AryHNEW[Node];
		}
	}
}

void CGDWater::CellBudget()
{
	//计算各网格单元间渗流量、网格单元源汇项和蓄量变化情况

	//更新网格单元的水头值和IBOUND(在考虑疏干/湿润变化时该值有可能变化)
	//清空有关水量平衡统计项
	PreBudget();

	//计算各网格单元的蓄量变化率
	CellStrgBudget();
	
	//计算各网格单元之间的渗流量
	CellFlowBudget();

	//计算流入定常水头单元(离开地下水系统,负)或
	//定常水头单元流出的水量(进入地下水系统,正)(L3/T)
	CHBBudget();

	//统计其他边界或源汇项通量情况
	long iBndObj;
	for (iBndObj = 0; iBndObj < m_pSimBndObjAry.size(); iBndObj++)
	{
		m_pSimBndObjAry[iBndObj]->Budget(m_AryIBOUND, m_AryHNEW, m_AryHOLD);
	}
}

void CGDWater::CellStrgBudget()
{
	//计算各网格单元的蓄量变化率

	if (m_SIMTYPE == 1)
	{
		//稳定流时无需计算, 直接返回
		return;
	}

	long k;
	//逐含水层计算各网格单元蓄量变化率
	for (k = 0; k<m_NUMLYR; k++)
	{
		m_pGDLayers[k]->CalLyrCellStgR(m_AryIBOUND, m_AryHNEW, m_AryHOLD, m_ArySC1, m_ArySC2);
	}
}

void CGDWater::PreBudget()
{
	//更新水位计算结果到网格单元, 清零统计变量

	long k;
	for (k = 0; k<m_NUMLYR; k++)
	{
		m_pGDLayers[k]->UpdateLyrCellStatus(m_AryHNEW, m_AryIBOUND);
	}
}

void CGDWater::CellFlowBudget()
{
	//计算各网格单元之间的地下水渗流量

	long k;
	//逐含水层计算各网格单元间地下水渗流量
	for (k = 0; k<m_NUMLYR; k++)
	{
		m_pGDLayers[k]->CalLyrCellFlowR(m_AryIBOUND, m_AryHNEW, m_AryCR, m_AryCC, m_AryCV, m_ICHFLG);
	}
}

void CGDWater::CHBBudget()
{
	//定常水头边界渗漏/补给计算

	long NumCHBCell = long(m_pCHBCellAry.size());
	if (NumCHBCell == 0)
	{
		//如果没有定常水头单元, 直接返回
		return;
	}

	long NumTask = long(NumCHBCell / m_NUMTD / 2.0) + 1;
#pragma omp parallel for num_threads(m_NUMTD) schedule(dynamic,NumTask)
	for (long i = 0; i < NumCHBCell; i++)
	{
		//注释: 以下变量只能在for块中定义以适应并行运算
		CGridCell* pGridCell = NULL;
		CCHBCell* pCHBCell = NULL;
		double CHBFlow;
		//注释结束
		pCHBCell = m_pCHBCellAry[i];
		pGridCell = pCHBCell->m_pGridCell;
		//统计流入/流出量
		CHBFlow = pGridCell->m_FlowRight + pGridCell->m_FlowLeft + pGridCell->m_FlowFront +
			pGridCell->m_FlowBack + pGridCell->m_FlowUp + pGridCell->m_FlowDown;
		//数据统计到地下水网格单元上
		pCHBCell->m_pGridCell->AddSinkR(m_CnhBndID, CHBFlow);
	}
}

void CGDWater::LinkDataSource(CString StrPath)
{
	//连接模型数据源

	_variant_t var;
	CString strLocalDB, strDBName,
		strAddname, strSQL1, strSQL2, strLink;

	//保存当前目录路径
	m_strCurrentDir = StrPath;

    //判断数据环境
	m_DataContext = 1;
	if (m_DataContext == 1)
	{
		//本次模拟采用TXT文件系统
		//确定模拟输入数据目录
		m_strDataInDir = m_strCurrentDir + _T("\\Data.in");
		if (!CheckFolderExist(m_strDataInDir))
		{
			//如果文件夹不存在, 报错
			printf(_T("\n"));
			printf(_T("当前目录下不存在COMUS模型的输入数据目录<Data.in>, 请检查!\n"));
			PauAndQuit();
		}
		m_strDataInDir = m_strDataInDir + _T("\\");
		//确定模拟结果输出目录
		m_strDataOutDir = m_strCurrentDir + _T("\\Data.out");
		if (!CheckFolderExist(m_strDataOutDir))
		{
			//如果文件夹不存在, 生成该文件夹
			CreateDirectory(m_strDataOutDir, 0);
		}
		m_strDataOutDir = m_strDataOutDir + _T("\\");
		//清空输出文件夹中的所有文件
		DelDirFiles(m_strDataOutDir);
	}
	else
	{
		//本次模拟采用sql-Server数据库
		strLocalDB = m_strCurrentDir + _T("\\模型数据.mdf");
		//获取当前登录用户的名称
		TCHAR UserName[256] = { 0 };
		DWORD dwSize = 256;
		GetUserName(UserName, &dwSize);
		//设置文件夹权限
		if (!EnableDirAccess(m_strCurrentDir, UserName))
		{
			printf(_T("设置文件夹访问权限失败 : %s"), LPCSTR(m_strCurrentDir));
		}

		//连接数据库
		try
		{
			m_pConnection.CreateInstance(__uuidof(Connection));
			//连接SQLSERVER
			CString strConnect = _T("Provider=SQLOLEDB; Integrated Security=SSPI; \
							 Persist Security Info=false; Data Source=(local)");
			m_pConnection->Open(LPCSTR(strConnect), _T(""), _T(""), adModeUnknown);
			m_pConnection->Execute(_T("USE master"), &var, adCmdText);

			//查看当前目录下的“模型数据.mdf”数据库是否已经在SQLSERVER服务器中
			strDBName = GetDbFromServer(m_pConnection, strLocalDB);
			BOOL bNeedAttach = FALSE;
			if (strDBName == _T("NotFind"))
			{
				bNeedAttach = TRUE;
			}
			if (bNeedAttach)
			{
				//若不在,执行附加操作
				CTime timeAttach;
				timeAttach = CTime::GetCurrentTime();
				strAddname = timeAttach.Format(_T("%Y_%m_%d_%H_%M_%S"));
				strAddname = _T("模型数据") + strAddname;
				strSQL1 = m_strCurrentDir;
				strSQL2 = _T("sp_attach_db '模型数据','");
				strSQL2 = _T("sp_attach_db ");
				strSQL2 = strSQL2 + _T("'") + strAddname + _T("','");
				strSQL2 = strSQL2 + strSQL1 + _T("\\模型数据.mdf',");
				strSQL2 = strSQL2 + _T("'") + strSQL1 + _T("\\模型数据_log.ldf'");
				m_pConnection->Execute(LPCSTR(strSQL2), &var, adCmdText);
			}
			//确定“模型数据.mdf”数据库在SQLSERVER服务器中的名称
			CString strLinkDBName;
			if (bNeedAttach)
			{
				//如果此前不在,使用本次给的数据库名称
				strLinkDBName = strAddname;
			}
			else
			{
				//否则使用原名称
				strLinkDBName = strDBName;
			}
			//切换使用“模型数据.mdf”数据库
			strSQL1 = _T("USE ");
			strSQL1 = strSQL1 + strLinkDBName;
			m_pConnection->Execute(_bstr_t(strSQL1), &var, adCmdText);
			//设置写入数据库时的连接语句
			CString strSqlCon = _T("Integrated Security=SSPI; \
							 Persist Security Info=false; Data Source=(local); ");
			strSqlCon = strSqlCon + _T("Initial Catalog=") + strLinkDBName;
			m_strSqlCon = gcnew String(strSqlCon);
			/*
			//调用存储过程把输出表的主键索引删掉以加快输出速度
			m_pCommand.CreateInstance(__uuidof(Command));
			m_pCommand->ActiveConnection = m_pConnection;
			m_pCommand->CommandText = bstr_t(_T("EXEC ModifyPK @ActPar = 0"));
			m_pCommand->Execute(NULL,NULL,adCmdText);
			*/
		}
		catch (_com_error e)
		{
			printf(LPCTSTR(e.Description()));
			exit(1);
		}

		//删除上次的计算结果
		try
		{
			var.Clear();   //一定要清空var变量,否则下面Execute出问题
			m_pConnection->Execute(_T("TRUNCATE TABLE 地下水系统水量平衡输出表"),
				&var, adCmdText);
			m_pConnection->Execute(_T("TRUNCATE TABLE 含水层水量平衡输出表"),
				&var, adCmdText);
			m_pConnection->Execute(_T("TRUNCATE TABLE 网格单元_水量平衡输出表"),
				&var, adCmdText);
			m_pConnection->Execute(_T("TRUNCATE TABLE 网格单元_水位输出表"),
				&var, adCmdText);
			m_pConnection->Execute(_T("TRUNCATE TABLE 网格单元_水位降深输出表"),
				&var, adCmdText);
			m_pConnection->Execute(_T("TRUNCATE TABLE 网格单元_流量输出表"),
				&var, adCmdText);
			m_pConnection->Execute(_T("TRUNCATE TABLE 网格单元观测点输出表"),
				&var, adCmdText);
			m_pConnection->Execute(_T("TRUNCATE TABLE 水库_水位库容曲线数据输出表"),
				&var, adCmdText);
			m_pConnection->Execute(_T("TRUNCATE TABLE 湖泊_水位库容曲线数据输出表"),
				&var, adCmdText);
			m_pConnection->Execute(_T("TRUNCATE TABLE 湖泊_水量平衡输出表"),
				&var, adCmdText);
			m_pConnection->Execute(_T("TRUNCATE TABLE 季节性河流_河道单元水量平衡输出表"),
				&var, adCmdText);
			m_pConnection->Execute(_T("TRUNCATE TABLE 季节性河流_河段单元水量平衡输出表"),
				&var, adCmdText);
			m_pConnection->Execute(_T("TRUNCATE TABLE 季节性河流_模拟次序输出表"),
				&var, adCmdText);
			m_pConnection->Execute(_T("TRUNCATE TABLE 统计分区_水量平衡输出表"),
				&var, adCmdText);
			m_pConnection->Execute(_T("TRUNCATE TABLE 统计分区_流量关系输出表"),
				&var, adCmdText);
			m_pConnection->Execute(_T("TRUNCATE TABLE 夹层体输出表"),
				&var, adCmdText);
			m_pConnection->Execute(_T("TRUNCATE TABLE 地面沉降_网格单元输出表"),
				&var, adCmdText);
			m_pConnection->Execute(_T("TRUNCATE TABLE 地面沉降_有延迟夹层体观测点输出表"),
				&var, adCmdText);
			m_pConnection->Execute(_T("TRUNCATE TABLE 地面沉降_粗颗粒沉积物输出表"),
				&var, adCmdText);
			m_pConnection->Execute(_T("TRUNCATE TABLE 地面沉降_无延迟夹层体输出表"),
				&var, adCmdText);
			m_pConnection->Execute(_T("TRUNCATE TABLE 地面沉降_有延迟夹层体输出表"),
				&var, adCmdText);
		}
		catch (_com_error *e)
		{
			printf((LPCTSTR)e->ErrorMessage());
		}

		_RecordsetPtr pGDWOutTable;
		_RecordsetPtr pObsOutTable;
		_RecordsetPtr pLyrOutTable;
		_RecordsetPtr pCellBDTable;
		_RecordsetPtr pCellHHTable;
		_RecordsetPtr pCellDDTable;
		_RecordsetPtr pCellFluxTable;
		_RecordsetPtr pLakeOutTable;
		_RecordsetPtr pRegOutTable;
		_RecordsetPtr pRegExcOutTable;
		_RecordsetPtr pStrSegmOutTable;
		_RecordsetPtr pStrRechOutTable;
		_RecordsetPtr pIBSOutTable;
		_RecordsetPtr pSubCellOutTable;
		_RecordsetPtr pObsDIBCellTable;
		_RecordsetPtr pSubCGOutTable;
		_RecordsetPtr pSubNDBOutTable;
		_RecordsetPtr pSubDBOutTable;

		CString strGCellBDTbName;
		CString strObsCellTbName;
		CString strGCellHHTbName;
		CString strGCellDDTbName;
		CString strGCellFluxTbName;
		CString strGLyrTbName;
		CString strGDWTbName;
		CString strLakeTbName;
		CString strRegTbName;
		CString strRegExcTbName;
		CString strStrSegmTbName;
		CString strStrRechTbName;
		CString strIBSTbName;
		CString strSubCellTbName;
		CString strObsDIBCellTbName;
		CString strSubCGTbName;
		CString strSubNDBTbName;
		CString strSubDBTbName;

		// 打开各输出表
		pGDWOutTable.CreateInstance(__uuidof(Recordset));
		pGDWOutTable->CursorLocation = adUseClient;
		pGDWOutTable->CacheSize = 1000;
		pObsOutTable.CreateInstance(__uuidof(Recordset));
		pObsOutTable->CursorLocation = adUseClient;
		pObsOutTable->CacheSize = 1000;
		pLyrOutTable.CreateInstance(__uuidof(Recordset));
		pLyrOutTable->CursorLocation = adUseClient;
		pLyrOutTable->CacheSize = 1000;
		pCellBDTable.CreateInstance(__uuidof(Recordset));
		pCellBDTable->CursorLocation = adUseClient;
		pCellBDTable->CacheSize = 1000;
		pCellHHTable.CreateInstance(__uuidof(Recordset));
		pCellHHTable->CursorLocation = adUseClient;
		pCellHHTable->CacheSize = 1000;
		pCellDDTable.CreateInstance(__uuidof(Recordset));
		pCellDDTable->CursorLocation = adUseClient;
		pCellDDTable->CacheSize = 1000;
		pCellFluxTable.CreateInstance(__uuidof(Recordset));
		pCellFluxTable->CursorLocation = adUseClient;
		pCellFluxTable->CacheSize = 1000;
		pLakeOutTable.CreateInstance(__uuidof(Recordset));
		pLakeOutTable->CursorLocation = adUseClient;
		pLakeOutTable->CacheSize = 1000;
		pRegOutTable.CreateInstance(__uuidof(Recordset));
		pRegOutTable->CursorLocation = adUseClient;
		pRegOutTable->CacheSize = 1000;
		pRegExcOutTable.CreateInstance(__uuidof(Recordset));
		pRegExcOutTable->CursorLocation = adUseClient;
		pRegExcOutTable->CacheSize = 1000;
		pStrSegmOutTable.CreateInstance(__uuidof(Recordset));
		pStrSegmOutTable->CursorLocation = adUseClient;
		pStrSegmOutTable->CacheSize = 1000;
		pStrRechOutTable.CreateInstance(__uuidof(Recordset));
		pStrRechOutTable->CursorLocation = adUseClient;
		pStrRechOutTable->CacheSize = 1000;
		pIBSOutTable.CreateInstance(__uuidof(Recordset));
		pIBSOutTable->CursorLocation = adUseClient;
		pIBSOutTable->CacheSize = 1000;
		pSubCellOutTable.CreateInstance(__uuidof(Recordset));
		pSubCellOutTable->CursorLocation = adUseClient;
		pSubCellOutTable->CacheSize = 1000;
		pObsDIBCellTable.CreateInstance(__uuidof(Recordset));
		pObsDIBCellTable->CursorLocation = adUseClient;
		pObsDIBCellTable->CacheSize = 1000;
		pSubCGOutTable.CreateInstance(__uuidof(Recordset));
		pSubCGOutTable->CursorLocation = adUseClient;
		pSubCGOutTable->CacheSize = 1000;
		pSubNDBOutTable.CreateInstance(__uuidof(Recordset));
		pSubNDBOutTable->CursorLocation = adUseClient;
		pSubNDBOutTable->CacheSize = 1000;
		pSubDBOutTable.CreateInstance(__uuidof(Recordset));
		pSubDBOutTable->CursorLocation = adUseClient;
		pSubDBOutTable->CacheSize = 1000;
		try
		{
			//全网格系统水量平衡输出表
			strGDWTbName = _T("地下水系统水量平衡输出表");
			pGDWOutTable->Open(_T("SELECT * FROM 地下水系统水量平衡输出表"),
				m_pConnection.GetInterfacePtr(), // 获取数据库的IDispatch指针
				adOpenStatic,
				adLockOptimistic,
				adCmdText);

			//网格单元观测点输出表
			strObsCellTbName = _T("网格单元观测点输出表");
			pObsOutTable->Open(_T("SELECT * FROM 网格单元观测点输出表"),
				m_pConnection.GetInterfacePtr(), // 获取数据库的IDispatch指针
				adOpenStatic,
				adLockOptimistic,
				adCmdText);

			//含水层水量平衡输出表
			strGLyrTbName = _T("含水层水量平衡输出表");
			pLyrOutTable->Open(_T("SELECT * FROM 含水层水量平衡输出表"),
				m_pConnection.GetInterfacePtr(), // 获取数据库的IDispatch指针
				adOpenStatic,
				adLockOptimistic,
				adCmdText);

			//网格单元水量平衡输出表
			strGCellBDTbName = _T("网格单元_水量平衡输出表");
			pCellBDTable->Open(_T("SELECT * FROM 网格单元_水量平衡输出表"),
				m_pConnection.GetInterfacePtr(), // 获取数据库的IDispatch指针
				adOpenStatic,
				adLockOptimistic,
				adCmdText);

			//网格单元水位输出表
			strGCellHHTbName = _T("网格单元_水位输出表");
			pCellHHTable->Open(_T("SELECT * FROM 网格单元_水位输出表"),
				m_pConnection.GetInterfacePtr(), // 获取数据库的IDispatch指针
				adOpenStatic,
				adLockOptimistic,
				adCmdText);

			//网格单元水位降深输出表
			strGCellDDTbName = _T("网格单元_水位降深输出表");
			pCellDDTable->Open(_T("SELECT * FROM 网格单元_水位降深输出表"),
				m_pConnection.GetInterfacePtr(), // 获取数据库的IDispatch指针
				adOpenStatic,
				adLockOptimistic,
				adCmdText);

			//网格单元流量输出表
			strGCellFluxTbName = _T("网格单元_流量输出表");
			pCellFluxTable->Open(_T("SELECT * FROM 网格单元_流量输出表"),
				m_pConnection.GetInterfacePtr(), // 获取数据库的IDispatch指针
				adOpenStatic,
				adLockOptimistic,
				adCmdText);

			// 地下水数值模拟分区水量平衡输出表
			strRegTbName = _T("统计分区_水量平衡输出表");
			pRegOutTable->Open(_T("SELECT * FROM 统计分区_水量平衡输出表"),
				m_pConnection.GetInterfacePtr(), // 获取数据库的IDispatch指针
				adOpenStatic,
				adLockOptimistic,
				adCmdText);

			// 地下水数值模拟分区水量交换量输出表
			strRegExcTbName = _T("统计分区_流量关系输出表");
			pRegExcOutTable->Open(_T("SELECT * FROM 统计分区_流量关系输出表"),
				m_pConnection.GetInterfacePtr(), // 获取数据库的IDispatch指针
				adOpenStatic,
				adLockOptimistic,
				adCmdText);

			// 地面沉降_网格单元输出表
			strSubCellTbName = _T("地面沉降_网格单元输出表");
			pSubCellOutTable->Open(_T("SELECT * FROM 地面沉降_网格单元输出表"),
				m_pConnection.GetInterfacePtr(), // 获取数据库的IDispatch指针
				adOpenStatic,
				adLockOptimistic,
				adCmdText);

			// 地面沉降_有延迟夹层体观测点输出表
			strObsDIBCellTbName = _T("地面沉降_有延迟夹层体观测点输出表");
			pObsDIBCellTable->Open(_T("SELECT * FROM 地面沉降_有延迟夹层体观测点输出表"),
				m_pConnection.GetInterfacePtr(), // 获取数据库的IDispatch指针
				adOpenStatic,
				adLockOptimistic,
				adCmdText);

			// 地面沉降_粗颗粒沉积物输出表
			strSubCGTbName = _T("地面沉降_粗颗粒沉积物输出表");
			pSubCGOutTable->Open(_T("SELECT * FROM 地面沉降_粗颗粒沉积物输出表"),
				m_pConnection.GetInterfacePtr(), // 获取数据库的IDispatch指针
				adOpenStatic,
				adLockOptimistic,
				adCmdText);

			// 地面沉降_无延迟夹层体输出表
			strSubNDBTbName = _T("地面沉降_无延迟夹层体输出表");
			pSubNDBOutTable->Open(_T("SELECT * FROM 地面沉降_无延迟夹层体输出表"),
				m_pConnection.GetInterfacePtr(), // 获取数据库的IDispatch指针
				adOpenStatic,
				adLockOptimistic,
				adCmdText);

			// 地面沉降_有延迟夹层体输出表
			strSubDBTbName = _T("地面沉降_有延迟夹层体输出表");
			pSubDBOutTable->Open(_T("SELECT * FROM 地面沉降_有延迟夹层体输出表"),
				m_pConnection.GetInterfacePtr(), // 获取数据库的IDispatch指针
				adOpenStatic,
				adLockOptimistic,
				adCmdText);
		}
		catch (_com_error *e)
		{
			printf((LPCTSTR)e->ErrorMessage());
		}

		//构建各输出表的缓存表
		try
		{
			//建表
			CGridCell::MakeBDDataTable(pCellBDTable, strGCellBDTbName);
			CGridCell::MakeHHDataTable(pCellHHTable, strGCellHHTbName);
			CGridCell::MakeDDDataTable(pCellDDTable, strGCellDDTbName);
			CGridCell::MakeFlowDataTable(pCellFluxTable, strGCellFluxTbName);
			CGridLyr::MakeDataTable(pLyrOutTable, strGLyrTbName);
			CGDWater::MakeDataTable(pGDWOutTable, strGDWTbName);
			CGDWater::MakeObsCellTable(pObsOutTable, strObsCellTbName);
			CRegion::MakeBDDataTable(pRegOutTable, strRegTbName);
			CRegion::MakeExcDataTable(pRegExcOutTable, strRegExcTbName);
			CLandSub::MakeSubCellDataTable(pSubCellOutTable, strSubCellTbName);
			CCGSediment::MakeCGDataTable(pSubCGOutTable, strSubCGTbName);
			CNDelayIntBed::MakeNDBDataTable(pSubNDBOutTable, strSubNDBTbName);
			CDelayIntBed::MakeDBDataTable(pSubDBOutTable, strSubDBTbName);
			CDelayIntBed::MakeObsDIBCellTable(pObsDIBCellTable, strObsDIBCellTbName);
		}
		catch (_com_error *e)
		{
			printf((LPCTSTR)e->ErrorMessage());
		}
	}
}

void CGDWater::GDWOutPutTXT()
{
	//输出地下水系统水量平衡结果(TXT)

	fprintf(m_GDWOutPutFile.m_pStream, _T(" %-10d"), m_iPer);
	if (m_IGDWBDPRN == 1)
	{
		fprintf(m_GDWOutPutFile.m_pStream, _T(" %-10d"), m_iStep);
	}
	else
	{
		fprintf(m_GDWOutPutFile.m_pStream, _T(" %-10d"), long(-1));
	}
	fprintf(m_GDWOutPutFile.m_pStream, _T(" %-16.8g"), m_CumTLen);
	for (size_t i = 0; i < m_ItemNamAry.size(); i++)
	{
		fprintf(m_GDWOutPutFile.m_pStream, _T(" %-16.8g"), m_ResultAry[i].cum_val);
	}
	fprintf(m_GDWOutPutFile.m_pStream, _T("\n"));
}

void CGDWater::OutPutFlowTXT()
{
	//输出网格单元流量模拟结果(TXT)

	long iRow, iCol, iLyr;
	CGridCell* pGridCell = NULL;
	CString strText;
	double OutVal;
	size_t i;
	//逐层逐行逐列写入单元的流量输出数据
	for (iLyr = 0; iLyr < m_NUMLYR; iLyr++)
	{
		for (iRow = 0; iRow < m_NUMROW; iRow++)
		{
			for (iCol = 0; iCol < m_NUMCOL; iCol++)
			{
				pGridCell = m_pGDLayers[iLyr]->m_pGridCellAry[iRow][iCol];
				fprintf(m_FLowOutFile.m_pStream, _T(" %-10d"), m_iPer);
				if (m_ICELLFLPRN == 1)
				{
					//时段输出控制
					fprintf(m_FLowOutFile.m_pStream, _T(" %-10d"), m_iStep);
					fprintf(m_FLowOutFile.m_pStream, _T(" %-16.8f"), m_DeltT);
				}
				else
				{
					//应力期输出控制
					fprintf(m_FLowOutFile.m_pStream, _T(" %-10d"), long(-1));
					fprintf(m_FLowOutFile.m_pStream, _T(" %-16.8f"), m_PerCumDeltT);
				}
				fprintf(m_FLowOutFile.m_pStream, _T(" %-10d"), pGridCell->m_ILYR);
				fprintf(m_FLowOutFile.m_pStream, _T(" %-10d"), pGridCell->m_IROW);
				fprintf(m_FLowOutFile.m_pStream, _T(" %-10d"), pGridCell->m_ICOL);
				fprintf(m_FLowOutFile.m_pStream, _T(" %-16.8f"), pGridCell->m_XCord);
				fprintf(m_FLowOutFile.m_pStream, _T(" %-16.8f"), pGridCell->m_YCord);
				for (i = 0; i < 3; i++)
				{
					if (pGridCell->m_INIIBOUND == 0)
					{
						//如果是初始无效网格单元
						OutVal = 0.0;
					}
					else
					{
						//如果是初始有效网格单元
						if (m_ICELLFLPRN == 1)
						{
							//按模拟时段输出
							OutVal = pGridCell->m_FlowRsltAry[i].step_val;
						}
						else
						{
							//按应力期输出
							OutVal = pGridCell->m_FlowRsltAry[i].cum_val;
							//清零当前应力期累计统计结果
							pGridCell->m_FlowRsltAry[i].cum_val = 0.0;
						}
					}
					fprintf(m_FLowOutFile.m_pStream, _T(" %-16.8e"), OutVal);
				}
				fprintf(m_FLowOutFile.m_pStream, _T("\n"));
			}
		}
	}
}

void CGDWater::OutPutFlowDB()
{
	//输出网格单元流量模拟结果(数据库)

	long iLyr;
	for (iLyr = 0; iLyr < m_NUMLYR; iLyr++)
	{
		m_pGDLayers[iLyr]->OutPutLyrCellFlow(m_ICELLFLPRN, m_iPer, m_iStep, GetPerLen(), m_DeltT);
	}
}

void CGDWater::OutPutCellBDTXT()
{
	//输出网格单元水量平衡模拟结果(TXT)

	long iRow, iCol, iLyr;
	CGridCell* pGridCell = NULL;
	double OutVal;
	size_t i;
	//逐层逐行逐列写入单元的各项平衡量输出数据
	for (iLyr = 0; iLyr < m_NUMLYR; iLyr++)
	{
		for (iRow = 0; iRow < m_NUMROW; iRow++)
		{
			for (iCol = 0; iCol < m_NUMCOL; iCol++)
			{
				pGridCell = m_pGDLayers[iLyr]->m_pGridCellAry[iRow][iCol];
				fprintf(m_CellBDOutFile.m_pStream, _T(" %-10d"), m_iPer);
				if (m_ICELLBDPRN == 1)
				{
					//时段输出控制
					fprintf(m_CellBDOutFile.m_pStream, _T(" %-10d"), m_iStep);
					fprintf(m_CellBDOutFile.m_pStream, _T(" %-16.8f"), m_DeltT);
				}
				else
				{
					//应力期输出控制
					fprintf(m_CellBDOutFile.m_pStream, _T(" %-10d"), long(-1));
					fprintf(m_CellBDOutFile.m_pStream, _T(" %-16.8f"), m_PerCumDeltT);
				}
				fprintf(m_CellBDOutFile.m_pStream, _T(" %-10d"), pGridCell->m_ILYR);
				fprintf(m_CellBDOutFile.m_pStream, _T(" %-10d"), pGridCell->m_IROW);
				fprintf(m_CellBDOutFile.m_pStream, _T(" %-10d"), pGridCell->m_ICOL);
				for (i = 0; i < CGridCell::m_ItemMeanAry.size(); i++)
				{
					if (pGridCell->m_INIIBOUND == 0)
					{
						//如果是初始无效网格单元
						OutVal = 0.0;
					}
					else
					{
						//如果是初始有效网格单元
						if (m_ICELLBDPRN == 1)
						{
							//按模拟时段输出
							OutVal = pGridCell->m_ResultAry[i].step_val;
						}
						else
						{
							//按应力期输出
							OutVal = pGridCell->m_ResultAry[i].cum_val;
							//清零当前应力期累计统计结果
							pGridCell->m_ResultAry[i].cum_val = 0.0;
						}
					}
					fprintf(m_CellBDOutFile.m_pStream, _T(" %-16.8e"), OutVal);
				}
				fprintf(m_CellBDOutFile.m_pStream, _T("\n"));
			}
		}
	}
}

void CGDWater::OutPutCellBDDB()
{
	//输出网格单元水量平衡模拟结果(数据库)

	long iLyr;
	for (iLyr = 0; iLyr < m_NUMLYR; iLyr++)
	{
		m_pGDLayers[iLyr]->OutPutLyrCellBD(m_ICELLBDPRN, m_iPer, m_iStep, GetPerLen(), m_DeltT);
	}
}

void CGDWater::OutPutObsCellDB()
{
	//输出网格单元观测点模拟结果(数据库)

	size_t i, j, nIterm;
	CGridCell* pGridCell = NULL;
	double DeltT = GDWMOD.GetDeltT();
	nIterm = 7 + CGridCell::m_ItemNamAry.size();
	long SimMthd, LyrCon;
	double HeadDry, HeadMin;
	HeadDry = GDWMOD.GetHNOFLO();
	SimMthd = GDWMOD.GetSIMMTHD();
	String^ strTemp;
	double ItermVal;
	try
	{
		DataRow^ dr;
		for (i = 0; i < m_pObsCellAry.size(); i++)
		{
			pGridCell = m_pObsCellAry[i];
			for (j = 0; j < nIterm; j++)
			{
				dr = m_pMemObsCellTable->GetDataTable()->NewRow();
				dr[0] = m_iPer;
				dr[1] = m_iStep;
				dr[2] = m_CumTLen;
				dr[3] = m_DeltT;
				dr[4] = pGridCell->m_ILYR;
				dr[5] = pGridCell->m_IROW;
				dr[6] = pGridCell->m_ICOL;
				dr[7] = j + 1;
				if (j == 0)
				{
					strTemp = gcnew String(_T("HEAD"));
					if (pGridCell->m_IBOUND == 0)
					{
						//如果网单元无效
						ItermVal = HeadDry;
					}
					else
					{
						//如果网格单元有效
						ItermVal = pGridCell->m_hNew;
						//检查特殊情况
						SimMthd = GDWMOD.GetSIMMTHD();
						if (SimMthd == 1)
						{
							//如果为全有效单元法
							LyrCon = GDWMOD.GetLyrCon(pGridCell->m_ILYR);
							if (LyrCon == 1 || LyrCon == 3)
							{
								//如果网格单元位于导水系数可变含水层
								if (pGridCell->m_DryHint == 0)
								{
									//如果为可疏干单元
									if (pGridCell->m_hNew <= pGridCell->m_BOT)
									{
										//如果单元水头低于或等于底板高程
										ItermVal = HeadDry;
									}
								}
								else
								{
									//如果为不可疏干单元
									HeadMin = pGridCell->m_BOT + GDWMOD.GetMinHSat();
									if (pGridCell->m_hNew < HeadMin)
									{
										//如果单元水头低于底板高程加上饱和厚度阈值
										ItermVal = HeadMin;
									}
								}
							}
						}
					}
				}
				else if (j == 1)
				{
					strTemp = gcnew String(_T("FFRONT"));
					ItermVal = pGridCell->m_FlowFront * DeltT;
				}
				else if (j == 2)
				{
					strTemp = gcnew String(_T("FBACK"));
					ItermVal = pGridCell->m_FlowBack * DeltT;
				}
				else if (j == 3)
				{
					strTemp = gcnew String(_T("FLEFT"));
					ItermVal = pGridCell->m_FlowLeft * DeltT;
				}
				else if (j == 4)
				{
					strTemp = gcnew String(_T("FRIGHT"));
					ItermVal = pGridCell->m_FlowRight * DeltT;
				}
				else if (j == 5)
				{
					strTemp = gcnew String(_T("FUP"));
					ItermVal = pGridCell->m_FlowUp * DeltT;
				}
				else if (j == 6)
				{
					strTemp = gcnew String(_T("FDOWN"));
					ItermVal = pGridCell->m_FlowDown * DeltT;
				}
				else
				{
					strTemp = gcnew String(CGridCell::m_ItemNamAry[j - 7]);
					ItermVal = pGridCell->m_ResultAry[j - 7].step_val;
				}
				dr[8] = strTemp;
				dr[9] = ItermVal;
				m_pMemObsCellTable->GetDataTable()->Rows->Add(dr);
			}
		}
	}
	catch (_com_error* e)
	{
		printf((LPCTSTR)e->ErrorMessage());
	}
}

void CGDWater::OutPutObsCellTXT()
{
	//输出网格单元观测点模拟结果(TXT)

	size_t i, j;
	CGridCell* pGridCell = NULL;
	double DeltT = GDWMOD.GetDeltT();
	long SimMthd, LyrCon;
	double HeadDry, HeadMin;
	HeadDry = GDWMOD.GetHNOFLO();
	SimMthd = GDWMOD.GetSIMMTHD();
	double HeadOut;
	for (i = 0; i < m_pObsCellAry.size(); i++)
	{
		pGridCell = m_pObsCellAry[i];
		fprintf(m_OBSCellOutFile.m_pStream, _T(" %-10d"), m_iPer);
		fprintf(m_OBSCellOutFile.m_pStream, _T(" %-10d"), m_iStep);
		fprintf(m_OBSCellOutFile.m_pStream, _T(" %-16.8g"), m_CumTLen);
		fprintf(m_OBSCellOutFile.m_pStream, _T(" %-16.8g"), m_DeltT);
		fprintf(m_OBSCellOutFile.m_pStream, _T(" %-10d"), pGridCell->m_ILYR);
		fprintf(m_OBSCellOutFile.m_pStream, _T(" %-10d"), pGridCell->m_IROW);
		fprintf(m_OBSCellOutFile.m_pStream, _T(" %-10d"), pGridCell->m_ICOL);
		if (pGridCell->m_IBOUND == 0)
		{
			//如果网单元无效
			HeadOut = HeadDry;
		}
		else
		{
			//如果网格单元有效
			HeadOut = pGridCell->m_hNew;
			//检查特殊情况
			SimMthd = GDWMOD.GetSIMMTHD();
			if (SimMthd == 1)
			{
				//如果为全有效单元法
				LyrCon = GDWMOD.GetLyrCon(pGridCell->m_ILYR);
				if (LyrCon == 1 || LyrCon == 3)
				{
					//如果网格单元位于导水系数可变含水层
					if (pGridCell->m_DryHint == 0)
					{
						//如果为可疏干单元
						if (pGridCell->m_hNew <= pGridCell->m_BOT)
						{
							//如果单元水头低于或等于底板高程
							HeadOut = HeadDry;
						}
					}
					else
					{
						//如果为不可疏干单元
						HeadMin = pGridCell->m_BOT + GDWMOD.GetMinHSat();
						if (pGridCell->m_hNew < HeadMin)
						{
							//如果单元水头低于底板高程加上饱和厚度阈值
							HeadOut = HeadMin;
						}
					}
				}
			}
		}
		fprintf(m_OBSCellOutFile.m_pStream, _T(" %-16.8g"), HeadOut);
		fprintf(m_OBSCellOutFile.m_pStream, _T(" %-16.8g"), pGridCell->m_FlowFront * DeltT);
		fprintf(m_OBSCellOutFile.m_pStream, _T(" %-16.8g"), pGridCell->m_FlowBack * DeltT);
		fprintf(m_OBSCellOutFile.m_pStream, _T(" %-16.8g"), pGridCell->m_FlowLeft * DeltT);
		fprintf(m_OBSCellOutFile.m_pStream, _T(" %-16.8g"), pGridCell->m_FlowRight * DeltT);
		fprintf(m_OBSCellOutFile.m_pStream, _T(" %-16.8g"), pGridCell->m_FlowUp * DeltT);
		fprintf(m_OBSCellOutFile.m_pStream, _T(" %-16.8g"), pGridCell->m_FlowDown * DeltT);
		for (j = 0; j < CGridCell::m_ItemNamAry.size(); j++)
		{
			fprintf(m_OBSCellOutFile.m_pStream, _T(" %-16.8g"), pGridCell->m_ResultAry[j].step_val);
		}
		fprintf(m_OBSCellOutFile.m_pStream, _T("\n"));
	}
}

void CGDWater::GDWOutPutDB()
{
	//输出地下水系统水量平衡结果(数据库)

	size_t i = 0;
	size_t size = m_ItemNamAry.size();
	String^ strTemp;
	try
	{
		DataRow^ dr;
		for (i = 0; i < size; i++)
		{
			dr = m_pMemDataTable->GetDataTable()->NewRow();
			dr[0] = m_iPer;
			if (m_IGDWBDPRN == 1)
			{
				//按每个计算时段输出
				dr[1] = m_iStep;
			}
			else
			{
				dr[1] = long(-1);
			}
			dr[2] = m_CumTLen;
			dr[3] = i + 1;
			strTemp = gcnew String(m_ItemNamAry[i]);	
			dr[4] = strTemp;
			dr[5] = m_ResultAry[i].cum_val;
			m_pMemDataTable->GetDataTable()->Rows->Add(dr);
		}
	}
	catch (_com_error *e)
	{
		printf((LPCTSTR)e->ErrorMessage());
	}
}