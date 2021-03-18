// DirTreeCtrl.cpp : implementation file
//

#include "stdafx.h"
#include "DirTreeCtrl.h"

#include "MainFrm.h"
#include "ThumbViewerDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDirTreeCtrl

CDirTreeCtrl::CDirTreeCtrl()
{
	m_bIncludeFiles = FALSE;
	m_pidlRoot = NULL;

	m_pDesktopFolder = NULL;
	SHGetDesktopFolder(&m_pDesktopFolder);

	m_pMalloc = NULL;
	SHGetMalloc(&m_pMalloc);

	m_SelectedDirectory=_T("");
	m_currentEditItemChildren=FALSE;
}

CDirTreeCtrl::~CDirTreeCtrl()
{
	if (m_pidlRoot != NULL)
		m_pMalloc->Free(m_pidlRoot);

	if (m_pMalloc != NULL)
		m_pMalloc->Release();

	if (m_pDesktopFolder != NULL)
		m_pDesktopFolder->Release();
}


BEGIN_MESSAGE_MAP(CDirTreeCtrl, CTreeCtrl)
	//{{AFX_MSG_MAP(CDirTreeCtrl)
	ON_NOTIFY_REFLECT(TVN_ITEMEXPANDING, OnItemexpanding)	//親アイテムの子アイテムのリストが展開または縮小されたことを通知します
	ON_NOTIFY_REFLECT(TVN_ENDLABELEDIT, OnEndlabeledit)		//ラベル編集の終了を通知します
	ON_NOTIFY_REFLECT(TVN_SELCHANGED, OnSelchanged)			//選択がアイテム間で変更されたことを通知します
	ON_NOTIFY_REFLECT(TVN_BEGINLABELEDIT, OnBeginlabeledit)	//インプレースラベル編集の開始を通知します
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDirTreeCtrl message handlers

void CDirTreeCtrl::Initialize()
{
	LPITEMIDLIST pidl = NULL;
	SHGetSpecialFolderLocation(*this, CSIDL_DESKTOP, &pidl);
    
    SHFILEINFO sfi;
	ZeroMemory(&sfi, sizeof(SHFILEINFO));
	HIMAGELIST hSysImageList = (HIMAGELIST) SHGetFileInfo((LPCTSTR)pidl, 0,
		&sfi, sizeof(SHFILEINFO), SHGFI_PIDL | SHGFI_SYSICONINDEX | SHGFI_SMALLICON);

	m_pMalloc->Free(pidl);
	
	TreeView_SetImageList(*this, hSysImageList, TVSIL_NORMAL);

	RefreshShellRoot(NULL);
}


LPITEMIDLIST CDirTreeCtrl::ILCombine(LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2)
{
	// Get the size of the resulting item identifier list
	int cb1 = ILGetLength(pidl1);
	int cb2 = ILGetLength(pidl2); 
	
	// Allocate a new item identifier list
	LPITEMIDLIST pidlNew = (LPITEMIDLIST)m_pMalloc->Alloc(cb1 + cb2 + sizeof(USHORT)); 
	if (pidlNew == NULL)
		return NULL;
	
	// Copy the first item identifier list 
	if (cb1 > 0)
		CopyMemory(pidlNew, pidl1, cb1 + sizeof(USHORT));

	// Copy the second item identifier list and terminating 0
	if (cb2 > 0)
		CopyMemory((((LPBYTE) pidlNew) + cb1), pidl2, cb2 + sizeof(USHORT));
	
	if (cb1 == 0 && cb2 == 0)
	{
		m_pMalloc->Free(pidlNew);
		return NULL;
	}

	return pidlNew;
}

int CDirTreeCtrl::ILGetLength(LPCITEMIDLIST pidl)
{
	if (pidl == NULL)
		return 0;

	// does not include terminating 0
	int length = 0, cb;
	
	do
	{
		cb = pidl->mkid.cb;
		pidl = (LPCITEMIDLIST)(((LPBYTE)pidl) + cb);
		length += cb;
	}
	while (cb != 0);

	return length;
}

LPCITEMIDLIST CDirTreeCtrl::ILGetNext(LPCITEMIDLIST pidl)
{
	if (pidl == NULL)
		return NULL;

	// Get the size of the specified item identifier. 
	int cb = pidl->mkid.cb; 
	
	// If the size is zero, it is the end of the list. 
	if (cb == 0) 
		return NULL; 
	
	// Add cb to pidl (casting to increment by bytes). 
	pidl = (LPCITEMIDLIST)(((LPBYTE)pidl) + cb); 
	
	// Return NULL if it is null-terminating, or a pidl otherwise. 
	return (pidl->mkid.cb == 0) ? NULL : pidl;
}

