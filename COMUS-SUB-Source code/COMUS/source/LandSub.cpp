#include "StdAfx.h"
#include "ClassInclude.h"

///////////////////////////////////
//可压缩性沉积物网格单元类函数定义
CCompSedmCell::CCompSedmCell()
{
	m_IROW = 0;                                             //网格的行号（从1开始编号）
	m_ICOL = 0;                                             //网格的列号（从1开始编号）
	m_SEDMCR = 0.0;                                         //网格单元处计算可压缩性沉积物弹性贮水率时所用的系数(-)
	m_COM = 0.0;                                            //网格单元处可压缩性沉积物的前期压实量(L). 用来统计总地面沉降量.
}

CCompSedmCell::~CCompSedmCell()
{

}

///////////////////////////////////
//粗颗粒沉积物网格单元类函数定义
CCoGrainCell::CCoGrainCell()
{
	m_CGTHK = 0.0;                                         //网格单元处粗颗粒沉积物的总厚度(L)
	m_CGTHKOLD = 0.0;                                      //网格单元处粗颗粒沉积物上个时段末的总厚度(L)
	m_CGTHETA = 0.0;                                       //网格单元处粗颗粒沉积物的孔隙率(即孔隙体积与总体积之比)(-)
	m_CGTHETAOLD = 0.0;                                    //网格单元处粗颗粒沉积物上个时段的孔隙率(即孔隙体积与总体积之比)(-)
	m_CGSKE = 0.0;                                         //网格单元处粗颗粒沉积物当前的弹性贮水系数(-)
}

CCoGrainCell::~CCoGrainCell()
{

}

void CCoGrainCell::UpdatePar(long iLyr, double CGCOM)
{
	//更新粗颗粒沉积物孔隙率和厚度

	m_CGTHK = m_CGTHKOLD - CGCOM;
	m_CGTHETA = 1.0 - (1.0 - m_CGTHETAOLD) * m_CGTHKOLD / m_CGTHK;
	if (m_CGTHETA <= 0.0 || m_CGTHETA >= 1.0)
	{
		//孔隙率小于零了(压实过度).报错
		printf(_T("\n"));
		printf(_T("网格单元(Lyr=%d,Row=%d,Col=%d)处粗颗粒沉积物模拟的孔隙率(%f)小于等于0或大于等于1，请检查合理性!"),
			iLyr, m_IROW, m_ICOL, m_CGTHETA);
		PauAndQuit();
	}
	if (m_CGTHK <= 0.0)
	{
		//孔隙率小于零了(压实过度).报错
		printf(_T("\n"));
		printf(_T("网格单元(Lyr=%d,Row=%d,Col=%d)处粗颗粒沉积物模拟的厚度(%f)小于等于零，请检查合理性!"),
			iLyr, m_IROW, m_ICOL, m_CGTHK);
		PauAndQuit();
	}
}

///////////////////////////////////
//无延迟夹层体类网格单元类函数定义
CNDIntBedCell::CNDIntBedCell()
{
	m_IBTHK = 0.0;           //网格单元处无延迟夹层体当前的总厚度(L)
	m_IBTHKOLD = 0.0;        //网格单元处无延迟夹层体上个时段末的总厚度(L)
	m_IBTHETA = 0.0;         //网格单元处无延迟夹层体当前的孔隙率(即孔隙体积与总体积之比)(-)
	m_IBTHETAOLD = 0.0;      //网格单元处无延迟夹层体上个时段末的孔隙率(即孔隙体积与总体积之比)(-)
	m_IBCC = 0.0;            //计算非弹性贮水率时所用的系数(-)
	m_PCS = 0.0;             //网格单元处无延迟夹层体的前期压缩固结应力(L), 注: 底板处
	m_SKE = 0.0;             //网格单元处无延迟夹层体当前的弹性贮水率(1/L)
	m_SKV = 0.0;             //网格单元处无延迟夹层体当前的非弹性贮水率(1/L)
}

CNDIntBedCell::~CNDIntBedCell()
{

}

void CNDIntBedCell::UpdatePar(long iLyr, long DBedID, double IBTCOM)
{
	//更新夹层体各节点单元孔隙率和厚度

	m_IBTHK = m_IBTHKOLD - IBTCOM;
	m_IBTHETA = 1.0 - (1.0 - m_IBTHETAOLD) * m_IBTHKOLD / m_IBTHK;
	if (m_IBTHETA <= 0.0 || m_IBTHETA >= 1.0)
	{
		//孔隙率小于零了(压实过度).报错
		printf(_T("\n"));
		printf(_T("网格单元(Lyr=%d,Row=%d,Col=%d)处编号为%d的无延迟夹层体模拟的孔隙率(%f)小于等于0或大于等于1，请检查合理性!"),
			iLyr, m_IROW, m_ICOL, DBedID, m_IBTHETA);
		PauAndQuit();
	}
	if (m_IBTHK <= 0.0)
	{
		//孔隙率小于零了(压实过度).报错
		printf(_T("\n"));
		printf(_T("网格单元(Lyr=%d,Row=%d,Col=%d)处编号为%d的无延迟夹层体模拟的厚度(%f)小于等于零，请检查合理性!"),
			iLyr, m_IROW, m_ICOL, DBedID, m_IBTHK);
		PauAndQuit();
	}
}

///////////////////////////////////
//有延迟夹层体类网格单元类函数定义
CDIntBedCell::CDIntBedCell()
{
	m_RNB = 0.0;             //网格单元处有延迟夹层体的等效夹层体数量(L). 该数值小于1时，认为单元处无该夹层体的分布.
	m_DSH = 0.0;             //网格单元处等效夹层体的初始水头(L). 该水头值将赋值给等效夹层体的各个空间离散点
	m_DHC = 0.0;             //网格单元处等效夹层体的初始预固结应力,或初始预固结应力与初始有效应力之差(以水柱高计)(L)
	m_IBCC = 0.0;            //计算非弹性贮水率时所用的系数(-)
	m_IMZ = 0;               //网格单元处等效夹层体对应的介质带的编号(编号从0~NMZ)
	m_pMatZone = NULL;       //网格单元处等效夹层体对应的介质带对象指针
	m_Node =0;               //等效夹层体半厚度离散或全厚度离散时的节点单元个数
	m_DeltZ = 0.0;           //等效夹层体各节点之间的的垂向离散间距(L)
	m_KDZ = 0.0;             //等效夹层体的垂向渗透系数除以垂向离散间距的值(1/L)
	m_CoupItem = 0.0;        //等效夹层体与三维地下水流方程的耦合项(L/T)
	m_TotComChg = 0.0;       //时段内等效夹层体的总压实/膨胀量(L) 
	m_ElsComChgR = 0.0;      //时段内等效夹层体的弹性压实/膨胀流量(L/T) 
	m_InElsComChgR = 0.0;    //时段内等效夹层体的非弹性压实/膨胀流量(L/T) 
	m_WtComChgR =0.0;        //时段内等效夹层体内水体的压实/膨胀流量(L/T) 
	m_BndHead = 0.0;         //当前求解计算夹层体水头扩散方程时所用到的边界处水头(L)
	m_SumStaIn = 0.0;        //等效夹层体贮水累计释放的水量(L)
	m_SumStaOut = 0.0;       //等效夹层体贮水累计吸收的水量(L)
	m_SumFluxIn = 0.0;       //从含水层累计进入等效夹层体的水量(L)
	m_SumFluxOut = 0.0;      //从等效夹层体累计排出给含水层的水量(L)
	m_RERR = 0.0;            //等效夹层体水头扩散方程模拟的相对水量平衡误差(%)
	m_AryHHNew = NULL;       //等效夹层体水流方程的当前迭代计算水头数组
	m_AryHHPre = NULL;       //等效夹层体水流方程的前次迭代计算水头数组
	m_AryHHOld = NULL;       //等效夹层体水流方程的时段初水头数组
	m_AryHHC = NULL;         //等效夹层体各离散节点的预固结应力数组
	m_AryHEST = NULL;        //等效夹层体各离散节点时段末的有效应力数组
	m_AryHESTOLD = NULL;     //等效夹层体各离散节点时段初的有效应力数组
	m_ArySKE = NULL;         //等效夹层体各离散节点处的综合弹性贮水参数数组
	m_ArySKV = NULL;         //等效夹层体各离散节点处的综合非弹性贮水参数数组
	m_AryDU = NULL;          //等效夹层体水流方程的上对角线项数组
	m_AryDA = NULL;          //等效夹层体水流方程的主对角线项数组
	m_AryR = NULL;           //等效夹层体水流方程的右端项数组
	m_AryThic = NULL;        //等效夹层体各节点单元当前的厚度(L)
	m_AryThicOld = NULL;     //等效夹层体各节点单元上个时段末的厚度(L)
	m_AryTheta = NULL;       //等效夹层体各节点单元当前的孔隙率(L)
	m_AryThetaOld = NULL;    //等效夹层体各节点单元上个时段末的孔隙率(L)
	m_AryNodTCom = NULL;     //等效夹层体各节点单元当前的压实量(L)
	m_AryNodTComPre = NULL;  //等效夹层体各节点单元当前的压实量(L)
	m_AryKz = NULL;          //等效夹层体各节点单元当前的垂向渗透系数(L/T)
	m_AryKzOld = NULL;       //等效夹层体各节点单元上个时段末的垂向渗透系数(L/T)
	m_AryKDZ = NULL;         //等效夹层体各节点单元上平均垂向渗透系数除以节点间距的值(1/L)
	m_AryZOff = NULL;        //等效夹层体各节点单元中心相对于夹层体中心的距离(L)
}

CDIntBedCell::~CDIntBedCell()
{
	//删除开辟的一维数组

	if (m_AryHHNew != NULL)
	{
		delete[] m_AryHHNew;
		m_AryHHNew = NULL;
	}
	if (m_AryHHPre != NULL)
	{
		delete[] m_AryHHPre;
		m_AryHHPre = NULL;
	}
	if (m_AryHHOld != NULL)
	{
		delete[] m_AryHHOld;
		m_AryHHOld = NULL;
	}
	if (m_AryHHC != NULL)
	{
		delete[] m_AryHHC;
		m_AryHHC = NULL;
	}
	if (m_AryHEST != NULL)
	{
		delete[] m_AryHEST;
		m_AryHEST = NULL;
	}
	if (m_AryHESTOLD != NULL)
	{
		delete[] m_AryHESTOLD;
		m_AryHESTOLD = NULL;
	}
	if (m_ArySKE != NULL)
	{
		delete[] m_ArySKE;
		m_ArySKE = NULL;
	}
	if (m_ArySKV != NULL)
	{
		delete[] m_ArySKV;
		m_ArySKV = NULL;
	}
	if (m_AryDU != NULL)
	{
		delete[] m_AryDU;
		m_AryDU = NULL;
	}
	if (m_AryDA != NULL)
	{
		delete[] m_AryDA;
		m_AryDA = NULL;
	}
	if (m_AryR != NULL)
	{
		delete[] m_AryR;
		m_AryR = NULL;
	}
	if (m_AryThic != NULL)
	{
		delete[] m_AryThic;
		m_AryThic = NULL;
	}
	if (m_AryThicOld != NULL)
	{
		delete[] m_AryThicOld;
		m_AryThicOld = NULL;
	}
	if (m_AryTheta != NULL)
	{
		delete[] m_AryTheta;
		m_AryTheta = NULL;
	}
	if (m_AryThetaOld != NULL)
	{
		delete[] m_AryThetaOld;
		m_AryThetaOld = NULL;
	}
	if (m_AryNodTCom != NULL)
	{
		delete[] m_AryNodTCom;
		m_AryNodTCom = NULL;
	}
	if (m_AryNodTComPre != NULL)
	{
		delete[] m_AryNodTComPre;
		m_AryNodTComPre = NULL;
	}
	if (m_AryKz != NULL)
	{
		delete[] m_AryKz;
		m_AryKz = NULL;
	}
	if (m_AryKzOld != NULL)
	{
		delete[] m_AryKzOld;
		m_AryKzOld = NULL;
	}
	if (m_AryKDZ != NULL)
	{
		delete[] m_AryKDZ;
		m_AryKDZ = NULL;
	}
	if (m_AryZOff != NULL)
	{
		delete[] m_AryZOff;
		m_AryZOff = NULL;
	}
}

void CDIntBedCell::InitSimulation(long DISOPT, long KMOPT)
{
	//进行模拟之前的初始化

	//先开辟计算数组
	assert(m_Node > 0);
	try
	{
		m_AryHHC = new double[m_Node]();
		m_AryHEST = new double[m_Node]();
		m_AryHESTOLD = new double[m_Node]();
		m_AryHHNew = new double[m_Node]();
		m_AryHHPre = new double[m_Node]();
		m_AryHHOld = new double[m_Node]();
		m_ArySKE = new double[m_Node]();
		m_ArySKV = new double[m_Node]();
		m_AryR = new double[m_Node]();
		m_AryDU = new double[m_Node]();
		m_AryDA = new double[m_Node]();
		m_AryThic = new double[m_Node]();
		m_AryThicOld = new double[m_Node]();
		m_AryTheta = new double[m_Node]();
		m_AryThetaOld = new double[m_Node]();
		m_AryNodTCom = new double[m_Node]();
		if (DISOPT == 1)
		{
			m_AryZOff = new double[m_Node]();
		}
		if (KMOPT == 1)
		{
			m_AryNodTComPre = new double[m_Node]();
			m_AryKz = new double[m_Node]();
			m_AryKzOld = new double[m_Node]();
			m_AryKDZ = new double[m_Node]();
		}
	}
	catch (bad_alloc &e)
	{
		printf(e.what());
		exit(1);
	}

	//计算垂向离散间距(m_DeltZ目前还是等效厚度)
	if (DISOPT == 1)
	{
		//全厚度离散格式
		m_DeltZ = m_DeltZ / double(m_Node);
	}
	else
	{
		//半厚度离散格式
		m_DeltZ = m_DeltZ / 2.0 / (double(m_Node) - 0.5);
	}

	//计算等效夹层体的垂向渗透系数除以垂向离散间距的值(1/L)
	m_KDZ = m_pMatZone->m_MZVK / m_DeltZ;

	//给数组赋初值
	long i;
	for (i = 0; i < m_Node; i++)
	{
		m_AryHHNew[i] = m_DSH;
		m_AryHHOld[i] = m_DSH;
		if (i == m_Node - 1 && DISOPT == 2)
		{
			//半厚度离散格式最后一个节点
			m_AryThic[i] = 0.5 * m_DeltZ;
		}
		else
		{
			m_AryThic[i] = m_DeltZ;
		}
		m_AryThicOld[i] = m_AryThic[i];
		m_AryTheta[i] = m_pMatZone->m_MZTHETA;
		m_AryThetaOld[i] = m_AryTheta[i];
		if (KMOPT == 1)
		{
			m_AryKz[i] = m_pMatZone->m_MZVK;
			m_AryKzOld[i] = m_pMatZone->m_MZVK;
			m_AryKDZ[i] = m_KDZ;
		}
	}

	if (DISOPT == 1)
	{
		//若为全厚度离散格式, 计算等效夹层体各节点相对于中心节点的距离(m)
        //以中心点位置为0, 上方节点距离中心点的距离为负值
		if (m_Node % 2 != 0)
		{
			//如果节点单元数量是奇数
			long CenterNode = long(m_Node / 2.0);
			for (i = 0; i < m_Node; i++)
			{
				m_AryZOff[i] = double(i - CenterNode) * m_DeltZ;
			}
		}
		else
		{
			//如果节点单元数量是偶数
			long HalfNode = long(m_Node / 2.0);
			for (i = 0; i < m_Node; i++)
			{
				m_AryZOff[i] = double(i - HalfNode) * m_DeltZ + 0.5 * m_DeltZ;
			}
		}
	}

	//清零统计变量
	m_SumStaIn = 0.0;        //等效夹层体贮水累计释放的水量(L)
	m_SumStaOut = 0.0;       //等效夹层体贮水累计吸收的水量(L)
	m_SumFluxIn = 0.0;       //从含水层累计进入等效夹层体的水量(L)
	m_SumFluxOut = 0.0;      //从等效夹层体累计排出给含水层的水量(L)
	m_RERR = 0.0;            //等效夹层体水头扩散方程模拟的相对水量平衡误差(%)
}

long CDIntBedCell::GauSolve(double DA[], double DU[], double R[], double X[],
	double X_1[], long NumIter, double& Abs_BigHChg0)
{
	//采用高斯消元法对三对角矩阵(必须是对称阵)进行求解
	//注: DA为主对角线数组; DU为上对角线数组; R为右端项
	//求解结果放到X数组中 

	long i, j, lcnvg;
	double F, C, Abs_HChg, Abs_BigHChg = 0.0;

	//1.对矩阵进行分解和处理
	for (i = 0; i < m_Node - 1; i++)
	{
		F = 1.0 / DA[i];
		C = DU[i] * F;
		j = i + 1;
		DA[j] = DA[j] - C * DU[i];
		DU[i] = C;
		R[j] = R[j] - C * R[i];
		R[i] = R[i] * F;
	}
	X[m_Node - 1] = R[m_Node - 1] / DA[m_Node - 1];

	//2.回代求解
	for (i = m_Node - 2; i >= 0; i--)
	{
		X[i] = R[i] - DU[i] * X[i + 1];
	}

	//3.判断是否收敛
	lcnvg = 0;
	if (NumIter == 1)
	{
		Abs_BigHChg0 = 1E+30;
	}
	for (i = 0; i < m_Node; i++)
	{
		Abs_HChg = fabs(X[i] - X_1[i]);
		if (Abs_HChg > Abs_BigHChg)
		{
			Abs_BigHChg = Abs_HChg;
		}
	}
	if (Abs_BigHChg < 1E-8)
	{
		lcnvg = 1;
	}
	else
	{
		if (NumIter > 3 && Abs_BigHChg - Abs_BigHChg0 <= 0.0)
		{
			lcnvg = 1;
		}
	}
	Abs_BigHChg0 = Abs_BigHChg;
	return lcnvg;
}

void CDIntBedCell::UpThetaAndThic(long iLyr, long DBedID)
{
	//更新夹层体各节点单元孔隙率和厚度

	for (long i = 0; i < m_Node; i++)
	{
		if (m_AryNodTCom[i] != 0.0)
		{
			m_AryThic[i] = m_AryThicOld[i] - m_AryNodTCom[i];
			m_AryTheta[i] = 1.0 - (1.0 - m_AryThetaOld[i]) *  m_AryThicOld[i] / m_AryThic[i];
			//注:m_AryNodTCom[i]必须清零
			m_AryNodTCom[i] = 0.0;
			if ((m_AryTheta[i] <= 0.0 || m_AryTheta[i] >= 1.0) || m_AryThic[i] <= 0.0)
			{
				//压实过度
				if (m_AryTheta[i] <= 0.0 || m_AryTheta[i] >= 1.0)
				{
					printf(_T("\n"));
					printf(_T("网格单元(Lyr=%d,Row=%d,Col=%d)处编号为%d的有延迟夹层体第%d个节点模拟的孔隙率(%f)小于等于零或大于等于1，请检查合理性!"),
						iLyr, m_IROW, m_ICOL, DBedID, i + 1, m_AryTheta[i]);
					PauAndQuit();
				}
				if (m_AryThic[i] <= 0.0)
				{
					printf(_T("\n"));
					printf(_T("网格单元(Lyr=%d,Row=%d,Col=%d)处编号为%d的有延迟夹层体第%d个节点模拟的厚度(%f)小于等于零，请检查合理性!"),
						iLyr, m_IROW, m_ICOL, DBedID, i + 1, m_AryThic[i]);
					PauAndQuit();
				}
			}
		}
	}
}

void CDIntBedCell::UpdateKz()
{
	//更新夹层体各节点单元的垂向渗透系数(全厚度离散)

	long i;
	//更新节点单元的渗透系数Kz
	for (i = 0; i < m_Node; i++)
	{
		if (m_AryTheta[i] != m_AryThetaOld[i])
		{
			m_AryKz[i] = m_AryKzOld[i] * pow(m_AryTheta[i] / m_AryThetaOld[i], 3.0) *
				pow((1.0 - m_AryThetaOld[i]) / (1.0 - m_AryTheta[i]), 2.0);
		}
	}
	//计算节点间平均渗透系数除以节点间距离的值
	for (i = 0; i < m_Node-1; i++)
	{
		m_AryKDZ[i] = 2.0 * m_AryKz[i] * m_AryKz[i + 1] /
			(m_AryThic[i + 1] * m_AryKz[i] + m_AryThic[i] * m_AryKz[i + 1]);
	}
}

void CDIntBedCell::UpdateKz2()
{
	//更新夹层体各节点单元的垂向渗透系数(半厚度离散)

	long i;
	//更新节点单元的渗透系数Kz
	for (i = 0; i < m_Node; i++)
	{
		if (m_AryTheta[i] != m_AryThetaOld[i])
		{
			m_AryKz[i] = m_AryKzOld[i] * pow(m_AryTheta[i] / m_AryThetaOld[i], 3.0) *
				pow((1.0 - m_AryThetaOld[i]) / (1.0 - m_AryTheta[i]), 2.0);
		}
	}
	//计算节点间平均渗透系数除以节点间距离的值
	for (i = 0; i < m_Node - 1; i++)
	{
		if (i != m_Node - 2)
		{
			m_AryKDZ[i] = 2.0 * m_AryKz[i] * m_AryKz[i + 1] /
				(m_AryThic[i + 1] * m_AryKz[i] + m_AryThic[i] * m_AryKz[i + 1]);
		}
		else
		{
			//需考虑最后一个节点单元的大小只有一半
			m_AryKDZ[i] = m_AryKz[i] * m_AryKz[i + 1] /
				(m_AryThic[i + 1] * m_AryKz[i] + 0.5 * m_AryThic[i] * m_AryKz[i + 1]);
		}
	}
}

void CDIntBedCell::UpdateZOff()
{
	//更新夹层体各节点单元中心点和与夹层体中心点的距离(仅全离散格式下调用)

	long i;
	long HalfNode = long(m_Node / 2.0);
	double IBFullThic = 0.0;
	double IBHalfThic;
	for (i = 0; i < m_Node; i++)
	{
		IBFullThic = IBFullThic + m_AryThic[i];
	}
	IBHalfThic = 0.5 * IBFullThic;
	m_AryZOff[0] = 0.5 * m_AryThic[0] - IBHalfThic;
	for (i = 1; i < m_Node; i++)
	{
		m_AryZOff[i] = m_AryZOff[i -1] + 0.5 * (m_AryThic[i-1] + m_AryThic[i]);
	}
}

double CDIntBedCell::SimHDDiff(long iLyr, long DBedID, double CellHead, double DeltT, CLandSub* pLandSub)
{
	//进行等效夹层体一维水头扩散方程的求解（全厚度离散）
	//注: CellHead为网格单元上的地下水水头
	//DeltT为模拟计算时段的时长

	long i, SumInIter, SumOutIter, lcnvg, KMOPT;
	double Ski = 0.0;
	double CellBot, Fact, Skv, Ske, CellSGS, ZDT, CellLyrCent, CellTotGL, RHO, CoefW;
	double Abs_BigHChg0, NodeEComR, NodeVComR, ThicChgR, MaxThicChgR, ESTCENTER;
	CoefW = pLandSub->GetWSS() / DeltT;
	KMOPT = pLandSub->GetKMOPT();
	SumInIter = 1;
	SumOutIter = 1;
	//记录求解一维水头扩散方程时所用到的边界水头
	m_BndHead = CellHead;
	CellLyrCent = pLandSub->GetLyrCent(iLyr, m_IROW, m_ICOL);
	CellTotGL = pLandSub->GetTotGL(iLyr, m_IROW, m_ICOL);
	CellBot = GDWMOD.GetCellBot(iLyr, m_IROW, m_ICOL);
	CellSGS = pLandSub->GetSGS(iLyr, m_IROW, m_ICOL);
	if (KMOPT == 1)
	{
		ZeroMemory(m_AryNodTComPre, sizeof(double) * m_Node);
	}
	for (; ;)
	{
		//根据节点单元压实量更新孔隙率和厚度(注:位置和顺序不能变,否者会引起夹层体水平衡误差)
		if (SumOutIter != 1)
		{
			UpThetaAndThic(iLyr, DBedID);
			if (KMOPT == 1)
			{
				//更新夹层体各节点单元的垂向渗透系数
				UpdateKz();
				//更新夹层体各节点单元中心点和与夹层体中心点的距离
				UpdateZOff();
			}
		}
		//构建节点矩阵方程并进行求解
		for (; ;)
		{
			//1.保存本次迭代计算的水头值到上一次迭代计算的水头值
			CopyMemory(m_AryHHPre, m_AryHHNew, sizeof(double) * m_Node);
			//2.构建等效夹层体一维水头扩散矩阵方程
			for (i = 0; i < m_Node; i++)
			{
				//(1)计算各节点处的有效应力(基于底板高程)
				m_AryHEST[i] = CellTotGL - m_AryHHNew[i] + CellBot;
				//(2)计算各节点处的弹性/非弹性贮水系数
				ZDT = m_DeltZ / DeltT;
				//注: (m_AryHEST[i] - (m_LYRCENT - CellBot- m_AryZOff[i]) * (CellSGS - 1.0))
				//为将当前底板处有效应力值EST修正到层中心处
				ESTCENTER = m_AryHEST[i] - (CellLyrCent - CellBot - m_AryZOff[i]) * (CellSGS - 1.0);
				//有时候模拟条件给得太恶劣, ESTCENTER会小于零, 这里修正一下
				ESTCENTER = max(ESTCENTER, 0.01);
				Fact = ZDT / ESTCENTER;
				m_ArySKE[i] = Fact * m_SEDMCR;
				m_ArySKV[i] = Fact * m_IBCC;
				Ske = m_ArySKE[i];
				Skv = m_ArySKV[i];
				if (m_AryHEST[i] > m_AryHHC[i])
				{
					//非弹性系数
					Ski = Skv;
				}
				else
				{
					//弹性系数
					Ski = Ske;
				}
				//(3)处理矩阵的上对角线项(矩阵方程是对称的, 只需记录上对角线)
				if (KMOPT == 0)
				{
					m_AryDU[i] = m_KDZ;
				}
				else
				{
					m_AryDU[i] = m_AryKDZ[i];
				}
				//(4)处理矩阵的主对角线项和右端项
				if (i == 0 || i == m_Node - 1)
				{
					//首个节点和最后一个节点
					if (KMOPT == 0)
					{
						m_AryDA[i] = -3.0 * m_KDZ - Ski;
						m_AryR[i] = -(Ski * (CellTotGL + CellBot) - (Ski - Ske) * m_AryHHC[i] -
							Ske * m_AryHESTOLD[i]) - 2.0 * m_KDZ * m_BndHead;
					}
					else
					{
						if (i == 0)
						{
							m_AryDA[i] = -2.0 * m_AryKz[i] / m_AryThic[i] - m_AryKDZ[i] - Ski;
						}
						else
						{
							m_AryDA[i] = -2.0 * m_AryKz[i] / m_AryThic[i] - m_AryKDZ[i - 1] - Ski;
						}
						m_AryR[i] = -(Ski * (CellTotGL + CellBot) - (Ski - Ske) * m_AryHHC[i] -
							Ske * m_AryHESTOLD[i]) - 2.0 * m_AryKz[i] / m_AryThic[i] * m_BndHead;
					}
				}
				else
				{
					//中间节点
					if (KMOPT == 0)
					{
						m_AryDA[i] = -2 * m_KDZ - Ski;
					}
					else
					{
						m_AryDA[i] = -m_AryKDZ[i - 1] - m_AryKDZ[i] - Ski;
					}
					m_AryR[i] = -(Ski * (CellTotGL + CellBot) - (Ski - Ske) * m_AryHHC[i] -
						Ske * m_AryHESTOLD[i]);
				}
			}
			//3.处理孔隙水的压缩/膨胀释水
			for (i = 0; i < m_Node; i++)
			{
				RHO = 0.5 * (m_AryThic[i] * m_AryTheta[i] + m_AryThicOld[i] * m_AryThetaOld[i]) * CoefW;
				m_AryDA[i] = m_AryDA[i] - RHO;
				m_AryR[i] = m_AryR[i] - RHO * m_AryHHOld[i];
			}
			//4.用高斯消元法对矩阵方程进行求解
			lcnvg = GauSolve(m_AryDA, m_AryDU, m_AryR, m_AryHHNew, m_AryHHPre, SumInIter, Abs_BigHChg0);
			if (lcnvg == 1)
			{
				SumInIter = 1;
				break;
			}
			else
			{
				SumInIter = SumInIter + 1;
			}
		}
		//计算分项耦合值
		m_ElsComChgR = 0.0;
		m_InElsComChgR = 0.0;
		m_WtComChgR = 0.0;
		for (i = 0; i < m_Node; i++)
		{
			NodeEComR = 0.0;
			NodeVComR = 0.0;
			Ske = m_ArySKE[i];
			Skv = m_ArySKV[i];
			//统计等效夹层体骨架释水/储水通量
			if (m_AryHEST[i] > m_AryHHC[i])
			{
				//有非弹性压缩
				NodeEComR = Ske * (m_AryHHC[i] - m_AryHESTOLD[i]);
				NodeVComR = Skv * (m_AryHEST[i] - m_AryHHC[i]);
			}
			else
			{
				//仅弹性压缩
				NodeEComR = Ske * (m_AryHEST[i] - m_AryHESTOLD[i]);
			}
			m_ElsComChgR = m_ElsComChgR + NodeEComR;
			m_InElsComChgR = m_InElsComChgR + NodeVComR;
			//计算节点单元压缩量(L)
			m_AryNodTCom[i] = (NodeEComR + NodeVComR) * DeltT;
			//统计等效夹层体内孔隙水压实/膨胀释水通量
			RHO = 0.5 * (m_AryThic[i] * m_AryTheta[i] + m_AryThicOld[i] * m_AryThetaOld[i]) * CoefW;
			m_WtComChgR = m_WtComChgR - RHO * (m_AryHHNew[i] - m_AryHHOld[i]);
		}
		if (KMOPT == 0)
		{
			//如果不考虑K值变化
			break;
		}
		else
		{
			//如果考虑K值变化
			MaxThicChgR = 0.0;
			for (i = 0; i < m_Node; i++)
			{
				ThicChgR = fabs((m_AryNodTCom[i] - m_AryNodTComPre[i]) / (m_AryThicOld[i] - m_AryNodTComPre[i]));
				if (ThicChgR > MaxThicChgR)
				{
					MaxThicChgR = ThicChgR;
				}
			}
			if (MaxThicChgR < 1E-4)
			{
				//当迭代前后最大的节点单元厚度相对变化小于万分之一时跳出
				break;
			}
			else
			{
				CopyMemory(m_AryNodTComPre, m_AryNodTCom, sizeof(double) * m_Node);
				SumOutIter = SumOutIter + 1;
			}
		}
	}
	//统计和返回耦合值
	m_CoupItem = m_ElsComChgR + m_InElsComChgR + m_WtComChgR;
	return m_CoupItem;
}

