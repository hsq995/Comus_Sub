#pragma once
#include "ClassDef.h"

///////////////////////////////////
//数据库高速缓存表类声明
class CMemTable  
{
public:
	CMemTable();
	virtual ~CMemTable();

protected:
    gcroot<DataTable^> m_pDataTable;                          //缓存表
    gcroot<SqlBulkCopy^> m_pBulkCopy;                         //数据块写入工具

public:
	void MakeDataTable(_RecordsetPtr pTable, CString TbName); //构建缓存表结构
	void WriteDatabase();                                     //将缓存表数据写入数据库
	gcroot<DataTable^>& GetDataTable();                       //返回缓存表对象
};
