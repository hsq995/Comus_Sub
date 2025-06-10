#include "StdAfx.h"
#include "ClassInclude.h"

//////////////////////////////////////////////////////////
//网格单元分区类函数定义
CRegion::CRegCell::CRegCell(void)
{
	m_iLyr = 0;                //统计分区网格单元的层号
	m_iRow = 0;                //统计分区网格单元的行号
	m_iCol = 0;                //统计分区网格单元的列号
}

CRegion::CRegCell::~CRegCell(void)
{

}

CRegion::CRegion(void)
{
	m_IREG = 0;                 //统计分区编号
	m_pBudgetSch = NULL;        //统计方案指针
	m_StrgIn = 0.0;             //当前计算时段内分区内单元蓄量减少(水头下降)释放的水量(L3)
	m_StrgOut = 0.0;            //当前计算时段内分区内单元蓄量增加(水头上升)蓄积的水量(L3)
	m_ExcIn = 0.0;              //当前计算时段内其他分区向本分区流入的水量(L3)
	m_ExcOut = 0.0;             //当前计算时段内本分区向其他分区流出的水量(L3)
	m_RegBalErr = 0.0;          //当前计算时段内的水量平衡误差(L3)
	m_ResultAry = NULL;         //模拟输出结果存储数组
}

CRegion::~CRegion(void)
{
	//清除开辟的数组

	size_t i;
	m_pRegGridCellAry.clear();
	m_RegBudgetCellAry.clear();
	m_MapNearbyReg.clear();
	for (i = 0; i < m_AryFlowToNearbyReg.size(); i++)
	{
		delete m_AryFlowToNearbyReg[i];
	}
	m_AryFlowToNearbyReg.clear();

	if (m_ResultAry != NULL)
	{
		delete[] m_ResultAry;
		m_ResultAry = NULL;
	}
}