void CDIntBedCell::CalBalErr(double DeltT, long KMOPT)
{
	//计算水量平衡误差(全厚度离散)

	double TotStChgR, FluxExcR, AbsErr, Denom;
	//1.计算水量平衡误差(以进入夹层体为正)
	//(1)统计夹层体骨架压缩/膨胀贮水变化
	TotStChgR = m_ElsComChgR + m_InElsComChgR;
	if (TotStChgR > 0.0)
	{
		//水量从夹层体的压缩释放
		m_SumStaIn = m_SumStaIn + TotStChgR * DeltT;
	}
	else
	{
		//水量被夹层体膨胀吸收
		m_SumStaOut = m_SumStaOut - TotStChgR * DeltT;
	}
	//(2)统计夹层体内孔隙水膨胀/压实蓄变量
	if (m_WtComChgR > 0.0)
	{
		//水量从层体内孔隙水膨胀释放
		m_SumStaIn = m_SumStaIn + m_WtComChgR * DeltT;
	}
	else
	{
		//水量被夹层体内孔隙水压缩存储
		m_SumStaOut = m_SumStaOut - m_WtComChgR * DeltT;
	}
	//(3)统计夹层体与含水层水量通量
	if (KMOPT == 0)
	{
		FluxExcR = 2.0 * m_KDZ * (2.0 * m_BndHead - m_AryHHNew[0] - m_AryHHNew[m_Node - 1]);
	}
	else
	{
		FluxExcR = 2.0 * (m_AryKz[0] / m_AryThic[0] * (m_BndHead - m_AryHHNew[0]) +
			m_AryKz[m_Node - 1] / m_AryThic[m_Node - 1] * (m_BndHead - m_AryHHNew[m_Node - 1]));
	}
	if (FluxExcR > 0.0)
	{
		//水量从含水层进入夹层体
		m_SumFluxIn = m_SumFluxIn + FluxExcR * DeltT;
	}
	else
	{
		//水量从夹层体进入含水层
		m_SumFluxOut = m_SumFluxOut - FluxExcR * DeltT;
	}
	//(4)计算绝对水量平衡误差(L)
	AbsErr = m_SumStaIn + m_SumFluxIn - m_SumStaOut - m_SumFluxOut;
	//(5)计算相对水量平衡误差(%)
	Denom = fabs(m_SumStaIn + m_SumFluxIn + m_SumStaOut + m_SumFluxOut);
	if (Denom != 0.0)
	{
		m_RERR = 100.0 * fabs(AbsErr) / Denom;
	}
	else
	{
		m_RERR = 0.0;
	}
}

double CDIntBedCell::SimHDDiff2(long iLyr, long DBedID, double CellHead, double DeltT, CLandSub* pLandSub)
{
	//进行等效夹层体一维水头扩散方程的求解(半厚度离散)
	//注: CellHead为网格单元上的地下水水头
	//DeltT为模拟计算时段的时长

	long i, SumInIter, SumOutIter, lcnvg, KMOPT;
	double Ski = 0.0;
	double CellBot, Fact, Skv, Ske, CellSGS, ZDT, CellLyrCent, CellTotGL, RHO, CoefW;
	double Abs_BigHChg0, NodeEComR, NodeVComR, ThicChgR, MaxThicChgR, ESTCENTER;
	CoefW = pLandSub->GetWSS() / DeltT;
	KMOPT = pLandSub->GetKMOPT();
	SumInIter = 1;
	SumOutIter = 1;
	//记录求解一维水头扩散方程时所用到的边界水头
	m_BndHead = CellHead;
	CellLyrCent = pLandSub->GetLyrCent(iLyr, m_IROW, m_ICOL);
	CellTotGL = pLandSub->GetTotGL(iLyr, m_IROW, m_ICOL);
	CellBot = GDWMOD.GetCellBot(iLyr, m_IROW, m_ICOL);
	CellSGS = pLandSub->GetSGS(iLyr, m_IROW, m_ICOL);
	if (KMOPT == 1)
	{
		ZeroMemory(m_AryNodTComPre, sizeof(double) * m_Node);
	}
	for (;;)
	{

		//根据节点单元压实量更新孔隙率和厚度(注:位置和顺序不能变,否者会引起夹层体水平衡误差)
		if (SumOutIter != 1)
		{
			UpThetaAndThic(iLyr, DBedID);
			if (KMOPT == 1)
			{
				//更新夹层体各节点单元的垂向渗透系数
				UpdateKz2();
			}
		}
		//构建节点矩阵方程并进行求解
		for (; ;)
		{
			//1.保存本次迭代计算的水头值到上一次迭代计算的水头值
			CopyMemory(m_AryHHPre, m_AryHHNew, sizeof(double) * m_Node);
			//2.构建等效夹层体一维水头扩散矩阵方程
			for (i = 0; i < m_Node; i++)
			{
				//(1)计算各节点处的有效应力(基于底板高程)
				m_AryHEST[i] = CellTotGL - m_AryHHNew[i] + CellBot;
				//(2)计算各节点处的弹性/非弹性贮水系数
				ZDT = m_DeltZ / DeltT;
				//注: (m_AryHEST[i] - (m_LYRCENT - CellBot) * (CellSGS - 1.0))
				//为将当前底板处有效应力值EST修正到层中心处
				ESTCENTER = m_AryHEST[i] - (CellLyrCent - CellBot) * (CellSGS - 1.0);
				//有时候模拟条件给得太恶劣, ESTCENTER会小于零, 这里修正一下
				ESTCENTER = max(ESTCENTER, 0.01);
				Fact = ZDT / ESTCENTER;
				if (i != m_Node - 1)
				{
					m_ArySKE[i] = Fact * m_SEDMCR;
					m_ArySKV[i] = Fact * m_IBCC;
				}
				else
				{
					//最后一个节点单元的初始大小只有m_DeltZ的一半
					m_ArySKE[i] = 0.5 * Fact * m_SEDMCR;
					m_ArySKV[i] = 0.5 * Fact * m_IBCC;
				}
				Ske = m_ArySKE[i];
				Skv = m_ArySKV[i];
				if (m_AryHEST[i] > m_AryHHC[i])
				{
					//非弹性系数
					Ski = Skv;
				}
				else
				{
					//弹性系数
					Ski = Ske;
				}
				//(3)处理矩阵的上对角线项(矩阵方程是对称的, 只需记录上对角线)
				if (KMOPT == 0)
				{
					m_AryDU[i] = m_KDZ;
				}
				else
				{
					m_AryDU[i] = m_AryKDZ[i];
				}
				//(4)处理矩阵的主对角线项和右端项
				if (i == 0)
				{
					//首个节点
					if (KMOPT == 0)
					{
						m_AryDA[i] = -3.0 * m_KDZ - Ski;
						m_AryR[i] = -(Ski * (CellTotGL + CellBot) - (Ski - Ske) * m_AryHHC[i] -
							Ske * m_AryHESTOLD[i]) - 2.0 * m_KDZ * m_BndHead;
					}
					else
					{
						m_AryDA[i] = -2.0 * m_AryKz[i] / m_AryThic[i] - m_AryKDZ[i] - Ski;
						m_AryR[i] = -(Ski * (CellTotGL + CellBot) - (Ski - Ske) * m_AryHHC[i] -
							Ske * m_AryHESTOLD[i]) - 2.0 * m_AryKz[i] / m_AryThic[i] * m_BndHead;
					}
				}
				else if (i > 0 && i < m_Node - 1)
				{
					//中间节点
					if (KMOPT == 0)
					{
						m_AryDA[i] = -2 * m_KDZ - Ski;
					}
					else
					{
						m_AryDA[i] = -m_AryKDZ[i - 1] - m_AryKDZ[i] - Ski;
					}
					m_AryR[i] = -(Ski * (CellTotGL + CellBot) - (Ski - Ske) * m_AryHHC[i] -
						Ske * m_AryHESTOLD[i]);
				}
				else
				{
					//最末节点
					if (KMOPT == 0)
					{
						m_AryDA[i] = -m_KDZ - Ski;
					}
					else
					{
						m_AryDA[i] = -m_AryKDZ[i - 1] - Ski;
					}
					m_AryR[i] = -(Ski * (CellTotGL + CellBot) - (Ski - Ske) * m_AryHHC[i] -
						Ske * m_AryHESTOLD[i]);
				}
			}
			//3.处理孔隙水的压缩/膨胀释水
			for (i = 0; i < m_Node; i++)
			{
				RHO = 0.5 * (m_AryThic[i] * m_AryTheta[i] + m_AryThicOld[i] * m_AryThetaOld[i]) * CoefW;
				m_AryDA[i] = m_AryDA[i] - RHO;
				m_AryR[i] = m_AryR[i] - RHO * m_AryHHOld[i];
			}
			//4.用高斯消元法对矩阵方程进行求解
			lcnvg = GauSolve(m_AryDA, m_AryDU, m_AryR, m_AryHHNew, m_AryHHPre, SumInIter, Abs_BigHChg0);
			if (lcnvg == 1)
			{
				SumInIter = 1;
				break;
			}
			else
			{
				SumInIter = SumInIter + 1;
			}
		}
		//计算分项耦合值
		m_ElsComChgR = 0.0;
		m_InElsComChgR = 0.0;
		m_WtComChgR = 0.0;
		for (i = 0; i < m_Node; i++)
		{
			NodeEComR = 0.0;
			NodeVComR = 0.0;
			Ske = m_ArySKE[i];
			Skv = m_ArySKV[i];
			//统计等效夹层体骨架释水/储水通量
			if (m_AryHEST[i] > m_AryHHC[i])
			{
				//有非弹性压缩
				NodeEComR = Ske * (m_AryHHC[i] - m_AryHESTOLD[i]);
				NodeVComR = Skv * (m_AryHEST[i] - m_AryHHC[i]);
			}
			else
			{
				//仅弹性压缩
				NodeEComR = Ske * (m_AryHEST[i] - m_AryHESTOLD[i]);
			}
			m_ElsComChgR = m_ElsComChgR + NodeEComR;
			m_InElsComChgR = m_InElsComChgR + NodeVComR;
			//计算节点单元压缩量(L)
			m_AryNodTCom[i] = (NodeEComR + NodeVComR) * DeltT;
			//统计等效夹层体内孔隙水压实/膨胀释水通量
			RHO = 0.5 * (m_AryThic[i] * m_AryTheta[i] + m_AryThicOld[i] * m_AryThetaOld[i]) * CoefW;
			m_WtComChgR = m_WtComChgR - RHO * (m_AryHHNew[i] - m_AryHHOld[i]);
		}
		if (KMOPT == 0)
		{
			//如果不考虑K值变化
			break;
		}
		else
		{
			//如果考虑K值变化
			MaxThicChgR = 0.0;
			for (i = 0; i < m_Node; i++)
			{
				ThicChgR = fabs((m_AryNodTCom[i] - m_AryNodTComPre[i]) / (m_AryThicOld[i] - m_AryNodTComPre[i]));
				if (ThicChgR > MaxThicChgR)
				{
					MaxThicChgR = ThicChgR;
				}
			}
			if (MaxThicChgR < 1E-4)
			{
				//当迭代前后最大的节点单元厚度相对变化小于万分之一时跳出
				break;
			}
			else
			{
				CopyMemory(m_AryNodTComPre, m_AryNodTCom, sizeof(double) * m_Node);
				SumOutIter = SumOutIter + 1;
			}
		}
	}
	//统计和返回耦合值
	m_CoupItem = m_ElsComChgR + m_InElsComChgR + m_WtComChgR;
	return m_CoupItem;
}

void CDIntBedCell::CalBalErr2(double DeltT, long KMOPT)
{
	//计算水量平衡误差(半厚度离散)

	double TotStChgR, FluxExcR, AbsErr, Denom;
	//1.计算水量平衡误差(以进入夹层体为正)
	//(1)统计夹层体骨架压缩/膨胀贮水变化
	TotStChgR = m_ElsComChgR + m_InElsComChgR;
	if (TotStChgR > 0.0)
	{
		//水量从夹层体的压缩释放
		m_SumStaIn = m_SumStaIn + TotStChgR * DeltT;
	}
	else
	{
		//水量被夹层体膨胀吸收
		m_SumStaOut = m_SumStaOut - TotStChgR * DeltT;
	}
	//(2)统计夹层体内孔隙水膨胀/压实蓄变量
	if (m_WtComChgR > 0.0)
	{
		//水量从层体内孔隙水膨胀释放
		m_SumStaIn = m_SumStaIn + m_WtComChgR * DeltT;
	}
	else
	{
		//水量被夹层体内孔隙水压缩存储
		m_SumStaOut = m_SumStaOut - m_WtComChgR * DeltT;
	}
	//(3)统计夹层体与含水层水量通量
	if (KMOPT == 0)
	{
		FluxExcR = 2.0 * m_KDZ * (m_BndHead - m_AryHHNew[0]);
	}
	else
	{
		FluxExcR = 2.0 * m_AryKz[0] / m_AryThic[0] * (m_BndHead - m_AryHHNew[0]);
	}
	if (FluxExcR > 0.0)
	{
		//水量从含水层进入夹层体
		m_SumFluxIn = m_SumFluxIn + FluxExcR * DeltT;
	}
	else
	{
		//水量从夹层体进入含水层
		m_SumFluxOut = m_SumFluxOut - FluxExcR * DeltT;
	}
	//(4)计算绝对水量平衡误差(L)
	AbsErr = m_SumStaIn + m_SumFluxIn - m_SumStaOut - m_SumFluxOut;
	//(5)计算相对水量平衡误差(%)
	Denom = fabs(m_SumStaIn + m_SumFluxIn + m_SumStaOut + m_SumFluxOut);
	if (Denom != 0.0)
	{
		m_RERR = 100.0 * fabs(AbsErr) / Denom;
	}
	else
	{
		m_RERR = 0.0;
	}
}

void CDIntBedCell::BudgetDIBS(long iLyr, double DeltT, double ThickFact, long DBedID, long DISOPT, 
	long KMOPT, CLandSub* pLandSub)
{
	//进行水量平衡和压实量统计

    long i;
	double Coef, CellTotGL, CellBot;

	//1.计算水量平衡误差(以进入夹层体为正)
	if (DISOPT == 1)
	{
		//全厚度离散格式
		CalBalErr(DeltT, KMOPT);
		Coef = ThickFact * m_RNB;
	}
	else
	{
		//半厚度离散格式
		CalBalErr2(DeltT, KMOPT);
		Coef = 2.0 * ThickFact * m_RNB;
	}

	//2.保存当前的水头值到下个时段初的水头值
	CopyMemory(m_AryHHOld, m_AryHHNew, sizeof(double) * m_Node);

	//3.计算本时段夹层体的压实量(L)、弹性和非弹性压实/膨胀流量(L/T)
	m_TotComChg = Coef * (m_ElsComChgR + m_InElsComChgR) * DeltT;
	m_ElsComChgR = Coef * m_ElsComChgR;
	m_InElsComChgR = Coef * m_InElsComChgR;
	m_WtComChgR = Coef * m_WtComChgR;

	//4.累计夹层体的压实量(L)
	m_COM = m_COM + m_TotComChg;

	//5.清零耦合项和边界水头
	m_CoupItem = 0.0;
	m_BndHead = 0.0;

	//6.保存和更新当前有效应力值、预固结应力值、孔隙率和厚度
	//根据节点单元压实量更新孔隙率和厚度
	UpThetaAndThic(iLyr, DBedID);
	if (KMOPT == 1)
	{
		//更新夹层体各节点单元的垂向渗透系数
		UpdateKz();
		if (DISOPT == 1)
		{
			//全厚度离散格式更新夹层体各节点单元中心点和与夹层体中心点的距离
			UpdateZOff();
		}
	}
	CellTotGL = pLandSub->GetTotGL(iLyr, m_IROW, m_ICOL);
	CellBot = GDWMOD.GetCellBot(iLyr, m_IROW, m_ICOL);
	for (i = 0; i < m_Node; i++)
	{
		//(1)保存当前有效应力值
		m_AryHEST[i] = CellTotGL - m_AryHHNew[i] + CellBot;
		m_AryHESTOLD[i] = m_AryHEST[i];
		//(2)检查各节点单元的预固结应力值是否需要更新
		if (m_AryHEST[i] > m_AryHHC[i])
		{
			m_AryHHC[i] = m_AryHEST[i];
		}
		//(3)保存孔隙率/厚度/渗透系数参数
		m_AryThetaOld[i] = m_AryTheta[i];
		m_AryThicOld[i] = m_AryThic[i];
		if (KMOPT == 1)
		{
			m_AryKzOld[i] = m_AryKz[i];
		}
	}
}

///////////////////////////////////
//粗颗粒沉积物类函数定义
CCGSediment::CCGSediment()
{
	m_ILYR = 0;              //无延迟夹层体类所位于的含水层层位
	m_pLandSub = NULL;       //地面沉降模拟类指针
}

CCGSediment::~CCGSediment()
{
	//清理开辟的指针对象

	long i;
	for (i = 0; i < m_pCGSedmCellAry.size(); i++)
	{
		delete m_pCGSedmCellAry[i];
	}
	m_pCGSedmCellAry.clear();
}

void CCGSediment::ReadCGCellData(CString& strData)
{
	//读入粗颗粒沉积物单元数据(TXT)

	long ILYR, IROW, ICOL;
	double CGCR, CGTHETA;
	CCoGrainCell* pCell = NULL;
	CString strFormat;
	strFormat = _T("%ld %ld %ld %lg %lg");
	if (sscanf_s(strData, strFormat, &ILYR, &IROW, &ICOL, &CGCR, &CGTHETA) != 5)
	{
		printf(_T("\n"));
		printf(_T("数据行中的数据不足或有错误,请检查 地面沉降_粗颗粒沉积物属性表.in !\n"));
		printf(_T("第%d层的网格单元数据 !\n"), ILYR);
		PauAndQuit();
	}
	//检查数据
	GDWMOD.CheckCellLimit(IROW, ICOL);
	if (CGCR < 0.0 || CGTHETA < 0.0)
	{
		printf(_T("\n"));
		printf(_T("数据表的CGCR、CGTHETA字段数据必须大于等于零,请检查！\n"));
		PauAndQuit();
	}
	//生成夹层体网格单元对象
	//加入单元数组
	long IBound = GDWMOD.GetCellIniIBOUND(m_ILYR, IROW, ICOL);
	if (IBound > 0)
	{
		//沉降计算仅对变水头单元有效
		pCell = new  CCoGrainCell;
		pCell->m_IROW = IROW;
		pCell->m_ICOL = ICOL;
		pCell->m_SEDMCR = CGCR;
		pCell->m_CGTHETA = CGTHETA;
		m_pCGSedmCellAry.push_back(pCell);
	}
}

void CCGSediment::ReadCGCellData(_RecordsetPtr& pTable)
{
	//读入粗颗粒沉积物单元数据(数据库)

	long i;
	long nCount = 0;
	long IBound;
	CCoGrainCell* pCell = NULL;
	_variant_t Value;
	try
	{
		pTable->MoveLast();
		nCount = pTable->GetRecordCount();  //得到单元个数
		pTable->MoveFirst();
		for (i = 0; i < nCount; i++)
		{
			pCell = new  CCoGrainCell;
			Value = pTable->GetCollect(_T("IROW"));
			pCell->m_IROW = Value.intVal;
			Value = pTable->GetCollect(_T("ICOL"));
			pCell->m_ICOL = Value.intVal;
			Value = pTable->GetCollect(_T("CGCR"));
			pCell->m_SEDMCR = Value.dblVal;
			Value = pTable->GetCollect(_T("CGTHETA"));
			pCell->m_CGTHETA = Value.dblVal;
			//检查参数的有效性
			if (pCell->m_SEDMCR < 0.0 || pCell->m_CGTHETA < 0.0)
			{
				printf(_T("\n"));
				printf(_T("数据表的CGCR、CGTHETA字段数据必须大于等于零,请检查！\n"));
				PauAndQuit();
			}
			//检查地下水单元的初始属性值
			IBound = GDWMOD.GetCellIniIBOUND(m_ILYR, pCell->m_IROW, pCell->m_ICOL);
			if (IBound > 0)
			{
				//沉降计算仅对变水头单元有效
				m_pCGSedmCellAry.push_back(pCell);
			}
			pTable->MoveNext();                                       //定位到下一条数据
		}
	}
	catch (_com_error* e)
	{
		printf((LPCTSTR)e->ErrorMessage());
	}
}

void CCGSediment::InitSimulation()
{
	//模拟之前进行初始化

	CCoGrainCell* pCell = NULL;
	long CSOPT = m_pLandSub->GetCSOPT();
	for (size_t i = 0; i < m_pCGSedmCellAry.size(); i++)
	{
		pCell = m_pCGSedmCellAry[i];
		//初始化上个时段的孔隙率和厚度
		pCell->m_CGTHKOLD = pCell->m_CGTHK;
		pCell->m_CGTHETAOLD = pCell->m_CGTHETA;
	}
}

void CCGSediment::ModifyPar()
{
	//根据模拟选项修改参数

	long CSOPT = m_pLandSub->GetCSOPT();
	double CellBot, CellArea, CellSGS, CellLyrCent, CellEST, EstCenter, Coef1, Coef2;
	long iRow, iCol;
	CCoGrainCell* pCell = NULL;
	for (size_t i = 0; i < m_pCGSedmCellAry.size(); i++)
	{
		pCell = m_pCGSedmCellAry[i];
		iRow = pCell->m_IROW;
		iCol = pCell->m_ICOL;
		CellBot = GDWMOD.GetCellBot(m_ILYR, iRow, iCol);
		CellArea = GDWMOD.GetCellArea(iRow, iCol);
		CellSGS = m_pLandSub->GetSGS(m_ILYR, iRow, iCol);
		CellLyrCent = m_pLandSub->GetLyrCent(m_ILYR, iRow, iCol);
		CellEST = m_pLandSub->GetEST(m_ILYR, iRow, iCol);
		EstCenter = CellEST - (CellLyrCent - CellBot) * (CellSGS - 1.0);
		Coef1 = EstCenter * CellArea * pCell->m_CGTHK;
		if (CSOPT == 1)
		{
			//输入的贮水参数是压缩指数
			Coef2 = 0.4342945 * CellArea * pCell->m_CGTHK /
				(1.0 + pCell->m_CGTHETA / (1.0 - pCell->m_CGTHETA));
			pCell->m_SEDMCR = Coef2 * pCell->m_SEDMCR;
			pCell->m_CGSKE = pCell->m_SEDMCR / Coef1;
		}
		else
		{
			//输入的贮水参数是贮水率
			pCell->m_CGSKE = pCell->m_SEDMCR;
			pCell->m_SEDMCR = Coef1 * pCell->m_SEDMCR;
		}
	}
}

CMemTable* CCGSediment::m_pMemCGDataTable = new CMemTable;
void CCGSediment::MakeCGDataTable(_RecordsetPtr pTable, CString TbName)
{
	//构建无延迟夹层体类模拟结果输出缓存表

	m_pMemCGDataTable->MakeDataTable(pTable, TbName);
}

void CCGSediment::WriteDatabase()
{
	//将内存数据写入数据库

	m_pMemCGDataTable->WriteDatabase();
}

void CCGSediment::CGOutPut(long PrnID, long iPer, long iStep, double CumTLen, CStdioFile& OutFile)
{
	//输出粗颗粒沉积物模拟结果(TXT)

	size_t i;
	size_t size = m_pCGSedmCellAry.size();
	CCoGrainCell* pCell = NULL;
	for (i = 0; i < size; i++)
	{
		pCell = m_pCGSedmCellAry[i];
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
		fprintf(OutFile.m_pStream, _T(" %-10d"), m_ILYR);
		fprintf(OutFile.m_pStream, _T(" %-10d"), pCell->m_IROW);
		fprintf(OutFile.m_pStream, _T(" %-10d"), pCell->m_ICOL);
		fprintf(OutFile.m_pStream, _T(" %-16.8g"), pCell->m_COM);
		fprintf(OutFile.m_pStream, _T(" %-16.8g"), pCell->m_CGTHETA);
		fprintf(OutFile.m_pStream, _T(" %-16.8g"), pCell->m_CGTHK);
		fprintf(OutFile.m_pStream, _T(" %-16.8g"), pCell->m_CGSKE);
		fprintf(OutFile.m_pStream, _T("\n"));
	}
}

void CCGSediment::CGOutPut(long PrnID, long iPer, long iStep, double CumTLen)
{
	//输出粗颗粒沉积物模拟结果(数据库)

	size_t i;
	size_t size = m_pCGSedmCellAry.size();
	CCoGrainCell* pCell = NULL;
	try
	{
		DataRow^ dr;
		//输出各网格单元内夹层体的模拟结果
		for (i = 0; i < size; i++)
		{
			pCell = m_pCGSedmCellAry[i];
			dr = m_pMemCGDataTable->GetDataTable()->NewRow();
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
			dr[3] = m_ILYR;
			dr[4] = pCell->m_IROW;
			dr[5] = pCell->m_ICOL;
			dr[6] = pCell->m_COM;
			dr[7] = pCell->m_CGTHETA;
			dr[8] = pCell->m_CGTHK;
			dr[9] = pCell->m_CGSKE;
			m_pMemCGDataTable->GetDataTable()->Rows->Add(dr);
		}
	}
	catch (_com_error* e)
	{
		printf((LPCTSTR)e->ErrorMessage());
	}
}

///////////////////////////////////
//介质带类函数定义
CMatZone::CMatZone()
{
	m_IMZ = 0;               //介质带的编号. (编号从0~NMZ-1). 
	m_MZVK = 0.0;            //介质带的垂向渗透系数(L/T)
	m_MZCR = 0.0;            //根据<地面沉降_模拟控制参数表>中CSOPT选项而定. CSOPT=1表示该参数为介质带的弹性压缩指数(-); 
	                         //CSOPT=2表示该参数为介质带的弹性贮水率(-)
	m_MZCC = 0.0;            //根据<地面沉降_模拟控制参数表>中CSOPT选项而定. CSOPT=1表示该参数为介质带的非弹性压缩指数(-); 
	                         //CSOPT=2表示该参数为介质带的非弹性贮水率(-)
	m_MZTHETA = 0.0;         //介质带的初始孔隙率(即孔隙体积与总体积之比)(-)
}

CMatZone::~CMatZone()
{

}

///////////////////////////////////
//无延迟夹层体类类函数定义
CNDelayIntBed::CNDelayIntBed()
{
	m_NDBedID = 0;           //无延迟夹层体类的编号(从1开始)
	m_ILYR = 0;              //无延迟夹层体类所位于的含水层层位
	m_pLandSub = NULL;       //地面沉降模拟类指针
}

CNDelayIntBed::~CNDelayIntBed()
{
	//清理开辟的指针对象

	long i;
	for (i = 0; i < m_pNDIntBedCellAry.size(); i++)
	{
		delete m_pNDIntBedCellAry[i];
	}
	m_pNDIntBedCellAry.clear();
}

