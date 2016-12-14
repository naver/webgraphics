#include "stdafx.h"
#include "resource.h"
#include "PanedWindowContainer.h"

void CPanedWindowContainer::EnableColoredHeader(BOOL bEnable)
{
    if (bEnable != m_bColoredHeader) {
        m_bColoredHeader = bEnable;
        RedrawWindow();
    }
}

BOOL CPanedWindowContainer::GetToolTipText(LPNMHDR lpnmh)
{
    // Be careful - this handler is called for both the ANSI and Unicode versions
    // of the message, so check lpnmh->code to see which version of the NMTTDISPINFO
    // struct was passed in.
    NMTTDISPINFOA* pnmdiA = (NMTTDISPINFOA*)lpnmh;
    NMTTDISPINFOW* pnmdiW = (NMTTDISPINFOW*)lpnmh;

    // The code here is the same for ANSI and Unicode, so I don't have to check
    // lpnmh->code. You would have to check if you are copying a string into
    // the szText member, or changing the lpszText member to point to a string.
    // This code is returning a string resource ID in lpszText, which isn't affected
    // by the character set.
    pnmdiA->hinst = _Module.GetResourceInstance();
    pnmdiA->uFlags |= TTF_DI_SETITEM;   // remember this string, don't call the handler again

    return 0;
}

void CPanedWindowContainer::DrawPaneTitle(CDCHandle dc)
{
    RECT rect;

    // If we're not using a colored header, let the base class do the
    // default drawing.
    if (!m_bColoredHeader) {
        CPaneContainerImpl<CPanedWindowContainer>::DrawPaneTitle(dc);
        return;
    }

    GetClientRect(&rect);

    if (IsVertical()) {
        TRIVERTEX tv[] = {{ rect.left, rect.top, 0xff00 }, { rect.left + m_cxyHeader, rect.bottom, 0, 0xff00 }};
        GRADIENT_RECT gr = { 0, 1 };

        dc.GradientFill(tv, 2, &gr, 1, GRADIENT_FILL_RECT_V);
    } else {
        TRIVERTEX tv[] = {{ rect.left, rect.top, 0xff00 }, { rect.right, rect.top + m_cxyHeader, 0, 0xff00 }};
        GRADIENT_RECT gr = { 0, 1 };

        dc.GradientFill(tv, 2, &gr, 1, GRADIENT_FILL_RECT_H);

        rect.bottom = rect.top + m_cxyHeader;

        // draw title only for horizontal pane container
        dc.SaveDC();

        dc.SetTextColor(RGB(255, 255, 255));
        dc.SetBkMode(TRANSPARENT);
        dc.SelectFont(GetTitleFont());

        rect.left += m_cxyTextOffset;
        rect.right -= m_cxyTextOffset;

        if (m_tb.m_hWnd != NULL)
            rect.right -= m_cxToolBar;

        dc.DrawText(m_szTitle, -1, &rect, DT_LEFT | DT_SINGLELINE | DT_VCENTER | DT_END_ELLIPSIS);

        dc.RestoreDC(-1);
    }
}

HFONT CPanedWindowContainer::GetTitleFont()
{
    // On 2000+, the base class returns MS Shell Dlg, which is fine for
    // dialogs, but gets mapped to MS Sans Serif in other windows. We'll change
    // that to MS Shell Dlg 2, which gets mapped to the right GUI font Tahoma.
    if (!m_titleFont) {
        LOGFONT lf = { 0 };
        CFontHandle(AtlGetDefaultGuiFont()).GetLogFont(lf);
        if (0 == _tcscmp(lf.lfFaceName, _T("MS Shell Dlg")))
            lstrcpyn(lf.lfFaceName, _T("MS Shell Dlg 2"), LF_FACESIZE);
        m_titleFont.CreateFontIndirect(&lf);
    }

    if (m_titleFont)
        return m_titleFont;
    else {
#if _MSC_VER >= 1300
        return __super::GetTitleFont();
#else
        return CPaneContainerImpl<CPanedWindowContainer>::GetTitleFont();
#endif
    }
}