LPITEMIDLIST CDirTreeCtrl::ILCloneFirst(LPCITEMIDLIST pidl)
{
	if (pidl == NULL)
		return NULL;

	// Get the size of the specified item identifier. 
	int cb = pidl->mkid.cb; 
	
	// Allocate a new item identifier list. 
	LPITEMIDLIST pidlNew = (LPITEMIDLIST)m_pMalloc->Alloc(cb + sizeof(USHORT)); 
	if (pidlNew == NULL) 
		return NULL; 
	
	// Copy the specified item identifier. 
	CopyMemory(pidlNew, pidl, cb); 
	
	// Append a terminating zero. 
	*((USHORT *) (((LPBYTE) pidlNew) + cb)) = 0; 
	
	return pidlNew; 
}

LPCITEMIDLIST CDirTreeCtrl::ILGetLast(LPCITEMIDLIST pidl)
{
	LPCITEMIDLIST ret = pidl, tmp = ILGetNext(pidl);
	while (tmp != NULL)
	{
		ret = tmp;
		tmp = ILGetNext(tmp);
	}
	return ret;
}

void CDirTreeCtrl::RefreshShellRoot(LPCITEMIDLIST pidlRoot, BOOL bIncludeFiles)
{
	m_bIncludeFiles = bIncludeFiles;

	// store root
	m_pidlRoot = ILClone(pidlRoot);

	//LPITEMIDLIST* itemlist=new LPITEMIDLIST;
	LPITEMIDLIST itemlist;
	TCHAR buf[MAX_PATH];
	SHGetSpecialFolderLocation (this->m_hWnd, CSIDL_DESKTOP, &itemlist);
	SHGetPathFromIDList(itemlist, buf );

	TV_INSERTSTRUCT tvstruct;
	SHFILEINFO sfi;
	ZeroMemory(&sfi, sizeof(SHFILEINFO));
	SHGetFileInfo( buf, NULL, 
				   &sfi, 
				   sizeof(sfi), 
				   //GFI_PIDL | SHGFI_SYSICONINDEX | SHGFI_SMALLICON);
				   SHGFI_DISPLAYNAME | SHGFI_ICON | SHGFI_SMALLICON);
	//UINT uFlags = SHGFI_PIDL | SHGFI_SYSICONINDEX | SHGFI_SMALLICON;
	tvstruct.hParent=TVI_ROOT;
	tvstruct.hInsertAfter=TVI_LAST;
	//tvstruct.item.pszText = sfi.szDisplayName;
	tvstruct.item.pszText = _T("デスクトップ");
	tvstruct.item.mask=TVIF_TEXT | TVIF_IMAGE|TVIF_SELECTEDIMAGE | TVIF_PARAM;
	tvstruct.item.iImage= tvstruct.item.iSelectedImage=4;
	//tvstruct.item.lParam=(LPARAM)itemlist;
	//m_desktop_root=InsertItem("Desktop", 0, 0, TVI_ROOT);
	tvstruct.item.lParam = (LPARAM)itemlist;
	m_desktop_root=InsertItem(&tvstruct);
	//SetItemData(m_desktop_root, (LPARAM)itemlist);
	//SetItemData(m_desktop_root, (LPARAM)buf);
	//RefreshSubItems(TVI_ROOT);
	RefreshSubItems(m_desktop_root);
	Expand(m_desktop_root, TVE_EXPAND);
	return;
}

void CDirTreeCtrl::RefreshSubItems(HTREEITEM hParent)
//void CDirTreeCtrl::RefreshSubItems(HTREEITEM hParent, NM_TREEVIEW* pNMTreeView)
{
//	if (hParent != TVI_ROOT && !ItemHasChildren(hParent))
	if(hParent!=m_desktop_root && !ItemHasChildren(hParent))
		return;

	DeleteChildren(hParent);
	//if (hParent == TVI_ROOT)
	if(hParent==m_desktop_root)
		PopulateRoot();
		//PopulateRoot(pNMTreeView);
	else
	{
		PreExpandItem(hParent);
		ExpandItem(hParent);
		//ExpandItem(pNMTreeView);
	}
	//SetRedraw();
}

