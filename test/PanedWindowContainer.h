#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CPanedWindowContainer : public CPaneContainerImpl<CPanedWindowContainer>
{
public:
    DECLARE_WND_CLASS_EX(_T("GfxPaneContainer"), 0, -1)

    CPanedWindowContainer() : m_bColoredHeader(false) { }

    // Operations
    void EnableColoredHeader(BOOL bEnable = true);

    // Overrides
    BOOL GetToolTipText(LPNMHDR lpnmh);
    void DrawPaneTitle(CDCHandle dc);
    HFONT GetTitleFont();

    // NOTE: There is a bug in CPanedWindowContainer that prevents this from being
    // called after you switch the container to vertical mode. Change
    // CalcSize() to pT->CalcSize() in CPanedWindowContainer::SetPaneContainerExtendedStyle()
    void CalcSize() { m_cxyHeader = 20; }

protected:
    BOOL m_bColoredHeader;
    CFont m_titleFont;
};
