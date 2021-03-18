// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

//#if !defined(AFX_STDAFX_H__BD20FFBC_BD69_4806_AD95_20F2F3F61682__INCLUDED_)
//#define AFX_STDAFX_H__BD20FFBC_BD69_4806_AD95_20F2F3F61682__INCLUDED_

//#if _MSC_VER > 1000
//#define for if (false) ; else for // For Scope Error
#pragma once
//#endif // _MSC_VER > 1000

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN		// Windows ヘッダーから使用されていない部分を除外します。
#endif

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#include <afxcview.h>
#include <afxdisp.h>        // MFC Automation classes
#include <afxdtctl.h>		// MFC support for Internet Explorer 4 Common Controls
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT

#define _SCB_REPLACE_MINIFRAME
#include "control_bar\sizecbar.h"
#include "control_bar\scbarg.h"
#include "control_bar\scbarcf.h"

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

//#endif // !defined(AFX_STDAFX_H__BD20FFBC_BD69_4806_AD95_20F2F3F61682__INCLUDED_)

//#define DLL_BUILD
#if defined(_DEBUG) || defined(DEBUG)
// Debugのとき
#if defined(DLL_BUILD)
	#pragma comment( lib, "cximagecrtd.lib" )
#else
	#pragma comment( lib, "pngd.lib" )
	#pragma comment( lib, "jpegd.lib" )
	#pragma comment( lib, "zlibd.lib" )
	#pragma comment( lib, "tiffd.lib" )
	#pragma comment( lib, "jbigd.lib" )
	#pragma comment( lib, "jasperd.lib" )
	#pragma comment( lib, "mngd.lib" )
	#pragma comment( lib, "libdcrd.lib" )
	#pragma comment( lib, "libpsdd.lib" )
	#pragma comment( lib, "cximaged.lib" )
#endif
#else
// Releaseのとき
#if defined(DLL_BUILD)
	#pragma comment( lib, "cximagecrt.lib" )
#else
	#pragma comment( lib, "png.lib" )
	#pragma comment( lib, "jpeg.lib" )
	#pragma comment( lib, "zlib.lib" )
	#pragma comment( lib, "tiff.lib" )
	#pragma comment( lib, "jbig.lib" )
	#pragma comment( lib, "jasper.lib" )
	#pragma comment( lib, "mng.lib" )
	#pragma comment( lib, "libdcr.lib" )
	#pragma comment( lib, "libpsd.lib" )
	#pragma comment( lib, "cximage.lib" )
#endif
#endif

