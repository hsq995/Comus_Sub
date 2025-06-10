#include "StdAfx.h"
#include "ClassInclude.h"

CPressPeriod::CPressPeriod()
{
	m_iPeriod = 0;                               //应力期编号，从1开始
	m_PeridLen = 0.0;                            //应力期长度(T)
	m_NStep = 0;                                 //应力期包含的模拟时段数量
	m_MultR = 0.0;                               //模拟时段的倍率因子(-)
}

CPressPeriod::~CPressPeriod()
{
}