void CNDelayIntBed::ReadNDIBCellData(CString& strData)
{
	//读入单元数据(TXT)

	long INDB, IROW, ICOL;
	double IBTHK, IBCR, IBCC, IBTHETA, COM, HCOFF;
	CNDIntBedCell* pCell = NULL;
	CString strFormat;
	strFormat = _T("%ld %ld %ld %lg %lg %lg %lg %lg %lg");
	if (sscanf_s(strData, strFormat, &INDB, &IROW, &ICOL, &IBTHK, &IBCR,
		&IBCC, &IBTHETA, &COM, &HCOFF) != 9)
	{
		printf(_T("\n"));
		printf(_T("数据行中的数据不足或有错误,请检查 地面沉降_无延迟夹层体属性表.in !\n"));
		printf(_T("第%d个夹层体类的网格单元数据 !\n"), m_NDBedID);
		PauAndQuit();
	}
	//检查数据
	GDWMOD.CheckCellLimit(IROW, ICOL);
	if (IBTHK <= 0.0 || IBTHETA <= 0.0)
	{
		printf(_T("\n"));
		printf(_T("数据表的IBTHK、IBTHETA字段数据必须大于零,请检查！\n"));
		PauAndQuit();
	}
	if (IBCR < 0.0 || IBCC < 0.0 || COM < 0.0)
	{
		printf(_T("\n"));
		printf(_T("数据表的IBCR、IBCC、COM字段数据必须大于等于零,请检查！\n"));
		PauAndQuit();
	}
	//生成夹层体网格单元对象
	pCell = new  CNDIntBedCell;
	pCell->m_IROW = IROW;
	pCell->m_ICOL = ICOL;
	pCell->m_IBTHK = IBTHK;
	pCell->m_SEDMCR = IBCR;
	pCell->m_IBCC = IBCC;
	pCell->m_IBTHETA = IBTHETA;
	pCell->m_COM = COM;
	pCell->m_PCS = HCOFF;
	//加入单元数组
	long IBound = GDWMOD.GetCellIniIBOUND(m_ILYR, pCell->m_IROW, pCell->m_ICOL);
	if (IBound > 0)
	{
		//夹层体贮水仅对变水头单元有效
		m_pNDIntBedCellAry.push_back(pCell);
	}
}

void CNDelayIntBed::ReadNDIBCellData(_RecordsetPtr& pTable)
{
	//读取无延迟夹层体类在网格单元上的分布参数数据(数据库)

	long i;
	long nCount = 0;
	long IBound;
	CNDIntBedCell* pCell = NULL;
	_variant_t Value;
	try
	{
		pTable->MoveLast();
		nCount = pTable->GetRecordCount();  //得到单元个数
		pTable->MoveFirst();
		for (i = 0; i<nCount; i++)
		{
			pCell = new  CNDIntBedCell;
			Value = pTable->GetCollect(_T("IROW"));
			pCell->m_IROW = Value.intVal;
			Value = pTable->GetCollect(_T("ICOL"));
			pCell->m_ICOL = Value.intVal;
			Value = pTable->GetCollect(_T("IBTHK"));
			pCell->m_IBTHK = Value.dblVal;
			Value = pTable->GetCollect(_T("IBCR"));
			pCell->m_SEDMCR = Value.dblVal;
			Value = pTable->GetCollect(_T("IBCC"));
			pCell->m_IBCC = Value.dblVal;
			Value = pTable->GetCollect(_T("IBTHETA"));
			pCell->m_IBTHETA = Value.dblVal;
			Value = pTable->GetCollect(_T("COM"));
			pCell->m_COM = Value.dblVal;
			Value = pTable->GetCollect(_T("HCOFF"));
			pCell->m_PCS = Value.dblVal;
			//检查参数的有效性
			if (pCell->m_IBTHK <= 0.0 || pCell->m_IBTHETA <= 0.0)
			{
				printf(_T("\n"));
				printf(_T("数据表的 IBTHK、IBTHETA 字段数据必须大于零,请检查！\n"));
				PauAndQuit();
			}
			if (pCell->m_SEDMCR < 0.0 || pCell->m_IBCC < 0.0 || pCell->m_COM < 0.0)
			{
				printf(_T("\n"));
				printf(_T("数据表的 IBCR、IBCC、COM字段数据必须大于等于零,请检查！\n"));
				PauAndQuit();
			}
			//检查地下水单元的初始属性值
			IBound = GDWMOD.GetCellIniIBOUND(m_ILYR, pCell->m_IROW, pCell->m_ICOL);
			if (IBound > 0)
			{
				//夹层体贮水仅对变水头单元有效
				m_pNDIntBedCellAry.push_back(pCell);
			}
			pTable->MoveNext();                                       //定位到下一条数据
		}
	}
	catch (_com_error *e)
	{
		printf((LPCTSTR)e->ErrorMessage());
	}
}

void CNDelayIntBed::InitSimulation()
{
	//模拟之前进行初始化

	CNDIntBedCell* pNDIntBedCell = NULL;
	for (size_t i = 0; i < m_pNDIntBedCellAry.size(); i++)
	{
		pNDIntBedCell = m_pNDIntBedCellAry[i];
		//初始化上个时段的孔隙率和厚度
		pNDIntBedCell->m_IBTHETAOLD = pNDIntBedCell->m_IBTHETA;
		pNDIntBedCell->m_IBTHKOLD = pNDIntBedCell->m_IBTHK;
	}
}

void CNDelayIntBed::SetIniPar()
{
	//设置初始模拟参数值

	double CellBot, CellSGS, CellEST, CellLyrCent, CellArea, EstCenter, Coef1, Coef2;
	CNDIntBedCell* pNDIntBedCell = NULL;
	long CSOPT = m_pLandSub->GetCSOPT();
	long PCSOPT = m_pLandSub->GetPCSOPT();
	for (size_t i = 0; i < m_pNDIntBedCellAry.size(); i++)
	{
		pNDIntBedCell = m_pNDIntBedCellAry[i];
		CellEST = m_pLandSub->GetEST(m_ILYR, pNDIntBedCell->m_IROW, pNDIntBedCell->m_ICOL);
		//计算网格单元底板处当前的初始预固结应力
		if (PCSOPT == 1)
		{
			//用户直接输入初始预固结应力值, 检查预固结应力值数据的合理性
			if (pNDIntBedCell->m_PCS < 0.0)
			{
				printf(_T("\n"));
				printf(_T("位于第%d层第%d行第%d列的无延迟夹层体，其初始预固结应力值(%f)小于0.0, 请检查!\n"),
					m_ILYR, pNDIntBedCell->m_IROW, pNDIntBedCell->m_ICOL, pNDIntBedCell->m_PCS);
				PauAndQuit();
			}
		}
		else
		{
			//用户输入为初始预固结应力值与初始有效应力值之差, 计算初始预固结应力值
			pNDIntBedCell->m_PCS = CellEST + pNDIntBedCell->m_PCS;
		}
		//预固结应力值必须大于等于当前有效应力值
		pNDIntBedCell->m_PCS = max(pNDIntBedCell->m_PCS, CellEST);
		//计算贮水参数
		CellBot = GDWMOD.GetCellBot(m_ILYR, pNDIntBedCell->m_IROW, pNDIntBedCell->m_ICOL);
		CellArea = GDWMOD.GetCellArea(pNDIntBedCell->m_IROW, pNDIntBedCell->m_ICOL);
		CellSGS = m_pLandSub->GetSGS(m_ILYR, pNDIntBedCell->m_IROW, pNDIntBedCell->m_ICOL);
		CellLyrCent = m_pLandSub->GetLyrCent(m_ILYR, pNDIntBedCell->m_IROW, pNDIntBedCell->m_ICOL);
		EstCenter = CellEST - (CellLyrCent - CellBot) * (CellSGS - 1.0);
		Coef1 = EstCenter * CellArea * pNDIntBedCell->m_IBTHK;
		if (CSOPT == 1)
		{
			//输入的贮水参数是压缩指数
			Coef2 = 0.4342945 * CellArea * pNDIntBedCell->m_IBTHK /
				(1.0 + pNDIntBedCell->m_IBTHETA / (1.0 - pNDIntBedCell->m_IBTHETA));
			pNDIntBedCell->m_SEDMCR = Coef2 * pNDIntBedCell->m_SEDMCR;
			pNDIntBedCell->m_IBCC = Coef2 * pNDIntBedCell->m_IBCC;
			pNDIntBedCell->m_SKE = pNDIntBedCell->m_SEDMCR / Coef1;
			pNDIntBedCell->m_SKV = pNDIntBedCell->m_IBCC / Coef1;
		}
		else
		{
			//输入的贮水参数是贮水率
			pNDIntBedCell->m_SKE = pNDIntBedCell->m_SEDMCR;
			pNDIntBedCell->m_SKV = pNDIntBedCell->m_IBCC;
			pNDIntBedCell->m_SEDMCR = Coef1 * pNDIntBedCell->m_SEDMCR;
			pNDIntBedCell->m_IBCC = Coef1 * pNDIntBedCell->m_IBCC;
		}
	}
}

CMemTable* CNDelayIntBed::m_pMemNDBDataTable = new CMemTable;
void CNDelayIntBed::MakeNDBDataTable(_RecordsetPtr pTable, CString TbName)
{
	//构建无延迟夹层体类模拟结果输出缓存表

	m_pMemNDBDataTable->MakeDataTable(pTable, TbName);
}

void CNDelayIntBed::WriteDatabase()
{
	//将内存数据写入数据库

	m_pMemNDBDataTable->WriteDatabase();
}

void CNDelayIntBed::NDIBOutPut(long PrnID, long iPer, long iStep, double CumTLen, CStdioFile& OutFile)
{
	//输出无延迟夹层体类模拟结果(TXT)

	size_t i;
	size_t size = m_pNDIntBedCellAry.size();
	CNDIntBedCell* pNDBCell = NULL;
	double CellLyrCent, CellBot, CellSGS, ESADJ;
	for (i = 0; i < size; i++)
	{
		pNDBCell = m_pNDIntBedCellAry[i];
		CellLyrCent = m_pLandSub->GetLyrCent(m_ILYR, pNDBCell->m_IROW, pNDBCell->m_ICOL);
		CellSGS = m_pLandSub->GetSGS(m_ILYR, pNDBCell->m_IROW, pNDBCell->m_ICOL);
		CellBot = GDWMOD.GetCellBot(m_ILYR, pNDBCell->m_IROW, pNDBCell->m_ICOL);
		ESADJ = (CellLyrCent - CellBot) * (CellSGS - 1.0);
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
		fprintf(OutFile.m_pStream, _T(" %-10d"), m_NDBedID);
		fprintf(OutFile.m_pStream, _T(" %-10d"), m_ILYR);
		fprintf(OutFile.m_pStream, _T(" %-10d"), pNDBCell->m_IROW);
		fprintf(OutFile.m_pStream, _T(" %-10d"), pNDBCell->m_ICOL);
		fprintf(OutFile.m_pStream, _T(" %-16.8g"), pNDBCell->m_COM);
		fprintf(OutFile.m_pStream, _T(" %-16.8g"), pNDBCell->m_PCS - ESADJ);
		fprintf(OutFile.m_pStream, _T(" %-16.8g"), pNDBCell->m_IBTHETA);
		fprintf(OutFile.m_pStream, _T(" %-16.8g"), pNDBCell->m_IBTHK);
		fprintf(OutFile.m_pStream, _T(" %-16.8g"), pNDBCell->m_SKE);
		fprintf(OutFile.m_pStream, _T(" %-16.8g"), pNDBCell->m_SKV);
		fprintf(OutFile.m_pStream, _T("\n"));
	}
}

void CNDelayIntBed::NDIBOutPut(long PrnID, long iPer, long iStep, double CumTLen)
{
	//输出无延迟夹层体类模拟结果(数据库)

	size_t i;
	size_t size = m_pNDIntBedCellAry.size();
	CNDIntBedCell* pNDBCell = NULL;
	double CellLyrCent, CellBot, CellSGS, ESADJ;
	try
	{
		DataRow^ dr;
		//输出各网格单元内夹层体的模拟结果
		for (i = 0; i < size; i++)
		{
			pNDBCell = m_pNDIntBedCellAry[i];
			CellLyrCent = m_pLandSub->GetLyrCent(m_ILYR, pNDBCell->m_IROW, pNDBCell->m_ICOL);
			CellSGS = m_pLandSub->GetSGS(m_ILYR, pNDBCell->m_IROW, pNDBCell->m_ICOL);
			CellBot = GDWMOD.GetCellBot(m_ILYR, pNDBCell->m_IROW, pNDBCell->m_ICOL);
			ESADJ = (CellLyrCent - CellBot) * (CellSGS - 1.0);
			dr = m_pMemNDBDataTable->GetDataTable()->NewRow();
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
			dr[3] = m_NDBedID;
			dr[4] = m_ILYR;
			dr[5] = pNDBCell->m_IROW;
			dr[6] = pNDBCell->m_ICOL;
			dr[7] = pNDBCell->m_COM;
			dr[8] = pNDBCell->m_PCS - ESADJ;
			dr[9] = pNDBCell->m_IBTHETA;
			dr[10] = pNDBCell->m_IBTHK;
			dr[11] = pNDBCell->m_SKE;
			dr[12] = pNDBCell->m_SKV;
			m_pMemNDBDataTable->GetDataTable()->Rows->Add(dr);
		}
	}
	catch (_com_error *e)
	{
		printf((LPCTSTR)e->ErrorMessage());
	}
}

///////////////////////////////////
//有延迟夹层体类类函数定义
CDelayIntBed::CDelayIntBed()
{
	m_DBedID = 0;            //有延迟夹层体类的编号(从1开始)
	m_ILYR = 0;              //有延迟夹层体类所位于的含水层层位
	m_pLandSub = NULL;       //地面沉降模拟类指针
}

CDelayIntBed::~CDelayIntBed()
{
	//清理开辟的指针对象

	for (size_t i = 0; i < m_pDIntBedCellAry.size(); i++)
	{
		delete m_pDIntBedCellAry[i];
	}
	m_pDIntBedCellAry.clear();
}

void CDelayIntBed::ReadDIBCellData(CString& strData, long Opt)
{
	//读入单元数据(TXT)

	long IDB, IROW, ICOL, IMZ;
	double RNB, DSH, HCOFF, COM, EZ;
	CDIntBedCell* pCell = NULL;
	CMatZone* pMatZone = NULL;
	CString strFormat;
	strFormat = _T("%ld %ld %ld %lg %lg %lg %lg %lg %ld");
	if (sscanf_s(strData, strFormat, &IDB, &IROW, &ICOL, &RNB, &DSH,
		&HCOFF, &COM, &EZ, &IMZ) != 9)
	{
		printf(_T("\n"));
		printf(_T("数据行中的数据不足或有错误,请检查 地面沉降_有延迟夹层体属性表.in !\n"));
		printf(_T("第%d个夹层体类的网格单元数据 !\n"), m_DBedID);
		PauAndQuit();
	}
	//检查数据
	assert(IDB == m_DBedID);
	GDWMOD.CheckCellLimit(IROW, ICOL);
	if (RNB < 0.0 || COM < 0.0 || HCOFF <0.0 || EZ < 0.0)
	{
		printf(_T("\n"));
		printf(_T("数据表的RNB、HCOFF、COM、EZ字段数据必须大于等于零,请检查！\n"));
		PauAndQuit();
	}
	pMatZone = m_pLandSub->GetMatZone(IMZ);
	if (pMatZone == NULL)
	{
		printf(_T("\n"));
		printf(_T("标号为%d的夹层体在第%d行第%d列的介质带的编号错误,请检查！\n"),
			m_DBedID, IROW, ICOL);
		PauAndQuit();
	}
	//生成夹层体网格单元对象
	pCell = new  CDIntBedCell;
	pCell->m_IROW = IROW;
	pCell->m_ICOL = ICOL;
	pCell->m_RNB = RNB;
	pCell->m_DSH = DSH;
	pCell->m_DHC = HCOFF;
	pCell->m_COM = COM;
	pCell->m_DeltZ = EZ;
	pCell->m_IMZ = IMZ;
	pCell->m_pMatZone = pMatZone;
	//检查地下水单元的初始属性值
	long IBound;
	IBound = GDWMOD.GetCellIniIBOUND(m_ILYR, pCell->m_IROW, pCell->m_ICOL);
	if (IBound > 0 && pCell->m_RNB >= 1.0)
	{
		//夹层体贮水仅对变水头单元有效
		m_pDIntBedCellAry.push_back(pCell);
	}
}

void CDelayIntBed::ReadDIBCellData(_RecordsetPtr& pTable, long Opt)
{
	//读取有延迟夹层体类在网格单元上的分布参数数据(数据库)

	long nCount = 0;
	long i, IBound;
	CDIntBedCell* pCell = NULL;
	CMatZone* pMatZone = NULL;
	_variant_t Value;
	try
	{
		pTable->MoveLast();
		nCount = pTable->GetRecordCount();  //得到单元个数
		pTable->MoveFirst();
		for (i = 0; i<nCount; i++)
		{
			pCell = new  CDIntBedCell;
			Value = pTable->GetCollect(_T("IROW"));
			pCell->m_IROW = Value.intVal;
			Value = pTable->GetCollect(_T("ICOL"));
			pCell->m_ICOL = Value.intVal;
			Value = pTable->GetCollect(_T("RNB"));
			pCell->m_RNB = Value.dblVal;
			if (Opt == 1)
			{
				//如果使用表中的数据
				Value = pTable->GetCollect(_T("DSH"));
				pCell->m_DSH = Value.dblVal;
			}
			Value = pTable->GetCollect(_T("HCOFF"));
			pCell->m_DHC = Value.dblVal;
			Value = pTable->GetCollect(_T("COM"));
			pCell->m_COM = Value.dblVal;
			Value = pTable->GetCollect(_T("EZ"));
			pCell->m_DeltZ = Value.dblVal;
			Value = pTable->GetCollect(_T("IMZ"));
			pCell->m_IMZ = Value.intVal;
			//检查参数的有效性
			if (pCell->m_RNB < 0.0 || pCell->m_COM < 0.0 || pCell->m_DHC < 0.0 || pCell->m_DeltZ < 0.0)
			{
				printf(_T("\n"));
				printf(_T("数据表的RNB、HCOFF、COM、EZ字段数据必须大于等于零,请检查！\n"));
				PauAndQuit();
			}
			//检查介质带的有效性
			pMatZone = m_pLandSub->GetMatZone(pCell->m_IMZ);
			if (pMatZone == NULL)
			{
				printf(_T("\n"));
				printf(_T("标号为%d的夹层体在第%d行第%d列的介质带的编号错误,请检查！\n"), 
                    m_DBedID, pCell->m_IROW, pCell->m_ICOL);
				PauAndQuit();
			}
			pCell->m_pMatZone = pMatZone;
			//检查地下水单元的初始属性值
			IBound = GDWMOD.GetCellIniIBOUND(m_ILYR, pCell->m_IROW,
				pCell->m_ICOL);
			if (IBound > 0 && pCell->m_RNB >= 1.0)
			{
				//夹层体贮水仅对变水头单元有效
				m_pDIntBedCellAry.push_back(pCell);
			}
			pTable->MoveNext();                                       //定位到下一条数据
		}
	}
	catch (_com_error *e)
	{
		printf((LPCTSTR)e->ErrorMessage());
	}
}

void CDelayIntBed::AddObsCell(long iRow, long iCol)
{
	//添加第iRow行第ICol列的无延迟夹层体观测点

	CDIntBedCell* pDIBCell = NULL;
	long IniIBound;
	for (size_t i = 0; i < m_pDIntBedCellAry.size(); i++)
	{
		if (m_pDIntBedCellAry[i]->m_IROW == iRow && m_pDIntBedCellAry[i]->m_ICOL == iCol)
		{
			pDIBCell = m_pDIntBedCellAry[i];
			break;
		}
	}
	if (pDIBCell == NULL)
	{
		//没有找到对应的夹层体类单元指针.报错
		printf(_T("\n"));
		printf(_T("编号为%d的有延迟夹层体类没有第%d行第%d列的网格单元数据! 请检查<地面沉降_有延迟夹层体观测点表>!\n"),
			m_DBedID, iRow, iCol);
		PauAndQuit();
	}
	else
	{
		IniIBound = GDWMOD.GetCellIniIBOUND(m_ILYR, pDIBCell->m_IROW, pDIBCell->m_ICOL);
		if (IniIBound <= 0)
		{
			//夹层体位于无效单元或定水头单元上.报错
			printf(_T("\n"));
			printf(_T("编号为%d的有延迟夹层体类第%d行第%d列的网格单元为无效单元或定水头单元! 请检查<地面沉降_有延迟夹层体观测点表>!\n"),
				m_DBedID, iRow, iCol);
			PauAndQuit();
		}
	}
	m_ObsDIBCellAry.push_back(pDIBCell);
}

void CDelayIntBed::InitSimulation(long Opt)
{
	//模拟之前进行初始化

	CDIntBedCell* pDIntBedCell = NULL;
	long iRow, iCol;
	CGridCell* pGridCell = NULL;
	for (size_t i = 0; i < m_pDIntBedCellAry.size(); i++)
	{
		pDIntBedCell = m_pDIntBedCellAry[i];
		//设置每个夹层体单元上等效夹层体的离散点个数
		pDIntBedCell->m_Node = m_pLandSub->GetNumDivN();
		//如果采用网格单元的初始水头数据作为有延迟夹层体的初始水头
		if (Opt == 2)
		{
			iRow = pDIntBedCell->m_IROW;
			iCol = pDIntBedCell->m_ICOL;
			pGridCell = GDWMOD.GetGridCell(m_ILYR, iRow, iCol);
			pDIntBedCell->m_DSH = pGridCell->GetStarHead();
		}
		//夹层体单元进行初始化
		pDIntBedCell->InitSimulation(m_pLandSub->GetDISOPT(), m_pLandSub->GetKMOPT());
	}
}

void CDelayIntBed::SetIniPar()
{
	//设置初始模拟参数值

	long iRow, iCol, CSOPT, PCSOPT;
	double CellBot, CellSGS, IniEST, IBEST, CellHead, CellTotGL, CellLyrCent, 
		ESTCenter, Coef;
	CDIntBedCell* pDIntBedCell = NULL;
	CSOPT = m_pLandSub->GetCSOPT();
	PCSOPT = m_pLandSub->GetPCSOPT();
	for (size_t i = 0; i < m_pDIntBedCellAry.size(); i++)
	{
		pDIntBedCell = m_pDIntBedCellAry[i];
		iRow = pDIntBedCell->m_IROW;
		iCol = pDIntBedCell->m_ICOL;
		CellBot = GDWMOD.GetCellBot(m_ILYR, iRow, iCol);
		CellTotGL = m_pLandSub->GetTotGL(m_ILYR, iRow, iCol);
		CellHead = GDWMOD.GetCellHNew(m_ILYR, iRow, iCol);
		CellLyrCent = m_pLandSub->GetLyrCent(m_ILYR, iRow, iCol);
		//计算网格单元底板处的初始有效应力IniEST
		IniEST = CellTotGL - max(CellHead - CellBot, 0.0);
		//计算夹层体自身的有效应力IBEST
		IBEST = CellTotGL - max(pDIntBedCell->m_DSH - CellBot, 0.0);
		//计算夹层体各单元的有效应力(注: 有效应力基于网格单元底板高程计算)
		for (size_t j = 0; j < pDIntBedCell->m_Node; j++)
		{
			pDIntBedCell->m_AryHEST[j] = IBEST;
			pDIntBedCell->m_AryHESTOLD[j] = pDIntBedCell->m_AryHEST[j];
		}
		//计算夹层体各单元的预固结应力(注: 预固结应力基于网格单元底板高程计算)
		if (PCSOPT == 1)
		{
			//输入参数为初始预固结应力的绝对值
			for (size_t j = 0; j < pDIntBedCell->m_Node; j++)
			{
				pDIntBedCell->m_AryHHC[j] = pDIntBedCell->m_DHC;
			}
		}
		else
		{
			//输入参数为初始预固结应力与夹层体初始有效应力的差值
			for (size_t j = 0; j < pDIntBedCell->m_Node; j++)
			{
				pDIntBedCell->m_AryHHC[j] = IBEST + pDIntBedCell->m_DHC;
			}
		}
		//设置夹层体的贮水参数
		if (CSOPT == 1)
		{
			//输入的贮水参数是压缩系数
			Coef = 0.4342945 / (1.0 + pDIntBedCell->m_pMatZone->m_MZTHETA /
				(1.0 - pDIntBedCell->m_pMatZone->m_MZTHETA));
			pDIntBedCell->m_SEDMCR = Coef * pDIntBedCell->m_pMatZone->m_MZCR;
			pDIntBedCell->m_IBCC = Coef * pDIntBedCell->m_pMatZone->m_MZCC;
		}
		else
		{
			//输入的贮水参数是贮水率
			CellSGS = m_pLandSub->GetSGS(m_ILYR, iRow, iCol);
			ESTCenter = IniEST - (CellLyrCent - CellBot) * (CellSGS - 1.0);
			pDIntBedCell->m_SEDMCR = ESTCenter * pDIntBedCell->m_pMatZone->m_MZCR;
			pDIntBedCell->m_IBCC = ESTCenter * pDIntBedCell->m_pMatZone->m_MZCC;
		}
	}
}

CMemTable* CDelayIntBed::m_pMemDBDataTable = new CMemTable;
CMemTable* CDelayIntBed::m_pMemObsDIBCellTable = new CMemTable;
void CDelayIntBed::MakeDBDataTable(_RecordsetPtr pTable, CString TbName)
{
	//构建水量平衡输出缓存表

	m_pMemDBDataTable->MakeDataTable(pTable, TbName);
}

void CDelayIntBed::MakeObsDIBCellTable(_RecordsetPtr pTable, CString TbName)
{
	//构建有延迟夹层体观测点输出缓存表

	m_pMemObsDIBCellTable->MakeDataTable(pTable, TbName);
}

void CDelayIntBed::WriteDatabase()
{
	//将内存数据写入数据库

	m_pMemDBDataTable->WriteDatabase();
	m_pMemObsDIBCellTable->WriteDatabase();
}

void CDelayIntBed::DIBOutPut(long PrnID, long iPer, long iStep, double CumTLen, CStdioFile& OutFile)
{
	//输出有延迟夹层体模拟结果(TXT)

	size_t i;
	size_t size = m_pDIntBedCellAry.size();
	CDIntBedCell* pDBCell = NULL;
	long DISOPT = m_pLandSub->GetDISOPT();
	long NN = m_pLandSub->GetNumDivN();
	double CellLyrCent, CellBot, CellSGS, ESADJ;
	long ObjNode;
	if (DISOPT == 1)
	{
		//全厚度离散格式
		ObjNode = long(NN / 2.0) + 1;
	}
	else
	{
		//半厚度离散格式
		ObjNode = NN - 1;
	}
	for (i = 0; i < size; i++)
	{
		pDBCell = m_pDIntBedCellAry[i];
		CellLyrCent = m_pLandSub->GetLyrCent(m_ILYR, pDBCell->m_IROW, pDBCell->m_ICOL);
		CellSGS = m_pLandSub->GetSGS(m_ILYR, pDBCell->m_IROW, pDBCell->m_ICOL);
		CellBot = GDWMOD.GetCellBot(m_ILYR, pDBCell->m_IROW, pDBCell->m_ICOL);
		ESADJ = (CellLyrCent - CellBot) * (CellSGS - 1.0);
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
		fprintf(OutFile.m_pStream, _T(" %-10d"), m_DBedID);
		fprintf(OutFile.m_pStream, _T(" %-10d"), m_ILYR);
		fprintf(OutFile.m_pStream, _T(" %-10d"), pDBCell->m_IROW);
		fprintf(OutFile.m_pStream, _T(" %-10d"), pDBCell->m_ICOL);
		fprintf(OutFile.m_pStream, _T(" %-16.8g"), pDBCell->m_AryHHNew[ObjNode]);
		fprintf(OutFile.m_pStream, _T(" %-16.8g"), pDBCell->m_COM);
		fprintf(OutFile.m_pStream, _T(" %-16.8g"), pDBCell->m_AryHEST[ObjNode] - ESADJ);
		fprintf(OutFile.m_pStream, _T(" %-16.8g"), pDBCell->m_AryHHC[ObjNode] - ESADJ);
		fprintf(OutFile.m_pStream, _T(" %-16.8g"), pDBCell->m_RERR);
		fprintf(OutFile.m_pStream, _T("\n"));
	}
}

void CDelayIntBed::DIBOutPut(long PrnID, long iPer, long iStep, double CumTLen)
{
	//输出有延迟夹层体模拟结果(数据库)

	size_t i;
	size_t size = m_pDIntBedCellAry.size();
	CDIntBedCell* pDBCell = NULL;
	long DISOPT = m_pLandSub->GetDISOPT();
	long NN = m_pLandSub->GetNumDivN();
	double CellLyrCent, CellBot, CellSGS, ESADJ;
	long ObjNode;
	if (DISOPT == 1)
	{
		//全厚度离散格式
		ObjNode = long(NN / 2.0) + 1;
	}
	else
	{
		//半厚度离散格式
		ObjNode = NN - 1;
	}
	try
	{
		DataRow^ dr;
		//输出各网格单元内夹层体的压实量和预固结水头
		for (i = 0; i < size; i++)
		{
			pDBCell = m_pDIntBedCellAry[i];
			CellLyrCent = m_pLandSub->GetLyrCent(m_ILYR, pDBCell->m_IROW, pDBCell->m_ICOL);
			CellSGS = m_pLandSub->GetSGS(m_ILYR, pDBCell->m_IROW, pDBCell->m_ICOL);
			CellBot = GDWMOD.GetCellBot(m_ILYR, pDBCell->m_IROW, pDBCell->m_ICOL);
			ESADJ = (CellLyrCent - CellBot) * (CellSGS - 1.0);
			dr = m_pMemDBDataTable->GetDataTable()->NewRow();
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
			dr[3] = m_DBedID;
			dr[4] = m_ILYR;
			dr[5] = pDBCell->m_IROW;
			dr[6] = pDBCell->m_ICOL;
			dr[7] = pDBCell->m_AryHHNew[ObjNode];
			dr[8] = pDBCell->m_COM;
			dr[9] = pDBCell->m_AryHEST[ObjNode] - ESADJ;
			dr[10] = pDBCell->m_AryHHC[ObjNode] - ESADJ;
			dr[11] = pDBCell->m_RERR;
			m_pMemDBDataTable->GetDataTable()->Rows->Add(dr);
		}
	}
	catch (_com_error *e)
	{
		printf((LPCTSTR)e->ErrorMessage());
	}
}

