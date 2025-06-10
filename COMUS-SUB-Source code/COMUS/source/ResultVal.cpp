#include "StdAfx.h"
#include "ClassInclude.h"

//////////////////////////////////////////////////////////
//计算结果数据类函数定义
CResultVal::CResultVal()
{
	step_val = 0.0;    //时步计算结果
	cum_val = 0.0;     //累加计算结果
}

CResultVal::~CResultVal()
{

}