//ツリー展開の前処理
void CDirTreeCtrl::PreExpandItem(HTREEITEM hItem)
{
	if (!NeedsChildren(hItem))
	{
		if (WantsRefresh(hItem))
		{
			// delete child items before populating
			DeleteChildren(hItem);
		}
		else
		{
			// doesn't want new items
			m_hItemToPopulate = NULL;
			return;
		}
	}
	// if it wants new child items, go on
	m_hItemToPopulate = hItem;

	// fix redraw when expanded programatically
//	UpdateWindow();
	// hide changes until it's expanded
//	SetRedraw(FALSE);
	// add wait msg, to allow item expansion
//	m_hItemMsg = InsertItem(m_sWaitMsg, m_hItemToPopulate);
	// zero progress
//	m_iItemCount = 1;
//	m_iItemIndex = 0;
}

//void CDirTreeCtrl::ExpandItem(HTREEITEM hItem)
void CDirTreeCtrl::ExpandItem(HTREEITEM hItem, NM_TREEVIEW* pNMTreeView)
{
	if (m_hItemToPopulate == NULL)
		return;	// just expand, doesn't want new items

	ASSERT(hItem == m_hItemToPopulate);	// should never fail!!!

//	if (m_bShowWaitMsg)
//	{
		// display wait msg now, make sure it's visible
//		SetRedraw();
//		EnsureVisible(m_hItemMsg);
//		UpdateWindow();
//	}
	// setup animation thread, call PreAnimation
//	StartAnimation();
	// draw icon
//	if (m_bShowWaitMsg)
//		DrawUserIcon();
	// delay redraw after populating
//	SetRedraw(FALSE);
	// del temporary item (wait msg still shown)
//	DeleteItem(m_hItemMsg);
	// fill in with sub items
	//BOOL bCheckChildren = PopulateItem(hItem);
	BOOL bCheckChildren = PopulateItem(hItem, pNMTreeView);
	// clean up animation thread, call PostAnimation
//	StopAnimation();
	
	// change parent to reflect current children number
	//	if (hItem != TVI_ROOT)
	if(hItem!=m_desktop_root)
	{
		TVITEM item;
		item.hItem = hItem;
		item.mask = TVIF_HANDLE | TVIF_CHILDREN;
		item.cChildren = NeedsChildren(hItem) ? 0 : 1;
		if (bCheckChildren)
			SetItem(&item);
		else if (item.cChildren == 0)
			// restore item's plus button if no children inserted
			SetItemState(hItem, 0, TVIS_EXPANDED);
	}
	// redraw now
//	SetRedraw();
}


inline BOOL CDirTreeCtrl::NeedsChildren(HTREEITEM hParent)
{
	return (GetChildItem(hParent) == NULL);
}

void CDirTreeCtrl::DeleteChildren(HTREEITEM hParent)
{
	HTREEITEM hChild = GetChildItem(hParent);
	HTREEITEM hNext;

	while (hChild != NULL)
	{
		hNext = GetNextSiblingItem(hChild);
		DeleteItem(hChild);
		hChild = hNext;
	}
}

void CDirTreeCtrl::PopulateRoot()
//void CDirTreeCtrl::PopulateRoot(NM_TREEVIEW* pNMTreeView)
{
	PreExpandItem(m_desktop_root);
	ExpandItem(m_desktop_root);
	//ExpandItem(pNMTreeView);
//	PreExpandItem(TVI_ROOT);
//	ExpandItem(TVI_ROOT);
	
	// force update, don't scroll
//	SetRedraw(FALSE);
//	SCROLLINFO si;
//	GetScrollInfo(SB_HORZ, &si);
//	EnsureVisible(GetChildItem(TVI_ROOT));
//	SetScrollInfo(SB_HORZ, &si, FALSE);
//	SetRedraw();
}


BOOL CDirTreeCtrl::WantsRefresh(HTREEITEM hItem)
{
	UNREFERENCED_PARAMETER(hItem);

	// 	デフォルトの実装、更新なし
	return FALSE;
}


