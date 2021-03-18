// ThumbViewerDoc.cpp : implementation of the CThumbViewerDoc class
//

#include "stdafx.h"
#include "ThumbViewer.h"

#include "MainFrm.h"
#include "ThumbViewerDoc.h"
#include "ThumbViewerView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CThumbViewerDoc

IMPLEMENT_DYNCREATE(CThumbViewerDoc, CDocument)

BEGIN_MESSAGE_MAP(CThumbViewerDoc, CDocument)
	//{{AFX_MSG_MAP(CThumbViewerDoc)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CThumbViewerDoc construction/destruction

CThumbViewerDoc::CThumbViewerDoc()
{
	// TODO: add one-time construction code here
	m_pSelectedImage=NULL;
	m_strCurrentDirectory=_T("");
	m_nSelectedItem=-1;
	//m_vFileName.push_back(_T(""));
}

CThumbViewerDoc::~CThumbViewerDoc()
{
	if(m_pSelectedImage!=NULL)
		delete m_pSelectedImage;
}

BOOL CThumbViewerDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CThumbViewerDoc serialization

void CThumbViewerDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{
		// TODO: add loading code here
	}
}

/////////////////////////////////////////////////////////////////////////////
// CThumbViewerDoc diagnostics

#ifdef _DEBUG
void CThumbViewerDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CThumbViewerDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CThumbViewerDoc commands
void CThumbViewerDoc::SelectDirectory(LPCTSTR pstr)
{
	CMainFrame* pFrame=(CMainFrame*)AfxGetMainWnd();

	m_strCurrentDirectory=pstr;
	m_vFileName.clear();
	//vector<CString> vFileName;
	//m_vFileName.push_back(_T(""));

	if(m_pSelectedImage!=NULL)
	{
		delete m_pSelectedImage;
		m_pSelectedImage=NULL;
	}
	pFrame->m_wndPreviewBar.SendMessage(WM_SIZE);

	CFileFind finder;
	CString strWildCard(pstr);
	strWildCard+=_T("\\*.*");

	BOOL bWorking=finder.FindFile(strWildCard);

	while(bWorking)
	{
		bWorking=finder.FindNextFile();

		if(finder.IsDots() || finder.IsDirectory())
			continue;
		else
		{
			CString filePath=finder.GetFileName();

			// Get Image File Name List
			if(GetTypeFromFileName(filePath)!=CXIMAGE_FORMAT_UNKNOWN)
			{
				// Make Lower for Sorting
				filePath.MakeLower();
				m_vFileName.push_back(filePath);
				//vFileName.push_back(filePath);
			}
		}
	}
	//m_vFileName = vFileName;

	finder.Close();

	// Sort FileName
	CString strTemp;
	QuickSortString(m_vFileName, 0, m_vFileName.size()-1, strTemp);
	CThumbViewerView* pView=(CThumbViewerView*)pFrame->GetActiveView();
	pView->LoadThumbImages();

	m_nSelectedItem=-1;
}

void CThumbViewerDoc::QuickSortString(vector<CString>& vString, int left, int right, CString& temp)
{
	if(left < right)
	{
		CString pivot=vString[left];
		const int nSize=vString.size();

		int i=left, j=right+1;
		do {
			do i++;
			while(i < nSize && vString[i].Compare(pivot) < 0);
			do j--;
			while(j < nSize && vString[j].Compare(pivot) > 0);

			if(i < j)
			{
				temp=vString[i];
				vString[i]=vString[j];
				vString[j]=temp;
			}
		}while(i<j);
		temp=vString[left];
		vString[left]=vString[j];
		vString[j]=temp;

		QuickSortString(vString, left, j-1, temp);
		QuickSortString(vString, j+1, right, temp);
	}
}

void CThumbViewerDoc::SelectItem(int nIndex)
{
	// Selected item is equal to current selected item.
	if(m_nSelectedItem==nIndex)
		return;
	m_nSelectedItem=nIndex;

	if(m_pSelectedImage!=NULL)
	{
		delete m_pSelectedImage;
		m_pSelectedImage=NULL;
	}
	
	if(nIndex < m_vFileName.size())
	{
		CString filePath;
		filePath=m_strCurrentDirectory+"\\"+m_vFileName[nIndex];
		int nImageType=GetTypeFromFileName(filePath);
		if(nImageType!=CXIMAGE_FORMAT_UNKNOWN)
		{
			m_pSelectedImage=new CxImage(filePath, nImageType);
			CMainFrame* pFrame=(CMainFrame*)AfxGetMainWnd();
			// Redraw Selected Image;
			pFrame->m_wndPreviewBar.SendMessage(WM_SIZE);
		}
	}
}

