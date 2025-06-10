#pragma once
#include "ClassDef.h"

//////////////////////////////////////////////////////////
//定常水头网格单元类声明
class CCHBCell
{
	friend class CGridLyr;
	friend class CGDWater;

public:
	CCHBCell();
	virtual ~CCHBCell();

protected:
	CGridCell* m_pGridCell;                       //对应的边界单元指针
	double m_BndHead;                             //对应的边界水头(L)
};
