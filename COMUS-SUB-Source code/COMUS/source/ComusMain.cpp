//ComusMain.cpp : Defines the entry point for the console application.
#include "StdAfx.h"
#include "ClassInclude.h"

//声明全局变量GDWMOD
extern CGDWater GDWMOD;

int main(int argc, char *argv[])
{
	CTime tBegin, tEnd;
	CString TimeNow;
	DWORD MiliStart, MiliEnd;
	long HourUse, MiniteUse;
	double SecondSpan, SecondUse;

	//输出版本信息
	printf("C++ Object-Oriented Model for UnderGround Water Simulation with Subsidence (CoMuS-SUB)\n");
	printf(" - Version: 2.5, 2025/3/16\n");
	printf(" - Author: Chuiyu Lu\n");
	printf(_T("\n"));

	//记录模拟之前的时刻
	tBegin = CTime::GetCurrentTime();
	TimeNow = tBegin.Format(_T("%Y-%m-%d: %H:%M:%S"));
	MiliStart = GetTickCount();
	printf(_T("Simulation starting time:\n"));
	printf(TimeNow);
	printf(_T("\n"));

	//进行地下水数值模拟
	CString StrPath;
	if (argc == 1)
	{
		//用户没有输入文件路径, 获取当前目录
		TCHAR CurrentDir[MAX_PATH];
		GetCurrentDirectory(MAX_PATH, CurrentDir);
		StrPath = CurrentDir;
	}
	else if(argc == 2)
	{
		//用户输入了文件路径
		StrPath = argv[1];
	}
	else
	{
		printf(_T("运行参数不能正确识别!\n"));
		PauAndQuit();
	}
	printf(_T("\n"));
	GDWMOD.Simulate(StrPath);

	printf(_T("\n"));
	printf(_T("Simulation completed! \n"));

	//记录模拟结束时刻
	printf(_T("\n"));
	tEnd = CTime::GetCurrentTime();
	TimeNow = tEnd.Format("%Y-%m-%d: %H:%M:%S");
	MiliEnd = GetTickCount();
	printf("Simulation ending time:\n");
	printf(TimeNow);
	printf(_T("\n"));
	printf(_T("\n"));

	//计算所用时间
	SecondSpan = (MiliEnd - MiliStart) / 1000.0;
	HourUse = long(SecondSpan / 3600.0);
	MiniteUse = long((SecondSpan - 3600.0 * HourUse) / 60.0);
	SecondUse = SecondSpan - 3600.0 * HourUse - MiniteUse * 60.0;

	//输出计算用时
	CString str;
	str.Format(_T("Time used: %d Hours + %d Minites + %.3f Seconds\n"), HourUse, MiniteUse, SecondUse);
	printf(str);
	printf(_T("\n"));

	system(_T("pause"));
	return(0);
}