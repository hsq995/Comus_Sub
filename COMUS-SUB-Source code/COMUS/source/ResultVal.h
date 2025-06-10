#pragma once
#include "ClassDef.h"

//////////////////////////////////////////////////////////
//结果输出数据类声明
class CResultVal  
{
public:
	CResultVal();
	virtual ~CResultVal();

public:
	double step_val;    //时步计算结果
	double cum_val;     //累加计算结果
};
