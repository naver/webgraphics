#pragma once

template <BOOL t_bVertical = true>
class CSplitWindowT : public CSplitterWindowImpl<CSplitWindowT<t_bVertical>>
{
public:
    DECLARE_WND_CLASS_EX(_T("SplitWindow"), CS_DBLCLKS, COLOR_WINDOW)

    CSplitWindowT() : CSplitterWindowImpl(t_bVertical), m_bPatternBar(false) { }

    // Operations
    void EnablePatternBar(BOOL bEnable = true)
    {
        if (bEnable != m_bPatternBar) {
            m_bPatternBar = bEnable;
            UpdateSplitterLayout();
        }
    }

    // Overrideables
    void DrawSplitterBar(CDCHandle dc)
    {
        // If we're not using a colored bar, let the base class do the
        // default drawing.
        if (!m_bPatternBar) {
            CSplitterWindowImpl<CSplitWindowT<t_bVertical>>::DrawSplitterBar(dc);
            return;
        }

        // Create a brush to paint with, if we haven't already done so.
        if (m_br.IsNull())
            m_br.CreateHatchBrush(HS_DIAGCROSS, t_bVertical ? RGB(255, 0, 0) : RGB(0, 0, 255));

        RECT rect;
        if (GetSplitterBarRect(&rect)) {
            dc.FillRect(&rect, m_br);

            // draw 3D edge if needed
            if ((GetExStyle() & WS_EX_CLIENTEDGE) != 0)
                dc.DrawEdge(&rect, EDGE_RAISED, t_bVertical ? (BF_LEFT | BF_RIGHT) : (BF_TOP | BF_BOTTOM));
        }
    }

protected:
    CBrush m_br;
    BOOL m_bPatternBar;
};

typedef CSplitWindowT<true> CGfxVerticalSplitterWindow;
typedef CSplitWindowT<false> CGfxHorizontalSplitterWindow;
