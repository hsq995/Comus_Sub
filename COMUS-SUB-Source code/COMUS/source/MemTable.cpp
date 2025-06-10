#include "StdAfx.h"
#include "ClassInclude.h"

CMemTable::CMemTable()
{
    m_pDataTable = NULL;
    m_pBulkCopy = NULL;
}

CMemTable::~CMemTable()
{
	delete m_pDataTable;
	m_pDataTable = NULL;
    delete m_pBulkCopy;
	m_pBulkCopy = NULL;
}

void CMemTable::MakeDataTable(_RecordsetPtr pTable, CString TbName)
{
	try
	{
		//构建缓存表
		m_pDataTable = gcnew DataTable();
		m_pBulkCopy = gcnew SqlBulkCopy(GDWMOD.GetstrSqlCon(),
			SqlBulkCopyOptions::UseInternalTransaction);
		String^ DTbname;
		DTbname = gcnew String(TbName);	
		m_pBulkCopy->DestinationTableName = DTbname;
		m_pBulkCopy->BulkCopyTimeout = 5000;	
		long size = pTable->GetFields()->Count;
		long i;
		CString strFiledname;
		String^ strFdnam;
		for(i=0; i<size; i++)
		{
			strFiledname = (LPCSTR)pTable->GetFields()->GetItem(i)->Name;
			strFdnam = gcnew String(strFiledname);
			m_pDataTable->Columns->Add(strFdnam);
			m_pBulkCopy->ColumnMappings->Add(strFdnam,strFdnam);
		}
		pTable->Close();
	}
	catch(_com_error *e)
	{
		printf((LPCTSTR)e->ErrorMessage());
	}
}

void CMemTable::WriteDatabase()
{
	//如果缓存表中有数据,将其输出并清空缓存
	try
	{
		if(m_pDataTable->Rows->Count > 0)
		{
			//写入数据
			m_pBulkCopy->BatchSize = m_pDataTable->Rows->Count;
			m_pBulkCopy->WriteToServer(m_pDataTable);
			//清空数据缓存
			m_pDataTable->Clear();
		}
	}
	catch(_com_error *e)
	{
		printf((LPCTSTR)e->ErrorMessage());
	}
}

gcroot<DataTable^>& CMemTable::GetDataTable()
{
	//返回缓存表对象

	return m_pDataTable;
}
