#include "StdAfx.h"
#include "ClassInclude.h"

//////////////////////////////////////////////////////////
//定常水头边界单元类函数定义
CCHBCell::CCHBCell()
{
	m_pGridCell = NULL;                       //对应的边界单元指针
	m_BndHead = 0.0;                          //对应的边界水头
}

CCHBCell::~CCHBCell()
{	

}