//項目を読み取る
//BOOL CDirTreeCtrl::PopulateItem(HTREEITEM hParent)
BOOL CDirTreeCtrl::PopulateItem(HTREEITEM hParent, NM_TREEVIEW* pNMTreeView)
{
	
	UNREFERENCED_PARAMETER(hParent);

	// must provide an implementation in the derived class
	PIDLIST_ABSOLUTE pidlParent;
	BOOL ret = FALSE;

	// get parent pidl
//	if (hParent == TVI_ROOT)
	IShellFolder* pParentFolder = NULL;
	HRESULT hr;
	if (hParent == m_desktop_root)
	{
		pidlParent = m_pidlRoot;
	}
	else
	{
		// get parent shell folder
		if (pNMTreeView)
		{
			pidlParent = (PIDLIST_ABSOLUTE)(pNMTreeView->itemNew.lParam);
		}
		else
		{
			pidlParent = (PIDLIST_ABSOLUTE)GetItemData(hParent);
		}

		//CString pathname = GetItemText(hParent);
		//AfxMessageBox(pathname);
		
		//hr = m_pDesktopFolder->BindToObject(pidlParent, NULL, IID_IShellFolder, (LPVOID*)&pParentFolder);
		hr = m_pDesktopFolder->BindToObject(pidlParent, NULL, IID_PPV_ARGS(&pParentFolder));
		if (hr != S_OK)
		{
			LPVOID string;
			FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
				FORMAT_MESSAGE_FROM_SYSTEM,
				NULL,
				hr,
				MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
				(LPTSTR)&string,
				0,
				NULL);
			if (string != NULL)
				OutputDebugString((LPCWSTR)string);
			LocalFree(string);
		}
	}

	// no folder object
	if (pParentFolder == NULL)
		if(hParent==m_desktop_root)
	//	if (hParent == TVI_ROOT)	// root is desktop
		{
			pParentFolder = m_pDesktopFolder;
			m_pDesktopFolder->AddRef();
		}
		else
			return TRUE;	//  don't try anymore

	// enum child pidls
	IEnumIDList* pEnumIDList = NULL;
	if (NOERROR == pParentFolder->EnumObjects(*this, SHCONTF_FOLDERS
		| ((m_bIncludeFiles) ? SHCONTF_NONFOLDERS : 0), &pEnumIDList))
	{
//		SetPopulationCount(0);

		LPITEMIDLIST pidl = NULL, pidlAbs;
		while (NOERROR == pEnumIDList->Next(1, &pidl, NULL))
		{
			// get an absolute pidl
			pidlAbs = ILCombine(pidlParent, pidl);
			// add item
			TVINSERTSTRUCT tvis;
			ZeroMemory(&tvis, sizeof(TVINSERTSTRUCT));
			tvis.hParent = hParent;
			tvis.hInsertAfter = TVI_LAST;
			tvis.item.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE
				| TVIF_CHILDREN | TVIF_PARAM;
			tvis.item.lParam = (LPARAM)pidlAbs;
			// fill with text, icons and children
			ret = FillItem(tvis.item, pidlAbs, pParentFolder, pidl);
			
		//	char buf[MAX_PATH];
		//	SHGetPathFromIDList(pidl, buf);
			HTREEITEM hItem=InsertItem(&tvis);
		//	SetItemData(hItem, (LPARAM)buf);
//			IncreasePopulation();

			// free enumerated object
			m_pMalloc->Free(pidl);
		}
		// free enum object
		pEnumIDList->Release();


	}

	// sort items
	TVSORTCB tvscb;
	tvscb.hParent = hParent;
	tvscb.lpfnCompare = CompareFunc;
	tvscb.lParam = (LPARAM)pParentFolder;
	SortChildrenCB(&tvscb);			//提供されたコールバック関数を使用して、コントロール内のアイテムを並べ替えます。

	// free folder object
	pParentFolder->Release();

//	SetPopulationCount(1,1);

	// do not check for children if parent is a removable media
	// just try (if it's a filesystem object, it has a path)
	if (!ret)
		return FALSE;

	TCHAR path[MAX_PATH];
	if (SHGetPathFromIDList(pidlParent, path))
	{			
		UINT type = GetDriveType(path);
		if (type != DRIVE_FIXED)
			return FALSE;
	}

	return TRUE;
}


