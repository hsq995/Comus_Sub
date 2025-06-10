// StdAfx.h : 标准系统包含文件的包含文件，
// 或是经常使用但不常更改的
// 特定于项目的包含文件
//

#pragma once
#include "targetver.h"
#define NO_WARN_MBCS_MFC_DEPRECATION

// TODO:  在此处引用程序需要的其他头文件
#include <assert.h>
#include <afxcmn.h>
#include <AclAPI.h>
#include <chrono>
#include <omp.h>
#include <math.h>
#include <vector>	   	    
#include <map>
#include <fstream>
#import "c:\program files\common files\system\ado\msado15.dll" \
	no_namespace \
	rename ("EOF", "adoEOF")
#using <mscorlib.dll> 
#using <System.Data.dll> 
#using <System.dll> 
using namespace System;
using namespace System::Data;
using namespace System::Data::SqlClient;
using namespace System::Runtime::InteropServices;