// return Image file type extraing from filename
int CThumbViewerDoc::GetTypeFromFileName(LPCTSTR pstr)
{
	CString fileName(pstr);
	CString ext3=fileName.Right(3);
	CString ext4=fileName.Right(4);
#if CXIMAGE_SUPPORT_BMP
	if(ext3.CompareNoCase(_T("bmp"))==0)
		return CXIMAGE_FORMAT_BMP;
#endif
#if CXIMAGE_SUPPORT_GIF
	if(ext3.CompareNoCase(_T("gif"))==0)
		return CXIMAGE_FORMAT_GIF;
#endif
#if CXIMAGE_SUPPORT_JPG
	if(ext3.CompareNoCase(_T("jpg"))==0 || ext4.CompareNoCase(_T("jpeg"))==0)
		return CXIMAGE_FORMAT_JPG;
#endif
#if CXIMAGE_SUPPORT_PNG
	if(ext3.CompareNoCase(_T("png"))==0)
		return CXIMAGE_FORMAT_PNG;
#endif
#if CXIMAGE_SUPPORT_MNG
	if(ext3.CompareNoCase(_T("mng"))==0 || ext3.CompareNoCase(_T("jng"))==0 ||ext3.CompareNoCase(_T("png"))==0)
		return CXIMAGE_FORMAT_MNG;
#endif
#if CXIMAGE_SUPPORT_ICO
	if(ext3.CompareNoCase(_T("ico"))==0)
		return CXIMAGE_FORMAT_ICO;
#endif
#if CXIMAGE_SUPPORT_TIF
	if(ext3.CompareNoCase(_T("tif"))==0 || ext4.CompareNoCase(_T("tiff"))==0)
		return CXIMAGE_FORMAT_TIF;
#endif
#if CXIMAGE_SUPPORT_TGA
	if(ext3.CompareNoCase(_T("tga"))==0)
		return CXIMAGE_FORMAT_TGA;
#endif
#if CXIMAGE_SUPPORT_PCX
	if(ext3.CompareNoCase(_T("pcx"))==0)
		return CXIMAGE_FORMAT_PCX;
#endif
#if CXIMAGE_SUPPORT_WBMP
	if(ext4.CompareNoCase(_T("wbmp"))==0)
		return CXIMAGE_FORMAT_WBMP;
#endif
#if CXIMAGE_SUPPORT_WMF
	if(ext3.CompareNoCase(_T("wmf"))==0 || ext3.CompareNoCase(_T("emf"))==0)
		return CXIMAGE_FORMAT_WMF;
#endif
#if CXIMAGE_SUPPORT_J2K
	if(ext3.CompareNoCase(_T("j2k"))==0 || ext3.CompareNoCase(_T("jp2"))==0)
		return CXIMAGE_FORMAT_J2K;
#endif
#if CXIMAGE_SUPPORT_JBG
	if(ext3.CompareNoCase(_T("jbg"))==0)
		return CXIMAGE_FORMAT_JBG;
#endif
#if CXIMAGE_SUPPORT_JP2
	if(ext3.CompareNoCase(_T("j2k"))==0 || ext3.CompareNoCase(_T("jp2"))==0)
		return CXIMAGE_FORMAT_JP2;
#endif
#if CXIMAGE_SUPPORT_JPC
	if(ext3.CompareNoCase(_T("j2c"))==0 || ext3.CompareNoCase(_T("jpc"))==0)
		return CXIMAGE_FORMAT_JPC;
#endif
#if CXIMAGE_SUPPORT_PGX
	if(ext3.CompareNoCase(_T("pgx"))==0)
		return CXIMAGE_FORMAT_PGX;
#endif
#if CXIMAGE_SUPPORT_PNM
	if(ext3.CompareNoCase(_T("pnm"))==0 || ext3.CompareNoCase(_T("pgm"))==0 || ext3.CompareNoCase(_T("ppm"))==0)
		return CXIMAGE_FORMAT_PNM;
#endif
#if CXIMAGE_SUPPORT_RAS
	if(ext3.CompareNoCase(_T("ras"))==0)
		return CXIMAGE_FORMAT_RAS;
#endif
	return CXIMAGE_FORMAT_UNKNOWN;
}
