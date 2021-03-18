// ThumbViewerView.cpp : implementation of the CThumbViewerView class
//

#include "stdafx.h"
#include "ThumbViewer.h"

#include "MainFrm.h"
#include "ThumbViewerDoc.h"
#include "ThumbViewerView.h"
//#include "memdc.h"

#include <process.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CThumbViewerView

IMPLEMENT_DYNCREATE(CThumbViewerView, CListView)

BEGIN_MESSAGE_MAP(CThumbViewerView, CListView)
	//{{AFX_MSG_MAP(CThumbViewerView)
	ON_NOTIFY_REFLECT(LVN_ITEMCHANGED, OnItemchanged)
	ON_COMMAND(ID_FILE_NEW, OnFileNew)
	//}}AFX_MSG_MAP
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, CListView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, CListView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, CListView::OnFilePrintPreview)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CThumbViewerView construction/destruction

CThumbViewerView::CThumbViewerView()
{
	// TODO: add construction code here

	m_bTerminate=false;
	m_bRunning=false;
	// Initialize Thread Handle and Create Event Handle for controlling thread
	//m_hThread = NULL;
	
	//�X���b�h�I�u�W�F�N�g�̏�����
	m_pRecvThread = NULL;
	//���L�������̏�����
	m_lpShoriFlag = NULL;
	m_hShoriFlag = NULL;

}

CThumbViewerView::~CThumbViewerView()
{
	TerminateThread();
}

BOOL CThumbViewerView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CListView::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
// CThumbViewerView drawing

void CThumbViewerView::OnDraw(CDC* pDC)
{
	CThumbViewerDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	// TODO: add draw code for native data here
}

void CThumbViewerView::OnInitialUpdate()
{
	CListView::OnInitialUpdate();

	CListCtrl& ListCtrl=GetListCtrl();

	// Initialize Imaget List and Attach it to ListCtrl
	m_ImageListThumb.Create(THUMBNAIL_WIDTH, THUMBNAIL_HEIGHT, ILC_COLOR24, 0, 1);
	ListCtrl.SetImageList(&m_ImageListThumb, LVSIL_NORMAL);

}

/////////////////////////////////////////////////////////////////////////////
// CThumbViewerView printing

BOOL CThumbViewerView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	return DoPreparePrinting(pInfo);
}

void CThumbViewerView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add extra initialization before printing
}

void CThumbViewerView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add cleanup after printing
}

/////////////////////////////////////////////////////////////////////////////
// CThumbViewerView diagnostics

#ifdef _DEBUG
void CThumbViewerView::AssertValid() const
{
	CListView::AssertValid();
}

void CThumbViewerView::Dump(CDumpContext& dc) const
{
	CListView::Dump(dc);
}

CThumbViewerDoc* CThumbViewerView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CThumbViewerDoc)));
	return (CThumbViewerDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CThumbViewerView message handlers

BOOL CThumbViewerView::Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext) 
{
	// TODO: Add your specialized code here and/or call the base class
	return CListView::Create(lpszClassName, _T("ListView"),
		dwStyle|LVS_SHOWSELALWAYS|LVS_ALIGNTOP|LVS_ICON|LVS_SINGLESEL|LVS_AUTOARRANGE   , rect, pParentWnd, nID, pContext);
}