// return FALSE if on a remote or removable media, TRUE otherwise
BOOL CDirTreeCtrl::FillItem(TVITEM& item, LPCITEMIDLIST pidl,
		IShellFolder* pParentFolder, LPCITEMIDLIST pidlRel)
{
	static CString sName; // must survive to this function
	SHFILEINFO sfi;
	DWORD dwAttributes;

	if (item.mask & TVIF_TEXT)
	{
		// get display name
		STRRET str;
		str.uType = STRRET_WSTR;
		//pParentFolder->GetDisplayNameOf(pidlRel, SHGDN_INFOLDER |
		//	SHGDN_INCLUDE_NONFILESYS, &str);

		pParentFolder->GetDisplayNameOf(pidlRel, SHGDN_INFOLDER, &str);

		switch (str.uType)
		{
		case STRRET_WSTR:
			sName = str.pOleStr;
			m_pMalloc->Free(str.pOleStr);
			break;
		case STRRET_CSTR:
			sName = str.cStr;
			break;
		case STRRET_OFFSET:
			sName = (char*)((LPBYTE)pidlRel+str.uOffset);
		}
		item.pszText = (LPTSTR)(LPCTSTR)sName;
	}

	if (item.mask & (TVIF_IMAGE | TVIF_SELECTEDIMAGE))
	{
		// get some attributes
		dwAttributes = SFGAO_FOLDER | SFGAO_LINK;
		pParentFolder->GetAttributesOf(1, &pidlRel, &dwAttributes);

		// get correct icon
		UINT uFlags = SHGFI_PIDL | SHGFI_SYSICONINDEX | SHGFI_SMALLICON;
		if (dwAttributes & SFGAO_LINK)
			uFlags |= SHGFI_LINKOVERLAY;

		if (item.mask & TVIF_IMAGE)
		{
			ZeroMemory(&sfi, sizeof(SHFILEINFO));
			SHGetFileInfo((LPCTSTR)pidl, 0, &sfi, sizeof(SHFILEINFO), uFlags);
			item.iImage = sfi.iIcon;
		}
		if (item.mask & TVIF_SELECTEDIMAGE)
		{
			if (dwAttributes & SFGAO_FOLDER)
				uFlags |= SHGFI_OPENICON;

			ZeroMemory(&sfi, sizeof(SHFILEINFO));
			SHGetFileInfo((LPCTSTR)pidl, 0, &sfi, sizeof(SHFILEINFO), uFlags);
			item.iSelectedImage = sfi.iIcon;
		}
	}

	if (item.mask & TVIF_CHILDREN)
	{
		// get some attributes
		dwAttributes = SFGAO_FOLDER | SFGAO_REMOVABLE;
		pParentFolder->GetAttributesOf(1, &pidlRel, &dwAttributes);

		// get children
		item.cChildren = 0;
		if (dwAttributes & SFGAO_FOLDER)
		{
			if (m_bIncludeFiles)
				item.cChildren = 1;
			else if (dwAttributes & SFGAO_REMOVABLE)
				item.cChildren = 1;
			else
			{
				dwAttributes = SFGAO_HASSUBFOLDER;
				pParentFolder->GetAttributesOf(1, &pidlRel, &dwAttributes);

				item.cChildren = (dwAttributes & SFGAO_HASSUBFOLDER) ? 1 : 0;
			}
		}
	}

	// check if removable or remote media
	SHDESCRIPTIONID sdi;
	ZeroMemory(&sdi, sizeof(SHDESCRIPTIONID));
	if (SUCCEEDED(SHGetDataFromIDList(pParentFolder, pidlRel,
		SHGDFIL_DESCRIPTIONID, &sdi, sizeof(SHDESCRIPTIONID))))
	{
		switch (sdi.dwDescriptionId)
		{
		case SHDID_COMPUTER_REMOVABLE:
		case SHDID_COMPUTER_DRIVE35:
		case SHDID_COMPUTER_DRIVE525:
		case SHDID_COMPUTER_NETDRIVE:
		case SHDID_COMPUTER_CDROM:
		case SHDID_NET_DOMAIN:
		case SHDID_NET_SERVER:
		case SHDID_NET_SHARE:
		case SHDID_NET_RESTOFNET:
		case SHDID_NET_OTHER:
			return FALSE;
		}
	}

	return TRUE;
}


int CALLBACK CDirTreeCtrl::CompareFunc(LPARAM lParam1,
		LPARAM lParam2, LPARAM lParamSort)
{
	HRESULT hr = ((IShellFolder*)lParamSort)->CompareIDs(0,
		ILGetLast((LPCITEMIDLIST)lParam1),
		ILGetLast((LPCITEMIDLIST)lParam2));
	if (FAILED(hr))
		return 0;	// error, don't sort
	
	short ret = (short)HRESULT_CODE(hr);
	if (ret < 0)
		return -1;
	if (ret > 0)
		return 1;
	return 0;
}

