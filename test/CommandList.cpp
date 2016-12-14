#include "stdafx.h"
#include "resource.h"
#include "CommandList.h"

CCommandList::CCommandList()
{
}

CCommandList::~CCommandList()
{
}

void CCommandList::OnDestroy()
{
}

DWORD CCommandList::OnPrePaint(int /*idCtrl*/, LPNMCUSTOMDRAW /*lpNMCustomDraw*/)
{
    return CDRF_NOTIFYITEMDRAW;
}

DWORD CCommandList::OnItemPrePaint(int /*idCtrl*/, LPNMCUSTOMDRAW pNMCD )
{
    if (pNMCD->lItemlParam > 0)
        ((NMLVCUSTOMDRAW*) pNMCD)->clrText = GetSysColor(COLOR_GRAYTEXT);

    return CDRF_DODEFAULT;
}

void CCommandList::Init()
{
    SetExtendedListViewStyle ( LVS_EX_FULLROWSELECT );

    InsertColumn(0, CString(LPCTSTR(_T("Step"))), LVCFMT_LEFT, 0, 0);
    InsertColumn(1, CString(LPCTSTR(_T("Object Reference"))), LVCFMT_LEFT, 0, 0);
    InsertColumn(2, CString(LPCTSTR(_T("Command Descriptions                                                    "))), LVCFMT_LEFT, 0, 1);
    InsertColumn(3, CString(LPCTSTR(_T("Running Time(ms)"))), LVCFMT_LEFT, 0, 2);
    InsertColumn(4, CString(LPCTSTR(_T("Replay Time(ms)    "))), LVCFMT_LEFT, 0, 2);

    SetColumnWidth(0, LVSCW_AUTOSIZE_USEHEADER);
    SetColumnWidth(1, LVSCW_AUTOSIZE_USEHEADER);
    SetColumnWidth(2, LVSCW_AUTOSIZE_USEHEADER);
    SetColumnWidth(3, LVSCW_AUTOSIZE_USEHEADER);
    SetColumnWidth(4, LVSCW_AUTOSIZE_USEHEADER);
}