//unsigned __stdcall CThumbViewerView::LoadThumbNail(LPVOID lpParam)
UINT CThumbViewerView::LoadThumbNail(LPVOID lpParam)
{
	//���L������
	HANDLE	hShoriFlag;
	BOOL*	lpShoriFlag;
	// ���L������Open
	hShoriFlag = ::OpenFileMapping( FILE_MAP_ALL_ACCESS, FALSE, L"SHORIFLAG");
	lpShoriFlag = (BOOL *)::MapViewOfFile( hShoriFlag, FILE_MAP_ALL_ACCESS, 0, 0, 0);

	//CoInitialize(NULL);

	CThumbViewerView* pView=(CThumbViewerView*)lpParam;
	CThumbViewerDoc* pDoc=pView->GetDocument();

	CListCtrl& ListCtrl=pView->GetListCtrl();
	CImageList* pImgList=&pView->m_ImageListThumb;
	vector<CString> vFileName = pDoc->m_vFileName;	//20200512 add m_vFileName�𒼐ڃX���b�h���ő��삷��ƃ��������[�N��m_vFileName��j�󂷂�


	// reset our image list
	for(int i=0; i<pImgList->GetImageCount(); i++)
		pImgList->Remove(i);	

	// remove all items from list view
	ListCtrl.DeleteAllItems();

	//pImgList->SetImageCount(pDoc->m_vFileName.size());
	pImgList->SetImageCount(vFileName.size());			//20200512 change

	TCHAR path[MAX_PATH];
	vector<CString>::iterator iter;
	
	// Set redraw to FALSE to avoid flickering during adding new items
	ListCtrl.SetRedraw(FALSE);
	int nIndex=0;
	//for(iter=pDoc->m_vFileName.begin(); iter!=pDoc->m_vFileName.end() && pView->m_bTerminate!=true; iter++, nIndex++)
	for(iter=vFileName.begin(); iter!=vFileName.end() && pView->m_bTerminate!=true; iter++, nIndex++)			//20200512 change
	{
		ListCtrl.InsertItem(nIndex, *iter, nIndex);
	}

	ListCtrl.SetRedraw(TRUE);
	ListCtrl.Invalidate();

	// Create Brushes for Border and BackGround
	//HBRUSH hBrushBorder = ::CreateSolidBrush(RGB(192, 192, 192));
	HBRUSH hBrushBorder = ::CreateSolidBrush(RGB(0, 0, 0));
	HBRUSH hBrushBk = ::CreateSolidBrush(RGB(255, 255, 255));

	// Border Size
	RECT rcBorder;
	rcBorder.left=rcBorder.top=0;
	rcBorder.right=THUMBNAIL_WIDTH;
	rcBorder.bottom=THUMBNAIL_HEIGHT;

	const float fRatio=(float)THUMBNAIL_HEIGHT/THUMBNAIL_WIDTH;

	int XDest, YDest, nDestWidth, nDestHeight;
	nIndex=0;
	//for(iter=pDoc->m_vFileName.begin(); iter!=pDoc->m_vFileName.end() && pView->m_bTerminate!=true; iter++, nIndex++)
	for(iter=vFileName.begin(); iter!=vFileName.end() && pView->m_bTerminate!=true; iter++, nIndex++)			//20200512 change
	{
		// Load Image File
		_stprintf(path, _T("%s\\%s"), pDoc->m_strCurrentDirectory, *iter);

		int nImageType=pDoc->GetTypeFromFileName(path);
		if(nImageType==CXIMAGE_FORMAT_UNKNOWN)
			continue;

		CxImage image(path, nImageType);

		if(image.IsValid()==false)
			continue;

		// Calculate Rect to fit to canvas
		const float fImgRatio=(float)image.GetHeight()/image.GetWidth();
		if(fImgRatio > fRatio)
		{
			nDestWidth=THUMBNAIL_HEIGHT/fImgRatio;
			XDest=(THUMBNAIL_WIDTH-nDestWidth)/2;
			YDest=0;
			nDestHeight=THUMBNAIL_HEIGHT;
		}
		else
		{
			XDest=0;
			nDestWidth=THUMBNAIL_WIDTH;
			nDestHeight=THUMBNAIL_WIDTH*fImgRatio;
			YDest=(THUMBNAIL_HEIGHT-nDestHeight)/2;
		}

		CClientDC cdc(pView);
		HDC hDC=::CreateCompatibleDC(cdc.m_hDC);
		HBITMAP bm = CreateCompatibleBitmap(cdc.m_hDC, THUMBNAIL_WIDTH, THUMBNAIL_HEIGHT);
		HBITMAP pOldBitmapImage = (HBITMAP)SelectObject(hDC,bm);
		// Draw Background
		::FillRect(hDC, &rcBorder, hBrushBk);

		// Draw Image
		image.Stretch(hDC, XDest, YDest, nDestWidth, nDestHeight);

		// Draw Border
		::FrameRect(hDC, &rcBorder, hBrushBorder);

		SelectObject(hDC, pOldBitmapImage);

		// Attach to Bitmap and Replace image in CImageList
		CBitmap bitmap;
		bitmap.Attach(bm);
		pImgList->Replace(nIndex, &bitmap, NULL);

		// Redraw only a current item for removing flickering and fast speed.
		ListCtrl.RedrawItems(nIndex, nIndex);

		// Release used DC and Object
		DeleteDC(hDC);
		DeleteObject(bm);
	}
	DeleteObject(hBrushBorder);
	DeleteObject(hBrushBk);

	ListCtrl.Invalidate();
	pView->m_bRunning=false;
	pView->m_bTerminate=false;

	//_endthreadex( 0 );

	//CoUninitialize();
	
	// ���L������Close
	::UnmapViewOfFile(lpShoriFlag);
	::CloseHandle(hShoriFlag);
	return 0;
}

