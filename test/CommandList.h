#pragma once

#include "stdafx.h"

typedef CWinTraitsOR<WS_BORDER | LVS_REPORT | LVS_SINGLESEL | LVS_NOSORTHEADER | LVS_SHOWSELALWAYS> CListTraits;

class CCommandList : public CWindowImpl<CCommandList, CListViewCtrl, CListTraits>,
    public CCustomDraw<CCommandList>
{
public:
    DECLARE_WND_SUPERCLASS(NULL, WC_LISTVIEW)

    CCommandList();
    ~CCommandList();

    BEGIN_MSG_MAP(CCommandList)
        MSG_WM_DESTROY(OnDestroy)
        CHAIN_MSG_MAP_ALT(CCustomDraw<CCommandList>, 1)
        DEFAULT_REFLECTION_HANDLER()
    END_MSG_MAP()

    // Message handlers
    void OnDestroy();

    // Notification handlers
    DWORD OnPrePaint(int /*idCtrl*/, LPNMCUSTOMDRAW /*lpNMCustomDraw*/);
    DWORD OnItemPrePaint(int /*idCtrl*/, LPNMCUSTOMDRAW pNMCD);

    void Init();
};