void CDelayIntBed::ObsOutPut(long iPer, long iStep, double CumTLen)
{
	//输出有延迟夹层体观测点模拟结果(数据库)

	size_t i, j;
	CDIntBedCell* pDBCell = NULL;
	long KMOPT = m_pLandSub->GetKMOPT();
	double DeltT = GDWMOD.GetDeltT();
	try
	{
		DataRow^ dr;
		for (i = 0; i < (long)m_ObsDIBCellAry.size(); i++)
		{
			pDBCell = m_ObsDIBCellAry[i];
			for (j = 0; j < pDBCell->m_Node; j++)
			{
				dr = m_pMemObsDIBCellTable->GetDataTable()->NewRow();
				dr[0] = iPer;
				dr[1] = iStep;
				dr[2] = CumTLen;
				dr[3] = m_DBedID;
				dr[4] = m_ILYR;
				dr[5] = pDBCell->m_IROW;
				dr[6] = pDBCell->m_ICOL;
				dr[7] = j + 1;
				//输出节点单元的厚度
				dr[8] = pDBCell->m_AryThic[j];
				//输出节点单元的水头
				dr[9] = pDBCell->m_AryHHNew[j];
				//输出节点单元的孔隙率
				dr[10] = pDBCell->m_AryTheta[j];
				//输出节点单元的垂向渗透系数
				if (KMOPT == 0)
				{
					dr[11] = pDBCell->m_pMatZone->m_MZVK;
				}
				else
				{
					dr[11] = pDBCell->m_AryKz[j];
				}
				//输出节点单元的弹性贮水率
				dr[12] = pDBCell->m_ArySKE[j] * DeltT / pDBCell->m_AryThic[j];
				//输出节点单元的非弹性贮水率
				dr[13] = pDBCell->m_ArySKV[j] * DeltT / pDBCell->m_AryThic[j];
				m_pMemObsDIBCellTable->GetDataTable()->Rows->Add(dr);
			}
		}
	}
	catch (_com_error* e)
	{
		printf((LPCTSTR)e->ErrorMessage());
	}
}

void CDelayIntBed::ObsOutPut(long iPer, long iStep, double CumTLen, CStdioFile& OutFile)
{
	//输出有延迟夹层体观测点模拟结果(数据库)
	long i, j;
	CDIntBedCell* pDBCell = NULL;
	long KMOPT = m_pLandSub->GetKMOPT();
	double DeltT = GDWMOD.GetDeltT();
	for (i = 0; i < (long)m_ObsDIBCellAry.size(); i++)
	{
		pDBCell = m_ObsDIBCellAry[i];
		for (j = 0; j < pDBCell->m_Node; j++)
		{
			fprintf(OutFile.m_pStream, _T(" %-10d"), iPer);
			fprintf(OutFile.m_pStream, _T(" %-10d"), iStep);
			fprintf(OutFile.m_pStream, _T(" %-16.8g"), CumTLen);
			fprintf(OutFile.m_pStream, _T(" %-10d"), m_DBedID);
			fprintf(OutFile.m_pStream, _T(" %-10d"), m_ILYR);
			fprintf(OutFile.m_pStream, _T(" %-10d"), pDBCell->m_IROW);
			fprintf(OutFile.m_pStream, _T(" %-10d"), pDBCell->m_ICOL);
			fprintf(OutFile.m_pStream, _T(" %-10d"), j + 1);
			fprintf(OutFile.m_pStream, _T(" %-16.8g"), pDBCell->m_AryThic[j]);
			fprintf(OutFile.m_pStream, _T(" %-16.8g"), pDBCell->m_AryHHNew[j]);
			fprintf(OutFile.m_pStream, _T(" %-16.8g"), pDBCell->m_AryTheta[j]);
			if (KMOPT == 0)
			{
				fprintf(OutFile.m_pStream, _T(" %-16.8g"), pDBCell->m_pMatZone->m_MZVK);
			}
			else
			{
				fprintf(OutFile.m_pStream, _T(" %-16.8g"), pDBCell->m_AryKz[j]);
			}
			fprintf(OutFile.m_pStream, _T(" %-16.8g"), pDBCell->m_ArySKE[j] * DeltT / pDBCell->m_AryThic[j]);
			fprintf(OutFile.m_pStream, _T(" %-16.8g"), pDBCell->m_ArySKV[j] * DeltT / pDBCell->m_AryThic[j]);
			fprintf(OutFile.m_pStream, _T("\n"));
		}
	}
}

//////////////////////////////////////////////////////////
//地面沉降网格单元类函数定义
CLandSubCell::CLandSubCell()
{
	m_STOPGL = 0.0;                            //应力期初网格单元处地面以上的应力负荷(以水柱高计)(L)
	m_ETOPGL = 0.0;                            //应力期末网格单元处地面以上的应力负荷(以水柱高计)(L)
}

CLandSubCell::~CLandSubCell()
{

}

///////////////////////////////////
//地面沉降模拟类函数定义
CLandSub::CLandSub()
{
	m_NNDB = 0;                                //无延迟夹层体类的数量
	m_NDB = 0;                                 //有延迟夹层体类的数量
	m_DISOPT =0;                               //仅NDB > 0时有效, 为等效夹层体采用的离散格式. 1: 采用全厚度离散格式; 2:采用半厚度离散格式.
	m_NMZ = 0;                                 //仅NDB>0时有效, 为介质带的数量
	m_WSS = 0.0;                               //纯水的贮水率(1 / L)
	m_NN = 0;                                  //仅NDB>0时有效, 为有延迟夹层体其等效夹层体全厚度或半厚度上离散点的数量
	m_DSHOPT = 0;                              //仅NDB>0时有效, 为如何确定有延迟夹层体初始水头值的选项. 
	m_CSOPT = 0;                               //夹层体(有/无延迟均适用)贮水参数使用选项. 1: 使用贮水率; 2: 使用压缩系数
	m_PCSOPT = 0;                              //夹层体(有 / 无延迟均适用)初始预固结应力(以水柱高计, 单位为L)输入选项. 1: 输入预固结应力的绝对数值; 
	                                           //2: 输入预固结应力与初始有效应力(模型自动计算)之差
	m_KMOPT = 0;                               //仅NDB>0时有效, 为模拟过程中是否根据形变调整夹层体的渗透系数K值选项. 0: 不调整; 1: 调整
	m_OBSOPT = 0;                              //仅NDB > 0时有效, 为是否输出有延迟夹层体观测点模拟结果选项. 0: 不输出; 1: 输出.
	m_PRNOPT = 0;                              //是否输出地下水网格单元内粗颗粒沉积物、有/无延迟夹层体模拟数据选项. 0: 不输出; 1: 输出
	m_CGBndID = 0;                             //粗颗粒沉积物弹性释水源汇项统计编号
	m_IBEBndID = 0;                            //夹层体弹性释水源汇项统计编号
	m_IBVBndID = 0;                            //夹层体非弹性释水源汇项统计编号
	m_WSBndID = 0;                             //水体弹性释水源汇项统计编号
	m_NumLyr = 0;                              //网格层数
	m_NumRow = 0;                              //网格行数
	m_NumCol = 0;                              //网格列数
	m_AryLyrIBS = NULL;                        //表示模型层是否模拟夹层体贮水的数组(一维)
	m_AryTopGL = NULL;                         //网格单元地面以上应力负荷数组(二维)
	m_ArySGM = NULL;                           //网格单元沉积物非饱和比重数组(三维)
	m_ArySGS = NULL;                           //网格单元沉积物饱和比重数组(三维)
	m_AryTotGL = NULL;                         //网格单元上的总地应力(底板处)数组(三维)
	m_AryLyrCent = NULL;                       //网格单元的层中心位置数组(三维)
	m_AryThicFact = NULL;                      //网格单元的饱和厚度比例(三维)
	m_AryEST = NULL;                           //网格单元上当前有效应力(底板处)数组(三维)
	m_AryESTOld = NULL;                        //网格单元上时段初有效应力(底板处)数组(三维)
	m_AryVertComp = NULL;                      //各层网格单元压实量数组(三维)
	m_AryVertDisp = NULL;                      //各层网格单元垂向位移量数组(三维)
}

CLandSub::~CLandSub()
{
	//清理开辟的指针对象
	
	long i, k;
	//删除粗颗粒沉积物指针数组
	for (i = 0; i < m_pCoGrainSedmAry.size(); i++)
	{
		delete m_pCoGrainSedmAry[i];
	}
	m_pCoGrainSedmAry.clear();
	//删除无延迟夹层体类指针对象
	for (i = 0; i < m_pNDIntBedAry.size(); i++)
	{
		delete m_pNDIntBedAry[i];
	}
	m_pNDIntBedAry.clear();
	//删除有延迟夹层体类指针对象
	for (i = 0; i < m_pDIntBedAry.size(); i++)
	{
		delete m_pDIntBedAry[i];
	}
	m_pDIntBedAry.clear();
	//删除介质带指针对象
	for (i = 0; i < m_pMatZoneAry.size(); i++)
	{
		delete m_pMatZoneAry[i];
	}
	m_pMatZoneAry.clear();
	//删除模型层是否模拟夹层体贮水标识数组
	if (m_AryLyrIBS != NULL)
	{
		delete[] m_AryLyrIBS;
	}
	//清除网格单元地面以上应力负荷数组(二维)
	if (m_AryTopGL != NULL)
	{
		for (i = 0; i < m_NumRow; i++)
		{
			delete[] m_AryTopGL[i];
		}
		delete[] m_AryTopGL;
		m_AryTopGL = NULL;
	}
	//清除网格单元沉积物非饱和比重数组(三维)
	if (m_ArySGM != NULL)
	{
		for (k = 0; k < m_NumLyr; k++)
		{
			for (i = 0; i < m_NumRow; i++)
			{
				delete[] m_ArySGM[k][i];                //释放列指针
			}
			delete[] m_ArySGM[k];                       //释放行指针
		}
		delete[] m_ArySGM;                              //释放层指针
		m_ArySGM = NULL;
	}
	//清除网格单元沉积物饱和比重数组(三维)
	if (m_ArySGS != NULL)
	{
		for (k = 0; k < m_NumLyr; k++)
		{
			for (i = 0; i < m_NumRow; i++)
			{
				delete[] m_ArySGS[k][i];                //释放列指针
			}
			delete[] m_ArySGS[k];                       //释放行指针
		}
		delete[] m_ArySGS;                              //释放层指针
		m_ArySGS = NULL;
	}
	//清除网格单元上的总地应力数组(三维)
	if (m_AryTotGL != NULL)
	{
		for (k = 0; k < m_NumLyr; k++)
		{
			for (i = 0; i < m_NumRow; i++)
			{
				delete[] m_AryTotGL[k][i];                //释放列指针
			}
			delete[] m_AryTotGL[k];                       //释放行指针
		}
		delete[] m_AryTotGL;                              //释放层指针
		m_AryTotGL = NULL;
	}
	//清除网格单元的层中心位置数组(三维)
	if (m_AryLyrCent != NULL)
	{
		for (k = 0; k < m_NumLyr; k++)
		{
			for (i = 0; i < m_NumRow; i++)
			{
				delete[] m_AryLyrCent[k][i];                //释放列指针
			}
			delete[] m_AryLyrCent[k];                       //释放行指针
		}
		delete[] m_AryLyrCent;                              //释放层指针
		m_AryLyrCent = NULL;
	}
	//清除网格单元的饱和厚度比例数组(三维)
	if (m_AryThicFact != NULL)
	{
		for (k = 0; k < m_NumLyr; k++)
		{
			for (i = 0; i < m_NumRow; i++)
			{
				delete[] m_AryThicFact[k][i];                //释放列指针
			}
			delete[] m_AryThicFact[k];                       //释放行指针
		}
		delete[] m_AryThicFact;                              //释放层指针
		m_AryThicFact = NULL;
	}
	//清除网格单元上的有效应力(底板处)数组(三维)
	if (m_AryEST != NULL)
	{
		for (k = 0; k < m_NumLyr; k++)
		{
			for (i = 0; i < m_NumRow; i++)
			{
				delete[] m_AryEST[k][i];                //释放列指针
			}
			delete[] m_AryEST[k];                       //释放行指针
		}
		delete[] m_AryEST;                              //释放层指针
		m_AryEST = NULL;
	}
	//清除网格单元上时段初有效应力(底板处)数组(三维)
	if (m_AryESTOld != NULL)
	{
		for (k = 0; k < m_NumLyr; k++)
		{
			for (i = 0; i < m_NumRow; i++)
			{
				delete[] m_AryESTOld[k][i];                //释放列指针
			}
			delete[] m_AryESTOld[k];                       //释放行指针
		}
		delete[] m_AryESTOld;                              //释放层指针
		m_AryESTOld = NULL;
	}
	//清除各层网格单元压实量数组(三维)
	if (m_AryVertComp != NULL)
	{
		for (k = 0; k < m_NumLyr; k++)
		{
			for (i = 0; i < m_NumRow; i++)
			{
				delete[] m_AryVertComp[k][i];                //释放列指针
			}
			delete[] m_AryVertComp[k];                       //释放行指针
		}
		delete[] m_AryVertComp;                              //释放层指针
		m_AryVertComp = NULL;
	}
	//清除各层网格单元垂向位移量数组(三维)
	if (m_AryVertDisp != NULL)
	{
		for (k = 0; k < m_NumLyr; k++)
		{
			for (i = 0; i < m_NumRow; i++)
			{
				delete[] m_AryVertDisp[k][i];                //释放列指针
			}
			delete[] m_AryVertDisp[k];                       //释放行指针
		}
		delete[] m_AryVertDisp;                              //释放层指针
		m_AryVertDisp = NULL;
	}
	//清除查询表
	m_NDIntBetMap.clear();
	m_DIntBetMap.clear();

	//关闭模拟结果输出文件
	if (m_SubOutFile.m_hFile != CFile::hFileNull)
	{
		m_SubOutFile.Close();
	}
	if (m_CGOutPutFile.m_hFile != CFile::hFileNull)
	{
		m_CGOutPutFile.Close();
	}
	if (m_NDBOutPutFile.m_hFile != CFile::hFileNull)
	{
		m_NDBOutPutFile.Close();
	}
	if (m_DBOutPutFile.m_hFile != CFile::hFileNull)
	{
		m_DBOutPutFile.Close();
	}
	if (m_OBSOutPutFile.m_hFile != CFile::hFileNull)
	{
		m_OBSOutPutFile.Close();
	}
}

void CLandSub::AllocMem()
{
	//分配数组 

	m_NumLyr = GDWMOD.GetNumLyr();
	m_NumRow = GDWMOD.GetNumRow();
	m_NumCol = GDWMOD.GetNumCol();
	long i, k;
	try
	{
		//开辟表示模型层是否模拟夹层体贮水的数组(一维)
		m_AryLyrIBS = new long[m_NumLyr]();
		//开辟地面负荷数组(二维)
		m_AryTopGL = new double* [m_NumRow]();
		for (i = 0; i < m_NumRow; i++)
		{
			m_AryTopGL[i] = new double[m_NumCol]();
		}
		//开辟各层网格单元压实量、垂向位移及沉积物比重数组(三维)
		m_ArySGM = new double** [m_NumLyr]();
		m_ArySGS = new double** [m_NumLyr]();
		m_AryTotGL = new double** [m_NumLyr]();
		m_AryLyrCent = new double** [m_NumLyr]();
		m_AryThicFact = new double** [m_NumLyr]();
		m_AryEST = new double** [m_NumLyr]();
		m_AryESTOld = new double** [m_NumLyr]();
		m_AryVertComp = new double** [m_NumLyr]();
		m_AryVertDisp = new double** [m_NumLyr]();
		for (k = 0; k < m_NumLyr; k++)
		{
			m_ArySGM[k] = new double* [m_NumRow]();
			m_ArySGS[k] = new double* [m_NumRow]();
			m_AryTotGL[k] = new double* [m_NumRow]();
			m_AryLyrCent[k] = new double* [m_NumRow]();
			m_AryThicFact[k] = new double* [m_NumRow]();
			m_AryEST[k] = new double* [m_NumRow]();
			m_AryESTOld[k] = new double* [m_NumRow]();
			m_AryVertComp[k] = new double* [m_NumRow]();
			m_AryVertDisp[k] = new double* [m_NumRow]();
			for (i = 0; i < m_NumRow; i++)
			{
				m_ArySGM[k][i] = new double[m_NumCol]();
				m_ArySGS[k][i] = new double[m_NumCol]();
				m_AryTotGL[k][i] = new double[m_NumCol]();
				m_AryLyrCent[k][i] = new double[m_NumCol]();
				m_AryThicFact[k][i] = new double[m_NumCol]();
				m_AryEST[k][i] = new double[m_NumCol]();
				m_AryESTOld[k][i] = new double[m_NumCol]();
				m_AryVertComp[k][i] = new double[m_NumCol]();
				m_AryVertDisp[k][i] = new double[m_NumCol]();
			}
		}
	}
	catch (bad_alloc& e)
	{
		printf(e.what());
		exit(1);
	}
}

