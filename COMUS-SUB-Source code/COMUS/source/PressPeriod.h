#pragma once
#include "ClassDef.h"

///////////////////////////////////
//应力期时间控制信息类声明
class CPressPeriod
{
	friend class CGDWater;

public:
	CPressPeriod();
	virtual ~CPressPeriod();

protected:
	long m_iPeriod;                               //应力期编号，从1开始
	double m_PeridLen;                            //应力期长度(T)
	long m_NStep;                                 //应力期包含的模拟时段数量
	double m_MultR;                               //模拟时段的倍率因子(-)
};