void CDirTreeCtrl::OnItemexpanding(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;
	// TODO: Add your control notification handler code here
	if (pNMTreeView->action & TVE_EXPAND)
	{
		if(ItemHasChildren(pNMTreeView->itemNew.hItem))
		{
			PreExpandItem(pNMTreeView->itemNew.hItem);
			//ExpandItem(pNMTreeView->itemNew.hItem);
			ExpandItem(pNMTreeView->itemNew.hItem, pNMTreeView);
		}
	}

	*pResult = 0;
}
/*
void CDirTreeCtrl::OnSelchanged(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;
	HTREEITEM hSel=pNMTreeView->itemNew.hItem;
	// TODO: Add your control notification handler code here
	//TVITEM& item = ((LPNMTREEVIEW)pNMHDR)->itemOld;
	//LPITEMIDLIST itemlist=(LPITEMIDLIST)item.lParam;
	LPITEMIDLIST itemlist = (LPITEMIDLIST)GetItemData(hSel);
	char buf[MAX_PATH];
	SHGetPathFromIDList(itemlist, buf);
	CString str;
	str.Format("%s", buf);
//	AfxMessageBox(str);
	//((CStatic*)GetDlgItem(IDC_LOCATION))->SetWindowText(buf);
	//	char* buf=(char*)GetItemData(hSel);
	//LPITEMIDLIST itemlist=(LPITEMIDLIST)GetItemData(hSel);
	//char buf[300];
//	SHGetPathFromIDList(itemlist, buf );
	*pResult = 0;
}
*/
CString CDirTreeCtrl::GetSelectedDirectory()
{
	return m_SelectedDirectory;
	/*CString str=_T("");
	HTREEITEM hSel=GetSelectedItem();
	if(hSel!=NULL)
	{
		LPITEMIDLIST itemlist=(LPITEMIDLIST)GetItemData(hSel);
		char buf[MAX_PATH];
		SHGetPathFromIDList(itemlist, buf);
		str.Format("%s", buf);
	}
	return str;*/
}

void CDirTreeCtrl::OnBeginlabeledit(NMHDR* pNMHDR, LRESULT* pResult) 
{
	TV_DISPINFO* pTVDispInfo = (TV_DISPINFO*)pNMHDR;
	// TODO: Add your control notification handler code here
	HTREEITEM hSel=pTVDispInfo->item.hItem;
	HTREEITEM hParent=GetParentItem(hSel);
	
	CString nameStr;
	CEdit* pEdit=GetEditControl();
	pEdit->GetWindowText(nameStr);

	if(hParent==NULL)
	{
		pEdit->SetWindowText(_T(""));
		PostMessage(TVM_ENDEDITLABELNOW,FALSE,0);
	}
	else
	{
		LPITEMIDLIST itemlist=(LPITEMIDLIST)GetItemData(hParent);
		TCHAR buf[MAX_PATH];
		SHGetPathFromIDList(itemlist, buf);
		m_SelectedDirectory.Format(_T("%s"), buf);
		if(!(buf[0]>='a' && buf[0]<='z') && !(buf[0]>='A' && buf[0]<='Z'))
		{
			pEdit->SetWindowText(_T(""));
			PostMessage(TVM_ENDEDITLABELNOW, FALSE, 0);
		}
	}

	*pResult = 0;
}