void CLandSub::ReadLandSubData(const CString& strFilePath)
{
	//读取地面沉降模拟控制参数(TXT)

	//输出数据表信息
	CString strTbInfo, strTemp;
	strTbInfo = _T("Sub_CtrlPar.in\n");
	printf(strTbInfo);

	//打开模拟控制参数数据文件
	CStdioFile CtrlDataFile;
	strTbInfo = strFilePath + _T("Sub_CtrlPar.in");
	if (!CtrlDataFile.Open(strTbInfo, CFile::modeRead | CFile::shareExclusive | CFile::typeText))
	{
		printf(_T("\n"));
		printf(_T("Fail to open <%s> file!\n"), _T("Sub_CtrlPar.in"));
		PauAndQuit();
	}
	//略过表头
	CtrlDataFile.ReadString(strTemp);

	//读入数据
	long NNDB, NDB, DISOPT, CSOPT, PCSOPT,  NMZ, NN, DSHOPT, KMOPT, OBSOPT, PRNOPT;
	double GAMAW, BETA;
	CString strTest;
	CString strFormat;
	strFormat = _T("%ld %ld %ld %ld %lg %lg %ld %ld %ld %ld %ld %ld %ld");
	long SumDataRow = 0;
	for (;;)
	{
		//逐行读入
		CtrlDataFile.ReadString(strTemp);
		//累加读入的数据行数
		SumDataRow = SumDataRow + 1;
		//测试读入的数据
		strTest = strTemp;
		strTest.Trim();
		//如果没有读到有效数据
		if (strTest.IsEmpty())
		{
			SumDataRow = SumDataRow - 1;
			if (CtrlDataFile.GetPosition() == CtrlDataFile.GetLength())
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
		if (sscanf_s(strTemp, strFormat, &NNDB, &NDB,  &CSOPT, &PCSOPT, &GAMAW, &BETA, 
			&DISOPT, &NMZ, &NN, &DSHOPT, &KMOPT, &OBSOPT, &PRNOPT) != 13)
		{
			//没有正常数据, 报错
			printf(_T("\n"));
			printf(_T("文件<%s>中第%d行数据的数据异常, 请检查!\n"),
				_T("地面沉降_模拟控制参数表.in"), SumDataRow);
			PauAndQuit();
		}
		//检查数据
		if (NNDB < 0 || NDB < 0)
		{
			printf(_T("\n"));
			printf(_T("NNDB 和 NDB 参数必须大于等于0, 请检查 地面沉降_模拟控制参数表.in !\n"));
			PauAndQuit();
		}
		if (CSOPT < 1 || CSOPT >2)
		{
			printf(_T("\n"));
			printf(_T("CSOPT 参数必须为1或2, 请检查 地面沉降_模拟控制参数表.in !\n"));
			PauAndQuit();
		}
		if (PCSOPT < 1 || PCSOPT >2)
		{
			printf(_T("\n"));
			printf(_T("PCSOPT 参数必须为1或2, 请检查 地面沉降_模拟控制参数表.in !\n"));
			PauAndQuit();
		}
		if (GAMAW < 0.0)
		{
			printf(_T("\n"));
			printf(_T("GAMAW 参数必须大于等于零, 请检查 地面沉降_模拟控制参数表.in !\n"));
			PauAndQuit();
		}
		if (BETA < 0.0)
		{
			printf(_T("\n"));
			printf(_T("BETA 参数必须大于等于零, 请检查 地面沉降_模拟控制参数表.in !\n"));
			PauAndQuit();
		}
		if (m_NDB > 0)
		{
			if (DISOPT < 1 || DISOPT >2)
			{
				printf(_T("\n"));
				printf(_T("DISOPT 参数必须为1或2, 请检查 地面沉降_模拟控制参数表.in !\n"));
				PauAndQuit();
			}
			if (NMZ <= 0)
			{
				printf(_T("\n"));
				printf(_T("模拟有延迟夹层体情况下 NMZ 参数不能为0, 请检查 地面沉降_模拟控制参数表.in !\n"));
				PauAndQuit();
			}
			if (NN <= 3)
			{
				printf(_T("\n"));
				printf(_T("NN 参数需大于等于3, 请检查 地面沉降_模拟控制参数表.in !\n"));
				PauAndQuit();
			}
			if (DSHOPT < 1 || DSHOPT >2)
			{
				printf(_T("\n"));
				printf(_T("DSHOPT 参数必须为1或2, 请检查 地面沉降_模拟控制参数表.in !\n"));
				PauAndQuit();
			}
			if (KMOPT < 0 || KMOPT >1)
			{
				printf(_T("\n"));
				printf(_T("KMOPT 参数必须为0或1, 请检查 地面沉降_模拟控制参数表.in !\n"));
				PauAndQuit();
			}
			if (OBSOPT < 0 || OBSOPT >1)
			{
				printf(_T("\n"));
				printf(_T("OBSOPT 参数必须为0或1, 请检查<地面沉降_模拟控制参数表>！\n"));
				PauAndQuit();
			}
		}
		if (PRNOPT < 0 || PRNOPT >1)
		{
			printf(_T("\n"));
			printf(_T("PRNOPT 参数必须为0或1, 请检查 地面沉降_模拟控制参数表.in !\n"));
			PauAndQuit();
		}
		//保存数据
		m_NNDB = NNDB;
		m_NDB = NDB;
		m_DISOPT = DISOPT;
		m_CSOPT = CSOPT;
		m_PCSOPT = PCSOPT;
		m_KMOPT = KMOPT;
		m_OBSOPT = OBSOPT;
		m_PRNOPT = PRNOPT;
		m_WSS = GAMAW * BETA;
		if (NDB > 0)
		{
			m_NMZ = NMZ;
			m_NN = NN;
			m_DSHOPT = DSHOPT;
		}
		//只需读入一行有效数据
		break;
	}

	//检查是否读到了有效数据
	if (SumDataRow == 0)
	{
		printf(_T("\n"));
		printf(_T("地面沉降_模拟控制参数表.in 中没有数据, 请检查!\n"));
		PauAndQuit();
	}

	//后面不会再读入数据了
	CtrlDataFile.Close();
}

void CLandSub::ReadLandSubData(_ConnectionPtr pConnection)
{
	//读取地面沉降基础数据(数据库)
	//注：必须用try块,否则数据表不存在时程序直接会报错,无法继续执行

	printf(_T("<地面沉降_模拟控制参数表>\n"));
	try
	{
		//打开数据表读数据
		CString strSQL;
		_RecordsetPtr pTable;
		pTable.CreateInstance(__uuidof(Recordset));
		double GAMAW, BETA;
		//将数据集游标设置在客户端(可提高性能)
		pTable->CursorLocation = adUseClient;
		//设置数据集缓存(1000条)
		pTable->CacheSize = 1000;
		strSQL = _T("SELECT * FROM 地面沉降_模拟控制参数表");
		pTable->Open(LPCSTR(strSQL),
			pConnection.GetInterfacePtr(),
			adOpenStatic,
			adLockOptimistic,
			adCmdText);
		if (pTable->BOF)
		{
			//数据表不存在或是空表, 报错退出
			//关闭表并返回FALSE
			pTable->Close();
			printf(_T("\n"));
			printf(_T("已选择模拟地面沉降，<地面沉降_模拟控制参数表>不能为空表,请检查！\n"));
			PauAndQuit();
		}
		//数据表存在且有数据
		pTable->MoveLast();
		long nCount;
		nCount = pTable->GetRecordCount();  //数据个数
		if (nCount != 1)
		{
			printf(_T("\n"));
			printf(_T("<地面沉降_模拟控制参数表>只能有1行数据记录,请检查！\n"));
			PauAndQuit();
		}
		//读入地面沉降模拟控制性参数
		_variant_t Value;
		pTable->MoveFirst();
		Value = pTable->GetCollect(_T("NNDB"));
		m_NNDB = Value.intVal;
		Value = pTable->GetCollect(_T("NDB"));
		m_NDB = Value.intVal;
		if (m_NNDB < 0 || m_NDB < 0)
		{
			printf(_T("\n"));
			printf(_T("NNDB 和 NDB 参数必须大于等于0, 请检查<地面沉降_模拟控制参数表>！\n"));
			PauAndQuit();
		}
		Value = pTable->GetCollect(_T("CSOPT"));
		m_CSOPT = Value.intVal;
		if (m_CSOPT < 1 || m_CSOPT >2)
		{
			printf(_T("\n"));
			printf(_T("CSOPT 参数必须为1或2, 请检查 地面沉降_模拟控制参数表.in !\n"));
			PauAndQuit();
		}
		Value = pTable->GetCollect(_T("PCSOPT"));
		m_PCSOPT = Value.intVal;
		if (m_PCSOPT < 1 || m_PCSOPT >2)
		{
			printf(_T("\n"));
			printf(_T("PCSOPT 参数必须为1或2, 请检查 地面沉降_模拟控制参数表.in !\n"));
			PauAndQuit();
		}
		Value = pTable->GetCollect(_T("GAMAW"));
		GAMAW = Value.dblVal;
		if (GAMAW < 0.0)
		{
			printf(_T("\n"));
			printf(_T("GAMAW 参数必须大于等于零, 请检查 地面沉降_模拟控制参数表.in !\n"));
			PauAndQuit();
		}
		Value = pTable->GetCollect(_T("BETA"));
		BETA = Value.dblVal;
		if (BETA < 0.0)
		{
			printf(_T("\n"));
			printf(_T("BETA 参数必须大于等于零, 请检查 地面沉降_模拟控制参数表.in !\n"));
			PauAndQuit();
		}
		m_WSS = GAMAW * BETA;
		if (m_NDB > 0)
		{
			Value = pTable->GetCollect(_T("DISOPT"));
			m_DISOPT = Value.intVal;
			if (m_DISOPT < 1 || m_DISOPT >2)
			{
				printf(_T("\n"));
				printf(_T("DISOPT 参数必须为1或2, 请检查 地面沉降_模拟控制参数表.in !\n"));
				PauAndQuit();
			}
			Value = pTable->GetCollect(_T("NMZ"));
			m_NMZ = Value.intVal;
			if (m_NMZ <= 0)
			{
				printf(_T("\n"));
				printf(_T("模拟有延迟夹层体情况下 NMZ 参数不能为0, 请检查<地面沉降_模拟控制参数表>！\n"));
				PauAndQuit();
			}
			Value = pTable->GetCollect(_T("NN"));
			m_NN = Value.intVal;
			if (m_NN <= 3)
			{
				printf(_T("\n"));
				printf(_T("NN 参数需大于等于3, 请检查<地面沉降_模拟控制参数表>！\n"));
				PauAndQuit();
			}
			Value = pTable->GetCollect(_T("DSHOPT"));
			m_DSHOPT = Value.intVal;
			if (m_DSHOPT < 1 || m_DSHOPT >2)
			{
				printf(_T("\n"));
				printf(_T("DSHOPT 参数必须为1或2, 请检查<地面沉降_模拟控制参数表>！\n"));
				PauAndQuit();
			}
			Value = pTable->GetCollect(_T("KMOPT"));
			m_KMOPT = Value.intVal;
			if (m_KMOPT < 0 || m_KMOPT >1)
			{
				printf(_T("\n"));
				printf(_T("KMOPT 参数必须为0或1, 请检查<地面沉降_模拟控制参数表>！\n"));
				PauAndQuit();
			}
			Value = pTable->GetCollect(_T("OBSOPT"));
			m_OBSOPT = Value.intVal;
			if (m_OBSOPT < 0 || m_OBSOPT >1)
			{
				printf(_T("\n"));
				printf(_T("OBSOPT 参数必须为0或1, 请检查<地面沉降_模拟控制参数表>！\n"));
				PauAndQuit();
			}
		}
		Value = pTable->GetCollect(_T("PRNOPT"));
		m_PRNOPT = Value.intVal;
		if (m_PRNOPT < 0 || m_PRNOPT > 1)
		{
			printf(_T("\n"));
			printf(_T("PRNOPT 参数必须为0或1, 请检查 地面沉降_模拟控制参数表.in !\n"));
			PauAndQuit();
		}
		pTable->Close();
	}
	catch (_com_error e)
	{
		printf(LPCTSTR(e.Description()));
		exit(1);
	}
}

void CLandSub::ReadCoGrainData(_ConnectionPtr pConnection)
{
	//读取粗颗粒沉积物属性表数据(数据库)

	long i, iLyr;
	_variant_t Value;
	CString strSQL, str1, str2;
	_RecordsetPtr pTable;

	//生成粗颗粒沉积物指针数组
	CCGSediment* pCCGSediment = NULL;
	for (iLyr = 0; iLyr < GDWMOD.GetNumLyr(); iLyr++)
	{
		pCCGSediment = new CCGSediment;
		pCCGSediment->m_ILYR = iLyr + 1;
		pCCGSediment->m_pLandSub = this;
		m_pCoGrainSedmAry.push_back(pCCGSediment);
	}
		
	//读入粗颗粒沉积物网格单元数据
	printf(_T("<地面沉降_粗颗粒沉积物属性表>\n"));
	try
	{
		pTable.CreateInstance(__uuidof(Recordset));
		//将数据集游标设置在客户端(可提高性能)
		pTable->CursorLocation = adUseClient;
		//设置数据集缓存(1000条)
		pTable->CacheSize = 1000;
		//逐一分层粗颗粒沉积物读入单元数据
		for (i = 0; i < m_pCoGrainSedmAry.size(); i++)
		{
			//从数据表中挑选出编号为i的数据记录
			strSQL = _T("SELECT * FROM 地面沉降_粗颗粒沉积物属性表");
			str1 = _T(" WHERE ILYR = ");
			str2.Format(_T("%d"), m_pCoGrainSedmAry[i]->m_ILYR);
			str1 = str1 + str2;
			str2 = _T(" ORDER BY IROW,ICOL");
			str1 = str1 + str2;
			//最终形成的SQL执行语句
			strSQL = strSQL + str1;
			//打开数据表	
			pTable->Open(LPCSTR(strSQL),
				pConnection.GetInterfacePtr(),
				adOpenStatic,
				adLockOptimistic,
				adCmdText);
			if (!pTable->BOF)
			{
				m_pCoGrainSedmAry[i]->ReadCGCellData(pTable);
			}
			pTable->Close();
		}
	}
	catch (_com_error e)
	{
		printf(LPCTSTR(e.Description()));
		exit(1);
	}
}

void CLandSub::ReadCoGrainData(const CString& strFilePath)
{
	//读取粗颗粒沉积物属性表数据(TXT)

	CStdioFile CellDataFile;
	CString strTbInfo, strTemp;
	CString strTest;
	CString strFormat;
	long iLyr, SumDataRow;

	//生成粗颗粒沉积物指针数组
	CCGSediment* pCCGSediment = NULL;
	for (iLyr = 0; iLyr < GDWMOD.GetNumLyr(); iLyr++)
	{
		pCCGSediment = new CCGSediment;
		pCCGSediment->m_ILYR = iLyr + 1;
		pCCGSediment->m_pLandSub = this;
		m_pCoGrainSedmAry.push_back(pCCGSediment);
	}

	//输出数据表信息
	strTbInfo = _T("Sub_CoGrain.in\n");
	printf(strTbInfo);

	//读入粗颗粒沉积物网格单元数据
	//打开数据文件
	strTbInfo = strFilePath + _T("Sub_CoGrain.in");
	if (!CellDataFile.Open(strTbInfo, CFile::modeRead | CFile::shareExclusive | CFile::typeText))
	{
		printf(_T("\n"));
		printf(_T("Fail to open <%s> file!\n"), _T("Sub_CoGrain.in"));
		PauAndQuit();
	}
	//略过表头
	CellDataFile.ReadString(strTemp);
	//读入数据
	strFormat = _T("%ld");
	SumDataRow = 0;
	for (;;)
	{
		//逐行读入
		CellDataFile.ReadString(strTemp);
		//累加读入的数据行数
		SumDataRow = SumDataRow + 1;
		//测试读入的数据
		strTest = strTemp;
		strTest.Trim();
		//如果没有读到有效数据
		if (strTest.IsEmpty())
		{
			SumDataRow = SumDataRow - 1;
			if (CellDataFile.GetPosition() == CellDataFile.GetLength())
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
		//确定数据所属的层位
		if (sscanf_s(strTemp, strFormat, &iLyr) != 1)
		{
			//没有正常数据, 报错
			printf(_T("\n"));
			printf(_T("文件<%s>中第%d行数据的数据异常, 请检查!\n"),
				_T("地面沉降_粗颗粒沉积物属性表.in"), SumDataRow);
			PauAndQuit();
		}
		if (iLyr < 1 || iLyr > GDWMOD.GetNumLyr())
		{
			//层位数据错误
			printf(_T("\n"));
			printf(_T("文件<%s>中第%d行数据的层位数据错误, 请检查!\n"),
				_T("地面沉降_粗颗粒沉积物属性表.in"), SumDataRow);
			PauAndQuit();
		}
		pCCGSediment = m_pCoGrainSedmAry[iLyr - 1];
		//粗颗粒沉积物读入单元数据
		pCCGSediment->ReadCGCellData(strTemp);
	}
	//关闭数据表
	CellDataFile.Close();
}

void CLandSub::ReadIBSLyrData(const CString& strFilePath)
{
	//读取各个夹层体类的层位数据(TXT)

	CStdioFile LyrDataFile;
	CString strTbInfo, strTemp;
	CString strTest;
	CString strFormat;
	long INDB, IDB, ILYR, SumDataRow;
	//先读入无延迟夹层体类数据
	if (m_NNDB > 0)
	{
		//输出数据表信息
		strTbInfo = _T("Sub_NDIBLyr.in\n");
		printf(strTbInfo);
		//打开数据文件
		strTbInfo = strFilePath + _T("Sub_NDIBLyr.in");
		if (!LyrDataFile.Open(strTbInfo, CFile::modeRead | CFile::shareExclusive | CFile::typeText))
		{
			printf(_T("\n"));
			printf(_T("打开文件<%s>失败!\n"), _T("Sub_NDIBLyr.in"));
			PauAndQuit();
		}
		//略过表头
		LyrDataFile.ReadString(strTemp);
		//读入数据
		strFormat = _T("%ld %ld");
		SumDataRow = 0;
		for (;;)
		{
			//逐行读入
			LyrDataFile.ReadString(strTemp);
			//累加读入的数据行数
			SumDataRow = SumDataRow + 1;
			//测试读入的数据
			strTest = strTemp;
			strTest.Trim();
			//如果没有读到有效数据
			if (strTest.IsEmpty())
			{
				SumDataRow = SumDataRow - 1;
				if (LyrDataFile.GetPosition() == LyrDataFile.GetLength())
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
			if (sscanf_s(strTemp, strFormat, &INDB, &ILYR) != 2)
			{
				//没有正常数据, 报错
				printf(_T("\n"));
				printf(_T("文件<%s>中第%d行数据的数据异常, 请检查!\n"),
					_T("地面沉降_无延迟夹层体层位表.in"), SumDataRow);
				PauAndQuit();
			}
			//检查数据
			if (SumDataRow != INDB)
			{
				printf(_T("\n"));
				printf(_T("地面沉降_无延迟夹层体层位表.in 中 NDB 字段数据必须从1开始编号并连续!\n"));
				printf(_T("请检查该文件中的第%d行数据!\n"), SumDataRow);
				PauAndQuit();
			}
			if (ILYR < 1 || ILYR > GDWMOD.GetNumLyr())
			{
				//夹层体类所位于的含水层层位不对. 报错
				printf(_T("\n"));
				printf(_T("地面沉降_无延迟夹层体层位表.in 中编号为%d的夹层体所位于的含水层层位不对, 请检查!\n"),
					INDB);
				PauAndQuit();
			}
			//产生一个无延迟夹层体类对象
			CNDelayIntBed* pNDIntBed = new CNDelayIntBed;
			pNDIntBed->m_NDBedID = INDB;
			pNDIntBed->m_ILYR = ILYR;
			pNDIntBed->m_pLandSub = this;
			//保存指针到数组
			m_pNDIntBedAry.push_back(pNDIntBed);
			m_NDIntBetMap[INDB] = pNDIntBed;
		}
		//关闭数据表
		LyrDataFile.Close();

		//检查无延迟夹层体数据数量
		if ((long)m_pNDIntBedAry.size() != m_NNDB)
		{
			printf(_T("\n"));
			printf(_T("地面沉降_无延迟夹层体层位表 中夹层体的数量与 地面沉降_模拟控制参数表.in 中的不符, 请检查!\n"));
			PauAndQuit();
		}
	}

	//再读入有延迟夹层体类数据
	if (m_NDB > 0)
	{
		//输出数据表信息
		strTbInfo = _T("Sub_DIBLyr.in\n");
		printf(strTbInfo);
		//打开数据文件
		strTbInfo = strFilePath + _T("Sub_DIBLyr.in");
		if (!LyrDataFile.Open(strTbInfo, CFile::modeRead | CFile::shareExclusive | CFile::typeText))
		{
			printf(_T("\n"));
			printf(_T("Fail to open <%s> file!\n"), _T("Sub_DIBLyr.in"));
			PauAndQuit();
		}
		//略过表头
		LyrDataFile.ReadString(strTemp);
		//读入数据
		strFormat = _T("%ld %ld");
		SumDataRow = 0;
		for (;;)
		{
			//逐行读入
			LyrDataFile.ReadString(strTemp);
			//累加读入的数据行数
			SumDataRow = SumDataRow + 1;
			//测试读入的数据
			strTest = strTemp;
			strTest.Trim();
			//如果没有读到有效数据
			if (strTest.IsEmpty())
			{
				SumDataRow = SumDataRow - 1;
				if (LyrDataFile.GetPosition() == LyrDataFile.GetLength())
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
			if (sscanf_s(strTemp, strFormat, &IDB, &ILYR) != 2)
			{
				//没有正常数据, 报错
				printf(_T("\n"));
				printf(_T("文件<%s>中第%d行数据的数据异常, 请检查!\n"),
					_T("地面沉降_有延迟夹层体层位表.in"), SumDataRow);
				PauAndQuit();
			}
			//检查数据
			if (SumDataRow != IDB)
			{
				printf(_T("\n"));
				printf(_T("地面沉降_有延迟夹层体层位表.in 中 IDB 字段数据必须从1开始编号并连续!\n"));
				printf(_T("请检查该文件中的第%d行数据!\n"), SumDataRow);
				PauAndQuit();
			}
			if (ILYR < 1 || ILYR > GDWMOD.GetNumLyr())
			{
				//夹层体类所位于的含水层层位不对. 报错
				printf(_T("\n"));
				printf(_T("地面沉降_有延迟夹层体层位表.in 中编号为%d的夹层体所位于的含水层层位不对, 请检查!\n"),
					IDB);
				PauAndQuit();
			}
			//产生一个有延迟夹层体类对象
			CDelayIntBed* pDIntBed = new CDelayIntBed;
			pDIntBed->m_DBedID = IDB;
			pDIntBed->m_ILYR = ILYR;
			pDIntBed->m_pLandSub = this;
			//保存指针到数组
			m_pDIntBedAry.push_back(pDIntBed);
			m_DIntBetMap[IDB] = pDIntBed;
		}
		//关闭数据表
		LyrDataFile.Close();

		//检查有延迟夹层体数据数量
		if ((long)m_pDIntBedAry.size() != m_NDB)
		{
			printf(_T("\n"));
			printf(_T("地面沉降有延迟夹层体层位表.in 中夹层体的数量与 地面沉降_模拟控制参数表.in 中的不符, 请检查!\n"));
			PauAndQuit();
		}
	}
}

void CLandSub::ReadIBSLyrData(_ConnectionPtr pConnection)
{
	//读取各个夹层体类的层位数据(数据库)
	//注：必须用try块,否则数据表不存在时程序直接会报错,无法继续执行

	long i, nCount, iInterBed, ILYR;
	_variant_t Value;
	CString strSQL, str1, str2;
	_RecordsetPtr pTable;

	//先读入无延迟夹层体类数据
	if (m_NNDB > 0)
	{
		printf(_T("<地面沉降_无延迟夹层体层位表>\n"));
		try
		{
			pTable.CreateInstance(__uuidof(Recordset));
			//将数据集游标设置在客户端(可提高性能)
			pTable->CursorLocation = adUseClient;
			//设置数据集缓存(1000条)
			pTable->CacheSize = 1000;
			strSQL = _T("SELECT * FROM 地面沉降_无延迟夹层体层位表 ORDER BY INDB,ILYR");
			pTable->Open(LPCSTR(strSQL),
				pConnection.GetInterfacePtr(),
				adOpenStatic,
				adLockOptimistic,
				adCmdText);
			if (!pTable->BOF)
			{
				pTable->MoveLast();
				nCount = pTable->GetRecordCount();  //数据个数
				if (nCount != m_NNDB)
				{
					//数据记录个数不对, 报错
					printf(_T("\n"));
					printf(_T("<地面沉降_无延迟夹层体层位表>中必须有%d条数据! 请检查\n"),
						m_NNDB);
					PauAndQuit();
				}
				pTable->MoveFirst();
				for (i = 0; i < nCount; i++)
				{
					Value = pTable->GetCollect(_T("INDB"));
					iInterBed = Value.intVal;
					if (iInterBed != i + 1)
					{
						//夹层体类的编号不对,没有从1开始. 报错
						printf(_T("\n"));
						printf(_T("<地面沉降_无延迟夹层体层位表>中 INDB 字段数据必须从1开始编号, 请检查!\n"));
						PauAndQuit();
					}
					Value = pTable->GetCollect(_T("ILYR"));
					ILYR = Value.intVal;
					if (ILYR < 1 || ILYR > GDWMOD.GetNumLyr())
					{
						//夹层体类所位于的含水层层位不对. 报错
						printf(_T("\n"));
						printf(_T("<地面沉降_无延迟夹层体层位表>中编号为%d的夹层体类所位于的含水层层位不对, 请检查!\n"),
							iInterBed);
						PauAndQuit();
					}
					//产生一个无延迟夹层体类对象
					CNDelayIntBed* pNDIntBed = new CNDelayIntBed;
					pNDIntBed->m_NDBedID = iInterBed;
					pNDIntBed->m_ILYR = ILYR;
					pNDIntBed->m_pLandSub = this;
					m_pNDIntBedAry.push_back(pNDIntBed);
					m_NDIntBetMap[iInterBed] = pNDIntBed;
					pTable->MoveNext();
				}
				//关闭数据表
				pTable->Close();
			}
			else
			{
				printf(_T("\n"));
				printf(_T("<地面沉降_无延迟夹层体层位表>是空表!请检查!\n"));
				PauAndQuit();
			}
		}
		catch (_com_error e)
		{
			printf(LPCTSTR(e.Description()));
			exit(1);
		}
	}

	//再读入有延迟夹层体数据
	if (m_NDB > 0)
	{
		printf(_T("<地面沉降_有延迟夹层体层位表>\n"));
		try
		{
			pTable.CreateInstance(__uuidof(Recordset));
			//将数据集游标设置在客户端(可提高性能)
			pTable->CursorLocation = adUseClient;
			//设置数据集缓存(1000条)
			pTable->CacheSize = 1000;
			strSQL = _T("SELECT * FROM 地面沉降_有延迟夹层体层位表 ORDER BY IDB,ILYR");
			pTable->Open(LPCSTR(strSQL),
				pConnection.GetInterfacePtr(),
				adOpenStatic,
				adLockOptimistic,
				adCmdText);
			if (!pTable->BOF)
			{
				pTable->MoveLast();
				nCount = pTable->GetRecordCount();  //数据个数
				if (nCount != m_NDB)
				{
					//数据记录个数不对, 报错
					printf(_T("\n"));
					printf(_T("<地面沉降_有延迟夹层体层位表>中必须有%d条数据! 请检查\n"),
						m_NDB);
					PauAndQuit();
				}
				pTable->MoveFirst();
				for (i = 0; i < nCount; i++)
				{
					Value = pTable->GetCollect(_T("IDB"));
					iInterBed = Value.intVal;
					if (iInterBed != i + 1)
					{
						//夹层体类的编号不对,没有从1开始. 报错
						printf(_T("\n"));
						printf(_T("<地面沉降_有延迟夹层体层位表>中 NDB 字段数据必须从1开始编号, 请检查!\n"));
						PauAndQuit();
					}
					Value = pTable->GetCollect(_T("ILYR"));
					ILYR = Value.intVal;
					if (ILYR < 1 || ILYR > GDWMOD.GetNumLyr())
					{
						//夹层体类所位于的含水层层位不对. 报错
						printf(_T("\n"));
						printf(_T("<地面沉降_有延迟夹层体层位表>中编号为%d的夹层体类所位于的含水层层位不对, 请检查!\n"),
							iInterBed);
						PauAndQuit();
					}
					//产生一个有延迟夹层体类对象
					CDelayIntBed* pDIntBed = new CDelayIntBed;
					pDIntBed->m_DBedID = iInterBed;
					pDIntBed->m_ILYR = ILYR;
					pDIntBed->m_pLandSub = this;
					m_pDIntBedAry.push_back(pDIntBed);
					m_DIntBetMap[iInterBed] = pDIntBed;
					pTable->MoveNext();
				}
				//关闭数据表
				pTable->Close();
			}
			else
			{
				printf(_T("\n"));
				printf(_T("<地面沉降_有延迟夹层体层位表>是空表!请检查!\n"));
				PauAndQuit();
			}
		}
		catch (_com_error e)
		{
			printf(LPCTSTR(e.Description()));
			exit(1);
		}
	}
}

void CLandSub::ReadIBSCellData(const CString& strFilePath)
{
	//读取各个夹层体类的单元数据(TXT)

	CStdioFile CellDataFile;
	CString strTbInfo, strTemp;
	CString strTest;
	CString strFormat;

	//先读入无延迟夹层体类数据
	CNDelayIntBed* pNDIntBed = NULL;
	long INDB, SumDataRow;
	if (m_NNDB > 0)
	{
		//输出数据表信息
		strTbInfo = _T("Sub_NDIBCell.in\n");
		printf(strTbInfo);
		//打开数据文件
		strTbInfo = strFilePath + _T("Sub_NDIBCell.in");
		if (!CellDataFile.Open(strTbInfo, CFile::modeRead | CFile::shareExclusive | CFile::typeText))
		{
			printf(_T("\n"));
			printf(_T("打开文件<%s>失败!\n"), _T("Sub_NDIBCell.in"));
			PauAndQuit();
		}
		//略过表头
		CellDataFile.ReadString(strTemp);
		//读入数据
		strFormat = _T("%ld");
		SumDataRow = 0;
		for (;;)
		{
			//逐行读入
			CellDataFile.ReadString(strTemp);
			//累加读入的数据行数
			SumDataRow = SumDataRow + 1;
			//测试读入的数据
			strTest = strTemp;
			strTest.Trim();
			//如果没有读到有效数据
			if (strTest.IsEmpty())
			{
				SumDataRow = SumDataRow - 1;
				if (CellDataFile.GetPosition() == CellDataFile.GetLength())
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
			//确定数据所属的夹层体类
			if (sscanf_s(strTemp, strFormat, &INDB) != 1)
			{
				//没有正常数据, 报错
				printf(_T("\n"));
				printf(_T("文件<%s>中第%d行数据的数据异常, 请检查!\n"),
					_T("地面沉降_无延迟夹层体属性表.in"), SumDataRow);
				PauAndQuit();
			}
			auto result = m_NDIntBetMap.find(INDB);
			if (result == m_NDIntBetMap.end())
			{
				//没有找到对应的夹层体类指针.报错
				printf(_T("\n"));
				printf(_T("系统中没有编号为%d的无延迟夹层体! 请检查文件<%s>第%d行的数据!\n"),
					INDB, _T("地面沉降_无延迟夹层体属性表.in"), SumDataRow);
				PauAndQuit();
			}
			pNDIntBed = result->second;
			//夹层体类读入单元数据
			pNDIntBed->ReadNDIBCellData(strTemp);
		}
		//关闭数据表
		CellDataFile.Close();
		//确定每个无延迟夹层体都有单元数据
		for (size_t i = 0; i < m_pNDIntBedAry.size(); i++)
		{
			if (m_pNDIntBedAry[i]->m_pNDIntBedCellAry.size() == 0)
			{
				printf(_T("\n"));
				printf(_T("编号为%d的无延迟夹层体类没有网格单元数据, 请检查 地面沉降_无延迟夹层体属性表.in !\n"),
					m_pNDIntBedAry[i]->m_NDBedID);
				PauAndQuit();
			}
		}
	}

	//再读入有延迟夹层体类数据
	CDelayIntBed* pDIntBed = NULL;
	long IDB;
	if (m_NDB > 0)
	{
		//输出数据表信息
		strTbInfo = _T("Sub_DIBCell.in\n");
		printf(strTbInfo);
		//打开数据文件
		strTbInfo = strFilePath + _T("Sub_DIBCell.in");
		if (!CellDataFile.Open(strTbInfo, CFile::modeRead | CFile::shareExclusive | CFile::typeText))
		{
			printf(_T("\n"));
			printf(_T("Fail to open <%s> file!\n"), _T("Sub_DIBCell.in"));
			PauAndQuit();
		}
		//略过表头
		CellDataFile.ReadString(strTemp);
		//读入数据
		strFormat = _T("%ld");
		SumDataRow = 0;
		for (;;)
		{
			//逐行读入
			CellDataFile.ReadString(strTemp);
			//累加读入的数据行数
			SumDataRow = SumDataRow + 1;
			//测试读入的数据
			strTest = strTemp;
			strTest.Trim();
			//如果没有读到有效数据
			if (strTest.IsEmpty())
			{
				SumDataRow = SumDataRow - 1;
				if (CellDataFile.GetPosition() == CellDataFile.GetLength())
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
			//确定数据所属的夹层体类
			if (sscanf_s(strTemp, strFormat, &IDB) != 1)
			{
				//没有正常数据, 报错
				printf(_T("\n"));
				printf(_T("文件<%s>中第%d行数据的数据异常, 请检查!\n"),
					_T("地面沉降_有延迟夹层体属性表.in"), SumDataRow);
				PauAndQuit();
			}
			auto result = m_DIntBetMap.find(IDB);
			if (result == m_DIntBetMap.end())
			{
				//没有找到对应的夹层体类指针.报错
				printf(_T("\n"));
				printf(_T("系统中没有编号为%d的有延迟夹层体! 请检查文件<%s>第%d行的数据!\n"),
					IDB, _T("地面沉降_有延迟夹层体属性表.in"), SumDataRow);
				PauAndQuit();
			}
			pDIntBed = result->second;
			//夹层体类读入单元数据
			pDIntBed->ReadDIBCellData(strTemp, m_DSHOPT);
		}
		//关闭数据表
		CellDataFile.Close();
		//确定每个有延迟夹层体都有单元数据
		for (size_t i = 0; i < m_pDIntBedAry.size(); i++)
		{
			if (m_pDIntBedAry[i]->m_pDIntBedCellAry.size() == 0)
			{
				printf(_T("\n"));
				printf(_T("编号为%d的有延迟夹层体类没有网格单元数据, 请检查 地面沉降_有延迟夹层体属性表.in !\n"),
					m_pDIntBedAry[i]->m_DBedID);
				PauAndQuit();
			}
		}
	}
}

void CLandSub::ReadIBSCellData(_ConnectionPtr pConnection)
{
	//读取各个夹层体类的单元数据

	long i;
	_variant_t Value;
	CString strSQL, str1, str2;
	_RecordsetPtr pTable;

	//先读入无延迟夹层体类的单元数据
	if (m_NNDB > 0)
	{
		printf(_T("<地面沉降_无延迟夹层体属性表>\n"));
		try
		{
			pTable.CreateInstance(__uuidof(Recordset));
			//将数据集游标设置在客户端(可提高性能)
			pTable->CursorLocation = adUseClient;
			//设置数据集缓存(1000条)
			pTable->CacheSize = 1000;
			//逐一夹层体读入单元数据
			for (i = 0; i<m_pNDIntBedAry.size(); i++)
			{
				//从数据表中挑选出编号为i的数据记录
				strSQL = _T("SELECT * FROM 地面沉降_无延迟夹层体属性表");
				str1 = _T(" WHERE INDB = ");
				str2.Format(_T("%d"), m_pNDIntBedAry[i]->m_NDBedID);
				str1 = str1 + str2;
				str2 = _T(" ORDER BY IROW,ICOL");
				str1 = str1 + str2;
                //最终形成的SQL执行语句
				strSQL = strSQL + str1;    
			    //打开数据表	
				pTable->Open(LPCSTR(strSQL),
					pConnection.GetInterfacePtr(),
					adOpenStatic,
					adLockOptimistic,
					adCmdText);
				if (pTable->BOF)
				{
					//不可能找不到数据
					printf(_T("\n"));
					printf(_T("找不到第%d个夹层体类的单元数据，请检查<地面沉降_无延迟夹层体属性表>\n"), i);
					PauAndQuit();
				}
				if (!pTable->BOF)
				{
					m_pNDIntBedAry[i]->ReadNDIBCellData(pTable);
				}
				pTable->Close();
			}
		}
		catch (_com_error e)
		{
			printf(LPCTSTR(e.Description()));
			exit(1);
		}
	}

	//再读入有延迟夹层体类的单元数据
	if (m_NDB > 0)
	{
		printf(_T("<地面沉降_有延迟夹层体属性表>\n"));
		try
		{
			pTable.CreateInstance(__uuidof(Recordset));
			//将数据集游标设置在客户端(可提高性能)
			pTable->CursorLocation = adUseClient;
			//设置数据集缓存(1000条)
			pTable->CacheSize = 1000;
			//逐一夹层体类读入单元数据
			for (i = 0; i<m_pDIntBedAry.size(); i++)
			{
				//从数据表中挑选出编号为i的数据记录
				strSQL = _T("SELECT * FROM 地面沉降_有延迟夹层体属性表");
				str1 = _T(" WHERE IDB = ");
				str2.Format(_T("%d"), m_pDIntBedAry[i]->m_DBedID);
				str1 = str1 + str2;
				str2 = _T(" ORDER BY IROW,ICOL");
				str1 = str1 + str2;
				//最终形成的SQL执行语句
				strSQL = strSQL + str1;
				//打开数据表	
				pTable->Open(LPCSTR(strSQL),
					pConnection.GetInterfacePtr(),
					adOpenStatic,
					adLockOptimistic,
					adCmdText);
				if (pTable->BOF)
				{
					//不可能找不到数据
					printf(_T("\n"));
					printf(_T("找不到第%d个夹层体的单元数据，请检查<地面沉降_有延迟夹层体属性表>\n"), i);
					PauAndQuit();
				}
				if (!pTable->BOF)
				{
					m_pDIntBedAry[i]->ReadDIBCellData(pTable, m_DSHOPT);
				}
				pTable->Close();
			}
		}
		catch (_com_error e)
		{
			printf(LPCTSTR(e.Description()));
			exit(1);
		}
	}
}

void CLandSub::ReadMatParData(const CString& strFilePath)
{
	//读取介质带的参数数据(TXT)

	//仅在模拟有延迟夹层体时需读入
	if (m_NDB == 0)
	{
		return;
	}

	//输出数据表信息
	CString strTbInfo, strTemp;
	strTbInfo = _T("Sub_Material.in\n");
	printf(strTbInfo);

	//打开数据文件
	CStdioFile MzDataFile;
	strTbInfo = strFilePath + _T("Sub_Material.in");
	if (!MzDataFile.Open(strTbInfo, CFile::modeRead | CFile::shareExclusive | CFile::typeText))
	{
		printf(_T("\n"));
		printf(_T("Fail to open <%s> file!\n"), _T("Sub_Material.in"));
		PauAndQuit();
	}
	//略过表头
	MzDataFile.ReadString(strTemp);

	//读入数据
	long IMZ;
	double MZVK, MZCR, MZCC, MZTHETA;
	CMatZone* pMatZone = NULL;
	CString strTest;
	CString strFormat;
	strFormat = _T("%ld %lg %lg %lg %lg");
	long SumDataRow = 0;
	for (;;)
	{
		//逐行读入
		MzDataFile.ReadString(strTemp);
		//累加读入的数据行数
		SumDataRow = SumDataRow + 1;
		//测试读入的数据
		strTest = strTemp;
		strTest.Trim();
		//如果没有读到有效数据
		if (strTest.IsEmpty())
		{
			SumDataRow = SumDataRow - 1;
			if (MzDataFile.GetPosition() == MzDataFile.GetLength())
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
		if (sscanf_s(strTemp, strFormat, &IMZ, &MZVK, &MZCR, &MZCC, &MZTHETA) != 5)
		{
			//没有正常数据, 报错
			printf(_T("\n"));
			printf(_T("文件<%s>中第%d行数据的数据异常, 请检查!\n"),
				_T("地面沉降_介质带属性表.in"), SumDataRow);
			PauAndQuit();
		}
		//检查数据
		if (SumDataRow != IMZ)
		{
			printf(_T("\n"));
			printf(_T("介质带编号没从1开始或不连续! 请检查 地面沉降_介质带属性表.in\n"));
			PauAndQuit();
		}
		if (MZVK <= 0.0 || MZTHETA <=0.0)
		{
			printf(_T("\n"));
			printf(_T("地面沉降_介质带属性表.in 中字段 MZVK、MZTHETA 必须大于0.0, 请检查!\n"));
			PauAndQuit();
		}
		if (MZCR < 0.0 || MZCC < 0.0)
		{
			printf(_T("\n"));
			printf(_T("地面沉降_介质带属性表.in 中字段 MZCR、MZCC 必须大于等于0.0, 请检查!\n"));
			PauAndQuit();
		}
		//生成介质带对象
		pMatZone = new CMatZone;
		pMatZone->m_IMZ = IMZ;
		pMatZone->m_MZVK = MZVK;
		pMatZone->m_MZCR = MZCR;
		pMatZone->m_MZCC = MZCC;
		pMatZone->m_MZTHETA = MZTHETA;
		//将指针保存到数组
		m_pMatZoneAry.push_back(pMatZone);
	}

	//检查介质带数量
	if ((long)m_pMatZoneAry.size() != m_NMZ)
	{
		printf(_T("\n"));
		printf(_T("地面沉降_介质带属性表.in 中介质带的数量与 地面沉降_模拟控制参数表.in 中的不符, 请检查!\n"));
		PauAndQuit();
	}

	//后面不会再读入数据了
	MzDataFile.Close();
}

void CLandSub::ReadMatParData(_ConnectionPtr pConnection)
{
	//读取介质带的参数数据(数据库)
	//注：必须用try块,否则数据表不存在时程序直接会报错,无法继续执行

	//仅在模拟有延迟夹层体时需读入
	if (m_NDB > 0)
	{
		try
		{
			//打开数据表读数据
			long i, nCount;
			CString strSQL;
			_variant_t Value;
			_RecordsetPtr pTable;
			CMatZone* pMatZone = NULL;
			pTable.CreateInstance(__uuidof(Recordset));
			//将数据集游标设置在客户端(可提高性能)
			pTable->CursorLocation = adUseClient;
			//设置数据集缓存(1000条)
			pTable->CacheSize = 1000;
			strSQL = _T("SELECT * FROM 地面沉降_介质带属性表");
			pTable->Open(LPCSTR(strSQL),
				pConnection.GetInterfacePtr(),
				adOpenStatic,
				adLockOptimistic,
				adCmdText);
			if (pTable->BOF)
			{
				printf(_T("\n"));
				printf(_T("模拟地面延迟沉降，<地面沉降_介质带属性表>不能是空表,请检查！\n"));
				PauAndQuit();
			}
			//数据表存在且有数据
			printf(_T("<地面沉降_介质带属性表>\n"));
			pTable->MoveLast();
			nCount = pTable->GetRecordCount();  //数据个数
			if (nCount != m_NMZ)
			{
				printf(_T("\n"));
				printf(_T("<地面沉降_介质带属性表>必须有%d行数据记录,请检查！\n"), m_NMZ);
				PauAndQuit();
			}
			//读入介质带参数
			pTable->MoveFirst();
			for (i = 0; i < nCount; i++)
			{
				pMatZone = new CMatZone;
				Value = pTable->GetCollect(_T("IMZ"));
				pMatZone->m_IMZ = Value.intVal;
				Value = pTable->GetCollect(_T("MZVK"));
				pMatZone->m_MZVK = Value.dblVal;
				Value = pTable->GetCollect(_T("MZCR"));
				pMatZone->m_MZCR = Value.dblVal;
				Value = pTable->GetCollect(_T("MZCC"));
				pMatZone->m_MZCC = Value.dblVal;
				Value = pTable->GetCollect(_T("MZTHETA"));
				pMatZone->m_MZTHETA = Value.dblVal;
				//检查数据
				if (pMatZone->m_IMZ != i + 1)
				{
					printf(_T("\n"));
					printf(_T("介质带的编号必须从1开始并连续，请检查<地面沉降_介质带属性表>！\n"));
					PauAndQuit();
				}
				if (pMatZone->m_MZVK <= 0.0 || pMatZone->m_MZTHETA <= 0.0)
				{
					printf(_T("\n"));
					printf(_T("<地面沉降_介质带属性表>中字段 MZVK、MZTHETA必须大于0.0, 请检查！\n"));
					PauAndQuit();
				}
				if (pMatZone->m_MZCR < 0.0 || pMatZone->m_MZCC < 0.0)
				{
					printf(_T("\n"));
					printf(_T("<地面沉降_介质带属性表>中字段 MZCR、MZCC 必须大于等于0.0, 请检查！\n"));
					PauAndQuit();
				}
				m_pMatZoneAry.push_back(pMatZone);
				//下一条数据记录
				pTable->MoveNext();
			}
			pTable->Close();
		}
		catch (_com_error e)
		{
			printf(LPCTSTR(e.Description()));
			exit(1);
		}
	}
}

void CLandSub::ReadObsDIBData(const CString& strFilePath)
{
	//读取有延迟夹层体观测点数据(TXT文件)

	CStdioFile ObsDataFile;
	CString strTbInfo, strTemp;
	CString strTest;
	CString strFormat;
	CDelayIntBed* pDIntBed = NULL;
	long IDB, IROW, ICOL, SumDataRow;
	if (m_NDB > 0)
	{
		//输出数据表信息
		strTbInfo = _T("Sub_DIBObs.in\n");
		printf(strTbInfo);
		//打开数据文件
		strTbInfo = strFilePath + _T("Sub_DIBObs.in");
		if (!ObsDataFile.Open(strTbInfo, CFile::modeRead | CFile::shareExclusive | CFile::typeText))
		{
			printf(_T("Fail to open <%s> file!\n"), _T("Sub_DIBObs.in"));
			PauAndQuit();
		}
		//略过表头
		ObsDataFile.ReadString(strTemp);
		//读入数据
		strFormat = _T("%ld %ld %ld");
		SumDataRow = 0;
		for (;;)
		{
			//逐行读入
			ObsDataFile.ReadString(strTemp);
			//累加读入的数据行数
			SumDataRow = SumDataRow + 1;
			//测试读入的数据
			strTest = strTemp;
			strTest.Trim();
			//如果没有读到有效数据
			if (strTest.IsEmpty())
			{
				SumDataRow = SumDataRow - 1;
				if (ObsDataFile.GetPosition() == ObsDataFile.GetLength())
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
			//确定数据所属的夹层体类
			if (sscanf_s(strTemp, strFormat, &IDB, &IROW, &ICOL) != 3)
			{
				//没有正常数据, 报错
				printf(_T("文件<%s>中第%d行数据的数据异常, 请检查!\n"),
					_T("地面沉降_无延迟夹层体属性表.in"), SumDataRow);
				PauAndQuit();
			}
			auto result = m_DIntBetMap.find(IDB);
			if (result == m_DIntBetMap.end())
			{
				//没有找到对应的夹层体类指针.报错
				printf(_T("\n"));
				printf(_T("系统中没有编号为%d的有延迟夹层体类! 请检查<地面沉降_有延迟夹层体观测点表>!\n"), IDB);
				PauAndQuit();
			}
			pDIntBed = result->second;
			pDIntBed->AddObsCell(IROW, ICOL);
		}
		//关闭数据表
		ObsDataFile.Close();
	}
}

void CLandSub::ReadObsDIBData(_ConnectionPtr pConnection)
{
	//读取有延迟夹层体观测点数据(数据库)

	long i, nCount, iDIB, iRow, iCol;
	_variant_t Value;
	CString strSQL, str1, str2;
	_RecordsetPtr pTable;
	CDelayIntBed* pDIntBed = NULL;
	if (m_NDB > 0)
	{
		printf(_T("<地面沉降_有延迟夹层体观测点表>\n"));
		try
		{
			pTable.CreateInstance(__uuidof(Recordset));
			//将数据集游标设置在客户端(可提高性能)
			pTable->CursorLocation = adUseClient;
			//设置数据集缓存(1000条)
			pTable->CacheSize = 1000;
			strSQL = _T("SELECT * FROM 地面沉降_有延迟夹层体观测点表");
			pTable->Open(LPCSTR(strSQL),
				pConnection.GetInterfacePtr(),
				adOpenStatic,
				adLockOptimistic,
				adCmdText);
			if (!pTable->BOF)
			{
				pTable->MoveLast();
				nCount = pTable->GetRecordCount();  //数据个数
				pTable->MoveFirst();
				for (i = 0; i < nCount; i++)
				{
					Value = pTable->GetCollect(_T("IDB"));
					iDIB = Value.intVal;
					Value = pTable->GetCollect(_T("IROW"));
					iRow = Value.intVal;
					Value = pTable->GetCollect(_T("ICOL"));
					iCol = Value.intVal;
					auto result = m_DIntBetMap.find(iDIB);
					if (result == m_DIntBetMap.end())
					{
						//没有找到对应的夹层体类指针.报错
						printf(_T("\n"));
						printf(_T("系统中没有编号为%d的有延迟夹层体类! 请检查<地面沉降_有延迟夹层体观测点表>!\n"), iDIB);
						PauAndQuit();
					}
					pDIntBed = result->second;
					pDIntBed->AddObsCell(iRow, iCol);
					//下一条数据记录
					pTable->MoveNext();
				}
			}
		}
		catch (_com_error e)
		{
			printf(LPCTSTR(e.Description()));
			exit(1);
		}
	}
}

long CLandSub::GetCSOPT()
{
	return m_CSOPT;
}

long CLandSub::GetPCSOPT()
{
	//返回夹层体(有/无延迟均适用)初始预固结应力输入选项

	return m_PCSOPT;
}

CMatZone* CLandSub::GetMatZone(long MatID)
{
	//返回介质带对象指针

	long i;
	CMatZone* pMatZone = NULL;
	for (i = 0; i < m_NMZ; i++)
	{
		if (MatID == m_pMatZoneAry[i]->m_IMZ)
		{
			pMatZone = m_pMatZoneAry[i];
			break;
		}
	}
	return pMatZone;
}

void CLandSub::ReadSpGravData(_ConnectionPtr pConnection)
{
	//读取沉积物比重参数数据(数据库)

	try
	{
		//打开数据表读数据
		long i, nCount, iLyr, iRow, iCol;
		double SGM, SGS;
		CString strSQL;
		_variant_t Value;
		_RecordsetPtr pTable;
		pTable.CreateInstance(__uuidof(Recordset));
		//将数据集游标设置在客户端(可提高性能)
		pTable->CursorLocation = adUseClient;
		//设置数据集缓存(1000条)
		pTable->CacheSize = 1000;
		strSQL = _T("SELECT * FROM 地面沉降_沉积物比重表");
		pTable->Open(LPCSTR(strSQL),
			pConnection.GetInterfacePtr(),
			adOpenStatic,
			adLockOptimistic,
			adCmdText);
		if (pTable->BOF)
		{
			printf(_T("\n"));
			printf(_T("<地面沉降_沉积物比重表>不能是空表,请检查！\n"));
			PauAndQuit();
		}
		//数据表存在且有数据
		printf(_T("<地面沉降_沉积物比重表>\n"));
		pTable->MoveLast();
		nCount = pTable->GetRecordCount();  //数据个数
		//读入网格平面上的沉积物比重
		pTable->MoveFirst();
		for (i = 0; i < nCount; i++)
		{
			Value = pTable->GetCollect(_T("ILYR"));
			iLyr = Value.intVal;
			Value = pTable->GetCollect(_T("IROW"));
			iRow = Value.intVal;
			Value = pTable->GetCollect(_T("ICOL"));
			iCol = Value.intVal;
			Value = pTable->GetCollect(_T("SGM"));
			SGM = Value.dblVal;
			Value = pTable->GetCollect(_T("SGS"));
			SGS = Value.dblVal;
			//检查数据
			GDWMOD.CheckCellLimit(iLyr, iRow, iCol);
			if (SGM <= 0.0 || SGS <= 0.0)
			{
				printf(_T("\n"));
				printf(_T("第%d层第%d行第%d列网格单元处的SMS或SGS参数必须大于零，请检查<地面沉降_沉积物比重表>！\n"), 
					iLyr, iRow, iCol);
				PauAndQuit();
			}
			m_ArySGM[iLyr - 1][iRow - 1][iCol - 1] = SGM;
			m_ArySGS[iLyr - 1][iRow - 1][iCol - 1] = SGS;
			//下一条数据记录
			pTable->MoveNext();
		}
		pTable->Close();
	}
	catch (_com_error e)
	{
		printf(LPCTSTR(e.Description()));
		exit(1);
	}
}

void CLandSub::ReadSpGravData(const CString& strFilePath)
{
	//读取沉积物比重参数数据(TXT)

	//输出数据表信息
	CString strTbInfo, strTemp;
	strTbInfo = _T("Sub_SpGrav.in\n");
	printf(strTbInfo);

	//打开数据文件
	CStdioFile SPWDataFile;
	strTbInfo = strFilePath + _T("Sub_SpGrav.in");
	if (!SPWDataFile.Open(strTbInfo, CFile::modeRead | CFile::shareExclusive | CFile::typeText))
	{
		printf(_T("\n"));
		printf(_T("Fail to open <%s> file!\n"), _T("Sub_SpGrav.in"));
		PauAndQuit();
	}
	//略过表头
	SPWDataFile.ReadString(strTemp);

	//读入数据
	long iLyr, iRow, iCol;
	double SGM, SGS;
	CString strTest;
	CString strFormat;
	strFormat = _T("%d %d %d %lg %lg");
	long SumDataRow = 0;
	for (;;)
	{
		//逐行读入
		SPWDataFile.ReadString(strTemp);
		//累加读入的数据行数
		SumDataRow = SumDataRow + 1;
		//测试读入的数据
		strTest = strTemp;
		strTest.Trim();
		//如果没有读到有效数据
		if (strTest.IsEmpty())
		{
			SumDataRow = SumDataRow - 1;
			if (SPWDataFile.GetPosition() == SPWDataFile.GetLength())
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
		if (sscanf_s(strTemp, strFormat, &iLyr, &iRow, &iCol, &SGM, &SGS) != 5)
		{
			//没有正常数据, 报错
			printf(_T("文件<%s>中第%d行数据的数据异常, 请检查!\n"),
				_T("地面沉降_沉积物比重表.in"), SumDataRow);
			PauAndQuit();
		}
		//检查数据
		GDWMOD.CheckCellLimit(iLyr, iRow, iCol);
		if (SGM <= 0.0 || SGS <= 0.0)
		{
			printf(_T("\n"));
			printf(_T("第%d层第%d行第%d列网格单元处的SMS或SGS参数必须大于零，请检查<地面沉降_沉积物比重表>！\n"),
				iLyr, iRow, iCol);
			PauAndQuit();
		}
		m_ArySGM[iLyr - 1][iRow - 1][iCol - 1] = SGM;
		m_ArySGS[iLyr - 1][iRow - 1][iCol - 1] = SGS;
	}

	//后面不会再读入数据了
	SPWDataFile.Close();
}

CBndCell* CLandSub::ReadPerCellData(_RecordsetPtr pCellDataRec)
{
	//读取应力期网格单元源汇项数据(数据库)

	//读取应力期网格单元源汇项数据(数据库)

	_variant_t Value;
	long iRow, iCol;
	double STOPGL = 0.0;
	double ETOPGL = 0.0;
	CLandSubCell* pLandSubCell = NULL;
	CGridCell* pGridCell = NULL;
	Value = pCellDataRec->GetCollect(_T("IROW"));
	iRow = Value.intVal;
	Value = pCellDataRec->GetCollect(_T("ICOL"));
	iCol = Value.intVal;
	GDWMOD.CheckCellLimit(iRow, iCol);
	Value = pCellDataRec->GetCollect(_T("STOPGL"));
	STOPGL = Value.dblVal;
	Value = pCellDataRec->GetCollect(_T("ETOPGL"));
	ETOPGL = Value.dblVal;
	if (STOPGL < 0.0)
	{
		printf(_T("\n"));
		printf(_T("第%d行第%d列网格单元处应力期初的地面应力负荷不能小于0, 请检查<地面沉降_应力期数据表>！\n"),
			iRow, iCol);
		PauAndQuit();
	}
	if (ETOPGL < 0.0)
	{
		printf(_T("\n"));
		printf(_T("第%d行第%d列网格单元处应力期末的地面应力负荷不能小于0, 请检查<地面沉降_应力期数据表>！\n"),
			iRow, iCol);
		PauAndQuit();
	}
	pLandSubCell = new CLandSubCell;
	pLandSubCell->m_IROW = iRow;
	pLandSubCell->m_ICOL = iCol;
	pLandSubCell->m_STOPGL = STOPGL;
	pLandSubCell->m_ETOPGL = ETOPGL;

	//返回数据指针
	return pLandSubCell;
}

CBndCell* CLandSub::ReadPerCellData(CString strData)
{
	//读取应力期网格单元源汇项数据(TXT)

	long iPer, iRow, iCol;
	double STOPGL, ETOPGL;
	CGridCell* pGridCell = NULL;
	CLandSubCell* pLandSubCell = NULL;
	CString strFormat;
	strFormat = _T("%d %d %d %lg %lg");
	if (sscanf_s(strData, strFormat, &iPer, &iRow, &iCol, &STOPGL, &ETOPGL) != 5)
	{
		printf(_T("\n"));
		printf(_T("地面沉降_应力期数据表.in 文件中的数据异常, 请检查!\n"));
		PauAndQuit();
	}
	//检查数据
	if (STOPGL < 0.0 || ETOPGL < 0.0)
	{
		printf(_T("\n"));
		printf(_T("网格单元(ROW=%d,COL=%d)处的STOPGL、ETOPGL不能小于0.0, 请检查 地面沉降_应力期数据表.in ！\n"),
			iRow, iCol);
		PauAndQuit();
	}
	pLandSubCell = new CLandSubCell;
	pLandSubCell->m_IROW = iRow;
	pLandSubCell->m_ICOL = iCol;
	pLandSubCell->m_STOPGL = STOPGL;
	pLandSubCell->m_ETOPGL = ETOPGL;

	//返回数据指针
	return pLandSubCell;
}

void CLandSub::AddBndSinkInfo(long& BndID)
{
	//添加地面沉降源汇项统计输出编号和名称

	//添加粗颗粒沉积物弹性释水源汇项统计输出编号和名称
	m_CGBndID = BndID;
	CGridCell::AddBndObjItem(BndID, CString(_T("CST")), CString(_T("CG. SEDM STORAGE")));
	//添加夹层体弹性释水源汇项统计输出编号和名称
	m_IBEBndID = BndID;
	CGridCell::AddBndObjItem(BndID, CString(_T("IBE")), CString(_T("IB. ELAS STORAGE")));
	//添加夹层体非弹性释水源汇项统计输出编号和名称
	m_IBVBndID = BndID;
	CGridCell::AddBndObjItem(BndID, CString(_T("IBV")), CString(_T("IB. INEL STORAGE")));
	//添加水体弹性释水源汇项统计输出编号和名称
	m_WSBndID = BndID;
	CGridCell::AddBndObjItem(BndID, CString(_T("WST")), CString(_T("WAT. COM STORAGE")));
}

void CLandSub::LinkDataInOut(const CString& strFilePath, long& BndID)
{
	//关联地面沉降的输入数据和输出项并检查(TXT)
	//分配数组
	AllocMem();
	//读取地面沉降模拟控制参数数据
	ReadLandSubData(strFilePath);
	//读入沉积物比重参数数据
	ReadSpGravData(strFilePath);
	//读入粗颗粒沉积物属性表数据
	ReadCoGrainData(strFilePath);
	//读入介质带的参数数据
	ReadMatParData(strFilePath);
	//读入各个夹层体类的层位数据
	ReadIBSLyrData(strFilePath);
	//读入各个夹层体类的单元参数数据
	ReadIBSCellData(strFilePath);
	//读入有延迟夹层体观测点数据
	if (m_OBSOPT == 1 && m_NDB > 0)
	{
		ReadObsDIBData(strFilePath);
	}
	//设置应力期数据表
	SetPerData(strFilePath);
	//添加地面沉降源汇项统计输出编号和名称
	AddBndSinkInfo(BndID);
}

void CLandSub::LinkDataInOut(_ConnectionPtr pConnection, long& BndID)
{
	//关联地面沉降的输入数据和输出项并检查(数据库)

	//分配数组
	AllocMem();
	//读取地面沉降模拟控制参数数据
	ReadLandSubData(pConnection);
	//读入沉积物比重参数数据
	ReadSpGravData(pConnection);
	//读入粗颗粒沉积物属性表数据
	ReadCoGrainData(pConnection);
	//读入介质带的参数数据
	ReadMatParData(pConnection);
	//读入各个夹层体类的层位数据
	ReadIBSLyrData(pConnection);
	//读入各个夹层体类的单元参数数据
	ReadIBSCellData(pConnection);
	//读入有延迟夹层体观测点数据
	if (m_OBSOPT == 1 && m_NDB > 0)
	{
		ReadObsDIBData(pConnection);
	}
	//设置应力期数据表
	SetPerData(pConnection);
	//添加地面沉降源汇项统计输出编号和名称
	AddBndSinkInfo(BndID);
}

long CLandSub::GetNumDivN()
{
	//返回为有延迟夹层体其等效夹层体半厚度上离散点的数量

	return m_NN;
}

void CLandSub::CalThikForCGSedm()
{
	//为各层粗颗粒沉积物计算厚度

	long k, i, j, IniIBOUND;
	long iLyr, iRow, iCol;
	double CellTop, CellBot, IBSThik;
	m_NumLyr = GDWMOD.GetNumLyr();
	m_NumRow = GDWMOD.GetNumRow();
	m_NumCol = GDWMOD.GetNumCol();

	//开辟三维数组
	double*** AryThik;
	long*** AryCheck;
	AryThik = new double** [m_NumLyr]();
	AryCheck = new long** [m_NumLyr]();
	for (k = 0; k < m_NumLyr; k++)
	{
		AryThik[k] = new double* [m_NumRow]();
		AryCheck[k] = new long* [m_NumRow]();
		for (i = 0; i < m_NumRow; i++)
		{
			AryThik[k][i] = new double[m_NumCol]();
			AryCheck[k][i] = new long[m_NumCol]();
		}
	}

	//根据网格单元顶/底板高程计算网格单元厚度
	for (k = 0; k < m_NumLyr; k++)
	{
		for (i = 0; i < m_NumRow; i++)
		{
			for (j = 0; j < m_NumCol; j++)
			{
				CellTop = GDWMOD.GetCellTop(k + 1, i + 1, j + 1);
				CellBot = GDWMOD.GetCellBot(k + 1, i + 1, j + 1);
				AryThik[k][i][j] = CellTop - CellBot;
			}
		}
	}

	//将无延迟夹层体的厚度从网格单元厚度中减去
	CNDelayIntBed* pNDIntBed = NULL;
	CNDIntBedCell* pNDCell = NULL;
	for (i = 0; i < m_pNDIntBedAry.size(); i++)
	{
		pNDIntBed = m_pNDIntBedAry[i];
		iLyr = pNDIntBed->m_ILYR;
		for (j = 0; j < pNDIntBed->m_pNDIntBedCellAry.size(); j++)
		{
			pNDCell = pNDIntBed->m_pNDIntBedCellAry[j];
			iRow = pNDCell->m_IROW;
			iCol = pNDCell->m_ICOL;
			IBSThik = pNDCell->m_IBTHK;
			AryThik[iLyr - 1][iRow - 1][iCol - 1] = AryThik[iLyr - 1][iRow - 1][iCol - 1] - IBSThik;
		}
	}

	//将有延迟夹层体的厚度从网格单元厚度中减去
	CDelayIntBed* pDIntBed = NULL;
	CDIntBedCell* pDCell = NULL;
	for (i = 0; i < m_pDIntBedAry.size(); i++)
	{
		pDIntBed = m_pDIntBedAry[i];
		iLyr = pDIntBed->m_ILYR;
		for (j = 0; j < pDIntBed->m_pDIntBedCellAry.size(); j++)
		{
			pDCell = pDIntBed->m_pDIntBedCellAry[j];
			iRow = pDCell->m_IROW;
			iCol = pDCell->m_ICOL;
			//m_DeltZ目前还是等效厚度
			IBSThik = pDCell->m_RNB * pDCell->m_DeltZ;
			AryThik[iLyr - 1][iRow - 1][iCol - 1] = AryThik[iLyr - 1][iRow - 1][iCol - 1] - IBSThik;
		}
	}

	//检查剩余厚度数据, 剩余厚度即为粗颗粒沉积物的厚度, 不可小于0.0
	for (k = 0; k < m_NumLyr; k++)
	{
		for (i = 0; i < m_NumRow; i++)
		{
			for (j = 0; j < m_NumCol; j++)
			{
				if (AryThik[k][i][j] < 0.0)
				{
					printf(_T("\n"));
					printf(_T("第%d层第%d行第%d列的网格单元处粗颗粒沉积物的计算厚度小于零, 请检查数据合理性!\n"),
						k+1, i+1, j+1);
					PauAndQuit();
				}
			}
		}
	}

	//给各层粗颗粒沉积物网格单元赋值厚度
	CCGSediment* pCGSedm = NULL;
	CCoGrainCell* pCGCell = NULL;
	for (i = 0; i < m_pCoGrainSedmAry.size(); i++)
	{
		pCGSedm = m_pCoGrainSedmAry[i];
		iLyr = pCGSedm->m_ILYR;
		for (j = 0; j < pCGSedm->m_pCGSedmCellAry.size(); j++)
		{
			pCGCell = pCGSedm->m_pCGSedmCellAry[j];
			iRow = pCGCell->m_IROW;
			iCol = pCGCell->m_ICOL;
			pCGCell->m_CGTHK = AryThik[iLyr-1][iRow-1][iCol-1];
		}
	}

	//检查合理性
	for (i = 0; i < m_pCoGrainSedmAry.size(); i++)
	{
		pCGSedm = m_pCoGrainSedmAry[i];
		iLyr = pCGSedm->m_ILYR;
		for (j = 0; j < pCGSedm->m_pCGSedmCellAry.size(); j++)
		{
			pCGCell = pCGSedm->m_pCGSedmCellAry[j];
			iRow = pCGCell->m_IROW;
			iCol = pCGCell->m_ICOL;
			AryCheck[iLyr - 1][iRow - 1][iCol - 1] = 1;
		}
	}
	for (k = 0; k < m_NumLyr; k++)
	{
		for (i = 0; i < m_NumRow; i++)
		{
			for (j = 0; j < m_NumCol; j++)
			{
				IniIBOUND = GDWMOD.GetCellIniIBOUND(k+1,i+1,j+1);
				if (IniIBOUND > 0)
				{
					if (AryCheck[k][i][j] == 0 && AryThik[k][i][j] > 0.0)
					{
						printf(_T("\n"));
						printf(_T("第%d层第%d行第%d列的网格单元处须有粗颗粒沉积物参数数据, 请检查数据合理性!\n"),
							k + 1, i + 1, j + 1);
						PauAndQuit();
					}
				}
			}
		}
	}
 
	//清除三维数组
	for (k = 0; k < m_NumLyr; k++)
	{
		for (i = 0; i < m_NumRow; i++)
		{
			delete[] AryThik[k][i];                //释放列指针
			delete[] AryCheck[k][i];               //释放列指针
		}
		delete[] AryThik[k];                       //释放行指针
		delete[] AryCheck[k];                      //释放行指针
	}
	delete[] AryThik;                              //释放层指针
	delete[] AryCheck;                             //释放层指针
}

void CLandSub::InitSimulation()
{
	//初始化数据
	
	long k; 
	size_t i, j;
	long iLyr, iRow, iCol;
	CGridCell* pGridCell = NULL;
	double CellSC1;

	//检查各网格单元的贮水率参数
	for (iLyr = 1; iLyr <= m_NumLyr; iLyr++)
	{
		for (iRow = 1; iRow <= m_NumRow; iRow++)
		{
			for (iCol = 1; iCol <= m_NumCol; iCol++)
			{
				if (GDWMOD.GetCellIniIBOUND(iLyr, iRow, iCol) > 0)
				{
					pGridCell = GDWMOD.GetGridCell(iLyr, iRow, iCol);
					CellSC1 = pGridCell->GetCellSC1();
					if (CellSC1 != 0.0 && iLyr != 1)
					{
						//除了第顶层(一般为纯潜水层)之外, 其它各层的贮水率必须为零
						printf(_T("\n"));
						printf(_T("SUB模块启用下第%d层第%d行第%d列网格单元处的贮水率参数(SC1)必须等于0.0!\n"),
							iLyr, iRow, iCol);
						printf(_T("请检查<LPF网格单元属性表>!\n"));
						PauAndQuit();
					}
				}
			}
		}
	}

	//检查m_ArySGM和m_ArySGS数组
	for (iLyr = 1; iLyr <= m_NumLyr; iLyr++)
	{
		for (iRow = 1; iRow <= m_NumRow; iRow++)
		{
			for (iCol = 1; iCol <= m_NumCol; iCol++)
			{
				if (GDWMOD.GetCellIniIBOUND(iLyr, iRow, iCol) > 0)
				{
					if (GetSGM(iLyr, iRow, iCol) <= 0.0 || GetSGS(iLyr, iRow, iCol) <= 0.0)
					{
						printf(_T("\n"));
						printf(_T("第%d层第%d行第%d列网格单元处的饱和/非饱和介质比重值小于等于0.0或没有输入值!\n"),
							iLyr, iRow, iCol);
						printf(_T("请检查<地面沉降_沉积物比重表>!\n"));
						PauAndQuit();
					}
				}
			}
		}
	}

	//标识模型层是否模拟夹层体贮水
	for (i = 0; i < m_pNDIntBedAry.size(); i++)
	{
		iLyr = m_pNDIntBedAry[i]->m_ILYR;
		if (m_AryLyrIBS[iLyr - 1] == 0)
		{
			m_AryLyrIBS[iLyr - 1] = 1;
		}
	}
	for (i = 0; i < m_pDIntBedAry.size(); i++)
	{
		iLyr = m_pDIntBedAry[i]->m_ILYR;
		if (m_AryLyrIBS[iLyr - 1] == 0)
		{
			m_AryLyrIBS[iLyr - 1] = 1;
		}
	}

	//为各层粗颗粒沉积物计算厚度
	CalThikForCGSedm();

	//粗颗粒沉积物初始化数据
	for (i = 0; i < m_pCoGrainSedmAry.size(); i++)
	{
		m_pCoGrainSedmAry[i]->InitSimulation();
	}
	//无延迟夹层体类初始化数据
	for (i = 0; i < m_pNDIntBedAry.size(); i++)
	{
		m_pNDIntBedAry[i]->InitSimulation();
	}
	//有延迟夹层体类对象初始化数据
	for (i = 0; i < m_pDIntBedAry.size(); i++)
	{
		m_pDIntBedAry[i]->InitSimulation(m_DSHOPT);
	}

	//统计各层网格单元的前期压实量
	CNDelayIntBed* pNDIntBed = NULL;
	CNDIntBedCell* pNDIntBCell = NULL;
	size_t NumIBSCell;
	//统计无压缩延迟的
	for (i = 0; i < m_pNDIntBedAry.size(); i++)
	{
		pNDIntBed = m_pNDIntBedAry[i];
		iLyr = pNDIntBed->m_ILYR;
		NumIBSCell = pNDIntBed->m_pNDIntBedCellAry.size();
		for (j = 0; j < NumIBSCell; j++)
		{
			pNDIntBCell = pNDIntBed->m_pNDIntBedCellAry[j];
			iRow = pNDIntBCell->m_IROW;
			iCol = pNDIntBCell->m_ICOL;
			m_AryVertComp[iLyr - 1][iRow - 1][iCol - 1] = m_AryVertComp[iLyr - 1][iRow - 1][iCol - 1] + 
				pNDIntBCell->m_COM;
		}
	}
	//统计有压缩延迟的
	CDelayIntBed* pDIntBed = NULL;
	CDIntBedCell* pDIntBCell = NULL;
	for (i = 0; i < m_pDIntBedAry.size(); i++)
	{
		pDIntBed = m_pDIntBedAry[i];
		iLyr = pDIntBed->m_ILYR;
		NumIBSCell = pDIntBed->m_pDIntBedCellAry.size();
		for (j = 0; j < NumIBSCell; j++)
		{
			pDIntBCell = pDIntBed->m_pDIntBedCellAry[j];
			iRow = pDIntBCell->m_IROW;
			iCol = pDIntBCell->m_ICOL;
			m_AryVertComp[iLyr - 1][iRow - 1][iCol - 1] = m_AryVertComp[iLyr - 1][iRow - 1][iCol - 1] +
				pDIntBCell->m_COM;
		}
	}

	//统计各层网格单元的前期垂向位移量
	//注: 这里k若定义为size_t则会出错, 因为size_t是无符号的, 不能小于0
	for (k = m_NumLyr - 1; k >= 0; k--)
	{
		for (i = 0; i < m_NumRow; i++)
		{
			for (j = 0; j < m_NumCol; j++)
			{
				if (k == m_NumLyr - 1)
				{
					m_AryVertDisp[k][i][j] = m_AryVertComp[k][i][j];
				}
				else
				{
					m_AryVertDisp[k][i][j] = m_AryVertDisp[k + 1][i][j] + m_AryVertComp[k][i][j];
				}
			}
		}
	}

	if (GDWMOD.GetDataContex() == 1)
	{
		//TXT文件数据环境, 打开模拟结果输出表
		CString FilePath, strOutFile;
		CString FileHead;
		FilePath = GDWMOD.GetOutPutPath();
		//打开地面沉降量输出表
		if (m_iPrnOpt != 0)
		{
			strOutFile = FilePath + _T("Sub_CellDisp.dat");
			if (!m_SubOutFile.Open(strOutFile, CFile::modeCreate | CFile::modeWrite | CFile::typeText | CFile::shareDenyNone))
			{
				printf(_T("\n"));
				printf(_T("Fail to open <%s> file!\n"), _T("Sub_CellDisp.dat"));
				PauAndQuit();
			}
			//写表头
			FileHead = _T(" IPER  ISTEP  TLEN  ILYR  IROW  ICOL  DISP  COM  TOTGL  EST");
			FileHead = FileHead + _T("\n");
			m_SubOutFile.WriteString(FileHead);
		}
		//打开粗颗粒沉积物输出表
		if (m_pCoGrainSedmAry.size() > 0 && m_iPrnOpt != 0 && m_PRNOPT != 0)
		{
			strOutFile = FilePath + _T("Sub_CoGrain.out");
			if (!m_CGOutPutFile.Open(strOutFile, CFile::modeCreate | CFile::modeWrite | CFile::typeText | CFile::shareDenyNone))
			{
				printf(_T("\n"));
				printf(_T("Fail to open <%s> file!\n"), _T("Sub_CoGrain.out"));
				PauAndQuit();
			}
			//写表头
			FileHead = _T(" IPER  ISTEP  TLEN  ILYR  IROW  ICOL  COM  THETA  THIK  SKE");
			FileHead = FileHead + _T("\n");
			m_CGOutPutFile.WriteString(FileHead);
		}
		//打开无延迟夹层体输出表
		if (m_NNDB > 0 && m_iPrnOpt != 0 && m_PRNOPT != 0)
		{
			strOutFile = FilePath + _T("Sub_NDIB.out");
			if (!m_NDBOutPutFile.Open(strOutFile, CFile::modeCreate | CFile::modeWrite | CFile::typeText | CFile::shareDenyNone))
			{
				printf(_T("\n"));
				printf(_T("打开文件失败!\n"));
				PauAndQuit();
			}
			//写表头
			FileHead = _T(" IPER  ISTEP  TLEN  INDB  ILYR  IROW  ICOL  COM  HC  THETA  THIK  SKE  SKV");
			FileHead = FileHead + _T("\n");
			m_NDBOutPutFile.WriteString(FileHead);
		}
		//打开有延迟夹层体输出表
		if (m_NDB > 0 && m_iPrnOpt != 0 && m_PRNOPT != 0)
		{
			strOutFile = FilePath + _T("Sub_DIB.out");
			if (!m_DBOutPutFile.Open(strOutFile, CFile::modeCreate | CFile::modeWrite | CFile::typeText | CFile::shareDenyNone))
			{
				printf(_T("\n"));
				printf(_T("Fail to open <%s> file!\n"), _T("Sub_DIB.out"));
				PauAndQuit();
			}
			//写表头
			FileHead = _T(" IPER  ISTEP  TLEN  IDB  ILYR  IROW  ICOL  CHEAD  COM  EST  HC  RERR");
			FileHead = FileHead + _T("\n");
			m_DBOutPutFile.WriteString(FileHead);
		}
		//打开有延迟夹层体观测点输出表
		if (m_NDB > 0 && m_iPrnOpt != 0 && m_OBSOPT != 0)
		{
			strOutFile = FilePath + _T("Sub_DIBObs.out");
			if (!m_OBSOutPutFile.Open(strOutFile, CFile::modeCreate | CFile::modeWrite | CFile::typeText | CFile::shareDenyNone))
			{
				printf(_T("\n"));
				printf(_T("Fail to open <%s> file!\n"), _T("Sub_DIBObs.out"));
				PauAndQuit();
			}
			//写表头
			FileHead = _T(" IPER  ISTEP  TLEN  IDB  ILYR  IROW  ICOL  NODE  THIK  HEAD  THETA  VK  SKE  SKV");
			FileHead = FileHead + _T("\n");
			m_OBSOutPutFile.WriteString(FileHead);
		}
	}
}

void CLandSub::PreStepSim(long IBOUND[], double HNEW[], double HOLD[])
{
	//时段模拟之前数据准备

	size_t i;
	long NumNodes = GDWMOD.GetNumRC() * GDWMOD.GetNumLyr();
	//1. 设置当前应力期的地面应力负荷数据
	UpdateTopGL();
	//2. 如果是首个应力期首个模拟时段, 计算初始预固结应力
	if (GDWMOD.GetPerID() == 1 && GDWMOD.GetStepID() == 1)
	{
		//计算网格单元当前的层中心位置
		CalLyrCenter();
		//计算网格单元底板处当前的总地应力
		CalTotGL();
		//计算网格单元底板处当前的有效应力
		CalEST();
		//保存当前有效应力. 注: 此处切记不要使用CopyMemory
		//因为m_AryEST是三维数组, CopyMemory只适用于一维数组
		SaveEST();
		//粗颗粒沉积物根据选项修改参数
		for (i = 0; i < m_pCoGrainSedmAry.size(); i++)
		{
			m_pCoGrainSedmAry[i]->ModifyPar();
		}
		//无延迟夹层体设置初始模拟参数值
		for (i = 0; i < m_pNDIntBedAry.size(); i++)
		{
			m_pNDIntBedAry[i]->SetIniPar();
		}
		//有延迟夹层体设置初始模拟参数值
		for (i = 0; i < m_pDIntBedAry.size(); i++)
		{
			m_pDIntBedAry[i]->SetIniPar();
		}
	}
}

void CLandSub::UpdateTopGL()
{
	//更新当前应力期的地面应力负荷数据

	size_t i;
	long iRow, iCol;
	CLandSubCell* pLandSubCell = NULL;
	//获得当前应力期时长
	double PerLen = GDWMOD.GetPerLen();
	//获得从当前应力开始到当前的计算时长
	double PerCumDeltT = GDWMOD.GetPerCumDeltT();
	//将之前的地面应力荷载数组清零
	for (iRow = 0; iRow < m_NumRow; iRow++)
	{
		for (iCol = 0; iCol < m_NumCol; iCol++)
		{
			m_AryTopGL[iRow][iCol] = 0.0;
		}
	}
	//设置本应力期的地面应力荷载数组
	for (i = 0; i < m_pCurPerData->m_pBndCellAry.size(); i++)
	{
		pLandSubCell = (CLandSubCell*)m_pCurPerData->m_pBndCellAry[i];
		iRow = pLandSubCell->m_IROW;
		iCol = pLandSubCell->m_ICOL;
		m_AryTopGL[iRow - 1][iCol - 1] = pLandSubCell->m_STOPGL + (pLandSubCell->m_ETOPGL - 
			pLandSubCell->m_STOPGL) * PerCumDeltT / PerLen;
	}
}

void CLandSub::CalLyrCenter()
{
	//计算网格单元当前的层中心位置

	long iLyr, iRow, iCol, IBOUND;
	double CellTop, CellBot, HH, LyrCent;
	long NUMLYR = GDWMOD.GetNumLyr();
	long NUMROW = GDWMOD.GetNumRow();
	long NUMCOL = GDWMOD.GetNumCol();
	for (iLyr = 1; iLyr <= NUMLYR; iLyr++)
	{
		for (iRow = 1; iRow <= NUMROW; iRow++)
		{
			for (iCol = 1; iCol <= NUMCOL; iCol++)
			{
				CellTop = GDWMOD.GetCellTop(iLyr, iRow, iCol);
				CellBot = GDWMOD.GetCellBot(iLyr, iRow, iCol);
				IBOUND = GDWMOD.GetCellIBOUND(iLyr, iRow, iCol);
				HH = GDWMOD.GetCellHNew(iLyr, iRow, iCol);
				m_AryLyrCent[iLyr - 1][iRow - 1][iCol - 1] = 0.0;
				LyrCent = 0.0;
				if (IBOUND == 0)
				{
					//如果网格单元无效
					LyrCent = CellBot;
				}
				else
				{
					//如果网格单元有效
					if (HH >= CellTop)
					{
						//如果网格单元水头高于单元顶板
						LyrCent = 0.5 * (CellTop + CellBot);
					}
					else if (HH <= CellBot)
					{
						//如果网格单元水头低于单元底板
						LyrCent = CellBot;
					}
					else
					{
						//如果网格单元水头位于单元底板和单元顶板之间
						LyrCent = 0.5 * (HH + CellBot);
					}
				}
				m_AryLyrCent[iLyr - 1][iRow - 1][iCol - 1] = LyrCent;
			}
		}
	}
}

void CLandSub::CalThickFact()
{
	//计算网格单元在时段内的平均饱和厚度比例

	long iLyr, iRow, iCol, IBOUND, LyrCon;
	double CellTop, CellBot, HHNew, HHOld, ThickFact;
	long NUMLYR = GDWMOD.GetNumLyr();
	long NUMROW = GDWMOD.GetNumRow();
	long NUMCOL = GDWMOD.GetNumCol();
	for (iLyr = 1; iLyr <= NUMLYR; iLyr++)
	{
		for (iRow = 1; iRow <= NUMROW; iRow++)
		{
			for (iCol = 1; iCol <= NUMCOL; iCol++)
			{
				ThickFact = 0.0;
				m_AryThicFact[iLyr - 1][iRow - 1][iCol - 1] = 0.0;
				IBOUND = GDWMOD.GetCellIBOUND(iLyr, iRow, iCol);
				if (IBOUND == 0)
				{
					//如果网格单元无效
					continue;
				}
				else
				{
					//如果网格单元有效
					LyrCon = GDWMOD.GetLyrCon(iLyr);
					if (LyrCon == 0 || LyrCon == 2)
					{
						//如果含水层类型为0和2(含水层永远不会疏干)
						//饱和段厚度比例恒定为1.0
						ThickFact = 1.0;
					}
					else
					{
						//如果含水层类型为1和3(含水层可能会疏干)
						//计算时段内平均饱和段厚度
						CellTop = GDWMOD.GetCellTop(iLyr, iRow, iCol);
						CellBot = GDWMOD.GetCellBot(iLyr, iRow, iCol);
						HHNew = GDWMOD.GetCellHNew(iLyr, iRow, iCol);
						HHOld = GDWMOD.GetCellHOld(iLyr, iRow, iCol);
						if (HHNew <= CellBot && HHOld <= CellBot)
						{
							//如果网格单元时段内始终是疏干的则饱和厚度为零
							ThickFact = 0.0;
						}
						else
						{
							//计算时段内平均的饱和厚度
							ThickFact = 0.5 * (min(CellTop - CellBot, max(HHNew - CellBot, 0.0)) +
								min(CellTop - CellBot, max(HHOld - CellBot, 0.0))) /
								(CellTop - CellBot);
						}
					}
				}
				m_AryThicFact[iLyr - 1][iRow - 1][iCol - 1] = ThickFact;
			}
		}
	}
}

void CLandSub::CalTotGL()
{
	//计算网格单元底板处当前的总地应力

	long iLyr, iRow, iCol, IBOUND, iSat;
	double CellTop, CellBot, HH, SGM, SGS, LyrGL, TotGL;
	long NUMLYR = GDWMOD.GetNumLyr();
	long NUMROW = GDWMOD.GetNumRow();
	long NUMCOL = GDWMOD.GetNumCol();
	for (iLyr = 1; iLyr <= NUMLYR; iLyr++)
	{
		for (iRow = 1; iRow <= NUMROW; iRow++)
		{
			for (iCol = 1; iCol <= NUMCOL; iCol++)
			{
				m_AryTotGL[iLyr - 1][iRow - 1][iCol - 1] = 0.0;
				TotGL = 0.0;
				//1. 先计算本层产生的地应力
				LyrGL = 0.0;
				IBOUND = GDWMOD.GetCellIBOUND(iLyr, iRow, iCol);
				CellTop = GDWMOD.GetCellTop(iLyr, iRow, iCol);
				CellBot = GDWMOD.GetCellBot(iLyr, iRow, iCol);
				HH = GDWMOD.GetCellHNew(iLyr, iRow, iCol);
				SGM = GetSGM(iLyr, iRow, iCol);
				SGS = GetSGS(iLyr, iRow, iCol);
				if (IBOUND != 0)
				{
					iSat = 1;
				}
				else
				{
					HH = 0.0;
					iSat = 0;
				}
				if (iSat == 0 || HH <= CellBot)
				{
					//如果网格单元无效或水头低于底板
					LyrGL = (CellTop - CellBot) * SGM;
				}
				else if (HH >= CellTop)
				{
					//如果网格单元有效且水头高于顶板高程
					LyrGL = (CellTop - CellBot) * SGS;
				}
				else
				{
					//如果网格单元有效且水头位于顶、底板之间
					LyrGL = (CellTop - HH) * SGM + (HH - CellBot) * SGS;
				}
				//2. 再计算本层作用的总地应力
				if (iLyr == 1)
				{
					//如果是首层网格单元, 总地应力等于本层产生的地应力加上地面负荷应力
					TotGL = LyrGL + GetTopGL(iRow, iCol);
				}
				else
				{
					//如果不是首层网格单元, 总地应力等于本层产生的地应力加上本层之上的总地应力
					TotGL = LyrGL + m_AryTotGL[iLyr - 2][iRow - 1][iCol - 1];
				}
				m_AryTotGL[iLyr - 1][iRow - 1][iCol - 1] = TotGL;
			}
		}
	}
}

void CLandSub::CalEST()
{
	//计算网格单元底板处当前的有效应力

	long iLyr, iRow, iCol, IBOUND;
	double HH, CellBot, EST;
	long NUMLYR = GDWMOD.GetNumLyr();
	long NUMROW = GDWMOD.GetNumRow();
	long NUMCOL = GDWMOD.GetNumCol();
	for (iLyr = 1; iLyr <= NUMLYR; iLyr++)
	{
		for (iRow = 1; iRow <= NUMROW; iRow++)
		{
			for (iCol = 1; iCol <= NUMCOL; iCol++)
			{
				IBOUND = GDWMOD.GetCellIBOUND(iLyr, iRow, iCol);
				CellBot = GDWMOD.GetCellBot(iLyr, iRow, iCol);
				m_AryEST[iLyr - 1][iRow - 1][iCol - 1] = 0.0;
				EST = 0.0;
				if (IBOUND != 0)
				{
					HH = GDWMOD.GetCellHNew(iLyr, iRow, iCol);
					//有效应力等于总应力减去底板处的静水压力
					EST = m_AryTotGL[iLyr-1][iRow-1][iCol-1] - max(HH - CellBot, 0.0);
					if (EST < 0.0)
					{
						printf(_T("\n"));
						printf(_T("网格单元(K=%d,I=%d,J=%d)底板处计算的有效应力小于0.0, 请检查！\n"), 
							iLyr, iRow, iCol);
					}
					EST = max(EST, 0.01);
					m_AryEST[iLyr - 1][iRow - 1][iCol - 1] = EST;
				}
			}
		}
	}
}

void CLandSub::SaveEST()
{
	//保存网格单元底板处当前的有效应力到下个时段初

	long iLyr, iRow, iCol;
	long NUMLYR = GDWMOD.GetNumLyr();
	long NUMROW = GDWMOD.GetNumRow();
	long NUMCOL = GDWMOD.GetNumCol();
	for (iLyr = 0; iLyr < NUMLYR; iLyr++)
	{
		for (iRow = 0; iRow < NUMROW; iRow++)
		{
			for (iCol = 0; iCol < NUMCOL; iCol++)
			{
				m_AryESTOld[iLyr][iRow][iCol] = m_AryEST[iLyr][iRow][iCol];
			}
		}
	}
}

void CLandSub::FormMatrix(long IBOUND[], double HNEW[],
	double HCOF[], double RHS[], double HOLD[], double HPRE[])
{
	//根据地面沉降计算参数形成/处理矩阵方程

	//如果模拟稳定流, 则直接返回
	long SimType = GDWMOD.GetSimType();
	if (SimType == 1)
	{
		return;
	}

	long NumTask;
	long iLyr, LyrCon;
	long i, NumCGCell, NumIBSCell;
	//从地下水模拟系统取得模拟时段
	double DeltT = GDWMOD.GetDeltT();
	double TLED = 1.0 / DeltT;

	//计算网格单元当前的层中心位置
	CalLyrCenter();
	//计算网格单元底板处当前的总地应力
	CalTotGL();
	//计算网格单元底板处当前的有效应力
	CalEST();
	//计算网格单元在时段内的平均饱和厚度比例
	CalThickFact();

	//处理粗颗粒沉积物源汇项
	CCGSediment* pCGSedm = NULL;
	for (i = 0; i < long(m_pCoGrainSedmAry.size()); i++)
	{
		pCGSedm = m_pCoGrainSedmAry[i];
		//逐个网格单元进行循环
		iLyr = pCGSedm->m_ILYR;
		LyrCon = GDWMOD.GetLyrCon(iLyr);
		NumCGCell = long(pCGSedm->m_pCGSedmCellAry.size());
		NumTask = long(NumCGCell / GDWMOD.m_NUMTD / 2.0) + 1;
#pragma omp parallel for num_threads(GDWMOD.m_NUMTD) schedule(dynamic,NumTask)
		for (long j = 0; j < NumCGCell; j++)
		{
			//注释: 以下几个变量的定义必须放在for块中不能共用, 以适应并行运算要求
			long iRow, iCol, node;
			double HHNew, HHOld, SFE, GL, EST, ESTOLD, LYRCENT, CellBot, Fact, 
				ThickFact, CellSGS, CellArea, WRHO, CoefW, CGCOM, ESTCENTER;
			CCoGrainCell* pCGCell = NULL;
			//注释完毕
			pCGCell = pCGSedm->m_pCGSedmCellAry[j];
			iRow = pCGCell->m_IROW;
			iCol = pCGCell->m_ICOL;
			node = GDWMOD.GetCellNode(iLyr, iRow, iCol);
			if (IBOUND[node] <= 0)
			{
				//不计算无效单元和水头单元
				continue;
			}
			HHNew = HNEW[node];
			HHOld = HOLD[node];
			ThickFact = GetThickFact(iLyr, iRow, iCol);
			GL = GetTotGL(iLyr, iRow, iCol);
			EST = GetEST(iLyr, iRow, iCol);
			ESTOLD = GetESTOld(iLyr, iRow, iCol);
			LYRCENT = GetLyrCent(iLyr, iRow, iCol);
			CellSGS = GetSGS(iLyr, iRow, iCol);
			CellBot = GDWMOD.GetCellBot(iLyr, iRow, iCol);
			CellArea = GDWMOD.GetCellArea(iRow, iCol);
			//注: EST - (LYRCENT - CellBot) * (CellSGS - 1.0)为将当前底板处有效应力值EST修正到层中心处
			ESTCENTER = EST - (LYRCENT - CellBot) * (CellSGS - 1.0);
			//有时候模拟条件给得太恶劣, ESTCENTER会小于零, 这里修正一下
			ESTCENTER = max(ESTCENTER, 0.01);
			Fact = ThickFact * TLED / ESTCENTER;
			SFE = Fact * pCGCell->m_SEDMCR;
			//处理矩阵方程并计算总压实量CGCOM
			CGCOM = 0.0;
			if (LyrCon == 0 || LyrCon == 2)
			{
				//如果含水层类型为0和2(含水层永远不会疏干)
				HCOF[node] = HCOF[node] - SFE;
				RHS[node] = RHS[node] - SFE * (GL + CellBot) + SFE * ESTOLD;
				//计算压实量
				CGCOM = SFE * (EST - ESTOLD);
			}
			else
			{
				//如果含水层类型为1和3(需考虑全有效单元法下疏干单元也是有效单元的问题)
				if (HHNew <= CellBot && HHOld <= CellBot)
				{
					//如果网格单元时段末是疏干的, 时段初是疏干的
					//此时什么也不会发生
					continue;
				}
				else if (HHNew <= CellBot && HHOld > CellBot)
				{
					//如果网格单元时段末是疏干的, 时段初是湿润的
					//水位低于底板高程之后夹层体不会发生释水/贮水
					RHS[node] = RHS[node] - SFE * GL + SFE * ESTOLD;
					//计算压实量
					CGCOM = SFE * (GL - ESTOLD);
				}
				else
				{
					//如果网格单元时段末是湿润的, 时段初是疏干或湿润的
					//此时处理方式与含水层类型为0和2时一致
					HCOF[node] = HCOF[node] - SFE;
					RHS[node] = RHS[node] - SFE * (GL + CellBot) + SFE * ESTOLD;
					//计算压实量
					CGCOM = SFE * (EST - ESTOLD);
				}
			}
			//更新粗颗粒沉积物的孔隙率和厚度
			CGCOM = CGCOM * DeltT / CellArea;
			pCGCell->UpdatePar(iLyr, CGCOM);
			//处理孔隙水的压缩/膨胀释水
			CoefW = ThickFact * m_WSS * CellArea * TLED;
			WRHO = 0.5 * (pCGCell->m_CGTHK * pCGCell->m_CGTHETA + 
				pCGCell->m_CGTHKOLD * pCGCell->m_CGTHETAOLD) * CoefW;
			HCOF[node] = HCOF[node] - WRHO;
			RHS[node] = RHS[node] - WRHO * HHOld;
		}
	}

	//处理无延迟夹层体源汇项
	CNDelayIntBed* pNDIntBed = NULL;
	if (m_NNDB > 0)
	{
		for (i = 0; i < m_NNDB; i++)
		{
			pNDIntBed = m_pNDIntBedAry[i];
		    //逐个网格单元进行循环
			iLyr = pNDIntBed->m_ILYR;
			LyrCon = GDWMOD.GetLyrCon(iLyr);
			NumIBSCell = long(pNDIntBed->m_pNDIntBedCellAry.size());
			NumTask = long(NumIBSCell / GDWMOD.m_NUMTD / 2.0) + 1;
#pragma omp parallel for num_threads(GDWMOD.m_NUMTD) schedule(dynamic,NumTask)
			for (long j = 0; j < NumIBSCell; j++)
			{
				//注释: 以下几个变量的定义必须放在for块中不能共用, 以适应并行运算要求
				long iRow, iCol, node;
				double HHNew, HHOld, HC, SFE, SFV, SSK, GL, EST, ESTOLD, LYRCENT, CellBot,
					ThickFact, Fact, CellSGS, CellArea, WRHO, CoefW, IBTCOM, ESTCENTER;
				CNDIntBedCell* pNDIntBCell = NULL;
				//注释完毕
				pNDIntBCell = pNDIntBed->m_pNDIntBedCellAry[j];
				iRow = pNDIntBCell->m_IROW;
				iCol = pNDIntBCell->m_ICOL;
				node = GDWMOD.GetCellNode(iLyr, iRow, iCol);
				if (IBOUND[node] <= 0)
				{
					//不计算无效单元和水头单元
					continue;
				}
				HHNew = HNEW[node];
				HHOld = HOLD[node];
				ThickFact = GetThickFact(iLyr, iRow, iCol);
				GL = GetTotGL(iLyr, iRow, iCol);
				EST = GetEST(iLyr, iRow, iCol);
				ESTOLD = GetESTOld(iLyr, iRow, iCol);
				LYRCENT = GetLyrCent(iLyr, iRow, iCol);
				HC = pNDIntBCell->m_PCS;
				CellSGS = GetSGS(iLyr, iRow, iCol);
				CellBot = GDWMOD.GetCellBot(iLyr, iRow, iCol);
				CellArea = GDWMOD.GetCellArea(iRow, iCol);
				//注: EST - (LYRCENT - CellBot) * (CellSGS - 1.0)为将当前底板处有效应力值EST修正到层中心处
				ESTCENTER = EST - (LYRCENT - CellBot) * (CellSGS - 1.0);
				//有时候模拟条件给得太恶劣, ESTCENTER会小于零, 这里修正一下
				ESTCENTER = max(ESTCENTER, 0.01);
				Fact = ThickFact * TLED / ESTCENTER;
				SFE = Fact * pNDIntBCell->m_SEDMCR;
				SFV = Fact * pNDIntBCell->m_IBCC;
				if (EST > HC)
				{
					SSK = SFV;
				}
				else
				{
					SSK = SFE;
				}
				//处理矩阵方程并计算总压实量IBTCOM
				IBTCOM = 0.0;
				if (LyrCon == 0 || LyrCon == 2)
				{
					//如果含水层类型为0和2(含水层永远不会疏干)
					RHS[node] = RHS[node] - SSK * (GL + CellBot) + HC * (SSK - SFE) + SFE * ESTOLD;
					HCOF[node] = HCOF[node] - SSK;
					//计算压实量
					IBTCOM = SSK * (EST - HC) + SFE * (HC - ESTOLD);
				}
				else
				{
					//如果含水层类型为1和3(需考虑全有效单元法下疏干单元也是有效单元的问题)
					if (HHNew <= CellBot && HHOld <= CellBot)
					{
						//如果网格单元时段末是疏干的, 时段初是疏干的
						//此时什么也不会发生
						continue;
					}
					else if (HHNew <= CellBot && HHOld > CellBot)
					{
						//如果网格单元时段末是疏干的, 时段初是湿润的
                        //水位低于底板高程之后夹层体不会发生释水/贮水
						RHS[node] = RHS[node] - SSK * GL + HC * (SSK - SFE) + SFE * ESTOLD;
						//计算压实量
						IBTCOM = SSK * (GL - HC) + SFE * (HC - ESTOLD);
					}
					else
					{
						//如果网格单元时段末是湿润的, 时段初是疏干或湿润的
                        //此时处理方式与含水层类型为0和2时一致
						RHS[node] = RHS[node] - SSK * (GL + CellBot) + HC * (SSK - SFE) + SFE * ESTOLD;
						HCOF[node] = HCOF[node] - SSK;
						//计算压实量
						IBTCOM = SSK * (EST - HC) + SFE * (HC - ESTOLD);
					}
				}
				//更新夹层体的孔隙率和厚度
				IBTCOM = IBTCOM * DeltT / CellArea;
				pNDIntBCell->UpdatePar(iLyr, pNDIntBed->m_NDBedID, IBTCOM);
				//处理孔隙水的压缩/膨胀释水
				CoefW = ThickFact * m_WSS * CellArea * TLED;
				WRHO = 0.5 * (pNDIntBCell->m_IBTHK * pNDIntBCell->m_IBTHETA + 
					pNDIntBCell->m_IBTHKOLD * pNDIntBCell->m_IBTHETAOLD) * CoefW;
				HCOF[node] = HCOF[node] - WRHO;
				RHS[node] = RHS[node] - WRHO * HHOld;
			}
		}
	}

	//处理有延迟夹层体源汇项
	CDelayIntBed* pDIntBed = NULL;
	if (m_NDB > 0)
	{
		for (i = 0; i < m_NDB; i++)
		{
			pDIntBed = m_pDIntBedAry[i];
			iLyr = pDIntBed->m_ILYR;
			LyrCon = GDWMOD.GetLyrCon(iLyr);
			NumIBSCell = long(pDIntBed->m_pDIntBedCellAry.size());
			NumTask = long(NumIBSCell / GDWMOD.m_NUMTD / 2.0) + 1;
			//逐个网格单元进行循环
#pragma omp parallel for num_threads(GDWMOD.m_NUMTD) schedule(dynamic,NumTask)
			for (long j = 0; j < NumIBSCell; j++)
			{
				//注释: 以下几个变量的定义必须放在for块中不能共用, 以适应并行运算要求
				long iRow, iCol, node;
				double HHNew, HHOld, CellArea, ThickFact, CoupItem;
				CDIntBedCell* pDIntBCell = NULL;
				//注释完毕
				pDIntBCell = pDIntBed->m_pDIntBedCellAry[j];
				iRow = pDIntBCell->m_IROW;
				iCol = pDIntBCell->m_ICOL;
				node = GDWMOD.GetCellNode(iLyr, iRow, iCol);
				if (IBOUND[node] <= 0)
				{
					//不计算无效单元和水头单元
					continue;
				}
				CellArea = GDWMOD.GetCellArea(iRow, iCol);
				HHNew = HNEW[node];
				HHOld = HOLD[node];
				ThickFact = GetThickFact(iLyr, iRow, iCol);
				//进行延迟压缩方程求解
				if (m_DISOPT == 1)			
				{
					//全厚度离散格式
					CoupItem = ThickFact * pDIntBCell->SimHDDiff(iLyr, pDIntBed->m_DBedID, HHNew, DeltT, this) *
						pDIntBCell->m_RNB * CellArea;
				}
				else
				{
					//半厚度离散格式
					CoupItem = 2.0 * ThickFact * pDIntBCell->SimHDDiff2(iLyr, pDIntBed->m_DBedID, HHNew, DeltT, this) *
						pDIntBCell->m_RNB * CellArea;
				}
				//处理矩阵方程的右端项
				RHS[node] = RHS[node] - CoupItem;
			}
		}
	}
}

void CLandSub::Budget(long IBOUND[], double HNEW[], double HOLD[])
{
	//计算地面沉降引起的地下水量通量

	//如果模拟稳定流, 则直接返回
	long SimType = GDWMOD.GetSimType();
	if (SimType == 1)
	{
		return;
	}

	long NumTask;
	long iLyr, LyrCon;
	long i, j, k, NumCGCell, NumIBSCell;
	double* HPRE = GDWMOD.GetAryHPRE();
	//从地下水模拟系统取得模拟时段
	double DeltT = GDWMOD.GetDeltT();
	double TLED = 1.0 / DeltT;

	//计算网格单元当前的层中心位置
	CalLyrCenter();
	//计算网格单元底板处当前的总地应力
	CalTotGL();
	//计算网格单元底板处当前的有效应力
	CalEST();
	//计算网格单元在时段内的平均饱和厚度比例
	CalThickFact();

	//处理粗颗粒沉积物源汇项
	CCGSediment* pCGSedm = NULL;
	for (i = 0; i < long(m_pCoGrainSedmAry.size()); i++)
	{
		pCGSedm = m_pCoGrainSedmAry[i];
		iLyr = pCGSedm->m_ILYR;
		LyrCon = GDWMOD.GetLyrCon(iLyr);
		NumCGCell = long(pCGSedm->m_pCGSedmCellAry.size());
		NumTask = long(NumCGCell / GDWMOD.m_NUMTD / 2.0) + 1;
		//逐个网格单元进行循环
#pragma omp parallel for num_threads(GDWMOD.m_NUMTD) schedule(dynamic,NumTask)
		for (long j = 0; j < NumCGCell; j++)
		{
			//注释: 以下几个变量的定义必须放在for块中不能共用, 以适应并行运算要求
			long iRow, iCol, node;
			double HHNew, HHOld, ThickFact, GL, EST, ESTOLD, LYRCENT, Fact, SFE, CellBot, CellArea, 
				ESTCENTER, CellSGS, ElsComR, WtComR, CoefW, WRHO, CellSub;
			CCoGrainCell* pCGCell = NULL;
			CGridCell* pGridCell = NULL;
			//注释完毕
			pCGCell = pCGSedm->m_pCGSedmCellAry[j];
			iRow = pCGCell->m_IROW;
			iCol = pCGCell->m_ICOL;
			node = GDWMOD.GetCellNode(iLyr, iRow, iCol);
			if (IBOUND[node] <= 0)
			{
				//不计算无效单元和水头单元
				continue;
			}
			CellArea = GDWMOD.GetCellArea(iRow, iCol);
			pGridCell = GDWMOD.GetGridCell(iLyr, iRow, iCol);
			CellSub = 0.0;
			HHNew = HNEW[node];
			HHOld = HOLD[node];
			ThickFact = GetThickFact(iLyr, iRow, iCol);
			GL = GetTotGL(iLyr, iRow, iCol);
			EST = GetEST(iLyr, iRow, iCol);
			ESTOLD = GetESTOld(iLyr, iRow, iCol);
			LYRCENT = GetLyrCent(iLyr, iRow, iCol);
			CellSGS = GetSGS(iLyr, iRow, iCol);
			CellBot = GDWMOD.GetCellBot(iLyr, iRow, iCol);
			CellArea = GDWMOD.GetCellArea(iRow, iCol);
			//注: EST - (LYRCENT - CellBot) * (CellSGS - 1.0)为将当前底板处有效应力值EST修正到层中心处
			ESTCENTER = EST - (LYRCENT - CellBot) * (CellSGS - 1.0);
			//有时候模拟条件给得太恶劣, ESTCENTER会小于零, 这里修正一下
			ESTCENTER = max(ESTCENTER, 0.01);
			Fact = ThickFact * TLED / ESTCENTER;
			SFE = Fact * pCGCell->m_SEDMCR;
			//统计粗颗粒沉积物骨架膨胀/压缩释水量
			if (LyrCon == 0 || LyrCon == 2)
			{
				//如果含水层类型为0和2(含水层永远不会疏干)
				ElsComR = SFE * (EST - ESTOLD);
			}
			else
			{
				//如果含水层类型为1和3(需考虑全有效单元法下疏干单元也是有效单元的问题)
				if (HHNew <= CellBot && HHOld <= CellBot)
				{
					//如果网格单元时段末是疏干的, 时段初是疏干的
					//此时什么也不会发生
					continue;
				}
				else if (HHNew <= CellBot && HHOld > CellBot)
				{
					//如果网格单元时段末是疏干的, 时段初是湿润的
					//水位低于底板高程之后夹层体不会发生释水/贮水
					ElsComR = SFE * (GL - ESTOLD);
				}
				else
				{
					//如果网格单元时段末是湿润的, 时段初是疏干或湿润的
					//此时处理方式与含水层类型为0和2时一致
					ElsComR = SFE * (EST - ESTOLD);
				}
			}
			//计算网格单元内粗颗粒沉积物的垂向压实量(L)
			CellSub = ElsComR / TLED / CellArea;
			pCGCell->m_COM = pCGCell->m_COM + CellSub;
			//将粗颗粒沉积物的垂向压实量统计到各层网格单元压实量数组
			m_AryVertComp[iLyr - 1][iRow - 1][iCol - 1] = m_AryVertComp[iLyr - 1][iRow - 1][iCol - 1] +
				CellSub;
			//源汇项数据统计到地下水网格单元上
			pGridCell->AddSinkR(m_CGBndID, ElsComR);
			//统计孔隙水的压缩/膨胀释水
			CoefW = ThickFact * m_WSS * CellArea * TLED;
			WRHO = 0.5 * (pCGCell->m_CGTHK * pCGCell->m_CGTHETA +
				pCGCell->m_CGTHKOLD * pCGCell->m_CGTHETAOLD) * CoefW;
			WtComR = -WRHO * (HHNew - HHOld);
			pGridCell->AddSinkR(m_WSBndID, WtComR);
			//进行时段末善后处理
			//1. 保存孔隙率和厚度到下个时段初
			pCGCell->m_CGTHETAOLD = pCGCell->m_CGTHETA;
			pCGCell->m_CGTHKOLD = pCGCell->m_CGTHK;
			//2. 计算粗颗粒沉积物当前的贮水率
			pCGCell->m_CGSKE = pCGCell->m_SEDMCR / (CellArea * pCGCell->m_CGTHK * ESTCENTER);
		}
	}

	//处理无延迟夹层体源汇项
	CNDelayIntBed* pNDIntBed = NULL;
	if (m_NNDB > 0)
	{
		for (i = 0; i < m_NNDB; i++)
		{
			pNDIntBed = m_pNDIntBedAry[i];
			iLyr = pNDIntBed->m_ILYR;
			LyrCon = GDWMOD.GetLyrCon(iLyr);
			NumIBSCell = long(pNDIntBed->m_pNDIntBedCellAry.size());
			NumTask = long(NumIBSCell / GDWMOD.m_NUMTD / 2.0) + 1;
			//逐个网格单元进行循环
#pragma omp parallel for num_threads(GDWMOD.m_NUMTD) schedule(dynamic,NumTask)
			for (long j = 0; j < NumIBSCell; j++)
			{
				//注释: 以下几个变量的定义必须放在for块中不能共用, 以适应并行运算要求
				long iRow, iCol, node;
				double HHNew, HHOld, ThickFact, EST, ESTOLD, GL, HC, Fact, SFE, SFV, LYRCENT,
					CellBot, CellArea, ESTCENTER, CellSGS, CellSub, Coef, ElsComR,
					InElsComR, WtComR, CoefW, WRHO;
				CNDIntBedCell* pNDIntBCell = NULL;
				CGridCell* pGridCell = NULL;
				//注释完毕
				pNDIntBCell = pNDIntBed->m_pNDIntBedCellAry[j];
				iRow = pNDIntBCell->m_IROW;
				iCol = pNDIntBCell->m_ICOL;
				node = GDWMOD.GetCellNode(iLyr, iRow, iCol);
				if (IBOUND[node] <= 0)
				{
					//不计算无效单元和水头单元
					continue;
				}
				HHNew = HNEW[node];
				HHOld = HOLD[node];
				ElsComR = 0.0;
				InElsComR = 0.0;
				WtComR = 0.0;
				ThickFact = GetThickFact(iLyr, iRow, iCol);
				GL = GetTotGL(iLyr, iRow, iCol);
				EST = GetEST(iLyr, iRow, iCol);
				ESTOLD = GetESTOld(iLyr, iRow, iCol);
				LYRCENT = GetLyrCent(iLyr, iRow, iCol);
				HC = pNDIntBCell->m_PCS;
				CellSGS = GetSGS(iLyr, iRow, iCol);
				CellBot = GDWMOD.GetCellBot(iLyr, iRow, iCol);
				CellArea = GDWMOD.GetCellArea(iRow, iCol);
				//注: EST - (LYRCENT - CellBot) * (CellSGS - 1.0)为将当前底板处有效应力值EST修正到层中心处
				ESTCENTER = EST - (LYRCENT - CellBot) * (CellSGS - 1.0);
				//有时候模拟条件给得太恶劣, ESTCENTER会小于零, 这里修正一下
				ESTCENTER = max(ESTCENTER, 0.01);
				Fact = ThickFact * TLED / ESTCENTER;
				SFE = Fact * pNDIntBCell->m_SEDMCR;
				SFV = Fact * pNDIntBCell->m_IBCC;
				//计算弹性和非弹性压密/膨胀释水量
				if (LyrCon == 0 || LyrCon == 2)
				{
					//如果含水层类型为0和2(含水层永远不会疏干)
					if (EST > HC)
					{
						//弹性压实和非弹性压实都有
						ElsComR = SFE * (HC - ESTOLD);
						InElsComR = SFV * (EST - HC);
					}
					else
					{
						//仅有弹性压实或膨胀
						ElsComR = SFE * (EST - ESTOLD);
					}
				}
				else
				{
					//如果含水层类型为1和3(需考虑全有效单元法下疏干单元也是有效单元的问题)
					if (HHNew <= CellBot && HHOld <= CellBot)
					{
						//如果网格单元时段末是疏干的, 时段初是疏干的
						//此时什么也不会发生
						continue;
					}
					else if (HHNew <= CellBot && HHOld > CellBot)
					{
						//如果网格单元时段末是疏干的, 时段初是湿润的
						//水位低于底板高程之后夹层体不会发生释水/贮水
						if (EST > HC)
						{
							//弹性压实和非弹性压实都有
							ElsComR = SFE * (HC - ESTOLD);
							InElsComR = SFV * (GL - HC);
						}
						else
						{
							//仅有弹性压实或膨胀
							ElsComR = SFE * (GL - ESTOLD);
						}
					}
					else
					{
						//如果网格单元时段末是湿润的, 时段初是疏干或湿润的
						//此时处理方式与含水层类型为0和2时一致
						if (EST > HC)
						{
							//弹性压实和非弹性压实都有
							ElsComR = SFE * (HC - ESTOLD);
							InElsComR = SFV * (EST - HC);
						}
						else
						{
							//仅有弹性压实或膨胀
							ElsComR = SFE * (EST - ESTOLD);
						}
					}
				}
				pGridCell = GDWMOD.GetGridCell(iLyr, iRow, iCol);
				//计算网格单元内无延迟夹层体的垂向压实量(L)				
				CellSub = (ElsComR + InElsComR) / TLED / CellArea;
				pNDIntBCell->m_COM = pNDIntBCell->m_COM + CellSub;
				//将无延迟夹层体的垂向压实量统计到各层网格单元压实量数组
				m_AryVertComp[iLyr - 1][iRow - 1][iCol - 1] = m_AryVertComp[iLyr - 1][iRow - 1][iCol - 1] +
					CellSub;
				//弹性释水/膨胀源汇项数据统计到地下水网格单元上
				pGridCell->AddSinkR(m_IBEBndID, ElsComR);
				//非弹性释水/膨胀源汇项数据统计到地下水网格单元上
				pGridCell->AddSinkR(m_IBVBndID, InElsComR);
				//统计孔隙水的压缩/膨胀释水
				CoefW = ThickFact * m_WSS * CellArea * TLED;
				WRHO = 0.5 * (pNDIntBCell->m_IBTHK * pNDIntBCell->m_IBTHETA +
					pNDIntBCell->m_IBTHKOLD * pNDIntBCell->m_IBTHETAOLD) * CoefW;
				WtComR = -WRHO * (HHNew - HHOld);
				pGridCell->AddSinkR(m_WSBndID, WtComR);
				//进行时段末善后处理
				//1. 检查HC的值是否需要更新
				EST = GetEST(iLyr, iRow, iCol);
				if (EST > pNDIntBCell->m_PCS)
				{
					//夹层体当前的有效应力大于预固结应力, 需要更新
					pNDIntBCell->m_PCS = EST;
				}
				//2. 保存孔隙率和厚度到下个时段初
				pNDIntBCell->m_IBTHETAOLD = pNDIntBCell->m_IBTHETA;
				pNDIntBCell->m_IBTHKOLD = pNDIntBCell->m_IBTHK;
				//3. 计算无延迟夹层体当前的贮水率
				Coef = CellArea * pNDIntBCell->m_IBTHK * ESTCENTER;
				pNDIntBCell->m_SKE = pNDIntBCell->m_SEDMCR / Coef;
				pNDIntBCell->m_SKV = pNDIntBCell->m_IBCC / Coef;
			}
		}
	}

	//处理有延迟夹层体源汇项
	CDelayIntBed* pDIntBed = NULL;
	if (m_NDB > 0)
	{
		for (i = 0; i < m_NDB; i++)
		{
			pDIntBed = m_pDIntBedAry[i];
			iLyr = pDIntBed->m_ILYR;
			LyrCon = GDWMOD.GetLyrCon(iLyr);
			NumIBSCell = long(pDIntBed->m_pDIntBedCellAry.size());
			NumTask = long(NumIBSCell / GDWMOD.m_NUMTD / 2.0) + 1;
			//逐个网格单元进行循环
#pragma omp parallel for num_threads(GDWMOD.m_NUMTD) schedule(dynamic,NumTask)
			for (long j = 0; j < NumIBSCell; j++)
			{
				//注释: 以下几个变量的定义必须放在for块中不能共用, 以适应并行运算要求
				long iRow, iCol, node;
				double CellArea, ThickFact, CellSub;
				CDIntBedCell* pDIntBCell = NULL;
				CGridCell* pGridCell = NULL;
				//注释完毕
				pDIntBCell = pDIntBed->m_pDIntBedCellAry[j];
				iRow = pDIntBCell->m_IROW;
				iCol = pDIntBCell->m_ICOL;
				node = GDWMOD.GetCellNode(iLyr, iRow, iCol);
				if (IBOUND[node] <= 0)
				{
					//不计算无效单元和水头单元
					continue;
				}
				pGridCell = GDWMOD.GetGridCell(iLyr, iRow, iCol);
				CellArea = GDWMOD.GetCellArea(iRow, iCol);
				//计算网格单元上有延迟夹层体的释水/储水通量
				ThickFact = GetThickFact(iLyr, iRow, iCol);
				//进行有延迟夹层体水量平衡统计
				pDIntBCell->BudgetDIBS(iLyr, DeltT, ThickFact, pDIntBed->m_DBedID, m_DISOPT, m_KMOPT, this);
				//源汇项数据统计到地下水网格单元上
				pGridCell->AddSinkR(m_IBEBndID, pDIntBCell->m_ElsComChgR * CellArea);
				pGridCell->AddSinkR(m_IBVBndID, pDIntBCell->m_InElsComChgR * CellArea);
				pGridCell->AddSinkR(m_WSBndID, pDIntBCell->m_WtComChgR * CellArea);
				CellSub = pDIntBCell->m_TotComChg;
				//将夹层体的垂向压实量统计到各层网格单元压实量数组
				m_AryVertComp[iLyr - 1][iRow - 1][iCol - 1] = m_AryVertComp[iLyr - 1][iRow - 1][iCol - 1] +
					CellSub;
			}
		}
	}

	//统计各层网格单元的累积垂向位移量
	for (k = m_NumLyr - 1; k >= 0; k--)
	{
		for (i = 0; i < m_NumRow; i++)
		{
			for (j = 0; j < m_NumCol; j++)
			{
				if (k == m_NumLyr - 1)
				{
					m_AryVertDisp[k][i][j] = m_AryVertComp[k][i][j];
				}
				else
				{
					m_AryVertDisp[k][i][j] = m_AryVertDisp[k + 1][i][j] + m_AryVertComp[k][i][j];
				}
			}
		}
	}

	//保存当前有效应力. 注: 此处切记不要使用CopyMemory
	//因为m_AryEST是三维数组, CopyMemory只适用于一维数组
	SaveEST();
}

CMemTable* CLandSub::m_pMemSubCellTable = new CMemTable;
void CLandSub::MakeSubCellDataTable(_RecordsetPtr pTable, CString TbName)
{
	//构建网格单元压实量输出缓存表

	m_pMemSubCellTable->MakeDataTable(pTable, TbName);
}

void CLandSub::WriteDatabase()
{
	//将内存数据写入数据库

	m_pMemSubCellTable->WriteDatabase();
	CCGSediment::WriteDatabase();
	CNDelayIntBed::WriteDatabase();
	CDelayIntBed::WriteDatabase();
}

void CLandSub::OutPutCellSubData()
{
	//输出地面沉降模拟数据

	long iRow, iCol, iLyr;
	long iStep, iPer, NumRow, NumCol, NumLyr;
	CString strText;
	double CumTLen;
	iStep = GDWMOD.GetStepID();
	iPer = GDWMOD.GetPerID();
	NumRow = GDWMOD.GetNumRow();
	NumCol = GDWMOD.GetNumCol();
	NumLyr = GDWMOD.GetNumLyr();
	CumTLen = GDWMOD.GetCumTLen();
	for (iLyr = 0; iLyr < NumLyr; iLyr++)
	{
		for (iRow = 0; iRow < NumRow; iRow++)
		{
			for (iCol = 0; iCol < NumCol; iCol++)
			{
				fprintf(m_SubOutFile.m_pStream, _T(" %-10d"), iPer);
				fprintf(m_SubOutFile.m_pStream, _T(" %-10d"), iStep);
				fprintf(m_SubOutFile.m_pStream, _T(" %-16.8g"), CumTLen);
				fprintf(m_SubOutFile.m_pStream, _T(" %-10d"), iLyr + 1);
				fprintf(m_SubOutFile.m_pStream, _T(" %-10d"), iRow + 1);
				fprintf(m_SubOutFile.m_pStream, _T(" %-10d"), iCol + 1);
				fprintf(m_SubOutFile.m_pStream, _T(" %-16.8g"), m_AryVertDisp[iLyr][iRow][iCol]);
				fprintf(m_SubOutFile.m_pStream, _T(" %-16.8g"), m_AryVertComp[iLyr][iRow][iCol]);
				fprintf(m_SubOutFile.m_pStream, _T(" %-16.8g"), m_AryTotGL[iLyr][iRow][iCol]);
				fprintf(m_SubOutFile.m_pStream, _T(" %-16.8g"), m_AryEST[iLyr][iRow][iCol]);
				fprintf(m_SubOutFile.m_pStream, _T("\n"));
			}
		}
	}
}

void CLandSub::BndOutPutTXT(BOOL bEndPer, long iPer, long iStep, double CumTLen, double PerLen, double DeltT)
{
	//输出模拟计算结果(TXT)

	if (m_iPrnOpt == 0)
	{
		//如果不输出模拟计算结果, 直接返回
		return;
	}

	long i;
	//输出网格单元地面沉降量模拟结果
	if (m_iPrnOpt == 1 || (m_iPrnOpt == 2 && bEndPer == TRUE))
	{
		OutPutCellSubData();
		//写入文件
		m_SubOutFile.Flush();
	}

	if (m_PRNOPT == 1)
	{
		//输出粗颗粒沉积物模拟结果
		if (m_iPrnOpt == 1 || (m_iPrnOpt == 2 && bEndPer == TRUE))
		{
			if (m_pCoGrainSedmAry.size() > 0)
			{
				CCGSediment* pCGSedm = NULL;
				for (i = 0; i < long(m_pCoGrainSedmAry.size()); i++)
				{
					pCGSedm = m_pCoGrainSedmAry[i];
					pCGSedm->CGOutPut(m_iPrnOpt, iPer, iStep, CumTLen, m_CGOutPutFile);
				}
				//写入文件
				m_CGOutPutFile.Flush();
			}
		}

		//输出无延迟夹层体类模拟结果
		if (m_iPrnOpt == 1 || (m_iPrnOpt == 2 && bEndPer == TRUE))
		{
			if (m_NNDB > 0)
			{
				CNDelayIntBed* pNDelayIntBed = NULL;
				for (i = 0; i < m_NNDB; i++)
				{
					pNDelayIntBed = m_pNDIntBedAry[i];
					pNDelayIntBed->NDIBOutPut(m_iPrnOpt, iPer, iStep, CumTLen, m_NDBOutPutFile);
				}
				//写入文件
				m_NDBOutPutFile.Flush();
			}
		}

		//输出有延迟夹层体类模拟结果
		if (m_iPrnOpt == 1 || (m_iPrnOpt == 2 && bEndPer == TRUE))
		{
			if (m_NDB > 0)
			{
				CDelayIntBed* pDelayIntBed = NULL;
				for (i = 0; i < m_NDB; i++)
				{
					pDelayIntBed = m_pDIntBedAry[i];
					pDelayIntBed->DIBOutPut(m_iPrnOpt, iPer, iStep, CumTLen, m_DBOutPutFile);
				}
				//写入文件
				m_DBOutPutFile.Flush();
			}
		}
	}

	//输出有延迟夹层体观测点模拟结果
	if (m_OBSOPT == 1)
	{
		if (m_NDB > 0)
		{
			CDelayIntBed* pDelayIntBed = NULL;
			for (i = 0; i < m_NDB; i++)
			{
				pDelayIntBed = m_pDIntBedAry[i];
				if (pDelayIntBed->m_ObsDIBCellAry.size() > 0)
				{
					pDelayIntBed->ObsOutPut(iPer, iStep, CumTLen, m_OBSOutPutFile);
				}
			}
			//写入文件
			m_OBSOutPutFile.Flush();
		}
	}
}

void CLandSub::BndOutPutDB(BOOL bEndPer, long iPer, long iStep, double CumTLen, double PerLen, double DeltT)
{
	//输出模拟计算结果(数据库)

	long i, j, k;
	CDIntBedCell* pDIBCell = NULL;

	if (m_iPrnOpt == 0)
	{
		//如果不输出模拟计算结果, 可以返回了
		return;
	}

	//输出网格单元地面沉降量模拟结果
	if (m_iPrnOpt == 1 || (m_iPrnOpt == 2 && bEndPer == TRUE))
	{
		try
		{
			DataRow^ dr;
			for (k = 0; k < m_NumLyr; k++)
			{
				for (i = 0; i < m_NumRow; i++)
				{
					for (j = 0; j < m_NumCol; j++)
					{
						dr = m_pMemSubCellTable->GetDataTable()->NewRow();
						dr[0] = iPer;
						if (m_iPrnOpt == 1)
						{
							dr[1] = iStep;
						}
						else
						{
							dr[1] = long(-1);
						}
						dr[2] = CumTLen;
						dr[3] = k + 1;
						dr[4] = i + 1;
						dr[5] = j + 1;
						//输出网格单元的垂向位移量
						dr[6] = m_AryVertDisp[k][i][j];
						//输出网格单元的压实量
						dr[7] = m_AryVertComp[k][i][j];
						//输出网格单元底板处的总地应力
						dr[8] = m_AryTotGL[k][i][j];
						//输出网格单元底板处的有效应力
						dr[9] = m_AryEST[k][i][j];
						m_pMemSubCellTable->GetDataTable()->Rows->Add(dr);
					}
				}
			}
		}
		catch (_com_error* e)
		{
			printf((LPCTSTR)e->ErrorMessage());
		}
	}

	if (m_PRNOPT == 1)
	{
		//输出粗颗粒沉积物模拟结果
		if (m_iPrnOpt == 1 || (m_iPrnOpt == 2 && bEndPer == TRUE))
		{
			if (m_pCoGrainSedmAry.size() > 0)
			{
				CCGSediment* pCGSedm = NULL;
				for (i = 0; i < long(m_pCoGrainSedmAry.size()); i++)
				{
					pCGSedm = m_pCoGrainSedmAry[i];
					pCGSedm->CGOutPut(m_iPrnOpt, iPer, iStep, CumTLen);
				}
			}
		}

		//输出无延迟夹层体类模拟结果
		if (m_iPrnOpt == 1 || (m_iPrnOpt == 2 && bEndPer == TRUE))
		{
			if (m_NNDB > 0)
			{
				CNDelayIntBed* pNDelayIntBed = NULL;
				for (i = 0; i < m_NNDB; i++)
				{
					pNDelayIntBed = m_pNDIntBedAry[i];
					pNDelayIntBed->NDIBOutPut(m_iPrnOpt, iPer, iStep, CumTLen);
				}
			}
		}

		//输出有延迟夹层体类模拟结果
		if (m_iPrnOpt == 1 || (m_iPrnOpt == 2 && bEndPer == TRUE))
		{
			if (m_NDB > 0)
			{
				CDelayIntBed* pDelayIntBed = NULL;
				for (i = 0; i < m_NDB; i++)
				{
					pDelayIntBed = m_pDIntBedAry[i];
					pDelayIntBed->DIBOutPut(m_iPrnOpt, iPer, iStep, CumTLen);
				}
			}
		}
	}

	//输出有延迟夹层体观测点模拟结果
	if (m_OBSOPT == 1)
	{
		if (m_NDB > 0)
		{
			CDelayIntBed* pDelayIntBed = NULL;
			for (i = 0; i < m_NDB; i++)
			{
				pDelayIntBed = m_pDIntBedAry[i];
				if (pDelayIntBed->m_ObsDIBCellAry.size() > 0)
				{
					pDelayIntBed->ObsOutPut(iPer, iStep, CumTLen);
				}
			}
		}
	}
}