void CThumbViewerView::LoadThumbImages()
{
	CThumbViewerDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	CListCtrl& ListCtrl=GetListCtrl();

	TerminateThread();
	//20200512 add ��������
	// �}�b�s���O�I�u�W�F�N�g�쐬
	m_hShoriFlag = ::CreateFileMapping( INVALID_HANDLE_VALUE,
										NULL,
										PAGE_READWRITE,
										0,
										sizeof(BOOL),		// ���L�������̃T�C�Y
										_T("SHORIFLAG") );	// ���L��������
	if( m_hShoriFlag == NULL ) {
		AfxMessageBox(_T("CreateFileMapping�G���["));
		return;
	}

	// ���L���������}�b�s���O
	m_lpShoriFlag = (BOOL *)::MapViewOfFile( m_hShoriFlag, FILE_MAP_WRITE, 0, 0, 0);
	if( m_lpShoriFlag == NULL){
		AfxMessageBox(_T("MapViewOfFile�G���["));
		return;
	}
		
	*m_lpShoriFlag = TRUE;

	// �I���C�x���g�쐬
	// m_hEventStop = ::CreateEvent(NULL, TRUE, FALSE, NULL);
		m_pRecvThread = NULL; // �X���b�h�I�u�W�F�N�g�̃|�C���^�N���A

	// �T�X�y���h��ԂŃX���b�h���쐬
	m_pRecvThread = ::AfxBeginThread(LoadThumbNail, (LPVOID)this, THREAD_PRIORITY_NORMAL, 0, CREATE_SUSPENDED,NULL);
	//m_pRecvThread = AfxBeginThread(&ThreadEntry, this);
	m_pRecvThread->m_bAutoDelete = FALSE;	// �����j���t���O�N���A
	m_pRecvThread->ResumeThread();			// �T�X�y���h����
	//20200512 add �����܂�
	//m_hThread = (HANDLE)_beginthreadex(NULL,0, LoadThumbNail,(LPVOID)this,0/* CREATE_SUSPENDED*/,&m_dwThreadID); 

	m_bRunning=true;
}

void CThumbViewerView::OnFileNew() 
{
	// TODO: Add your command handler code here
	CListCtrl& ListCtrl=GetListCtrl();
	ListCtrl.DeleteAllItems();

	// reset our image list
	for(int i=0; i<m_ImageListThumb.GetImageCount(); i++)
		m_ImageListThumb.Remove(i);
	m_ImageListThumb.SetImageCount(0);

	CThumbViewerDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	// Delete Selected Image and clear Image File Name
	pDoc->m_vFileName.clear();
	if(pDoc->m_pSelectedImage!=NULL)
	{
		delete pDoc->m_pSelectedImage;
		pDoc->m_pSelectedImage=NULL;
	}
	// Redraw Selected Image in PreviewBar
	CMainFrame* pFrame=(CMainFrame*)AfxGetMainWnd();
	pFrame->m_wndPreviewBar.SendMessage(WM_SIZE);
}

void CThumbViewerView::OnItemchanged(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	// TODO: Add your control notification handler code here
	CThumbViewerDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	pDoc->SelectItem(pNMListView->iItem);
	*pResult = 0;
}

BOOL CThumbViewerView::TerminateThread()
{
	if ( !m_bRunning )
	{
		// �X���b�h�I�u�W�F�N�g�j��
		if(m_pRecvThread)delete m_pRecvThread;
		m_pRecvThread = NULL;

		// ���L�������̔j��
		if(m_lpShoriFlag){
			::UnmapViewOfFile(m_lpShoriFlag);		// ���L��������close
			::CloseHandle( m_hShoriFlag);			 // ���L�������̍폜
			m_lpShoriFlag = NULL;
			m_hShoriFlag = NULL;
		}

		return TRUE;
	}
  
  m_bTerminate=true;
 
  for( ; ; )
  {
    if ( ::WaitForSingleObject(m_hThread, 0) == WAIT_OBJECT_0 )
      break;
 
    MSG msg;
    while (::PeekMessage(&msg,NULL,0,0,PM_NOREMOVE)) 
    { 
      if (!AfxGetApp()->PumpMessage()) 
        break; 
    } 
  }
 
  ::CloseHandle(m_hThread);
  return TRUE;

}