vector<CString> CRegion::m_ItemNamAry;  //输出项名称数组
vector<int> CRegion::m_ItemAtriAry;     //输出项性质数组
void CRegion::SetOutputItem()
{
	//设置数据库输出项名称

	CString ItemNam;
	size_t iBndOrSink;
	size_t ArySize = CGridCell::m_ActSimBndAry.size();
	long SimType = GDWMOD.GetSimType();
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
	//进入区域的单元间流量
	m_ItemNamAry.push_back(_T("EXCIN"));
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
	//离开区域的单元间流量
	m_ItemNamAry.push_back(_T("EXCOUT"));
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

void CRegion::SaveResult()
{
	//保存计算结果

	//计算本分区的水量平衡误差
	CalBalError();

	//将计算结果保存到结果输出数组
	size_t i = 0;
	size_t iBndOrSink;
	size_t ArySize = CGridCell::m_ActSimBndAry.size();
	long SimType = GDWMOD.GetSimType();
	//进入地下水系统的水量
	if (SimType == 2)
	{
		//如果是非稳定流模拟
		//蓄量变化(进)
		assert(m_ItemNamAry[i] == _T("STAIN"));
		m_ResultAry[i].step_val = m_StrgIn;
		i = i + 1;
	}
	//进入区域的单元间流量(进)
	assert(m_ItemNamAry[i] == _T("EXCIN"));
	m_ResultAry[i].step_val = m_ExcIn;
	i = i + 1;
	//边界上流量(进)
	for (iBndOrSink = 0; iBndOrSink < ArySize; iBndOrSink++)
	{
		assert(m_ItemNamAry[i] == CGridCell::m_ActSimBndAry[iBndOrSink].m_BndNamAbb + _T("IN"));
		m_ResultAry[i].step_val = m_AryRegSinkIn[iBndOrSink];
		i = i + 1;
	}
	//离开地下水系统的水量
	if (SimType == 2)
	{
		//如果是非稳定流模拟
		//蓄量变化(出)
		assert(m_ItemNamAry[i] == _T("STAOUT"));
		m_ResultAry[i].step_val = m_StrgOut;
		i = i + 1;
	}
	//离开区域的单元间流量(进)
	assert(m_ItemNamAry[i] == _T("EXCOUT"));
	m_ResultAry[i].step_val = m_ExcOut;
	i = i + 1;
	//边界上流量(出)
	for (iBndOrSink = 0; iBndOrSink < ArySize; iBndOrSink++)
	{
		assert(m_ItemNamAry[i] == CGridCell::m_ActSimBndAry[iBndOrSink].m_BndNamAbb + _T("OUT"));
		m_ResultAry[i].step_val = m_AryRegSinkOut[iBndOrSink];
		i = i + 1;
	}
	//保存水量平衡误差
	assert(m_ItemNamAry[i] == _T("ABER"));
	m_ResultAry[i].step_val = m_RegBalErr;

	//统计累加量结果
	for (i = 0; i<m_ItemNamAry.size(); i++)
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

	//统计分区间交换量计算结果
	for (i = 0; i<m_AryFlowToNearbyReg.size(); i++)
	{
		m_AryFlowToNearbyReg[i]->cum_val = m_AryFlowToNearbyReg[i]->cum_val +
			m_AryFlowToNearbyReg[i]->step_val;
	}
}

void CRegion::CalBalError()
{
	//计算时段内地下水平衡误差

	//初始化各项水量平衡项
	m_StrgIn = 0.0;             //当前计算时段内分区内单元蓄量减少(水头下降)释放的水量(L3)
	m_StrgOut = 0.0;            //当前计算时段内分区内单元蓄量增加(水头上升)蓄积的水量(L3)
	m_ExcIn = 0.0;              //当前计算时段内其他分区向本分区流入的水量(L3)
	m_ExcOut = 0.0;             //当前计算时段内本分区向其他分区流出的水量(L3)
	m_RegBalErr = 0.0;          //当前计算时段内的水量平衡误差(L3)

	//清零源汇项统计数组
	size_t iBndOrSink;
	size_t ArySize = CGridCell::m_ActSimBndAry.size();
	for (iBndOrSink = 0; iBndOrSink < ArySize; iBndOrSink++)
	{
		m_AryRegSinkIn[iBndOrSink] = 0.0;
		m_AryRegSinkOut[iBndOrSink] = 0.0;
	}

	//分区交换量数组置零
	size_t i;
	CResultVal* pRsVal = NULL;
	for (i = 0; i < m_AryFlowToNearbyReg.size(); i++)
	{
		m_AryFlowToNearbyReg[i]->step_val = 0.0;
	}

	//统计分区各项平衡量
	long iUp, iDown, iLeft, iRight, iFront, iBack;
	long CellRegID, NearCellRegID;
	CGridCell* pGridCell = NULL;
	CGridCell* pCellNear = NULL;
	double delt = GDWMOD.GetDeltT();
	double excout;
	for (i = 0; i < m_pRegGridCellAry.size(); i++)
	{
		pGridCell = m_pRegGridCellAry[i];
		CellRegID = m_pBudgetSch->GetCellRegID(pGridCell->m_ILYR, pGridCell->m_IROW, pGridCell->m_ICOL);
		if (pGridCell->m_IBOUND != 0)      //仅针对当前有效的单元
		{
			m_StrgIn = m_StrgIn - ((pGridCell->m_STRGR<0) ? pGridCell->m_STRGR : 0.0) * delt;
			m_StrgOut = m_StrgOut + ((pGridCell->m_STRGR>0) ? pGridCell->m_STRGR : 0.0) * delt;
			//计算分区水量交换
			//单元前侧
			iFront = pGridCell->m_IROW + 1;
			if (iFront <= GDWMOD.GetNumRow())
			{
				pCellNear = GDWMOD.GetGridCell(pGridCell->m_ILYR, iFront, pGridCell->m_ICOL);
				NearCellRegID = m_pBudgetSch->GetCellRegID(pCellNear->m_ILYR, pCellNear->m_IROW, pCellNear->m_ICOL);
				if (CellRegID != NearCellRegID)
				{
					//前侧不为同一分区内的单元
					m_ExcIn = m_ExcIn - ((pGridCell->m_FlowFront<0) ? pGridCell->m_FlowFront : 0.0) * delt;
					excout = ((pGridCell->m_FlowFront>0) ? pGridCell->m_FlowFront : 0.0) * delt;
					m_ExcOut = m_ExcOut + excout;
					if (m_MapNearbyReg.size() > 0 && NearCellRegID > 0)
					{
						auto result = m_MapNearbyReg.find(NearCellRegID);
						if (result != m_MapNearbyReg.end())
						{
							pRsVal = result->second;
							pRsVal->step_val = pRsVal->step_val + excout;
						}
					}
				}
			}
			//单元后侧
			iBack = pGridCell->m_IROW - 1;
			if (iBack >= 1)
			{
				pCellNear = GDWMOD.GetGridCell(pGridCell->m_ILYR, iBack, pGridCell->m_ICOL);
				NearCellRegID = m_pBudgetSch->GetCellRegID(pCellNear->m_ILYR, pCellNear->m_IROW, pCellNear->m_ICOL);
				if (CellRegID != NearCellRegID)
				{
					//后侧不为同一分区内的单元
					m_ExcIn = m_ExcIn - ((pGridCell->m_FlowBack<0) ? pGridCell->m_FlowBack : 0.0) * delt;
					excout = ((pGridCell->m_FlowBack>0) ? pGridCell->m_FlowBack : 0.0) * delt;
					m_ExcOut = m_ExcOut + excout;
					if (m_MapNearbyReg.size() > 0 && NearCellRegID > 0)
					{
						auto result = m_MapNearbyReg.find(NearCellRegID);
						if (result != m_MapNearbyReg.end())
						{
							pRsVal = result->second;
							pRsVal->step_val = pRsVal->step_val + excout;
						}
					}
				}
			}
			//单元左侧
			iLeft = pGridCell->m_ICOL - 1;
			if (iLeft >= 1)
			{
				pCellNear = GDWMOD.GetGridCell(pGridCell->m_ILYR, pGridCell->m_IROW, iLeft);
				NearCellRegID = m_pBudgetSch->GetCellRegID(pCellNear->m_ILYR, pCellNear->m_IROW, pCellNear->m_ICOL);
				if (CellRegID != NearCellRegID)
				{
					//左侧不为同一分区内的单元
					m_ExcIn = m_ExcIn - ((pGridCell->m_FlowLeft<0) ? pGridCell->m_FlowLeft : 0.0) * delt;
					excout = ((pGridCell->m_FlowLeft>0) ? pGridCell->m_FlowLeft : 0.0) * delt;
					m_ExcOut = m_ExcOut + excout;
					if (m_MapNearbyReg.size() > 0 && NearCellRegID > 0)
					{
						auto result = m_MapNearbyReg.find(NearCellRegID);
						if (result != m_MapNearbyReg.end())
						{
							pRsVal = result->second;
							pRsVal->step_val = pRsVal->step_val + excout;
						}
					}
				}
			}
			//单元右侧
			iRight = pGridCell->m_ICOL + 1;
			if (iRight <= GDWMOD.GetNumCol())
			{
				pCellNear = GDWMOD.GetGridCell(pGridCell->m_ILYR, pGridCell->m_IROW, iRight);
				NearCellRegID = m_pBudgetSch->GetCellRegID(pCellNear->m_ILYR, pCellNear->m_IROW, pCellNear->m_ICOL);
				if (CellRegID != NearCellRegID)
				{
					//右侧不为同一分区内的单元
					m_ExcIn = m_ExcIn - ((pGridCell->m_FlowRight<0) ? pGridCell->m_FlowRight : 0.0) * delt;
					excout = ((pGridCell->m_FlowRight>0) ? pGridCell->m_FlowRight : 0.0) * delt;
					m_ExcOut = m_ExcOut + excout;
					if (m_MapNearbyReg.size() > 0 && NearCellRegID > 0)
					{
						auto result = m_MapNearbyReg.find(NearCellRegID);
						if (result != m_MapNearbyReg.end())
						{
							pRsVal = result->second;
							pRsVal->step_val = pRsVal->step_val + excout;
						}
					}
				}
			}
			//单元上侧
			iUp = pGridCell->m_ILYR - 1;
			if (iUp >= 1)
			{
				pCellNear = GDWMOD.GetGridCell(iUp, pGridCell->m_IROW, pGridCell->m_ICOL);
				NearCellRegID = m_pBudgetSch->GetCellRegID(pCellNear->m_ILYR, pCellNear->m_IROW, pCellNear->m_ICOL);
				if (CellRegID != NearCellRegID)
				{
					//上侧不为同一分区内的单元
					m_ExcIn = m_ExcIn - ((pGridCell->m_FlowUp<0) ? pGridCell->m_FlowUp : 0.0) * delt;
					excout = ((pGridCell->m_FlowUp>0) ? pGridCell->m_FlowUp : 0.0) * delt;
					m_ExcOut = m_ExcOut + excout;
					if (m_MapNearbyReg.size() > 0 && NearCellRegID > 0)
					{
						auto result = m_MapNearbyReg.find(NearCellRegID);
						if (result != m_MapNearbyReg.end())
						{
							pRsVal = result->second;
							pRsVal->step_val = pRsVal->step_val + excout;
						}
					}
				}
			}
			//单元下侧
			iDown = pGridCell->m_ILYR + 1;
			if (iDown <= GDWMOD.GetNumLyr())
			{
				pCellNear = GDWMOD.GetGridCell(iDown, pGridCell->m_IROW, pGridCell->m_ICOL);
				NearCellRegID = m_pBudgetSch->GetCellRegID(pCellNear->m_ILYR, pCellNear->m_IROW, pCellNear->m_ICOL);
				if (CellRegID != NearCellRegID)
				{
					//下侧不为同一分区内的单元
					m_ExcIn = m_ExcIn - ((pGridCell->m_FlowDown<0) ? pGridCell->m_FlowDown : 0.0) * delt;
					excout = ((pGridCell->m_FlowDown>0) ? pGridCell->m_FlowDown : 0.0) * delt;
					m_ExcOut = m_ExcOut + excout;
					if (m_MapNearbyReg.size() > 0 && NearCellRegID > 0)
					{
						auto result = m_MapNearbyReg.find(NearCellRegID);
						if (result != m_MapNearbyReg.end())
						{
							pRsVal = result->second;
							pRsVal->step_val = pRsVal->step_val + excout;
						}
					}
				}
			}
			//源汇项
			for (iBndOrSink = 0; iBndOrSink < ArySize; iBndOrSink++)
			{
				m_AryRegSinkIn[iBndOrSink] = m_AryRegSinkIn[iBndOrSink] + ((pGridCell->m_ArySinkR[iBndOrSink] > 0.0) ?
					pGridCell->m_ArySinkR[iBndOrSink] : 0.0) * delt;
				m_AryRegSinkOut[iBndOrSink] = m_AryRegSinkOut[iBndOrSink] - ((pGridCell->m_ArySinkR[iBndOrSink] < 0.0) ?
					pGridCell->m_ArySinkR[iBndOrSink] : 0.0) * delt;
			}
		}
	}

	double SumSinkIn, SumSinkOut;
	SumSinkIn = 0.0;
	SumSinkOut = 0.0;
	for (iBndOrSink = 0; iBndOrSink < ArySize; iBndOrSink++)
	{
		SumSinkIn = SumSinkIn + m_AryRegSinkIn[iBndOrSink];
		SumSinkOut = SumSinkOut + m_AryRegSinkOut[iBndOrSink];
	}

	//水量平衡计算
	m_RegBalErr = (m_StrgIn + m_ExcIn + SumSinkIn) - (m_StrgOut + m_ExcOut + SumSinkOut);
}

void CRegion::InitSimulation()
{
	//模拟之前进行初始化工作

	size_t i;
	long iLyr, iRow, iCol;
	long iUp, iDown, iLeft, iRight, iFront, iBack;
	long CellRegID, NearCellRegID;
	CGridCell* pGridCell = NULL;
	CGridCell* pCellNear = NULL;
	CString strRegName;

	//给分区添加网格单元指针
	m_pRegGridCellAry.clear();
	for (i = 0; i < m_RegBudgetCellAry.size(); i++)
	{
		iLyr = m_RegBudgetCellAry[i].m_iLyr;
		iRow = m_RegBudgetCellAry[i].m_iRow;
		iCol = m_RegBudgetCellAry[i].m_iCol;
		pGridCell = GDWMOD.GetGridCell(iLyr, iRow, iCol);
		if (pGridCell->m_INIIBOUND != 0)
		{
			//仅将初始有效单元添加进统计分区的网格单元指针数组(含变水头单元和定常水头单元)
			m_pRegGridCellAry.push_back(pGridCell);
		}
	}
	//统计分区的网格单元数组数据没用了, 清除
	m_RegBudgetCellAry.clear();

	//分区关系分析
	for (i = 0; i < m_pRegGridCellAry.size(); i++)
	{
		pGridCell = m_pRegGridCellAry[i];
		CellRegID = m_pBudgetSch->GetCellRegID(pGridCell->m_ILYR, pGridCell->m_IROW, pGridCell->m_ICOL);
		//单元前侧
		iFront = pGridCell->m_IROW + 1;
		if (iFront <= GDWMOD.GetNumRow())
		{
			pCellNear = GDWMOD.GetGridCell(pGridCell->m_ILYR, iFront, pGridCell->m_ICOL);
			NearCellRegID = m_pBudgetSch->GetCellRegID(pCellNear->m_ILYR, pCellNear->m_IROW, pCellNear->m_ICOL);
			if (NearCellRegID > 0 && CellRegID != NearCellRegID)
			{
				//前侧不为同一分区内的单元
				//将其添加到分区间水量统计数组
				AddNearbyReg(NearCellRegID);
			}
		}
		//单元后侧
		iBack = pGridCell->m_IROW - 1;
		if (iBack >= 1)
		{
			pCellNear = GDWMOD.GetGridCell(pGridCell->m_ILYR, iBack, pGridCell->m_ICOL);
			NearCellRegID = m_pBudgetSch->GetCellRegID(pCellNear->m_ILYR, pCellNear->m_IROW, pCellNear->m_ICOL);
			if (NearCellRegID > 0 && CellRegID != NearCellRegID)
			{
				//后侧不为同一分区内的单元
				//将其添加到分区间水量统计数组
				AddNearbyReg(NearCellRegID);
			}
		}
		//单元左侧
		iLeft = pGridCell->m_ICOL - 1;
		if (iLeft >= 1)
		{
			pCellNear = GDWMOD.GetGridCell(pGridCell->m_ILYR, pGridCell->m_IROW, iLeft);
			NearCellRegID = m_pBudgetSch->GetCellRegID(pCellNear->m_ILYR, pCellNear->m_IROW, pCellNear->m_ICOL);
			if (NearCellRegID > 0 && CellRegID != NearCellRegID)
			{
				//左侧不为同一分区内的单元
				//将其添加到分区间水量统计数组
				AddNearbyReg(NearCellRegID);
			}
		}
		//单元右侧
		iRight = pGridCell->m_ICOL + 1;
		if (iRight <= GDWMOD.GetNumCol())
		{
			pCellNear = GDWMOD.GetGridCell(pGridCell->m_ILYR, pGridCell->m_IROW, iRight);
			NearCellRegID = m_pBudgetSch->GetCellRegID(pCellNear->m_ILYR, pCellNear->m_IROW, pCellNear->m_ICOL);
			if (NearCellRegID > 0 && CellRegID != NearCellRegID)
			{
				//右侧不为同一分区内的单元
				//将其添加到分区间水量统计数组
				AddNearbyReg(NearCellRegID);
			}
		}
		//单元上侧
		iUp = pGridCell->m_ILYR - 1;
		if (iUp >= 1)
		{
			pCellNear = GDWMOD.GetGridCell(iUp, pGridCell->m_IROW, pGridCell->m_ICOL);
			NearCellRegID = m_pBudgetSch->GetCellRegID(pCellNear->m_ILYR, pCellNear->m_IROW, pCellNear->m_ICOL);
			if (NearCellRegID > 0 && CellRegID != NearCellRegID)
			{
				//上侧不为同一分区内的单元
				//将其添加到分区间水量统计数组
				AddNearbyReg(NearCellRegID);
			}
		}
		//单元下侧
		iDown = pGridCell->m_ILYR + 1;
		if (iDown <= GDWMOD.GetNumLyr())
		{
			pCellNear = GDWMOD.GetGridCell(iDown, pGridCell->m_IROW, pGridCell->m_ICOL);
			NearCellRegID = m_pBudgetSch->GetCellRegID(pCellNear->m_ILYR, pCellNear->m_IROW, pCellNear->m_ICOL);
			if (NearCellRegID > 0 && CellRegID != NearCellRegID)
			{
				//下侧不为同一分区内的单元
				//将其添加到分区间水量统计数组
				AddNearbyReg(NearCellRegID);
			}
		}
	}

	//设置模拟结果输出数组
	long ItemArySize = long(m_ItemNamAry.size());
	m_ResultAry = new CResultVal[ItemArySize]();

	//设置边界上出入流量数组
	size_t iBndOrSink;
	size_t ArySize = CGridCell::m_ActSimBndAry.size();
	m_AryRegSinkIn.clear();
	m_AryRegSinkOut.clear();
	for (iBndOrSink = 0; iBndOrSink < ArySize; iBndOrSink++)
	{
		m_AryRegSinkIn.push_back(0.0);
		m_AryRegSinkOut.push_back(0.0);
	}
	m_AryRegSinkIn.shrink_to_fit();
	m_AryRegSinkOut.shrink_to_fit();
}

CMemTable* CRegion::m_pBDMemDataTable = new CMemTable;
CMemTable* CRegion::m_pExcMemDataTable = new CMemTable;
void CRegion::MakeBDDataTable(_RecordsetPtr pTable, CString TbName)
{
	//构建缓存表

	m_pBDMemDataTable->MakeDataTable(pTable, TbName);
}

void CRegion::MakeExcDataTable(_RecordsetPtr pTable, CString TbName)
{
	//构建缓存表

	m_pExcMemDataTable->MakeDataTable(pTable, TbName);
}

void CRegion::WriteDatabase()
{
	//将内存数据写入数据库

	m_pBDMemDataTable->WriteDatabase();
	m_pExcMemDataTable->WriteDatabase();
}

void CRegion::AddRegCell(long iLyr, long iRow, long iCol)
{
	//为统计分区添加统计网格单元

	CRegCell RegBudgetCell;
	RegBudgetCell.m_iLyr = iLyr;
	RegBudgetCell.m_iRow = iRow;
	RegBudgetCell.m_iCol = iCol;
	m_RegBudgetCellAry.push_back(RegBudgetCell);
}

void CRegion::AddNearbyReg(long iRegID)
{
	//添加邻近分区到分区水量交换数组

	long key;
	CResultVal* pRsVal = NULL;
	key = iRegID;
	if (m_MapNearbyReg.find(key) == m_MapNearbyReg.end())
	{
		pRsVal = new CResultVal;
		m_MapNearbyReg[key] = pRsVal;
		m_AryFlowToNearbyReg.push_back(pRsVal);
	}
}

void CRegion::RegOutPut(long PrnID, long iPer, long iStep, double CumTLen, CStdioFile& BDOutFile, CStdioFile& FLOutFile)	
{
	//输出统计分区统计结果(TXT)

	//输出水量平衡表
	fprintf(BDOutFile.m_pStream, _T(" %-10d"), iPer);
	if (PrnID == 1)
	{
		fprintf(BDOutFile.m_pStream, _T(" %-10d"), iStep);
	}
	else
	{
		fprintf(BDOutFile.m_pStream, _T(" %-10d"), long(-1));
	}
	fprintf(BDOutFile.m_pStream, _T(" %-16.8g"), CumTLen);
	fprintf(BDOutFile.m_pStream, _T(" %-10d"), m_pBudgetSch->GetRegScheID());
	fprintf(BDOutFile.m_pStream, _T(" %s"), LPCSTR(m_pBudgetSch->GetRegScheNam()));
	fprintf(BDOutFile.m_pStream, _T(" %-10d"), m_IREG);
	fprintf(BDOutFile.m_pStream, _T(" %s"), LPCSTR(m_REGNAM));
	for (size_t i = 0; i < m_ItemNamAry.size(); i++)
	{
		fprintf(BDOutFile.m_pStream, _T(" %-16.8g"), m_ResultAry[i].cum_val);
	}
	fprintf(BDOutFile.m_pStream, _T("\n"));

	//输出分区水量交换表
	long iToReg;
	CResultVal* pRsVal = NULL;
	size_t nReg = m_MapNearbyReg.size();
	if (nReg > 0)
	{
		map<long, CResultVal*> ::iterator pIter;
		for (pIter = m_MapNearbyReg.begin(); pIter != m_MapNearbyReg.end(); pIter++)
		{
			iToReg = pIter->first;
			pRsVal = pIter->second;
			fprintf(BDOutFile.m_pStream, _T(" %-10d"), iPer);
			if (PrnID == 1)
			{
				fprintf(BDOutFile.m_pStream, _T(" %-10d"), iStep);
			}
			else
			{
				fprintf(BDOutFile.m_pStream, _T(" %-10d"), long(-1));
			}
			fprintf(BDOutFile.m_pStream, _T(" %-16.8g"), CumTLen);
			fprintf(BDOutFile.m_pStream, _T(" %-10d"), m_pBudgetSch->GetRegScheID());
			fprintf(BDOutFile.m_pStream, _T(" %s"), LPCSTR(m_pBudgetSch->GetRegScheNam()));
			fprintf(BDOutFile.m_pStream, _T(" %-10d"), m_IREG);
			fprintf(BDOutFile.m_pStream, _T(" %s"), LPCSTR(m_REGNAM));
			fprintf(BDOutFile.m_pStream, _T(" %-10d"), iToReg);
			fprintf(BDOutFile.m_pStream, _T(" %s"), LPCSTR(m_pBudgetSch->GetRegNam(iToReg)));
			fprintf(BDOutFile.m_pStream, _T(" %-16.8g"), pRsVal->cum_val);
			fprintf(BDOutFile.m_pStream, _T("\n"));
		}
	}
}

void CRegion::RegOutPut(long PrnID, long iPer, long iStep, double CumTLen)
{
	//输出统计分区统计结果(数据库)

	size_t i = 0;
	String^ strTemp;
	size_t size = m_ItemNamAry.size();
	size_t nReg;
	long iToReg;
	CResultVal* pRsVal = NULL;
	try
	{
		//输出水量平衡表
		DataRow^ dr;
		for (i = 0; i < size; i++)
		{
			dr = m_pBDMemDataTable->GetDataTable()->NewRow();
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
			dr[3] = m_pBudgetSch->GetRegScheID();
			strTemp = gcnew String(m_pBudgetSch->GetRegScheNam());
			dr[4] = strTemp;
			dr[5] = m_IREG;
			strTemp = gcnew String(m_REGNAM);
			dr[6] = strTemp;
			dr[7] = i + 1;
			strTemp = gcnew String(m_ItemNamAry[i]);
			dr[8] = strTemp;
			dr[9] = m_ResultAry[i].cum_val;
			m_pBDMemDataTable->GetDataTable()->Rows->Add(dr);
		}

		//输出分区水量交换表
		nReg = m_MapNearbyReg.size();
		if (nReg>0)
		{
			map<long, CResultVal*> ::iterator pIter;
			for (pIter = m_MapNearbyReg.begin(); pIter != m_MapNearbyReg.end(); pIter++)
			{
				iToReg = pIter->first;
				pRsVal = pIter->second;
				dr = m_pExcMemDataTable->GetDataTable()->NewRow();
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
				dr[3] = m_pBudgetSch->GetRegScheID();
				strTemp = gcnew String(m_pBudgetSch->GetRegScheNam());
				dr[4] = strTemp;
				dr[5] = m_IREG;
				strTemp = gcnew String(m_REGNAM);
				dr[6] = strTemp;
				dr[7] = iToReg;
				strTemp = gcnew String(m_pBudgetSch->GetRegNam(iToReg));
				dr[8] = strTemp;
				dr[9] = pRsVal->cum_val;
				m_pExcMemDataTable->GetDataTable()->Rows->Add(dr);
			}
		}
	}
	catch (_com_error *e)
	{
		printf((LPCTSTR)e->ErrorMessage());
	}
}

//////////////////////////////////////////////////////////
//分区水量平衡统计方案类函数定义
CRegSche::CRegScheCell::CRegScheCell(void)
{
	m_IREG = 0;                //统计分区编号
	m_iLyr = 0;                //统计分区网格单元的层号
	m_iRow = 0;                //统计分区网格单元的行号
	m_iCol = 0;                //统计分区网格单元的列号
}

CRegSche::CRegScheCell::~CRegScheCell(void)
{

}

CRegSche::CRegSche(void)
{
	m_SCHEID = 0;
	m_pCellRegIDAry = NULL;                    //统计网格单元指针数组(三维)
	m_NumLyr = 0;                              //地下水系统对象的网格层数
	m_NumRow = 0;                              //地下水系统对象的网格行数
	m_NumCol = 0;                              //地下水系统对象的网格列数
}

CRegSche::~CRegSche(void)
{
	size_t i, k;
	//清空指针数组
	for (i = 0; i < m_pRegAry.size(); i++)
	{
		delete m_pRegAry[i];
	}
	m_pRegAry.clear();

	//清空平原区地下水水量平衡统计分区名称字典
	m_RegNamMap.clear();

	//清空统计单元数组
	m_RegScheCellAry.clear();

	//清除网格单元指针数组(三维)
	if (m_pCellRegIDAry != NULL)
	{
		for (k = 0; k < m_NumLyr; k++)
		{
			for (i = 0; i < m_NumRow; i++)
			{
				delete[] m_pCellRegIDAry[k][i];              //释放列指针
			}
			delete[] m_pCellRegIDAry[k];                     //释放行指针
		}
		delete[] m_pCellRegIDAry;                            //释放层指针
		m_pCellRegIDAry = NULL;
	}
	else
	{
		assert(m_pCellRegIDAry == NULL);
	}
}

void CRegSche::AddRegScheCell(long RegID, CString RegNam, long iLyr, long iRow, long iCol)
{
	//该函数添加统计方案的网格单元

	CRegScheCell SchemeCell;
	SchemeCell.m_IREG = RegID;
	SchemeCell.m_RegNam = RegNam;
	SchemeCell.m_iLyr = iLyr;
	SchemeCell.m_iRow = iRow;
	SchemeCell.m_iCol = iCol;
	m_RegScheCellAry.push_back(SchemeCell);
	if (m_RegNamMap.find(RegID) == m_RegNamMap.end())
	{
		m_RegNamMap[RegID] = RegNam;
	}
}

void CRegSche::InitRegScheData()
{
	//该函数初始化统计方案数据

	//将数据按统计分区编号的升序排序(冒泡法)
	size_t i, j, k, LastExcID;
	CRegScheCell Cell_i, Cell_j, Cell_j1;
	i = m_RegScheCellAry.size() - 1;
	while (i > 0)
	{
		LastExcID = 0;
		for (j = 0; j<i; j++)
		{
			if (m_RegScheCellAry[j + 1].m_IREG < m_RegScheCellAry[j].m_IREG)
			{
				Cell_j = m_RegScheCellAry[j];
				Cell_j1 = m_RegScheCellAry[j + 1];
				m_RegScheCellAry[j] = Cell_j1;
				m_RegScheCellAry[j + 1] = Cell_j;
				LastExcID = j;
			}
		}
		i = LastExcID;
	}

	//检查统计方案网格单元
	for (i = 0; i < m_RegScheCellAry.size(); i++)
	{
		Cell_i = m_RegScheCellAry[i];
		for (j = i + 1; j < m_RegScheCellAry.size(); j++)
		{
			Cell_j = m_RegScheCellAry[j];
			//检查有无重复数据
			if (Cell_i.m_IREG == Cell_j.m_IREG && Cell_i.m_iLyr == Cell_j.m_iLyr && 
				Cell_i.m_iRow == Cell_j.m_iRow && Cell_i.m_iCol == Cell_j.m_iCol)
			{
				printf(_T("\n"));
				printf(_T("编号为%d的统计方案有重复的统计网格单元!\n"), m_SCHEID);
				printf(_T("请检查<水量平衡统计分区表>!\n"));
				PauAndQuit();
			}
			//检查有无统计分区编号和统计分区名称不一致的情况
			if (Cell_i.m_IREG == Cell_j.m_IREG && Cell_i.m_RegNam != Cell_j.m_RegNam)
			{
				printf(_T("\n"));
				printf(_T("编号为%d的统计方案中编号为%d的分区有多个分区名称!\n"), m_SCHEID, Cell_i.m_IREG);
				printf(_T("请检查<水量平衡统计分区表>!\n"));
				PauAndQuit();
			}
			//检查有无同一网格单元属于多个统计分区的情况
			if (Cell_i.m_IREG != Cell_j.m_IREG)
			{
				if (Cell_i.m_iLyr == Cell_j.m_iLyr && Cell_i.m_iRow == Cell_j.m_iRow && 
					Cell_i.m_iCol == Cell_j.m_iCol)
				{
					printf(_T("\n"));
					printf(_T("编号为%d的统计方案有同一网格单元被多个统计分区包含的情况!\n"), m_SCHEID);
					printf(_T("请检查<水量平衡统计分区表>!\n"));
					PauAndQuit();
				}
			}
		}
	}
	m_NumLyr = GDWMOD.GetNumLyr();
	m_NumRow = GDWMOD.GetNumRow();
	m_NumCol = GDWMOD.GetNumCol();

	//先清除网格单元分区号数组(三维)
	if (m_pCellRegIDAry != NULL)
	{
		for (k = 0; k < m_NumLyr; k++)
		{
			for (i = 0; i < m_NumRow; i++)
			{
				delete[] m_pCellRegIDAry[k][i];              //释放列指针
			}
			delete[] m_pCellRegIDAry[k];                     //释放行指针
		}
		delete[] m_pCellRegIDAry;                            //释放层指针
		m_pCellRegIDAry = NULL;
	}
	//开辟新的网格单元分区号数组(三维)
	m_pCellRegIDAry = new long**[m_NumLyr]();
	for (k = 0; k < m_NumLyr; k++)
	{
		m_pCellRegIDAry[k] = new long*[m_NumRow]();
		for (i = 0; i < m_NumRow; i++)
		{
			m_pCellRegIDAry[k][i] = new long[m_NumCol]();
		}
	}

	//设置统计网格单元的分区号
	for (i = 0; i < m_RegScheCellAry.size(); i++)
	{
		Cell_i = m_RegScheCellAry[i];
		m_pCellRegIDAry[Cell_i.m_iLyr - 1][Cell_i.m_iRow - 1][Cell_i.m_iCol - 1] = Cell_i.m_IREG;
	}

	//建立统计分区数组
	size_t iReg;
	BOOL lFind;
	CRegion* pRegion = NULL;
	for (i = 0; i < m_RegScheCellAry.size(); i++)
	{
		Cell_i = m_RegScheCellAry[i];
		lFind = FALSE;
		for (iReg = 0; iReg < m_pRegAry.size(); iReg++)
		{
			if (Cell_i.m_IREG == m_pRegAry[iReg]->m_IREG)
			{
				lFind = TRUE;
				pRegion = m_pRegAry[iReg];
				break;
			}
		}
		if (lFind == FALSE)
		{
			//新建统计分区
			pRegion = new CRegion;
			pRegion->m_pBudgetSch = this;
			pRegion->m_IREG = Cell_i.m_IREG;
			pRegion->m_REGNAM = GetRegNam(Cell_i.m_IREG);
			m_pRegAry.push_back(pRegion);
		}
		//给统计分区添加统计网格单元
		pRegion->AddRegCell(Cell_i.m_iLyr, Cell_i.m_iRow, Cell_i.m_iCol);
	}

	//检查并初始化统计分区对象
	for (iReg = 0; iReg < m_pRegAry.size(); iReg++)
	{
		if (m_pRegAry[iReg]->m_IREG != iReg + 1)
		{
			printf(_T("\n"));
			printf(_T("编号为%d的统计方案其统计分区的编号没从1开始或不连续!\n"), m_SCHEID);
			printf(_T("请检查<水量平衡统计分区表>!\n"));
			PauAndQuit();
		}
		m_pRegAry[iReg]->InitSimulation();
	}

	//数组数据没用了, 清除掉
	m_RegScheCellAry.clear();
}

void CRegSche::ScheOutPut(long PrnID, long iPer, long iStep, double CumTLen, CStdioFile& BDOutFile, CStdioFile& FLOutFile)	
{
	//输出统计方案各统计分区的统计结果(TXT)

	for (size_t i = 0; i < m_pRegAry.size(); i++)
	{
		m_pRegAry[i]->RegOutPut(PrnID, iPer, iStep, CumTLen, BDOutFile, FLOutFile);
	}
}

void CRegSche::ScheOutPut(long PrnID, long iPer, long iStep, double CumTLen)
{
	//输出统计方案各统计分区的统计结果(数据库)

	for (size_t i = 0; i < m_pRegAry.size(); i++)
	{
		m_pRegAry[i]->RegOutPut(PrnID, iPer, iStep, CumTLen);
	}
}

void CRegSche::SaveResult()
{
	//该函数保存统计方案各统计分区的计算结果

#pragma omp parallel for num_threads(GDWMOD.m_NUMTD) schedule(static,1)
	for (long iReg = 0; iReg<m_pRegAry.size(); iReg++)
	{
		m_pRegAry[iReg]->SaveResult();
	}
}

CString& CRegSche::GetRegScheNam()
{
	//该函数返回统计方案的名称

	return m_SCHENAM;
}

long CRegSche::GetRegScheID()
{
	//该函数返回统计方案的编号

	return m_SCHEID;
}

long CRegSche::GetCellRegID(long iLyr, long iRow, long iCol)
{
	//该函数返回统计方案的网格单元的统计分区编号

	assert(iLyr >= 1 && iLyr <= m_NumLyr );
	assert(iRow >= 1 && iRow <= m_NumRow);
	assert(iCol >= 1 && iCol <= m_NumCol);
	return m_pCellRegIDAry[iLyr - 1][iRow - 1][iCol - 1];
}

CString CRegSche::GetRegNam(long RegID)
{
	//该函数返回对应统计分区编号的统计分区名称

	auto result = m_RegNamMap.find(RegID);
	if (result == m_RegNamMap.end())
	{
		//未找到相应编号的分区名称, 报错
		printf(_T("\n"));
		printf(_T("系统中没有编号为%d水量平衡统计分区!\n"), RegID);
		printf(_T("请检查<水量平衡统计分区表>!\n"));
		PauAndQuit();
	};

	return result->second;
}

//////////////////////////////////////////////////////////
//分区水量平衡统计处理类函数定义
CRegBudget::CRegBudget()
{
	m_iPrnOpt = 0;                                      //输出控制. 0:不输出；1:逐应力期内模拟时段输出；2:逐应力期输出
}

CRegBudget:: ~CRegBudget()
{
	long k;
	for (k = 0; k < m_BudgetSchAry.size(); k++)
	{
		delete m_BudgetSchAry[k];
	}
	m_BudgetSchAry.clear();

	//关闭模拟结果输出文件
	if (m_BDOutPutFile.m_hFile != CFile::hFileNull)
	{
		m_BDOutPutFile.Close();
	}
	if (m_FlowOutPutFile.m_hFile != CFile::hFileNull)
	{
		m_FlowOutPutFile.Close();
	}
}

void CRegBudget::ReadRBData(const CString& strFilePath)
{
	//读取水量平衡统计方案数据(TXT文件)

	//输出数据表信息
	CString strTbInfo, strTemp;
	strTbInfo = _T("水量平衡统计分区表.in\n");
	printf(strTbInfo);

	//打开模拟控制参数数据文件
	CStdioFile StRegDataFile;
	strTbInfo = strFilePath + _T("水量平衡统计分区表.in");
	if (!StRegDataFile.Open(strTbInfo, CFile::modeRead | CFile::shareExclusive | CFile::typeText))
	{
		printf(_T("\n"));
		printf(_T("打开文件<%s>失败!\n"), _T("水量平衡统计分区表.in"));
		PauAndQuit();
	}
	//略过表头
	StRegDataFile.ReadString(strTemp);

	//读入数据
	long SCHID, IREG, ILYR, IROW, ICOL;
	char SCHNAM[50];
	char REGNAM[50];
	CGridCell* pGridCell = NULL;
	CRegSche* pScheme = NULL;
	CString str;
	CString strTest;
	CString strFormat;
	strFormat = _T("%ld %s %ld %s %ld %ld %ld");
	long SumDataRow = 0;
	long PreSCHID, PreIREG;
	BOOL bNewSche;
	for (;;)
	{
		//逐行读入
		StRegDataFile.ReadString(strTemp);
		//累加读入的数据行数
		SumDataRow = SumDataRow + 1;
		//测试读入的数据
		strTest = strTemp;
		strTest.Trim();
		//如果没有读到有效数据
		if (strTest.IsEmpty())
		{
			SumDataRow = SumDataRow - 1;
			if (StRegDataFile.GetPosition() == StRegDataFile.GetLength())
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
		if (sscanf_s(strTemp, strFormat, &SCHID, &SCHNAM, 50, &IREG, &REGNAM, 50,
			&ILYR, &IROW, &ICOL) != 7)
		{
			//没有正常数据, 报错
			printf(_T("\n"));
			printf(_T("文件<%s>中第%d行数据的数据异常, 请检查!\n"),
				_T("水量平衡统计分区表.in"), SumDataRow);
			PauAndQuit();
		}
		//检查数据
		GDWMOD.CheckCellLimit(ILYR, IROW, ICOL);
		bNewSche = FALSE;
		if (SumDataRow == 1)
		{
			//如果是第1条数据
			if (SCHID != 1 || IREG != 1)
			{
				printf(_T("\n"));
				printf(_T("文件<%s>中统计方案和统计分区都必须从1开始, 请检查!\n"),
					_T("水量平衡统计分区表.in"));
				PauAndQuit();
			}
			//标注将生成新的统计方案
			bNewSche = TRUE;
		}
		else
		{
			//如果不是第1条数据
			if (SCHID != PreSCHID)
			{
				//如果数据的统计方案编号前后不同(表明为新的统计方案的数据)
				if (SCHID != PreSCHID + 1)
				{
					//新的统计方案的编号必须为之前的编号加1
					printf(_T("\n"));
					printf(_T("文件<%s>中编号为%d的统计方案没有按顺序编号, 请检查!\n"),
						_T("水量平衡统计分区表.in"), SCHID);
					PauAndQuit();
				}
				if (IREG != 1)
				{
					//新的统计方案, 其首个统计分区的编号必须从1开始
					printf(_T("\n"));
					printf(_T("文件<%s>中编号为%d的统计方案的统计分区没有从1开始编号, 请检查!\n"),
						_T("水量平衡统计分区表.in"), SCHID);
					PauAndQuit();
				}
				//标注将生成新的统计方案
				bNewSche = TRUE;
			}
			else
			{
				//如果数据的统计方案编号前后相同(表明为同一统计方案下的数据)
				if (pScheme->m_SCHENAM != CString(SCHNAM))
				{
					//检查统计方案的名称是否一致
					printf(_T("\n"));
					printf(_T("编号为%d的统计方案的名称与之前记录的不一致!\n"), pScheme->m_SCHEID);
					printf(_T("请检查<水量平衡统计分区表>!\n"));
					PauAndQuit();
				}
				if (IREG != PreIREG)
				{
					//如果数据统计分区的编号前后不同(表明为新的统计分区的数据)
					if (IREG != PreIREG + 1)
					{
						//新的统计分区的编号必须为之前的统计分区的编号加1
						printf(_T("\n"));
						printf(_T("文件<%s>中编号为%d统计方案编号为%d的分区编号不连续, 请检查!\n"),
							_T("水量平衡统计分区表.in"), SCHID, IREG);
						PauAndQuit();

					}
				}
			}
		}
		//记录当前数据行的统计方案编号和统计分区编号
		PreSCHID = SCHID;
		PreIREG = IREG;
		//视情况生成对象
		if (bNewSche == TRUE)
		{
			pScheme = new CRegSche;
			pScheme->m_SCHEID = SCHID;
			pScheme->m_SCHENAM = SCHNAM;
			//将统计方案加入方案数组
			m_BudgetSchAry.push_back(pScheme);
		}
		//保存数据
		pScheme->AddRegScheCell(IREG, REGNAM, ILYR, IROW, ICOL);
	}

	//后面不会再读入数据了
	StRegDataFile.Close();
}

void CRegBudget::ReadRBData(_ConnectionPtr pConnection)
{
	//读取水量平衡统计方案数据
	//注：必须用try块,否则数据表不存在时程序直接会报错,无法继续执行
	try
	{
		//打开数据表读数据
		CString strSQL;
		CString strSchNam, strRegNam;
		_RecordsetPtr pTable;
		pTable.CreateInstance(__uuidof(Recordset));
		//将数据集游标设置在客户端(可提高性能)
		pTable->CursorLocation = adUseClient;
		//设置数据集缓存(1000条)
		pTable->CacheSize = 1000;
		strSQL = _T("SELECT * FROM 水量平衡统计分区表 ORDER BY SCHID, IREG");
		pTable->Open(LPCSTR(strSQL),
			pConnection.GetInterfacePtr(),
			adOpenStatic,
			adLockOptimistic,
			adCmdText);
		if (pTable->BOF)
		{
			//数据表不存在或是空表, 报错退出
			pTable->Close();
			printf(_T("\n"));
			printf(_T("已选择启用分区水量平衡统计功能，<水量平衡统计分区表>不能为空表,请检查！\n"));
			PauAndQuit();
		}
		//数据表存在且有数据
		printf(_T("<水量平衡统计分区表>\n"));
		//读入分区单元数据
		long i, nCount, iLyr, iRow, iCol;
		_variant_t Value;
		long RegIndex = 0;
		long SchemeID = 0;
		long iCurSchID = 0;
		CRegion* pGridReg = NULL;
		CGridCell* pGridCell = NULL;
		CRegSche* pScheme = NULL;
		pTable->MoveLast();
		nCount = pTable->GetRecordCount();  //数据个数
		pTable->MoveFirst();
		assert(nCount > 0);
		for (i = 0; i<nCount; i++)
		{
			Value = pTable->GetCollect(_T("SCHID"));
			SchemeID = Value.intVal;
			if (SchemeID < 1)
			{
				printf(_T("\n"));
				printf(_T("统计方案的编号应大于等于1,请检查<水量平衡统计分区表>! \n"));
				PauAndQuit();
			}
			Value = pTable->GetCollect(_T("SCHNAM"));     
			strSchNam = (LPCSTR)_bstr_t(Value);
			TrimBlank(strSchNam);
			Value = pTable->GetCollect(_T("IREG"));
			RegIndex = Value.intVal;
			if (RegIndex < 1)
			{
				printf(_T("\n"));
				printf(_T("统计分区的编号应大于等于1,请检查<水量平衡统计分区表>! \n"));
				PauAndQuit();
			}
			Value = pTable->GetCollect(_T("REGNAM"));   
			strRegNam = (LPCSTR)_bstr_t(Value);
			TrimBlank(strRegNam);
			Value = pTable->GetCollect(_T("ILYR"));
			iLyr = Value.intVal;
			Value = pTable->GetCollect(_T("IROW"));
			iRow = Value.intVal;
			Value = pTable->GetCollect(_T("ICOL"));
			iCol = Value.intVal;
			GDWMOD.CheckCellLimit(iLyr, iRow, iCol);
			if (iCurSchID != SchemeID)
			{
				//新的统计方案
				if (iCurSchID + 1 != SchemeID)
				{
					//统计方案编号需从1开始并连续
					printf(_T("\n"));
					printf(_T("编号为%d的水量平衡统计方案的编号没从1开始或不连续!\n"), SchemeID);
					printf(_T("请检查<水量平衡统计分区表>!\n"));
					PauAndQuit();
				}
				if (RegIndex != 1)
				{
					//新的统计方案第1个数据的统计分区编号必须为1
					printf(_T("\n"));
					printf(_T("编号为%d的水量平衡统计方案的统计分区编号没从1开始!\n"), SchemeID);
					printf(_T("请检查<水量平衡统计分区表>!\n"));
					PauAndQuit();
				}
				//新建统计方案
				pScheme = new CRegSche;
				pScheme->m_SCHEID = SchemeID;
				pScheme->m_SCHENAM = strSchNam;
				//将统计方案加入方案数组
				m_BudgetSchAry.push_back(pScheme);
				//记录当前的统计方案编号
				iCurSchID = SchemeID;
			}
			else
			{
				//已有统计方案的数据
				pScheme = m_BudgetSchAry[m_BudgetSchAry.size() - 1];
				if (pScheme->m_SCHENAM != strSchNam)
				{
					printf(_T("\n"));
					printf(_T("编号为%d的统计方案的名称与之前记录的不一致!\n"), pScheme->m_SCHEID);
					printf(_T("请检查<水量平衡统计分区表>!\n"));
					PauAndQuit();
				}
			}
			//将网格数据加入统计方案
			pScheme->AddRegScheCell(RegIndex, strRegNam, iLyr, iRow, iCol);
			pTable->MoveNext();                               //定位到下一条数据
		}
		pTable->Close();
	}
	catch (_com_error *e)
	{
		printf((LPCTSTR)e->ErrorMessage());
	}
}

void CRegBudget::SetPrnOpt(long iPrnOpt)
{
	//设置水量平衡统计结果输出控制

	m_iPrnOpt = iPrnOpt;
}

void CRegBudget::InitSimulation()
{
	//对水量平衡统计方案进行初始化

	long i;
	for (i = 0; i < m_BudgetSchAry.size(); i++)
	{
		m_BudgetSchAry[i]->InitRegScheData();
	}

	if (GDWMOD.GetDataContex() == 1)
	{
		//TXT文件数据环境, 打开模拟结果输出表
		CString FilePath, strOutFile;
		CString FileHead;
		FilePath = GDWMOD.GetOutPutPath();
		//统计分区_水量平衡输出表
		if (m_iPrnOpt != 0)
		{
			strOutFile = FilePath + _T("统计分区_水量平衡输出表.out");
			if (!m_BDOutPutFile.Open(strOutFile, CFile::modeCreate | CFile::modeWrite | CFile::typeText | CFile::shareDenyNone))
			{
				printf(_T("\n"));
				printf(_T("打开文件失败!\n"));
				PauAndQuit();
			}
			//写表头
			FileHead = _T(" IPER  ISTEP  TLEN  SCHID  SCHNAM  REGID  REGNAM");
			for (size_t i = 0; i < CRegion::m_ItemNamAry.size(); i++)
			{
				FileHead = FileHead + _T("  ") + CRegion::m_ItemNamAry[i];
			}
			FileHead = FileHead + _T("\n");
			m_BDOutPutFile.WriteString(FileHead);

			//统计分区_流量关系输出表
			strOutFile = FilePath + _T("统计分区_流量关系输出表.out");
			if (!m_FlowOutPutFile.Open(strOutFile, CFile::modeCreate | CFile::modeWrite | CFile::typeText | CFile::shareDenyNone))
			{
				printf(_T("\n"));
				printf(_T("打开文件失败!\n"));
				PauAndQuit();
			}
			//写表头
			FileHead = _T(" IPER  ISTEP  TLEN  SCHID  SCHNAM  FMREGID  FMREGNAM  TOREGID  TOREGNAM  EXCWT");
			FileHead = FileHead + _T("\n");
			m_FlowOutPutFile.WriteString(FileHead);
		}
	}
}

void CRegBudget::SaveResult()
{
	//保存水量平衡分析结果

	if (m_iPrnOpt == 0)
	{
		//如果不输出分区水平衡统计分析结果, 直接返回
		return;
	}

	long i;
	for (i = 0; i < m_BudgetSchAry.size(); i++)
	{
		m_BudgetSchAry[i]->SaveResult();
	}
}

void CRegBudget::BudgetOutPutTXT(BOOL bEndPer, long iPer, long iStep, double CumTLen)
{
	//输出统计分区水量平衡统计结果(TXT)

	if (!(m_iPrnOpt == 1 || (m_iPrnOpt == 2 && bEndPer == TRUE)))
	{
		//如果没有到输出时机,直接返回
		return;
	}

	long i;
	for (i = 0; i < m_BudgetSchAry.size(); i++)
	{
		m_BudgetSchAry[i]->ScheOutPut(m_iPrnOpt, iPer, iStep, CumTLen, m_BDOutPutFile, m_FlowOutPutFile);
	}
	//写入文件
	m_BDOutPutFile.Flush();
	m_FlowOutPutFile.Flush();
}

void CRegBudget::BudgetOutPutDB(BOOL bEndPer, long iPer, long iStep, double CumTLen)
{
	//输出统计分区水量平衡统计结果(数据库)

	if (!(m_iPrnOpt == 1 || (m_iPrnOpt == 2 && bEndPer == TRUE)))
	{
		//如果没有到输出时机,直接返回
		return;
	}

	long i;
	for (i = 0; i < m_BudgetSchAry.size(); i++)
	{
		m_BudgetSchAry[i]->ScheOutPut(m_iPrnOpt, iPer, iStep, CumTLen);
	}
}