void CDirTreeCtrl::OnEndlabeledit(NMHDR* pNMHDR, LRESULT* pResult) 
{
	TV_DISPINFO* pTVDispInfo = (TV_DISPINFO*)pNMHDR;
	// TODO: Add your control notification handler code here
	CString nameStr;
	CEdit* pEdit=GetEditControl();
	pEdit->GetWindowText(nameStr);

	HTREEITEM hSel=pTVDispInfo->item.hItem;
	HTREEITEM hParent=GetParentItem(hSel);

	// ESCｸｦ ｴｭｷｶｰﾅｳｪ, ｾﾆｹｫｰﾍｵｵ ｾﾂ ｻｲｿ｡ｼｭ Enterｸｦ ｴｩｸ｣ｸ・ﾃ・ﾒ
	if(nameStr==_T(""))
	{
		TVITEM tvItem;
		tvItem.hItem=hParent;
		GetItem(&tvItem);
		tvItem.cChildren=m_currentEditItemChildren;
		SetItem(&tvItem);
		DeleteItem(hSel);
		return;
	}

	SetItemText(hSel, nameStr);
//	SelectItem(hSel);

	CString path, parentPath;
	LPITEMIDLIST itemlist=(LPITEMIDLIST)GetItemData(hParent);
	TCHAR buf[MAX_PATH];
	SHGetPathFromIDList(itemlist, buf);
	parentPath.Format(_T("%s"), buf);
	path.Format(_T("%s\\%s"), parentPath, nameStr);
	_tmkdir(path);

	DeleteChildren(hParent);
	PreExpandItem(hParent);
	//ExpandItem(hParent);
	/*
	LPITEMIDLIST pidlParent;
	BOOL ret = FALSE;

	// get parent pidl
//	if (hParent == TVI_ROOT)
	if(hParent==m_desktop_root)
		pidlParent = m_pidlRoot;
	else
		pidlParent = (LPITEMIDLIST)GetItemData(hParent);

	LPITEMIDLIST pidl = NULL, pidlAbs;
	// get an absolute pidl
	pidlAbs = ILCombine(pidlParent, pidl);

	SetItemData(hSel, (LPARAM)pidlAbs);*/
	*pResult = 0;
}

HTREEITEM CDirTreeCtrl::AddNewDirectory()
{
	HTREEITEM currentSelectedItem=GetSelectedItem();
	if(currentSelectedItem==NULL)
		return NULL;

	if(Expand(currentSelectedItem, TVE_EXPAND)==FALSE)
		return NULL;

	HTREEITEM item;
	TV_INSERTSTRUCT tvstruct;
	tvstruct.hParent=currentSelectedItem;
	tvstruct.item.pszText=_T("");
	tvstruct.hInsertAfter=TVI_LAST;
	tvstruct.item.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE| TVIF_PARAM;
	tvstruct.item.iImage=5;
	tvstruct.item.iSelectedImage=5;
	item=InsertItem(&tvstruct);
	SetFocus();

	TVITEM tvItem;
	tvItem.hItem=currentSelectedItem;
	BOOL t=GetItem(&tvItem);
	m_currentEditItemChildren=tvItem.cChildren;
	tvItem.cChildren=1;
	SetItem(&tvItem);
	//UINT test3=GetItemState(currentSelectedItem, TVIF_CHILDREN);
	//UINT test4=TVIS_BOLD;
	BOOL test=Expand(currentSelectedItem, TVE_EXPAND);
	BOOL test2=EnsureVisible(item);
	EditLabel(item);

	return item;
}

void CDirTreeCtrl::OnSelchanged(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;
	// TODO: Add your control notification handler code here
	HTREEITEM hSel=pNMTreeView->itemNew.hItem;
	if(hSel!=NULL)
	{
		LPITEMIDLIST itemlist=(LPITEMIDLIST)GetItemData(hSel);
		TCHAR buf[MAX_PATH];
		SHGetPathFromIDList(itemlist, buf);
		m_SelectedDirectory.Format(_T("%s"), buf);

		CMainFrame* pFrame=(CMainFrame*)AfxGetMainWnd();
		CThumbViewerDoc* pDoc=(CThumbViewerDoc*)pFrame->GetActiveDocument();
		pDoc->SelectDirectory(m_SelectedDirectory);
	}
	*pResult = 0;
}

BOOL CDirTreeCtrl::PreTranslateMessage(MSG* pMsg) 
{
	// TODO: Add your specialized code here and/or call the base class
	if(pMsg->message==WM_KEYDOWN)
	{
		// Treeｿ｡ｼｭ ｻﾎｿ・Itemﾀｻ ﾃﾟｰ｡ﾇﾏｿｴﾀｻ ｶｧ, Enterｳｪ ESCｸｦ ｴｩｸ｣ｸ・ｱﾗｿ｡ ｸﾂｴﾂ ﾇ犒ｿﾀｻ ﾇﾏｵｵｷﾏ
		if(GetFocus()==GetEditControl())
		{
			if(pMsg->wParam==VK_RETURN)
			{
				PostMessage(TVM_ENDEDITLABELNOW,FALSE,0);
				return TRUE;
			}
			else if(pMsg->wParam==VK_ESCAPE)
			{
				CEdit *pEdit=GetEditControl();
				pEdit->SetWindowText(_T(""));
				PostMessage(TVM_ENDEDITLABELNOW,FALSE,0);
				return TRUE;
			}
		}
	}
	return CTreeCtrl::PreTranslateMessage(pMsg);
}
